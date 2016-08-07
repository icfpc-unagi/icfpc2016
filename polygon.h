#pragma once
#include <complex>
#include <vector>

#include "base/base.h"
#include "boost/multiprecision/gmp.hpp"

typedef boost::multiprecision::mpz_int Z;
typedef boost::multiprecision::number<boost::multiprecision::gmp_rational,
                                      boost::multiprecision::et_off> Q;

namespace std {
// Dummy functions for std::complex
bool isnan(const Q& q) { return false; }
bool isinf(const Q& q) { return false; }
Q copysign(const Q& x, const Q& y) { return x.sign() != y.sign() ? -x : x; }
}

struct Vertex {
  Q x;
  Q y;
};

typedef vector<Vertex> Polygon;

template <typename T>
void ResolveIndexReference(const vector<int>& index, const vector<T>& ref,
                           vector<T>* out) {
  for (int i : index) {
    out->emplace_back(ref[i]);
  }
}

bool is_ccw(const Polygon& p) {
  Q area;
  for (int i = 1; i < p.size(); ++i) {
    area += (p[i].x - p[0].x) * (p[i + 1].y - p[0].y) -
            (p[i].y - p[0].y) * (p[i + 1].x - p[0].x);
  }
  LOG_IF(ERROR, area == 0) << "Unexpected zero area";
  return area < 0;
}

Q consume_rational(std::istream& is) {
  while (is.good() && isspace(is.peek())) {
    is.get();
  }
  string buf;
  while (is.good()) {
    int c = is.peek();
    if (isdigit(c) || c == '-' || c == '/') {
      buf += c;
      is.get();
    } else {
      break;
    }
  }
  CHECK(!buf.empty());
  return Q(buf);
}

std::istream& operator>>(std::istream& is, Vertex& v) {
  v.x = consume_rational(is);
  while (is.good() && isspace(is.peek())) is.get();
  int c = is.peek();
  LOG_IF(ERROR, c != ',' && !isdigit(c)) << "Expected comma but was " << (char)c
                                         << ":" << c;
  if (c == ',') is.get();
  v.y = consume_rational(is);
  return is;
}

std::ostream& operator<<(std::ostream& os, const Vertex& v) {
  return os << v.x << ',' << v.y;
}

std::istream& operator>>(std::istream& is, Polygon& p) {
  int n_verts;
  CHECK(is >> n_verts);
  p.resize(n_verts);
  for (int i = 0; i < n_verts; ++i) {
    CHECK(is >> p[i]);
  }
  return is;
}

std::ostream& operator<<(std::ostream& os, const Polygon& p) {
  os << p.size() << '\n';
  for (const auto& v : p) {
    os << v << '\n';
  }
  return os;
}
