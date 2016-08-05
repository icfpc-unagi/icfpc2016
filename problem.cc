#include "base/base.h"
#include "boost/multiprecision/cpp_int.hpp"
#include "boost/rational.hpp"
#include <sstream>

using boost::rational;
using boost::rational_cast;

typedef boost::multiprecision::cpp_int Integer;
typedef rational<Integer> Q;

struct Vertex {
  Q x;
  Q y;
};

typedef vector<Vertex> Polygon;

istream& operator>>(istream& is, Q& q) {
  int num, den;
  is >> num;
  if (is.get() == '/') {
    is >> den;
    q = Q(num, den);
  } else {
    is.unget();
    q = Q(num);
  }
  return is;
}

istream& operator>>(istream& is, Vertex& v) {
  char comma;
  return is >> v.x >> comma >> v.y;
}

istream& operator>>(istream& is, Polygon& p) {
  int n_verts;
  is >> n_verts;
  p.resize(n_verts);
  for (int i = 0; i < n_verts; ++i) {
    is >> p[i];
  }
  return is;
}

int main() {
  int n_polys;
  cin >> n_polys;
  vector<Polygon> polys(n_polys);
  for (int i = 0; i < n_polys; ++i) {
    cin >> polys[i];
  }
  // TODO: Parse skelton

  printf(
      R"(<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="300px" height="300px" viewBox="0 0 1 1"><rect x="0" y="0" width="1" height="1" fill="none" stroke="blue" stroke-width="0.01"/>)");
  for (int i = 0; i < n_polys; ++i) {
    std::stringstream path;
    for (int j = 0; j < polys[i].size(); ++j) {
      path << (j == 0 ? "M " : "L ") << rational_cast<double>(polys[i][j].x)
           << " " << rational_cast<double>(polys[i][j].y) << " ";
    }
    path << "Z";
    printf(R"(<path d="%s" fill="silver" stroke="gray" stroke-width="0.01" />)",
           path.str().c_str());
  }
  printf("</svg>");

  return 0;
}