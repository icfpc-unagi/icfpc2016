#include "solution.h"

#include <complex>
#include <vector>
#include <fstream>

#include "base/base.h"
#include "boost/multiprecision/cpp_int.hpp"
#include "boost/rational.hpp"
#include "polygon.h"

DEFINE_bool(expand_viewbox, true,
            "Expand viewbox to covert the entire silhouette.");
DEFINE_bool(shrink_viewbox, false,
            "Shrink viewbox to fit silhouette and hide the original rect.");
DEFINE_string(input, "/dev/stdin", "input solution file");

using boost::rational;
using boost::rational_cast;
using namespace std;

int main(int argc, char** argv) {
  ParseCommandLineFlags(&argc, &argv);

  std::ifstream ifs(FLAGS_input);

  Solution solution;
  ReadSolution(ifs, &solution);

  // viewbox size
  Q min_x = 0, min_y = 0, max_x = 1, max_y = 1;
  if (FLAGS_shrink_viewbox) {
    min_x = max_x = solution.dst_verts[0].x;
    min_y = max_y = solution.dst_verts[0].y;
  }
  if (FLAGS_expand_viewbox) {
    for (const auto& v : solution.dst_verts) {
      if (v.x < min_x) min_x = v.x;
      if (max_x < v.x) max_x = v.x;
      if (v.y < min_y) min_y = v.y;
      if (max_y < v.y) max_y = v.y;
    }
  }
  // Translate dst to reasonable coordinates
  if (FLAGS_shrink_viewbox) {
    if (min_x < 0 != 1 < max_x) {
      for (auto& v : solution.dst_verts) {
        v.x -= min_x;
      }
      LOG(INFO) << "Translate X " << min_x;
      max_x -= min_x;
      min_x = 0;
    }
    if (min_y < 0 != 1 < max_y) {
      for (auto& v : solution.dst_verts) {
        v.y -= min_y;
      }
      LOG(INFO) << "Translate Y " << min_y;
      max_y -= min_y;
      min_y = 0;
    }
  }
  printf(
      R"(<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="400px" height="400px" viewBox="%.3f %.3f %.3f %.3f" stroke-linejoin="round" stroke-linecap="round">)",
      min_x.convert_to<double>() - 0.005, min_y.convert_to<double>() - 0.005,
      (max_x - min_x).convert_to<double>() + 0.01,
      (max_y - min_y).convert_to<double>() + 0.01);
  printf(
      R"q(<style>path:hover{fill:orange}</style><g transform="translate(0,%.3f) scale(1,-1)"><rect x="0" y="0" width="1" height="1" fill="none" stroke="blue" stroke-width="0.005"/>)q",
      (max_y - min_y).convert_to<double>() + 0.01);
  printf(
      R"(<defs><g id="p">)");
  for (int i = 0; i < solution.facets.size(); ++i) {
    printf(
        R"(<path id="i%d" pointer-events="painted" d=")", i);
    for (int j = 0; j < solution.facets[i].size(); ++j) {
      printf("%c%.3f %.3f", j == 0 ? 'M' : 'L',
             solution.dst_verts[solution.facets[i][j]].x.convert_to<double>(),
             solution.dst_verts[solution.facets[i][j]].y.convert_to<double>());
    }
    printf(R"(Z"/>)");
  }
  printf(
      R"(</g></defs><use fill="silver" pointer-events="painted" xlink:href="#p"/><use fill="none" stroke="gray" stroke-width="0.005" xlink:href="#p"/>)");
  for (int i = 0; i < solution.dst_verts.size(); ++i) {
    printf(
        R"(<circle fill="black" cx="%.3f" cy="%.3f" r="0.008"/>)",
        solution.dst_verts[i].x.convert_to<double>(),
        solution.dst_verts[i].y.convert_to<double>());
  }
  printf("</g></svg>");

  return 0;
}