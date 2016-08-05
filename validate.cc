#include <complex>
#include <vector>
#include <fstream>

#include "base/base.h"
#include "boost/multiprecision/cpp_int.hpp"
#include "boost/rational.hpp"
#include "polygon.h"
#include "problem.h"
#include "solution.h"

DEFINE_string(problem, "", "input problem file");
DEFINE_string(solution, "", "input solution file");

using boost::rational;
using boost::rational_cast;
using namespace std;

template <typename T>
void normalize_pair(std::pair<T, T>* p) {
  if (p->second < p->first) std::swap(p->first, p->second);
}

int main(int argc, char** argv) {
  ParseCommandLineFlags(&argc, &argv);

  Problem problem;
  std::ifstream problem_ifs(FLAGS_problem);
  ReadProblem(problem_ifs, &problem);
  Solution solution;
  std::ifstream solution_ifs(FLAGS_solution);
  ReadSolution(solution_ifs, &solution);

  // edge (normalized pair of vertex ids) to facet ids
  map<pair<int, int>, set<int>> edge_to_facet;
  for (int i = 0; i < solution.facets.size(); ++i) {
    for (int j = 0; j < solution.facets[i].size() - 1; ++j) {
      pair<int, int> edge(solution.facets[i][j], solution.facets[i][j + 1]);
      normalize_pair(&edge);
      edge_to_facet[edge].insert(i);
    }
  }
  for (const auto& it : edge_to_facet) {
    const auto& edge = it.first;
    const auto& adj_facets = it.second;
    if (adj_facets.size() > 2) {
      LOG(ERROR) << "Edge[" << edge.first << "," << edge.second
                 << "] is shared by more than 2 facets ("
                 << strings::JoinInts(adj_facets, ",") << ")";
      return 1;
    }
  }

  // TODO: Verify src facets congruence vs dst

  // TODO: Verify dst facets shape problem silhouette

  // TODO: Calculate score?

  return 0;
}
