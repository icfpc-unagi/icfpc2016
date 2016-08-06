#include "common.h"

// Problem
vector<Polygon> silhouette_polygons;
vector<bool> silhouette_flags;
vector<Segment> skeleton;

// Solution
int num_points, num_facets;
vector<Point> src, dst;
vector<vector<int>> facets;

void InputProblem(istream &is) {
  istringstream ss(ReadAllAndRemoveComma(is));

  int num_polygons;
  ss >> num_polygons;
  rep (i, num_polygons) {
    int num_points;
    ss >> num_points;
    vector<Point> ps(num_points);
    rep (j, num_points) ps[j] = ReadPoint(ss);

    Polygon p;
    ps.emplace_back(ps[0]);
    bg::assign_points(p, ps);

    bool f = bg::area(p) > 0;
    if (!f) {
      reverse(all(ps));
      bg::assign_points(p, ps);
    }

    assert(bg::area(p) > 0);
    silhouette_polygons.emplace_back(p);
    silhouette_flags.push_back(f);
  }

  int num_segments;
  ss >> num_segments;
  rep (i, num_segments) skeleton.emplace_back(ReadSegment(ss));
}

void InputSolution(istream &is_) {
  istringstream is(ReadAllAndRemoveComma(is_));
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

  if (bg::area(po) < 0) {
    reverse(all(ps));
    bg::assign_points(po, ps);
  }

  return po;
}

Polygon TotalSilhouettePolygon() {
  assert(count(all(silhouette_flags), true) == 1);
  Polygon p;
  rep (i, silhouette_polygons.size()) {
    if (silhouette_flags[i]) p = silhouette_polygons[i];
  }
  rep (i, silhouette_polygons.size()) {
    if (silhouette_flags[i]) continue;
    vector<Polygon> dif;
    bg::difference(p, silhouette_polygons[i], dif);
    assert(dif.size() == 1);
    p = dif[0];
  }
  return p;
}

Polygon TotalSolutionPolygon() {
  Polygon po;

  map<pii, vector<int>> adj;
  rep (i, num_facets) {
    auto &f = facets[i];
    rep (j, f.size()) {
      adj[MakePairUnordered(f[j], f[(j + 1) % f.size()])].emplace_back(i);
    }
  }

  queue<int> que;
  vector<bool> vis(num_facets);
  que.push(0);
  vis[0] = true;
  vector<int> ord;
  while (!que.empty()) {
    int v = que.front();
    que.pop();
    ord.emplace_back(v);

    auto &f = facets[v];

    // Traverse
    rep (j, f.size()) {
      for (int w : adj[MakePairUnordered(f[j], f[(j + 1) % f.size()])]) {
        if (vis[w]) continue;
        que.push(w);
        vis[w] = true;
      }
    }
  }

  po = FacetPolygon(facets[ord[0]], dst);
  for (int i = 1; i < num_facets; ++i) {
    vector<Polygon> tmp;
    // cout << bg::dsv(po) << bg::dsv(FacetPolygon(facets[ord[i]], dst)) << endl;
    bg::union_(po, FacetPolygon(facets[ord[i]], dst), tmp);
    // printf("%d\n", (int)tmp.size());
    assert(tmp.size() == 1);
    po = tmp[0];
  }

  return po;
}

Bigrat SumOfArea(const vector<Polygon> &polygons) {
  Bigrat a = 0;
  for (const auto &p : polygons) a += bg::area(p);
  return a;
}

int main(int argc, char **argv) {
  {
    ifstream ifs(argv[1]);
    InputProblem(ifs);
  }
  {
    ifstream ifs(argv[2]);
    InputSolution(ifs);
  }

  Polygon total_silhouette = TotalSilhouettePolygon();
  Polygon total_solution = TotalSolutionPolygon();

  // cout << bg::area(total_silhouette) << endl;
  // cout << bg::area(total_solution) << endl;

  if (bg::within(total_silhouette, total_solution) &&
      bg::within(total_solution, total_silhouette)) {
    puts("1");
  } else {
    Bigrat num, den;
    {
      vector<Polygon> tmp;
      bg::intersection(total_silhouette, total_solution, tmp);
      num = SumOfArea(tmp);
    }
    {
      vector<Polygon> tmp;
      bg::union_(total_silhouette, total_solution, tmp);
      den = SumOfArea(tmp);
    }

    int score = static_cast<int>(num / den * 1000000);
    printf("0.%06d\n", score);
  }
  return 0;
}
