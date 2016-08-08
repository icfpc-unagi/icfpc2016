// Receives prefiltered input.

#include "common.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <gflags/gflags.h>

using Edge = pair<int, Bigrat>;
using EdgeList = vector<Edge>;
using Graph = vector<EdgeList>;
using DAG = vector<map<Bigrat, vector<Edge>>>;

using R = Bigrat;
using I = Bignum;
using P = Point;

int num_points;
vector<Point> coords;
vector<vector<int>> regions;

DEFINE_int32(search_limit, 100000, "");
DEFINE_int32(visit_limit, 1, "");
DEFINE_int32(restart, 1, "");
DEFINE_string(algorithm, "dfs", "");
DEFINE_string(sources, "all", "");

Graph adj;
vector<vector<int>> adj_all;  // including irrational

void Input() {
  istringstream is(ReadAllAndRemoveComma(cin));
  is >> num_points;
  coords.resize(num_points);
  rep (i, num_points) coords[i] = ReadPoint(is);
  int num_regions;
  is >> num_regions;
  rep (i, num_regions) {
    int n;
    is >> n;
    vector<int> f(n);
    rep (i, n) is >> f[i];
    regions.emplace_back(f);
  }
}

bool Distance(int a, int b, R &d) {  // Returns whether rational
  // if (coords[a].x() < Bigrat(2, 3)) return false;
  // if (coords[b].x() < Bigrat(2, 3)) return false;

  R d2 =
      Sqr(coords[a].x() - coords[b].x()) +
      Sqr(coords[a].y() - coords[b].y());

  // cerr << d2 << endl;

  I num2 = numerator(d2), den2 = denominator(d2);
  I num, numR, den, denR;
  num = Sqrt(num2, numR);
  den = Sqrt(den2, denR);
  // cerr << num2 << " " << num << " " << numR << endl;
  // cerr << den2 << " " << den << " " << denR << endl;
  if (numR != 0 || denR != 0) {
    return false;
  } else {
    assert(num * num == num2);
    assert(den * den == den2);
    // cerr << num << " " << den << endl;
    d = R(num, den);
    assert(d * d == d2);
    // cerr << "Edge: " << a << "--" << b << ": " << d << " " << d2 << endl;

    return true;
  }
}

bool IsOnSegment(int end1, int end2, int mid) {
  R a, b, c;
  Distance(end1, mid, a);
  Distance(end2, mid, b);
  Distance(end1, end2, c);
  return a + b == c;
}

void ConstructGraph() {
  vector<pair<int, int>> edge_candidates;
  for (const auto &f : regions) {
    rep (i, f.size()) {
      int a = f[i], b = f[(i + 1) % f.size()];
      edge_candidates.emplace_back(MakePairUnordered(a, b));
    }
  }
  sort(all(edge_candidates));
  edge_candidates.erase(unique(all(edge_candidates)), edge_candidates.end());

  int num_edges = 0;
  adj.resize(num_points);
  adj_all.resize(num_points);
  for (const auto &e : edge_candidates) {
    int a, b;
    tie(a, b) = e;
    adj_all[a].emplace_back(b);
    adj_all[b].emplace_back(a);

    Bigrat d2 =
        Sqr(coords[a].x() - coords[b].x()) +
        Sqr(coords[a].y() - coords[b].y());

    Bigrat dummy;
    if (!Distance(a, b, dummy)) continue;

    Bignum num2 = numerator(d2), den2 = denominator(d2);
    Bignum num, numR, den, denR;
    num = bm::sqrt(num2, numR);
    den = bm::sqrt(den2, denR);
    if (numR != 0 || denR != 0) {
      continue;
      // Distance not rational!!
    } else {
      assert(num * num == num2);
      assert(den * den == den2);
      // cerr << num << " " << den << endl;
      Bigrat d = Bigrat(num, den);
      assert(d * d == d2);
      // cerr << "Edge: " << a << "--" << b << ": " << d << " " << d2 << endl;

      ++num_edges;
      adj[a].emplace_back(b, d);
      adj[b].emplace_back(a, d);
    }
  }

  cerr
      << "All edges: " << edge_candidates.size() << endl
      << "Rational edges: " << num_edges << endl;
}


//
// Search algorithms
//

vector<Edge> path;

void EnumeratePaths(const DAG &dag, const Edge &state) {
  path.emplace_back(state);

  if (state.second == 0) {
    printf("(len=%d) ", (int)path.size());
    rep (i, path.size()) {
      printf("%d ", path[i].first);
    }
    puts("");
  } else {
    const auto &prvs = dag[state.first].at(state.second);
    for (const auto &prv : prvs) {
      EnumeratePaths(dag, prv);
    }
  }

  path.pop_back();
}

DAG ComputeDAG(const Graph &graph, int src_v) {
  DAG dag(graph.size());
queue<Edge> que;
// stack<Edge> que;
  que.push({src_v, 0});

  rep (iter, FLAGS_search_limit) {
    if (que.empty()) break;
    Edge crr = que.front();
    que.pop();

    if (crr.second == 1) {
      printf("%d -> %d\n", src_v, crr.first);
      EnumeratePaths(dag, crr);
    }
// if (iter % 1000 == 0) cerr << src_v << " " << iter << ": " << static_cast<double>(crr.second) << endl;

    for (const auto &e : graph[crr.first]) {
      Edge nxt(e.first, crr.second + e.second);
      if (nxt.second > 1) continue;

      auto &v = dag[nxt.first][nxt.second];
      if (v.empty()) que.push(nxt);
      v.emplace_back(crr);
    }
  }

  return dag;
}


DAG ComputeDAG2(const Graph &graph, int src_v) {
  DAG dag(graph.size());
  multimap<Bignum, Edge> que;
// stack<Edge> que;
  que.insert(make_pair(1, Edge{src_v, 0}));

  rep (iter, FLAGS_search_limit) {
    if (que.empty()) break;
    Edge crr = que.begin()->second;
    que.erase(que.begin());

    if (crr.second == 1) {
      printf("%d -> %d\n", src_v, crr.first);
      EnumeratePaths(dag, crr);
    }
// if (iter % 1000 == 0) cerr << src_v << " " << iter << ": " << static_cast<double>(crr.second) << endl;

    for (const auto &e : graph[crr.first]) {
      Edge nxt(e.first, crr.second + e.second);
      if (nxt.second > 1) continue;

      auto &v = dag[nxt.first][nxt.second];
      if (v.empty()) que.insert(make_pair(denominator(nxt.second), nxt));
      v.emplace_back(crr);
    }
  }

  return dag;
}

namespace dfs {
vector<int> vis;
vector<int> path;
set<vector<int>> cache;
int num_visited_nodes = 0;

// i0 -> i1 -> i2
bool IsTurnable(int i0, int i1, int i2) {
  Point v1 = coords[i1];
  Point v2 = coords[i2];
  bg::subtract_point(v1, coords[i0]);
  bg::subtract_point(v2, coords[i1]);

  Bigrat d1, d2;
  CHECK(Distance(i0, i1, d1));
  CHECK(Distance(i1, i2, d2));
  v1 = Point(v1.x() / d1, v1.y() / d1);
  v2 = Point(v2.x() / d2, v2.y() / d2);

  Point va = v1;
  bg::add_point(va, v2);
  if (va.x() == 0 && va.y() == 0) {  // Case 1: U-turn case
    for (int j : adj_all[i1]) {
      Point vb = coords[j];
      bg::subtract_point(vb, coords[i1]);
      if (bg::dot_product(v1, vb) == 0) return true;
    }
  } else {
    // cerr << Det(va, v1) << " " << Det(va, v2) << endl;
    for (int j : adj_all[i1]) {
      Point vb = coords[j];
      bg::subtract_point(vb, coords[i1]);

      if (Det(va, vb) == 0) {
        // cerr << "--" << endl;
        // PrintPoint(coords[i0]);
        // PrintPoint(coords[i1]);
        // PrintPoint(coords[i2]);
        // cout << coords[i0] << " " << coords[i1] << " " << coords[i2] << endl;
        return true;  // TODO: check dot?
      }
    }
  }
  // puts("NO");
  return false;
}

int num_hints = 0;

void DFS(int v, Bigrat d) {
  if (++num_visited_nodes > FLAGS_search_limit) return;

  if (d > 1) return;
  if (vis[v] >= FLAGS_visit_limit) return;
  if (d == 1) {
    if (cache.insert(path).second) {
      printf("%d\n", (int)path.size() + 1);
      for (int p : path) printf("%d ", p);
      printf("%d\n", v);

      ofstream ofs(string("hints/") + to_string(num_hints) + ".txt");
      ofs << path.size() + 1 << endl;
      for (int p : path) ofs << p << " ";
      ofs << v << endl;
      ++num_hints;
    }
    return;
  }

  ++vis[v];
  path.emplace_back(v);

  for (const auto &e : adj[v]) {
    bool is_straight = path.size() >= 2 &&
        IsOnSegment(path[path.size() - 2], e.first, path[path.size() - 1]);
    bool is_turnable = path.size() >= 2 &&
        IsTurnable(path[path.size() - 2], path[path.size() - 1], e.first);
    if (is_straight) assert(is_turnable);

    if (path.size() >= 2 && !is_turnable) continue;

    DFS(e.first, d + e.second);
  }

  --vis[v];
  path.pop_back();
}
}  // namespace dfs


//
// Entry point
//

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  Input();
  cerr << "Number of ponits: " << coords.size() << endl;
  ConstructGraph();

  vector<int> src_vs;
  if (FLAGS_sources == "all") {
    rep (i, coords.size()) src_vs.emplace_back(i);
  } else {
    src_vs = ParseCommaSeparatedString<int>(FLAGS_sources);
  }

  for (int v : src_vs) {
    // cerr << "[[ " << v << " ]]" << endl;

    if (FLAGS_algorithm == "dfs") {
      rep (iter, FLAGS_restart) {
        for (auto &a : adj) random_shuffle(all(a));
        dfs::vis.assign(coords.size(), false);
        dfs::num_visited_nodes = 0;
        dfs::DFS(v, 0);
      }
    } else if (FLAGS_algorithm == "best_first") {
      ComputeDAG2(adj, v);
    } else {
      puts("UNKNOWN ALGORITHM");
      exit(EXIT_FAILURE);
    }
  }
}
