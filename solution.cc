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

namespace bm = boost::multiprecision;

using boost::rational;
using boost::rational_cast;
using namespace std;

int main(int argc, char** argv) {
  ParseCommandLineFlags(&argc, &argv);

  std::ifstream ifs(FLAGS_input);

  Solution solution;
  ReadSolution(ifs, &solution);

  // Find perimeter path
  map<pair<Q, Q>, int> src_perimeter_map;
  for (const auto& facet : solution.facets) {
    for (int i = 0; i < facet.size(); ++i) {
      int j = (i + 1) % facet.size();
      pair<Q, Q> a = {solution.src_verts[facet[i]].x,
                      solution.src_verts[facet[i]].y},
                 b = {solution.src_verts[facet[j]].x,
                      solution.src_verts[facet[j]].y};
      if ((a.first == 0 && b.first == 0) || (a.second == 1 && b.second == 1)) {
        if (a < b) {
          src_perimeter_map.emplace(a, facet[j]);
        } else {
          src_perimeter_map.emplace(b, facet[i]);
        }
      } else if ((a.first == 1 && b.first == 1) ||
                 (a.second == 0 && b.second == 0)) {
        if (a < b) {
          src_perimeter_map.emplace(b, facet[i]);
        } else {
          src_perimeter_map.emplace(a, facet[j]);
        }
      }
    }
  }
  vector<int> perimeter_path;
  set<int> angles;
  pair<Q, Q> p(0, 0);
  do {
    auto it = src_perimeter_map.find(p);
    if (it == src_perimeter_map.end()) {
      LOG(WARNING) << "Failed to find perimeter path.";
      perimeter_path.clear();
      break;
    }
    int i = it->second;
    const Q& x = solution.src_verts[i].x;
    const Q& y = solution.src_verts[i].y;
    perimeter_path.push_back(i);
    if (bm::denominator(x) == 1 && bm::denominator(y) == 1) {
      angles.insert(i);
    }
    p.first = x;
    p.second = y;
  } while (p.first != 0 || p.second != 0);

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
      R"(<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="400px" height="400px" viewBox="%.3f %.3f %.3f %.3f" stroke-linejoin="round" stroke-linecap="round" fill="none">)",
      min_x.convert_to<double>() - 0.005, -max_y.convert_to<double>() - 0.005,
      (max_x - min_x).convert_to<double>() + 0.01,
      (max_y - min_y).convert_to<double>() + 0.01);
  printf(
      R"q(<style>polygon:hover{fill:orange}</style><g transform="scale(1,-1)">)q");
  if (!FLAGS_shrink_viewbox) {
    printf(
        R"(<rect x="0" y="0" width="1" height="1" stroke="skyblue" stroke-width="0.005"/>)");
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
  printf(R"(<g fill="silver">)");
  for (int i = 0; i < points.size(); ++i) {
    printf(
        R"(<polygon id="i%d" pointer-events="painted" points="%s"/>)", i,
        points[i].c_str());
  }
  printf(R"(</g><g stroke="gray" stroke-width="0.005">)");
  for (int i = 0; i < points.size(); ++i) {
    printf(
        R"(<polygon points="%s"/>)", points[i].c_str());
  }
  printf("</g>");
  if (perimeter_path.size() > 0) {
    printf(R"(<path stroke="blue" stroke-width="0.005" d=")");
    for (int i = 0; i < perimeter_path.size(); ++i) {
      printf("%c%.3f %.3f", i == 0 ? 'M' : 'L',
             solution.dst_verts[perimeter_path[i]].x.convert_to<double>(),
             solution.dst_verts[perimeter_path[i]].y.convert_to<double>());
    }
    printf(R"(Z"/>)");
  }
  for (int i = 0; i < solution.dst_verts.size(); ++i) {
    printf(
        R"(<circle fill="black" cx="%.3f" cy="%.3f" r=".008"/>)",
        solution.dst_verts[i].x.convert_to<double>(),
        solution.dst_verts[i].y.convert_to<double>());
  }
  for (int i : angles) {
    printf(R"(<circle fill="magenta" cx="%.3f" cy="%.3f" r=".008"/>)",
           solution.dst_verts[i].x.convert_to<double>(),
           solution.dst_verts[i].y.convert_to<double>());
  }
  printf("</g></svg>");

  return 0;
}