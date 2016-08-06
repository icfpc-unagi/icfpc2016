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
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <boost/geometry/algorithms/area.hpp>

#define all(c) (c).begin(), (c).end()
#define iter(c) __typeof((c).begin())
#define cpresent(c, e) (find(all(c), (e)) != (c).end())
#define rep(i, n) for (int i = 0; i < (int)(n); i++)
#define tr(c, i) for (iter(c) i = (c).begin(); i != (c).end(); ++i)
#define pb(e) push_back(e)
#define mp(a, b) make_pair(a, b)

using namespace std;
namespace bm = boost::multiprecision;
namespace bg = boost::geometry;

// using Bigrat = boost::multiprecision::cpp_rational;
// using Bigrat = bm::number<bm::cpp_rational, bm::et_off>;
using Bigrat = bm::number<bm::gmp_rational, bm::et_off>;
using Point = bg::model::d2::point_xy<Bigrat>;
using Segment = bg::model::segment<Point>;
using Polygon = bg::model::polygon<Point, false>;
using pii = pair<int, int>;

pii MakePairUnordered(int a, int b) {
  return mp(min(a, b), max(a,b));
}

inline string ReadAllAndRemoveComma(istream &is) {
  string in;
  for (string line; getline(is, line); ) {
    in += line + "\n";
  }
  for (char &c : in) if (c == ',') c = ' ';
  return in;
}

inline Point ReadPoint(istream &is) {
  Bigrat x, y;
  is >> x >> y;
  return Point(x, y);
}

inline Segment ReadSegment(istream &is) {
  return Segment(ReadPoint(is), ReadPoint(is));
}

inline int Quadrant(const Point &p) {
  assert(p.x() != 0 || p.y() != 0);
  if (p.y() >= 0 && p.x() > 0) return 0;
  if (p.x() <= 0 && p.y() > 0) return 1;
  if (p.y() <= 0 && p.x() < 0) return 2;
  else return 3;
}

inline Bigrat Det(const Point &a, const Point &b) {
  return a.x() * b.y() - a.y() * b.x();
}

// NOTE: it returns all the regions, including outer one!
vector<vector<int>> EnumerateRegions
(const vector<Point> &coords, const vector<vector<int>> &adj) {
  map<pii, pii> nxt;
  rep (i, coords.size()) {
    const Point &p = coords[i];
    vector<int> ord = adj[i];
    sort(all(ord), [&] (const int a, const int b) -> bool {
        Point av = coords[a], bv = coords[b];
        bg::subtract_point(av, p);
        bg::subtract_point(bv, p);
        int aq = Quadrant(av), bq = Quadrant(bv);
        if (aq != bq) return aq < bq;
        else return Det(av, bv) > 0;
      });

    rep (j, ord.size()) {
      nxt[mp(ord[(j + 1) % ord.size()], i)] = mp(i, ord[j]);
    }
  }

  vector<vector<int>> regions;
  set<pii> usd;
  rep (i, coords.size()) {
    for (int j : adj[i]) {
      if (usd.count(mp(i, j))) continue;

      vector<int> reg;
      int a = i, b = j;
      while (usd.count(mp(a, b)) == 0) {
        reg.emplace_back(a);
        usd.insert(mp(a, b));
        tie(a, b) = nxt.at(mp(a, b));
      }
      regions.emplace_back(reg);
    }
  }

  return regions;
}
