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
      min_x.convert_to<double>() - 0.005, -max_y.convert_to<double>() - 0.005,
      (max_x - min_x).convert_to<double>() + 0.01,
      (max_y - min_y).convert_to<double>() + 0.01);
  printf(
      R"q(<style>.f :hover{fill:orange}</style><g transform="scale(1,-1)">)q");
  if (!FLAGS_shrink_viewbox) {
    printf(
        R"(<rect x="0" y="0" width="1" height="1" fill="none" stroke="blue" stroke-width="0.005"/>)");
  }
  vector<string> points;
  for (const auto& facet : solution.facets) {
    vector<string> strs;
    for (const auto& j : facet) {
      strs.push_back(StringPrintf(
          "%.3f,%.3f", solution.dst_verts[j].x.convert_to<double>(),
          solution.dst_verts[j].y.convert_to<double>()));
    }
    points.push_back(strings::Join(strs, " "));
  }
  printf(R"(<g fill="silver" class="f">)");
  for (int i = 0; i < points.size(); ++i) {
    printf(
        R"(<polygon id="i%d" pointer-events="painted" points="%s"/>)", i,
        points[i].c_str());
  }
  printf(R"(</g><g fill="none" stroke="gray" stroke-width="0.005">)");
  for (int i = 0; i < points.size(); ++i) {
    printf(
        R"(<polygon points="%s"/>)", points[i].c_str());
  }
  printf("</g>");
  for (int i = 0; i < solution.dst_verts.size(); ++i) {
    printf(
        R"(<circle cx="%.3f" cy="%.3f" r="0.008"/>)",
        solution.dst_verts[i].x.convert_to<double>(),
        solution.dst_verts[i].y.convert_to<double>());
  }
  printf("</g></svg>");

  return 0;
}