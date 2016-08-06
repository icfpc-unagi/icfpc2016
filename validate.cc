#include <complex>
#include <vector>
#include <fstream>
// boost/geometry/algorithms/detail/overlay/handle_colocations.hpp:198:10:
// error: no member named 'cout' in namespace 'std'
#include <iostream>

#include "base/base.h"
#include "boost/rational.hpp"
#include <boost/geometry.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/sym_difference.hpp>
// #include <boost/geometry/algorithms/equals.hpp>
// #include <boost/geometry/algorithms/is_empty.hpp>
#include <boost/geometry/algorithms/union.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/multiprecision/gmp.hpp>
#include "polygon.h"
#include "problem.h"
#include "solution.h"

DEFINE_string(problem, "", "input problem file");
DEFINE_string(solution, "", "input solution file");

namespace bg = boost::geometry;

using gPoint = bg::model::d2::point_xy<Q>;
using ccwPolygon = bg::model::polygon<gPoint, false, false>;
using ccwMultiPolygon = bg::model::multi_polygon<ccwPolygon>;
using namespace std;

// namespace boost {
// namespace multiprecision {
// // False sqrt for bg::equals
// // boost/geometry/algorithms/detail/equals/collect_vectors.hpp:138:48
// // that uses sqrt to compare vector directions with unit vectors.
// Q sqrt(const Q& q) {
//   long double x = sqrtl(q.convert_to<long double>());
//   long double i;
//   long double f = modfl(x, &i);
//   if (f == 0) return Q((long long)i);
//   int exp;
//   x = frexpl(x, &exp);
//   const int kBias = 20;
//   x = ldexpl(x, exp + 20);
//   return Q((long long)roundl(x), 1ll << kBias);
// }
// }
// }

template <typename T>
void normalize_pair(std::pair<T, T>* p) {
  if (p->second < p->first) std::swap(p->first, p->second);
}

int main(int argc, char** argv) {
  ParseCommandLineFlags(&argc, &argv);

  VLOG(2) << "Loading file " << FLAGS_problem;
  Problem problem;
  std::ifstream problem_ifs(FLAGS_problem);
  CHECK(ReadProblem(problem_ifs, &problem)) << "Failed to load file "
                                            << FLAGS_problem;
  VLOG(2) << "Loading file " << FLAGS_solution;
  Solution solution;
  std::ifstream solution_ifs(FLAGS_solution);
  CHECK(ReadSolution(solution_ifs, &solution)) << "Failed to load file "
                                               << FLAGS_solution;
  VLOG(2) << "Files loaded";

  // edge (normalized pair of vertex ids) to facet ids
  map<pair<int, int>, set<int>> edge_to_facet;
  for (int i = 0; i < solution.facets.size(); ++i) {
    for (int j = 0; j < solution.facets[i].size() - 1; ++j) {
      pair<int, int> edge(solution.facets[i][j], solution.facets[i][j + 1]);
      normalize_pair(&edge);
      edge_to_facet[edge].insert(i);
    }
  }
  for (const auto& it : edge_to_facet) {
    const auto& edge = it.first;
    const auto& adj_facets = it.second;
    if (adj_facets.size() > 2) {
      LOG(FATAL) << "Edge[" << edge.first << "," << edge.second
                 << "] is shared by more than 2 facets ("
                 << strings::JoinInts(adj_facets, ",") << ")";
    }
  }

  // TODO: Verify src facets congruence vs dst

  // Verify dst facets shape problem silhouette
  ccwMultiPolygon dst_mpoly;
  for (const auto& facet : solution.facets) {
    ccwPolygon poly;
    for (const auto& i : facet) {
      bg::append(poly,
                 gPoint(solution.dst_verts[i].x, solution.dst_verts[i].y));
    }
    if (bg::area(poly) < 0) {
      bg::reverse(poly);
    }
    ccwMultiPolygon tmp;
    bg::union_(dst_mpoly, poly, tmp);
    dst_mpoly.swap(tmp);
  }
  ccwPolygon silhouette;
  for (const auto& poly : problem.polygons) {
    ccwPolygon gpoly;
    for (const auto& v : poly) {
      bg::append(gpoly, gPoint(v.x, v.y));
    }
    vector<ccwPolygon> out;
    bg::union_(silhouette, gpoly, out);
    LOG_IF(FATAL, out.size() == 0) << "Unexpected error on silhouette.";
    LOG_IF(ERROR, out.size() > 1) << "Problem has disjoint polygons.";
    silhouette = out[0];
  }
  ccwMultiPolygon diff;
  bg::sym_difference(silhouette, dst_mpoly, diff);
  if (bg::num_geometries(diff) != 0) {
    LOG(INFO) << "Destionation silhouette matches to the problem.";
  } else {
    LOG(WARNING) << "Destionation silhouette doesn't match to the problem.";
  }

  // TODO: Calculate score?

  VLOG(2) << "Exiting successfully";

  return 0;
}
