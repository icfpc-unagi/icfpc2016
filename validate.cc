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
using ccwRing = bg::model::ring<gPoint, false, false>;
using ccwPolygon = bg::model::polygon<gPoint, false, false>;
using ccwMultiPolygon = bg::model::multi_polygon<ccwPolygon>;
using namespace std;

gPoint rotCCW(const gPoint& p) { return gPoint(-p.y(), p.x()); }

Q cross(const gPoint& lhs, const gPoint& rhs) {
  return lhs.x() * rhs.y() - lhs.y() * rhs.x();
}

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

  // Verify src facets congruence vs dst
  vector<ccwRing> dst_rings;
  for (const auto& facet : solution.facets) {
    LOG_IF(FATAL, facet.size() < 3) << "Facet has less than 3 vertices ("
                                    << strings::JoinInts(facet, ",") << ")";
    ccwRing src_ring, dst_ring;
    for (const auto& i : facet) {
      bg::append(src_ring,
                 gPoint(solution.src_verts[i].x, solution.src_verts[i].y));
      bg::append(dst_ring,
                 gPoint(solution.dst_verts[i].x, solution.dst_verts[i].y));
    }
    bool mirror;
    bool error = false;
    for (int i = 1; i < src_ring.size() - 1; ++i) {
      gPoint s1 = src_ring[i];
      gPoint s2 = src_ring[i + 1];
      bg::subtract_point(s1, src_ring[0]);
      bg::subtract_point(s2, src_ring[0]);
      Q gs1 = bg::dot_product(s1, s2);
      Q gs2 = bg::dot_product(rotCCW(s1), s2);
      gPoint d1 = dst_ring[i];
      gPoint d2 = dst_ring[i + 1];
      bg::subtract_point(d1, dst_ring[0]);
      bg::subtract_point(d2, dst_ring[0]);
      Q gd1 = bg::dot_product(d1, d2);
      Q gd2 = bg::dot_product(rotCCW(d1), d2);
      // find if flipped or not with the first angle
      if (i == 1) mirror = gs2.sign() != gd2.sign();
      bool congruent = gs1 == gd1 && gs2 == (mirror ? -gd2 : gd2);
      LOG_IF(FATAL, !congruent)
          << "Facet is not congruent between source and destination: "
          << bg::wkt(src_ring) << " vs " << bg::wkt(dst_ring);
    }

    bg::correct(dst_ring);
    dst_rings.push_back(std::move(dst_ring));
  }

  // Verify dst facets shape problem silhouette
  ccwMultiPolygon dst_mpoly;
  for (const auto& ring : dst_rings) {
    ccwMultiPolygon tmp;
    bg::union_(dst_mpoly, ring, tmp);
    dst_mpoly.swap(tmp);
  }
  ccwPolygon silhouette;
  for (const auto& poly : problem.polygons) {
    ccwRing ring;
    for (const auto& v : poly) {
      bg::append(ring, gPoint(v.x, v.y));
    }
    ccwMultiPolygon out;
    if (bg::area(ring) > 0) {
      bg::union_(silhouette, ring, out);
    } else {
      bg::correct(ring);
      bg::difference(silhouette, ring, out);
    }
    if (out.size() == 0) {
      LOG(ERROR) << "Found holes before positive silhouette.";
    } else {
      LOG_IF(ERROR, out.size() > 1)
          << "Problem silhouette has disjoint polygons: " << bg::wkt(out);
      silhouette = out[0];
    }
  }
  LOG_IF(ERROR, !bg::is_valid(silhouette)) << "Problem silhouette is invalid."
                                           << bg::wkt(silhouette);
  auto silhouette_area = bg::area(silhouette);
  LOG_IF(ERROR, silhouette_area < 0 || 1 < silhouette_area)
      << "Silhouette doesn't have area between [0,1]: " << silhouette_area;
  ccwMultiPolygon diff;
  bg::sym_difference(silhouette, dst_mpoly, diff);
  if (!bg::area(diff).is_zero()) {
    LOG(WARNING) << "Destionation silhouette doesn't match.";
    ccwMultiPolygon diff1, diff2;
    bg::difference(silhouette, dst_mpoly, diff1);
    bg::difference(dst_mpoly, silhouette, diff2);
    LOG(WARNING) << "Diff1: " << bg::wkt(diff1);
    LOG(WARNING) << "Diff2: " << bg::wkt(diff2);
  }

  // TODO: Calculate score?

  LOG(INFO) << "Validated";

  return 0;
}
