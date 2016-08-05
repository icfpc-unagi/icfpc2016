#include "base/base.h"
#include "boost/multiprecision/cpp_int.hpp"
#include "boost/rational.hpp"

using namespace std;

using boost::rational;
using boost::rational_cast;

typedef boost::multiprecision::cpp_rational Q;

struct Vertex {
  Q x;
  Q y;
};

typedef vector<Vertex> Polygon;

std::istream& operator>>(istream& is, Vertex& v) {
  string r;
  while (isspace(is.peek())) {
    is.get();
  }
  std::getline(is, r, ',');
  v.x.assign(r);
  while (isspace(is.peek())) {
    is.get();
  }
  std::getline(is, r);
  v.y.assign(r);
  return is;
}

std::istream& operator>>(istream& is, Polygon& p) {
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

  Q min_x = 0, min_y = 0, max_x = 1, max_y = 1;
  for (int i = 0; i < n_polys; ++i) {
    for (const auto& v : polys[i]) {
      if (v.x < min_x) min_x = v.x;
      if (max_x < v.x) max_x = v.x;
      if (v.y < min_y) min_y = v.y;
      if (max_y < v.y) max_y = v.y;
    }
  }
  printf(
      R"(<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="300px" height="300px" viewBox="%f %f %f %f"><rect x="0" y="0" width="1" height="1" fill="none" stroke="blue" stroke-width="0.01"/>)",
      min_x.convert_to<double>(), min_y.convert_to<double>(),
      Q(max_x - min_x).convert_to<double>(),
      Q(max_y - min_y).convert_to<double>());
  for (int i = 0; i < n_polys; ++i) {
    printf(R"(<path d=")");
    for (int j = 0; j < polys[i].size(); ++j) {
      printf("%c %.8f %.8f ", j == 0 ? 'M' : 'L',
             polys[i][j].x.convert_to<double>(),
             polys[i][j].y.convert_to<double>());
    }
    printf(R"(Z" fill="silver" stroke="gray" stroke-width="0.01" />)");
  }
  printf("</svg>");

  return 0;
}
