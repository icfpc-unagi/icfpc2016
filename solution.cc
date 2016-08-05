#include <complex>
#include <vector>
#include <fstream>

#include "base/base.h"
#include "boost/multiprecision/cpp_int.hpp"
#include "boost/rational.hpp"
#include "polygon.h"

DEFINE_bool(expand_viewbox, true,
            "Expand viewbox to covert the entire silhouette.");
DEFINE_bool(shrink_viewbox, true,
            "Shrink viewbox to fit silhouette and hide the original rect.");
DEFINE_string(input, "/dev/stdin", "input solution file");

using boost::rational;
using boost::rational_cast;
using namespace std;

int main(int argc, char** argv) {
  ParseCommandLineFlags(&argc, &argv);

  std::ifstream ifs(FLAGS_input);

  int n_verts;
  ifs >> n_verts;
  vector<Vertex> src_verts(n_verts);
  for (int i = 0; i < n_verts; ++i) {
    ifs >> src_verts[i];
  }
  int n_facets;
  ifs >> n_facets;
  vector<vector<int>> facets(n_facets);
  for (int i = 0; i < n_facets; ++i) {
    int n_indices;
    ifs >> n_indices;
    facets[i].resize(n_indices);
    for (int j = 0; j < n_indices; ++j) {
      ifs >> facets[i][j];
    }
  }
  vector<Vertex> dst_verts(n_verts);
  for (int i = 0; i < n_verts; ++i) {
    ifs >> dst_verts[i];
  }

  // viewbox size
  Q min_x = 0, min_y = 0, max_x = 1, max_y = 1;
  if (FLAGS_shrink_viewbox) {
    min_x = max_x = dst_verts[0].x;
    min_y = max_y = dst_verts[0].y;
  }
  if (FLAGS_expand_viewbox) {
    for (const auto& v : dst_verts) {
      if (v.x < min_x) min_x = v.x;
      if (max_x < v.x) max_x = v.x;
      if (v.y < min_y) min_y = v.y;
      if (max_y < v.y) max_y = v.y;
    }
  }
  // Translate dst_verts to reasonable coordinates
  if (FLAGS_shrink_viewbox) {
    if (min_x < 0 != 1 < max_x) {
      for (auto& v : dst_verts) {
        v.x -= min_x;
      }
      LOG(INFO) << "Translate X " << min_x;
      max_x -= min_x;
      min_x = 0;
    }
    if (min_y < 0 != 1 < max_y) {
      for (auto& v : dst_verts) {
        v.y -= min_y;
      }
      LOG(INFO) << "Translate Y " << min_y;
      max_y -= min_y;
      min_y = 0;
    }
  }
  printf(
      R"(<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="400px" height="400px" viewBox="-0.0025 -0.0025 1.005 1.005" stroke-linejoin="round" stroke-linecap="round">)");
  printf(
      R"(<style>path:hover{fill:orange}</style>)");
  printf(
      R"(<rect x="0" y="0" width="1" height="1" fill="none" stroke="blue" stroke-width="0.005"/>)");
  for (int i = 0; i < n_facets; ++i) {
    printf(
        R"(<path fill="silver" stroke="gray" stroke-width="0.005" id="i%d" pointer-events="painted" d=")",
        i);
    for (int j = 0; j < facets[i].size(); ++j) {
      printf("%c%.3f %.3f", j == 0 ? 'M' : 'L',
             dst_verts[facets[i][j]].x.convert_to<double>(),
             dst_verts[facets[i][j]].y.convert_to<double>());
    }
    printf(R"(Z"/>)");
  }
  for (int i = 0; i < n_verts; ++i) {
    printf(
        R"(<circle fill="black" cx="%.3f" cy="%.3f" r="0.008"/>)",
        dst_verts[i].x.convert_to<double>(),
        dst_verts[i].y.convert_to<double>());
  }
  printf("</svg>");

  return 0;
}
