// Detect and remove unnecessary points

#include "common.h"

// Input
int num_points, num_facets;
vector<Point> src, dst;
vector<vector<int>> facets;
vector<Polygon> src_polygons, dst_polygons;

// Relabel
vector<int> old_to_new;
vector<int> new_to_old;

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


void Output(ostream &os, bool flip_x, bool flip_y, bool swap_xy) {
  os << new_to_old.size() << endl;
  rep (k, new_to_old.size()) {
    int i = new_to_old[k];
    // transformation
    Bigrat x = src[i].x(), y = src[i].y();
    if (flip_x) x = 1 - x;
    if (flip_y) y = 1 - y;
    if (swap_xy) swap(x, y);
    os << x << "," << y << endl;
  }
  os << facets.size() << endl;
  rep (i, facets.size()) {
    vector<int> new_facet;
    for (int v : facets[i]) {
      if (old_to_new[v] != -1) new_facet.emplace_back(old_to_new[v]);
    }

    os << new_facet.size();
    for (int v : new_facet) os << " " << v;
    os << endl;
  }
  rep (k, new_to_old.size()) {
    int i = new_to_old[k];
    os << dst[i].x() << "," << dst[i].y() << endl;
  }
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

  old_to_new.assign(num_points, 0);
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

  // Relabeling
  new_to_old.clear();
  rep (v, num_points) {
    if (old_to_new[v] != -1) {
      new_to_old.emplace_back(v);
    }
  }
  map<int, int> cnt;
  for (const auto &f : facets) for (int v : f) cnt[v] += 1;
  sort(all(new_to_old), [&](int v, int w) {
      return cnt.at(v) > cnt.at(w);
    });

  rep (i, new_to_old.size()) {
    old_to_new[new_to_old[i]] = i;
  }
  int new_num_points = new_to_old.size();
  cerr << "Number of points: " << num_points << " -> "
       << new_num_points << endl;

  // Output
  string out;
  rep (flip_x, 2) rep (flip_y, 2) rep (swap_xy, 2) {
    ostringstream oss;
    Output(oss, flip_x, flip_y, swap_xy);
    string tmp = oss.str();
    // cerr << tmp.length() << endl;
    if (out.empty() || out.length() > tmp.length()) out = tmp;
  }
  cout << out;
}
