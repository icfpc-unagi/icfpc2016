#pragma once
#include <complex>
#include <vector>
#include <fstream>

#include "base/base.h"
#include "boost/multiprecision/cpp_int.hpp"
#include "boost/rational.hpp"
#include "polygon.h"

using boost::rational;
using boost::rational_cast;

struct Solution {
  vector<Vertex> src_verts;
  vector<Vertex> dst_verts;
  vector<vector<int>> facets;
};

bool ReadSolution(std::istream& is, Solution* solution) {
  int n_verts;
  is >> n_verts;
  solution->src_verts.resize(n_verts);
  for (int i = 0; i < n_verts; ++i) {
    is >> solution->src_verts[i];
  }
  int n_facets;
  is >> n_facets;
  solution->facets.resize(n_facets);
  for (int i = 0; i < n_facets; ++i) {
    int n_indices;
    is >> n_indices;
    solution->facets[i].resize(n_indices);
    for (int j = 0; j < n_indices; ++j) {
      is >> solution->facets[i][j];
    }
  }
  solution->dst_verts.resize(n_verts);
  for (int i = 0; i < n_verts; ++i) {
    is >> solution->dst_verts[i];
  }
  return is.good();
}
