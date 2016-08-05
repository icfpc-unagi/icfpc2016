#include <complex>
#include <vector>
#include <fstream>
#include <sstream>

#include "base/base.h"
#include "boost/rational.hpp"
#include "polygon.h"
#include "problem.h"
#include "solution.h"
#include <boost/geometry.hpp>
#include <boost/geometry/algorithms/union.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/iterators/closing_iterator.hpp>
#include <boost/multiprecision/gmp.hpp>

DEFINE_string(input, "/dev/stdin", "input solution file");

namespace bg = boost::geometry;

using boost::rational;
using gPoint = bg::model::d2::point_xy<Q>;
using ccwPolygon = bg::model::polygon<gPoint, false, false>;
using ccwMultiPolygon = bg::model::multi_polygon<ccwPolygon>;
using LineString = bg::model::linestring<gPoint>;
using MultiLineString = bg::model::multi_linestring<LineString>;
using namespace std;

Vertex ConvertToVertex(const gPoint& p) {
  Vertex v;
  v.x = p.x();
  v.y = p.y();
  return v;
}

template <typename Ring>
void ReduceRingAnchorPoints(Ring* ring) {
  for (int i = 0; i < ring->size(); ++i) {
    auto a = (*ring)[i];
    auto b = (*ring)[(i + 1) % ring->size()];
    auto c = (*ring)[(i + 2) % ring->size()];
    bg::subtract_point(c, b);
    bg::subtract_point(b, a);
    if (b.x() * c.y() - b.y() * c.x() == 0) {
      ring->erase(ring->begin() + (i + 1) % ring->size());
      i--;
    }
  }
}

int main(int argc, char** argv) {
  ParseCommandLineFlags(&argc, &argv);

  // Input
  Solution solution;
  std::ifstream solution_ifs(FLAGS_input);
  ReadSolution(solution_ifs, &solution);

  // Combine
  ccwMultiPolygon mpoly, mpoly_tmp;
  MultiLineString mls, mls_tmp;
  for (const auto& facet : solution.facets) {
    ccwPolygon poly;
    for (const auto& i : facet) {
      bg::append(poly,
                 gPoint(solution.dst_verts[i].x, solution.dst_verts[i].y));
    }
    if (bg::area(poly) < 0) {
      bg::reverse(poly);
    }
    LOG(INFO) << "union polygon: " << bg::wkt(poly);
    bg::union_(mpoly, poly, mpoly_tmp);
    mpoly.swap(mpoly_tmp);
    bg::clear(mpoly_tmp);
    LOG(INFO) << "polygon: " << bg::wkt(mpoly);

    LineString ls;
    for (const auto& i : facet) {
      bg::append(ls, gPoint(solution.dst_verts[i].x, solution.dst_verts[i].y));
    }
    bg::append(ls, gPoint(solution.dst_verts[facet[0]].x,
                          solution.dst_verts[facet[0]].y));
    LOG(INFO) << "union line: " << bg::wkt(ls);
    bg::union_(mls, ls, mls_tmp);
    mls.swap(mls_tmp);
    bg::clear(mls_tmp);
    LOG(INFO) << "lines: " << bg::wkt(mls);
  }

  // Optimize
  for (auto& poly : mpoly) {
    ReduceRingAnchorPoints(&bg::exterior_ring(poly));
    for (auto& iring : bg::interior_rings(poly)) {
      ReduceRingAnchorPoints(&iring);
    }
  }

  // Output
  Problem problem;
  for (const auto& poly : mpoly) {
    const auto& xring = bg::exterior_ring(poly);
    problem.polygons.emplace_back(xring.size());
    std::transform(xring.begin(), xring.end(), problem.polygons.back().begin(),
                   &ConvertToVertex);
    const auto& irings = bg::interior_rings(poly);
    for (const auto& iring : irings) {
      problem.polygons.emplace_back(iring.size());
      std::transform(iring.begin(), iring.end(),
                     problem.polygons.back().begin(), &ConvertToVertex);
    }
  }
  for (const auto& ls : mls) {
    for (int i = 0; i < ls.size() - 1; ++i) {
      problem.skelton.emplace_back(ConvertToVertex(ls[i]),
                                   ConvertToVertex(ls[i + 1]));
    }
  }
  WriteProblem(problem, cout);

  return 0;
}
