#pragma once
#include <complex>
#include <vector>

#include "base/base.h"
#include "boost/multiprecision/gmp.hpp"
#include "boost/rational.hpp"

using boost::rational;
using boost::rational_cast;

typedef boost::multiprecision::number<boost::multiprecision::gmp_rational,
                                      boost::multiprecision::et_off> Q;
typedef std::complex<Q> C;

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
  while (isspace(is.peek())) {
    is.get();
  }
  string buf;
  while (true) {
    int c = is.peek();
    if (isdigit(c) || c == '-' || c == '/') {
      buf += c;
      is.get();
    } else {
      break;
    }
  }
  return Q(buf);
}

std::istream& operator>>(std::istream& is, Vertex& v) {
  v.x = consume_rational(is);
  int c = is.get();
  if (c != ',') {
    LOG(ERROR) << "Expected comma but was " << (char)c << ":" << c;
  }
  v.y = consume_rational(is);
  return is;
}

std::ostream& operator<<(std::ostream& os, const Vertex& v) {
  return os << v.x << ',' << v.y;
}

std::istream& operator>>(std::istream& is, Polygon& p) {
  int n_verts;
  is >> n_verts;
  p.resize(n_verts);
  for (int i = 0; i < n_verts; ++i) {
    is >> p[i];
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
