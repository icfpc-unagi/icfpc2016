#include "problem.h"

#include <complex>
#include <vector>
#include <fstream>

#include "base/base.h"
#include <boost/multiprecision/gmp.hpp>
#include "polygon.h"

DEFINE_bool(expand_viewbox, true,
            "Expand viewbox to covert the entire silhouette.");
DEFINE_bool(shrink_viewbox, true,
            "Shrink viewbox to fit silhouette and hide the original rect.");
DEFINE_string(input, "/dev/stdin", "input problem file");
DEFINE_bool(filtered, false, "set if input is filtered by prefilter");

namespace bm = boost::multiprecision;

using boost::rational;
using boost::rational_cast;
using namespace std;

bool is_rational_distance(Q x, Q y) {
  Q norm = x * x + y * y;
  Z r, n = bm::numerator(norm), d = bm::denominator(norm);
  bm::sqrt(n, r);
  if (!r.is_zero()) return false;
  bm::sqrt(d, r);
  if (!r.is_zero()) return false;
  return true;
}

int main(int argc, char** argv) {
  ParseCommandLineFlags(&argc, &argv);

  std::ifstream ifs(FLAGS_input);
  Problem problem;
  FilteredProblem filtered_problem;
  if (FLAGS_filtered) {
    ReadFilteredProblem(ifs, &filtered_problem);
    problem.polygons.resize(filtered_problem.polygons.size());
    for (int i = 0; i < filtered_problem.polygons.size(); ++i) {
      ResolveIndexReference(filtered_problem.polygons[i],
                            filtered_problem.vertices, &problem.polygons[i]);
    }
  } else {
    ReadProblem(ifs, &problem);
  }

  // viewbox size
  Q min_x = 0, min_y = 0, max_x = 1, max_y = 1;
  if (FLAGS_shrink_viewbox) {
    min_x = max_x = problem.polygons[0][0].x;
    min_y = max_y = problem.polygons[0][0].y;
  }
  if (FLAGS_expand_viewbox) {
    for (int i = 0; i < problem.polygons.size(); ++i) {
      for (const auto& v : problem.polygons[i]) {
        if (v.x < min_x) min_x = v.x;
        if (max_x < v.x) max_x = v.x;
        if (v.y < min_y) min_y = v.y;
        if (max_y < v.y) max_y = v.y;
      }
    }
  }
  if (FLAGS_shrink_viewbox) {
    // Translate to reasonable coordinates
    for (auto& p : problem.polygons) {
      for (auto& v : p) {
        v.x -= min_x;
        v.y -= min_y;
      }
    }
    for (auto& e : problem.skelton) {
      e.first.x -= min_x;
      e.first.y -= min_y;
      e.second.x -= min_x;
      e.second.y -= min_y;
    }
    for (auto& v : filtered_problem.vertices) {
      v.x -= min_x;
      v.y -= min_y;
    }
    LOG(INFO) << "Translate " << min_x << "," << min_y;
    max_x -= min_x;
    max_y -= min_y;
    min_x = min_y = 0;
  }
  printf(
      R"q(<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="400px" height="400px" viewBox="%.3f %.3f %.3f %.3f" stroke-linejoin="round" stroke-linecap="round"><g transform="scale(1,-1)"><style>circle:hover{fill:red}</style>)q",
      min_x.convert_to<double>() - 0.005, -max_y.convert_to<double>() - 0.005,
      (max_x - min_x).convert_to<double>() + 0.01,
      (max_y - min_y).convert_to<double>() + 0.01);
  if (!FLAGS_shrink_viewbox) {
    printf(
        R"(<rect x="0" y="0" width="1" height="1" fill="none" stroke="blue" stroke-width="0.005"/>)");
  }
  printf(
      R"(<path fill="silver" stroke="gray" stroke-width="0.005" fill-rule="nonzero" d=")");
  for (int i = 0; i < problem.polygons.size(); ++i) {
    for (int j = 0; j < problem.polygons[i].size(); ++j) {
      printf("%c%.3f %.3f", j == 0 ? 'M' : 'L',
             problem.polygons[i][j].x.convert_to<double>(),
             problem.polygons[i][j].y.convert_to<double>());
    }
    printf("Z");
  }
  printf(
      R"("/><g fill="none">)");
  for (const auto& e : problem.skelton) {
    bool is_rational =
        is_rational_distance(e.second.x - e.first.x, e.second.y - e.first.y);
    printf(
        R"(<path stroke="%s" stroke-width=".003" d="M%.3f %.3fL%.3f %.3f"/>)",
        is_rational ? "purple" : "gray", e.first.x.convert_to<double>(),
        e.first.y.convert_to<double>(), e.second.x.convert_to<double>(),
        e.second.y.convert_to<double>());
  }
  printf("</g>");
  for (int i = 0; i < filtered_problem.vertices.size(); ++i) {
    printf(
        R"(<circle id="v%d" fill="black" pointer-events="painted" cx="%.3f" cy="%.3f" r="0.008"/>)",
        i, filtered_problem.vertices[i].x.convert_to<double>(),
        filtered_problem.vertices[i].y.convert_to<double>());
  }
  printf("</g></svg>");

  return 0;
}
