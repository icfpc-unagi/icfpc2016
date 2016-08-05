// Detect unnecessary boundaries and merge facets

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

Polygon FacetPolygon(const vector<int> &vs, const vector<Point> &coords) {
  vector<Point> ps;
  for (int v : vs) ps.emplace_back(coords[v]);
  ps.emplace_back(ps[0]);
  Polygon po;
  bg::assign_points(po, ps);
  return po;
}

int main() {
  Input();

  // Construct polygons and force counter clockwise
  src_polygons.resize(num_facets);
  rep (i, num_facets) src_polygons[i] = FacetPolygon(facets[i], src);
  dst_polygons.resize(num_facets);
  rep (i, num_facets) dst_polygons[i] = FacetPolygon(facets[i], dst);

  // Connection
  map<pair<int, int>, vector<int>> edge_to_facets;
  rep (i, num_facets) {
    const auto &f = facets[i];
    rep (j, f.size()) {
      edge_to_facets[MakePairUnordered(f[j], f[(j + 1) % f.size()])]
          .emplace_back(i);
    }
  }

  // Detect unnecessary edges
  set<pii> unnecessary_edges;
  for (const auto &t : edge_to_facets) {
    vector<int> fs = t.second;
    if (fs.size() == 1) continue;
    assert(fs.size() == 2);

    vector<bool> src_ccw(2), dst_ccw(2);
    rep (i, 2) {
      src_ccw[i] = bg::area(src_polygons[fs[i]]) > 0;
      dst_ccw[i] = bg::area(dst_polygons[fs[i]]) > 0;
    }
    if (src_ccw == dst_ccw) {
      cerr << "JOIN: " << fs[0] << "-" << fs[1]
           << "(edge: " << t.first.first << "," << t.first.second << ")" << endl;
      unnecessary_edges.insert(t.first);
    }
  }
  cerr << "unnecessary edges: " << unnecessary_edges.size() << endl;

  // Regions
  vector<vector<int>> adj(num_points);
  for (const auto &f : facets) {
    rep (i, f.size()) {
      int a = f[i], b = f[(i + 1) % f.size()];
      if (unnecessary_edges.count(MakePairUnordered(a, b))) continue;
      adj[a].emplace_back(b);
      adj[b].emplace_back(a);
    }
  }
  for (auto &a : adj) {
    sort(all(a));
    a.erase(unique(all(a)), a.end());
  }
  vector<vector<int>> new_facets = EnumerateRegions(src, adj);
  new_facets.erase(
      remove_if(all(new_facets), [&](const vector<int> &f) -> bool {
          vector<Point> ps(f.size());
          rep (i, f.size()) ps[i] = src[f[i]];
          ps.emplace_back(ps[0]);
          Polygon po;
          bg::assign_points(po, ps);
          return bg::area(po) < 0;
        }), new_facets.end());

  // Output
  cout << num_points << endl;
  rep (i, num_points) cout << src[i].x() << "," << src[i].y() << endl;
  cout << new_facets.size() << endl;
  rep (i, new_facets.size()) {
    cout << new_facets[i].size();
    for (int v : new_facets[i]) cout << " " << v;
    cout << endl;
  }
  rep (i, num_points) cout << dst[i].x() << "," << dst[i].y() << endl;
}
