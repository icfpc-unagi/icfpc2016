// Receives prefiltered input.

#include "common.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <gflags/gflags.h>

using Edge = pair<int, Bigrat>;
using EdgeList = vector<Edge>;
using Graph = vector<EdgeList>;
using DAG = vector<map<Bigrat, vector<Edge>>>;

int num_points;
vector<Point> coords;
vector<vector<int>> regions;

DEFINE_int32(search_limit, 100000, "");
DEFINE_int32(visit_limit, 1, "");
DEFINE_int32(restart, 1, "");
DEFINE_string(algorithm, "dfs", "");
DEFINE_string(sources, "all", "");

Graph adj;

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

inline Bigrat Sqr(Bigrat x) {
  return x * x;
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
  for (const auto &e : edge_candidates) {
    int a, b;
    tie(a, b) = e;

    Bigrat d2 =
        Sqr(coords[a].x() - coords[b].x()) +
        Sqr(coords[a].y() - coords[b].y());

    Bignum num2 = numerator(d2), den2 = denominator(d2);
    Bignum num, numR, den, denR;
    num = bm::sqrt(num2, numR);
    den = bm::sqrt(den2, denR);
    if (numR != 0 || denR != 0) continue;  // Distance not rational!!
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

  cerr
      << "All edges: " << edge_candidates.size() << endl
      << "Rational edges: " << num_edges << endl;
}



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

void DFS(int v, Bigrat d) {
  if (++num_visited_nodes > FLAGS_search_limit) return;

  if (d > 1) return;
  if (vis[v] >= FLAGS_visit_limit) return;
  if (d == 1) {
    if (cache.insert(path).second) {
      printf("%d\n", (int)path.size() + 1);
      for (int p : path) printf("%d ", p);
      printf("%d\n", v);
    }
    return;
  }

  ++vis[v];
  path.emplace_back(v);

  for (const auto &e : adj[v]) {
    DFS(e.first, d + e.second);
  }

  --vis[v];
  path.pop_back();
}
}

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
    cerr << "[[ " << v << " ]]" << endl;

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





/*
vector<vector<vector<pair<int, Bigrat>>>> solutions;

int src_v;
vector<vector<pair<int, Bigrat>>> tmp;

void search(int step, int v, int prv_v, const Bigrat &d) {
  if (d > 1) return;

  tmp[step].emplace_back(v, d);

  if (d == 1) {
    if (step == 3) {
      if (src_v === v) {

      }
    } else {
      search(step + 1, v, prv_v, 0);
    }
  } else {
    rep (iter, 2) {
      for (const auto &e : adj[v]) {
        if ((e.first == prv_v) != (iter == 1)) continue;

        Bigrat nxt_d = d + e.second;
        search(step, e.first, v, nxt_d);
      }
    }

  }

  tmp[step].pop_back();
}
*/
