#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cmath>
#include <cassert>
#include <sstream>
#include <fstream>

#include <boost/assert.hpp>
#include <boost/multiprecision/cpp_int.hpp>
// #include <boost/multiprecision/gmp.hpp>

#define all(c) (c).begin(), (c).end()
#define iter(c) __typeof((c).begin())
#define cpresent(c, e) (find(all(c), (e)) != (c).end())
#define rep(i, n) for (int i = 0; i < (int)(n); i++)
#define tr(c, i) for (iter(c) i = (c).begin(); i != (c).end(); ++i)
#define pb(e) push_back(e)
#define mp(a, b) make_pair(a, b)


#define CHECK(expr)                                                     \
  if (expr) {                                                           \
  } else {                                                              \
    fprintf(stderr, "CHECK Failed (%s:%d): %s\n",                       \
            __FILE__, __LINE__, #expr);                                 \
    ::exit(EXIT_FAILURE);                                               \
  }

#define CHECK_PERROR(expr)                                              \
  if (expr) {                                                           \
  } else {                                                              \
    fprintf(stderr, "CHECK Failed (%s:%d): %s: ",                       \
            __FILE__, __LINE__, #expr);                                 \
    perror(nullptr); \
    ::exit(EXIT_FAILURE);                       \
  }

#define FAIL_PERROR() \
    do {              \
      fprintf(stderr, "Error (%s:%d): ", __FILE__, __LINE__); \
      perror(nullptr); \
      ::exit(EXIT_FAILURE);                     \
    } while (0)



using namespace std;
namespace bm = boost::multiprecision;
// namespace bg = boost::geometry;

// using Bigrat = boost::multiprecision::cpp_rational;
using R = bm::cpp_rational;
using I = bm::cpp_int;
// using Bigrat = bm::number<bm::gmp_rational, bm::et_off>;
// using Point = bg::model::d2::point_xy<Bigrat>;
// using Segment = bg::model::segment<Point>;
// using Polygon = bg::model::polygon<Point, false>;
using pii = pair<int, int>;

inline pii MakePairUnordered(int a, int b) {
  return mp(min(a, b), max(a,b));
}

template<typename T>
std::vector<T> ParseSpaceSeparatedString(const std::string &str) {
  std::istringstream ss(str);
  std::vector<T> res;
  for (T t; ss >> t; ) res.emplace_back(t);
  return res;
}

template<typename T>
std::vector<T> ParseCommaSeparatedString(std::string str) {
  std::replace(str.begin(), str.end(), ',', ' ');
  return ParseSpaceSeparatedString<T>(str);
}

inline string ReadAllAndRemoveComma(istream &is) {
  string in;
  for (string line; getline(is, line); ) {
    in += line + "\n";
  }
  for (char &c : in) if (c == ',') c = ' ';
  return in;
}

//
// Point
//
struct P {
  R x, y;

  P() {}
  P(const R &x_, const R &y_) : x(x_), y(y_) {}

  ofPoint OFPoint() const {
    return ofPoint(static_cast<double>(x), static_cast<double>(y));
  }
};

inline P operator+(const P &a, const P &b) {
  return P(a.x + b.x, a.y + b.y);
}

inline P operator-(const P &a, const P &b) {
  return P(a.x - b.x, a.y - b.y);
}

inline P operator*(const R &t, const P &a) {
  return P(t * a.x, t * a.y);
}

inline P operator*(const P &a, const R &t) {
  return P(t * a.x, t * a.y);
}

inline std::istream &operator>>(std::istream &is, P &p) {
  return is >> p.x >> p.y;
};

inline std::ostream &operator<<(std::ostream &os, const P &p) {
  return os << "(" << p.x << "," << p.y << ")";
};

inline int Quadrant(const P &p) {
  assert(p.x != 0 || p.y != 0);
  if (p.y >= 0 && p.x > 0) return 0;
  if (p.x <= 0 && p.y > 0) return 1;
  if (p.y <= 0 && p.x < 0) return 2;
  else return 3;
}

inline R Det(const P &a, const P &b) {
  return a.x * b.y - a.y * b.x;
}
