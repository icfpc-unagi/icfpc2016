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
using namespace std;

struct Problem {
  vector<Polygon> polygons;
  vector<pair<Vertex, Vertex>> skelton;
};

bool ReadProblem(std::istream& is, Problem* p) {
  int n_polys;
  is >> n_polys;
  p->polygons.resize(n_polys);
  for (int i = 0; i < n_polys; ++i) {
    is >> p->polygons[i];
  }
  int n_edges;
  is >> n_edges;
  p->skelton.resize(n_edges);
  for (int i = 0; i < n_edges; ++i) {
    is >> p->skelton[i].first >> p->skelton[i].second;
  }
  return is.good();
}

void WriteProblem(const Problem& p, std::ostream& os) {
  os << p.polygons.size() << '\n';
  for (const auto& poly : p.polygons) {
    os << poly;
  }
  os << p.skelton.size() << '\n';
  for (const auto& seg : p.skelton) {
    os << seg.first << ' ' << seg.second << '\n';
  }
}

struct FilteredProblem {
  vector<Vertex> vertices;
  vector<vector<int>> polygons;
};

bool ReadFilteredProblem(std::istream& is, FilteredProblem* p) {
  int n_verts;
  is >> n_verts;
  p->vertices.resize(n_verts);
  for (int i = 0; i < n_verts; ++i) {
    is >> p->vertices[i];
  }
  int n_polys;
  is >> n_polys;
  p->polygons.resize(n_polys);
  for (int i = 0; i < n_polys; ++i) {
    int n;
    is >> n;
    p->polygons[i].resize(n);
    for (int j = 0; j < n; ++j) {
      is >> p->polygons[i][j];
    }
  }
  return is.good();
}
