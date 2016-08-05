// Detect and remove unnecessary points

#include "common.h"

int num_points, num_facets;
vector<Point> src, dst;
vector<vector<int>> facets;
vector<Polygon> src_polygons, dst_polygons;

void Input() {
  istringstream is(ReadAllAndRemoveComma(cin));
  is >> num_points;

  src.resize(num_points);
  rep (i, num_points) src[i] = ReadPoint(is);

  is >> num_facets;
  facets.resize(num_facets);
  rep (i, num_facets) {
    int n;
    is >> n;
    facets[i].resize(n);
    for (int &f : facets[i]) is >> f;
  }

  dst.resize(num_points);
  rep (i, num_points) dst[i] = ReadPoint(is);
}



int main() {
  Input();

  vector<vector<int>> adj(num_points);
  for (const auto &f : facets) {
    rep (i, f.size()) {
      int a = f[i], b = f[(i + 1) % f.size()];
      adj[a].emplace_back(b);
      adj[b].emplace_back(a);
    }
  }
  for (auto &a : adj) {
    sort(all(a));
    a.erase(unique(all(a)), a.end());
  }

  vector<int> old_to_new(num_points);
  rep (v, num_points) {
    const Point &p = src[v];
    if (adj[v].empty()) {
      old_to_new[v] = -1;
    } else if (adj[v].size() == 2) {
      Point a = src[adj[v][0]], b = src[adj[v][1]];
      bg::subtract_point(a, p);
      bg::subtract_point(b, p);
      Bigrat det = Det(a, b);
      if (det == 0) old_to_new[v] = -1;
    }
  }

  int new_num_points = 0;
  for (auto &x : old_to_new) {
    if (x != -1) x = new_num_points++;
  }
  cerr << "Number of points: " << num_points << " -> " << new_num_points << endl;

  // Output
  cout << new_num_points << endl;
  rep (i, num_points) {
    if (old_to_new[i] != -1) {
      cout << src[i].x() << "," << src[i].y() << endl;
    }
  }
  cout << facets.size() << endl;
  rep (i, facets.size()) {
    vector<int> new_facet;
    for (int v : facets[i]) {
      if (old_to_new[v] != -1) new_facet.emplace_back(old_to_new[v]);
    }

    cout << new_facet.size();
    for (int v : new_facet) cout << " " << v;
    cout << endl;
  }
  rep (i, num_points) {
    if (old_to_new[i] != -1) {
      cout << dst[i].x() << "," << dst[i].y() << endl;
    }
  }
}
