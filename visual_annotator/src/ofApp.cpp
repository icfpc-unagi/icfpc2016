/*
YUUSEN:
TODO:
- 90do 45do
- shinkouhoukou ni taishite ima eranderu sen no aida ni aru sen
- emscripten



(- entyousen matomoni? )

DONE:
- entyousen
- susumu toki zenpou ni
- tyokusen ka douka de iro ga kawaru
- kakudai
- enter -> hint solver kidou
 */

#include "ofApp.h"
#include "common.h"
#include <boost/variant.hpp>

const double kDefaultDisplaySize = 500;

// using namespace iwiwi;

//
// Algorithms
//

using Edge = pair<int, R>;
using EdgeList = vector<Edge>;
using Graph = vector<EdgeList>;
string solver_command;
string submit_command;
string output_directory;
string problem_id;

namespace {
int num_points;
vector<P> coords;
vector<vector<int>> regions;
Graph adj_rational;
vector<vector<int>> adj_irrational;
}  // namespace

inline R Sqr(R x) {
  return x * x;
}

inline I Sqrt(I x, I &r) {
  I s;
  s = bm::sqrt(x, r);

  for (int dif = -2; dif <= 2; ++dif) {
    if (s + dif < 0) continue;
    I ts = s + dif;
    if (ts * ts == x) {
      r = 0;
      return ts;
    }
  }
  return s;
}

bool Distance(int a, int b, R &d) {  // Returns whether rational
  R d2 =
      Sqr(coords[a].x - coords[b].x) +
      Sqr(coords[a].y - coords[b].y);

  // cerr << d2 << endl;

  I num2 = numerator(d2), den2 = denominator(d2);
  I num, numR, den, denR;
  num = Sqrt(num2, numR);
  den = Sqrt(den2, denR);
  // cerr << num2 << " " << num << " " << numR << endl;
  // cerr << den2 << " " << den << " " << denR << endl;
  if (numR != 0 || denR != 0) {
    return false;
  } else {
    assert(num * num == num2);
    assert(den * den == den2);
    // cerr << num << " " << den << endl;
    d = R(num, den);
    assert(d * d == d2);
    // cerr << "Edge: " << a << "--" << b << ": " << d << " " << d2 << endl;

    return true;
  }
}

bool IsOnSegment(int end1, int end2, int mid) {
  R a, b, c;
  Distance(end1, mid, a);
  Distance(end2, mid, b);
  Distance(end1, end2, c);
  return a + b == c;
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
  adj_rational.resize(num_points);
  adj_irrational.resize(num_points);
  for (const auto &e : edge_candidates) {
    int a, b;
    tie(a, b) = e;

    R d;
    if (Distance(a, b, d)) {
      ++num_edges;
      adj_rational[a].emplace_back(b, d);
      adj_rational[b].emplace_back(a, d);
    } else {
      adj_irrational[a].emplace_back(b);
      adj_irrational[b].emplace_back(a);
    }
  }

  cerr
      << "All edges: " << edge_candidates.size() << endl
      << "Rational edges: " << num_edges << endl;

  // Sorting
  rep (i, num_points) {
    P p = coords[i];
    auto &ord = adj_rational[i];
    sort(all(ord), [&] (const Edge &ae, const Edge &be) -> bool {
        int a = ae.first, b = be.first;
        P av = coords[a], bv = coords[b];
        av = av - p;
        bv = bv - p;
        int aq = Quadrant(av), bq = Quadrant(bv);
        if (aq != bq) return aq < bq;
        else return Det(av, bv) > 0;
      });
  }
}

int NextCandidate(const vector<int> &path, int crr_candidate) {
  if (path.empty()) {  // The first point
    do {
      (crr_candidate += 1) %= num_points;
    } while (adj_rational[crr_candidate].empty());
    return crr_candidate;
  }
  else {
    int pivot = path.back();
    const auto &a = adj_rational[pivot];
    if (crr_candidate == -1) {
      if (path.size() >= 2) {
        for (const auto &e : a) {
          if (IsOnSegment(path[path.size() - 2], e.first, path[path.size() - 1])) {
            return e.first;
          }
        }
      }
      return a[0].first;
    } else {
      rep (i, a.size()) {
        if (a[i].first == crr_candidate) {
          return a[(i + 1) % a.size()].first;
        }
      }
      assert(false);
    }
  }
}

int PrevCandidate(const vector<int> &path, int crr_candidate) {
  if (path.empty()) {  // The first point
    do {
      (crr_candidate += -1 + num_points) %= num_points;
    } while (adj_rational[crr_candidate].empty());
    return crr_candidate;
  } else {
    int pivot = path.back();
    const auto &a = adj_rational[pivot];
    if (crr_candidate == -1) {
      if (path.size() >= 2) {
        for (const auto &e : a) {
          if (IsOnSegment(path[path.size() - 2], e.first, path[path.size() - 1])) {
            return e.first;
          }
        }
      }
      return a[0].first;
    } else {
      rep (i, a.size()) {
        if (a[i].first == crr_candidate) {
          return a[(i - 1 + a.size()) % a.size()].first;
        }
      }
      assert(false);
    }
  }
}

R PathDistance(const vector<int> &path) {
  R d = 0;
  rep (i, int(path.size()) - 1) {
    R td;
    Distance(path[i], path[i + 1], td);
    d += td;
  }
  return d;
}


//
// External events, which are mainly assumed as user operations via javascript.
//
namespace ext_event {
struct SetInput {
  string input;
};
struct SaveHintAndExecuteSolver {};

using Info = boost::variant<
  SetInput,
  SaveHintAndExecuteSolver
  >;

queue<Info> queue;
mutex queue_mutex;

void push(Info i) {
  lock_guard<mutex> l(queue_mutex);
  queue.push(i);
}

void push_set_input(const string &txt) {
  ext_event::queue.push(ext_event::SetInput{txt});
}

void PushSaveHintAndExecuteSolver() {
  ext_event::queue.push(ext_event::SaveHintAndExecuteSolver());
}
}  // namespace ext_event

#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/bind.h>

EMSCRIPTEN_BINDINGS(my_module) {
  emscripten::function("setInput", &ext_event::push_set_input);
  // emscripten::function("setSolution", &ext_event::push_set_solution);
  // emscripten::function("setInputAndSolution", &ext_event::push_set_input_and_solution);
}
#endif


//
// Global variables
//
namespace {
double display_size = kDefaultDisplaySize;

vector<int> path;
int candidate;

string raw_problem;
}


void PrintPathDistance(const vector<int> &path, ostream &os) {
  R d = PathDistance(path);
  if (d > 0 && denominator(d) == 1) {
    os << "-----------------------" << endl
       << "  I N T E G E R ! ! !  " << endl
       << "-----------------------" << endl;
  }
  os << "[Distance: " << static_cast<double>(d) << " (" << d << ")";
  if (path.size() >= 2) {
    R d2;
    Distance(path[path.size() - 2], path[path.size() - 1], d2);
    os << ", Edge: " << static_cast<double>(d2) << " (" << d2 << ")";
  }
  os << "]" << endl;
}

void PrintHint(const vector<int> &path, ostream &os) {
  R d = PathDistance(path);
  if (d == 4) {
    os << path.size() - 1 << endl;
    rep (i, path.size() - 1) os << path[i] << " ";
    os << endl;
  } else {
    os << path.size() << endl;
    for (int v : path) os << v << " ";
    os << endl;
  }
}

string PathSummary(const vector<int> &path) {
  ostringstream os;
  PrintPathDistance(path, os);
  PrintHint(path, os);

  return os.str();
}

void SetInput(const string &txt) {
  raw_problem = txt;

  // Input
  istringstream is(txt);
  is >> num_points;
  coords.resize(num_points);
  rep (i, num_points) {
    is >> coords[i];
    coords[i].y = coords[i].y * -1;
  }
  cerr << "Number of points: " << num_points << endl;

  int num_regions;
  is >> num_regions;
  regions.resize(num_regions);
  rep (i, num_regions) {
    auto &f = regions[i];
    int n;
    is >> n;
    f.resize(n);
    rep (j, n) is >> f[j];
  }

  // Construct the graph
  ConstructGraph();
  path.clear();
  candidate = NextCandidate(path, -1);
}

void ofApp::setup(){
  ofSetFrameRate(30);
  ofBackground(255, 255, 255);

  ofTrueTypeFont::setGlobalDpi(72);
  font.load("verdana.ttf", 12, true, true);

  // gui.setup();
  // gui.add(step_slider.setup("step", 0, 0, sol.size()));
  // gui.add(speed_slider.setup("speed", 2.0, -1, 3));
  // gui.add(history_slider.setup("history", 100, 1, 1000));
  // gui.setPosition(N * scale, 0);

  ofSetWindowShape(display_size * 2, display_size);
}

//
// Event handlers
//
struct ext_event_handler : boost::static_visitor<void> {
  ext_event_handler(ofApp &app) : app_(app) {}

  void operator()(ext_event::SetInput &t) const {
    SetInput(t.input);
  }

  void operator()(ext_event::SaveHintAndExecuteSolver&) const {
    cout << PathSummary(path) << endl;
    string hinted_problem_path = output_directory + "/" + to_string(getpid()) + ".hinted_problem.txt";
    string solution_path = output_directory + "/" + to_string(getpid()) + ".solution.txt";
    {
      ostringstream os;
      os << raw_problem;
      PrintHint(path, os);

      ofBuffer buffer = ofBuffer(os.str());
      ofBufferToFile(hinted_problem_path, buffer);
      cerr << "Hinted problem written to: " << hinted_problem_path << endl;
    }
    if (solver_command.empty()) {
      cerr << "WARNING: solver not specified" << endl;
    } else {
      {
        string cmd = solver_command
            + " < " + hinted_problem_path
            + " > " + solution_path;
        cerr << "Command to solve: " << cmd << endl;
        cerr << string(80, '-') << endl;
        system(cmd.c_str());
        cerr << string(80, '-') << endl;
        cerr << "Solution written to: " << solution_path << endl;
      }

      if (submit_command.empty()) {
        cerr << "WARNING: submit command not specified" << endl;
      } else {
        string cmd = submit_command
            + " --problem_id=" + problem_id
            + " --solution=" + solution_path;
        cerr << "Command to submit: " << cmd << endl;
        cerr << string(80, '-') << endl;
        system(cmd.c_str());
        cerr << string(80, '-') << endl;
      }
    }
  }

  template<typename T>
  void operator()(T &t) const {
    CHECK(false);
    assert(false);
  }

  ofApp &app_;
};

void ofApp::update(){
  // External events
  {
    lock_guard<mutex> l(ext_event::queue_mutex);

    while (!ext_event::queue.empty()) {
      ext_event::Info i = ext_event::queue.front();
      ext_event::queue.pop();
      boost::apply_visitor(ext_event_handler(*this), i);
    }
  }
}

ofPoint GetPoint(int v, const P &padding, const R &scale) {
  return (coords[v] * scale + padding).OFPoint() * display_size;
}

void DrawLine(ofPoint p1, ofPoint p2, const ofRectangle &clip) {
  if (!clip.inside(p1)) swap(p1, p2);
  if (!clip.inside(p1)) return;

  if (!clip.inside(p2)) {
    ofVec2f v = p2 - p1;
    double l = 0.0, r = 1.0;
    rep (iter, 100) {
      double m = (l + r) / 2;
      if (clip.inside(p1 + v * m)) l = m;
      else r = m;
    }
    p2 = p1 + v * (l + r) / 2;
  }

  ofDrawLine(p1, p2);
}

void DrawLine(int v, int w, const P &padding, const R &scale,
              const ofRectangle &clip) {
  ofPoint p1 = GetPoint(v, padding, scale);
  ofPoint p2 = GetPoint(w, padding, scale);
  DrawLine(p1, p2, clip);
}

void DrawPoint(int v, const P &padding, const R &scale, const ofRectangle &clip) {
  ofPoint p = GetPoint(v, padding, scale);
  if (clip.inside(p)) ofDrawCircle(p, 0.01 * display_size);
}

void Draw(const P &padding, const R &scale, const ofRectangle &clip) {
  ofSetLineWidth(1);
  ofSetColor(200, 200, 200);
  rep (v, num_points) {
    for (int w : adj_irrational[v]) DrawLine(v, w, padding, scale, clip);
  }

  ofSetColor(0, 0, 0);
  rep (v, num_points) {
    for (const Edge &e : adj_rational[v]) {
      int w = e.first;
      DrawLine(v, w, padding, scale, clip);
    }
  }

  ofSetColor(255, 0, 0);
  rep (i, int(path.size()) - 1) DrawLine(path[i], path[i + 1], padding, scale, clip);

  bool is_straight = false;
  if (!path.empty()) {
    ofSetColor(0, 0, 0);
    DrawPoint(path.back(), padding, scale, clip);
    //ofDrawCircle(GetPoint(path.back(), padding, scale), 0.01 * display_size);

    ofSetColor(255, 0, 0);
    // ofDrawCircle(GetPoint(path[0], padding, scale), 0.01 * display_size);
    DrawPoint(path[0], padding, scale, clip);


    is_straight = path.size() >= 2 &&
        IsOnSegment(path[path.size() - 2], candidate, path[path.size() - 1]);

    if (is_straight) ofSetColor(0, 0, 150);
    else ofSetColor(0, 150, 0);
    ofSetLineWidth(2);
    DrawLine(path.back(), candidate, padding, scale, clip);
    // Entyou sen
    {
      ofSetLineWidth(4);
      if (is_straight) ofSetColor(0, 0, 150, 70);
      else ofSetColor(0, 150, 0, 70);
      ofPoint a = GetPoint(path.back(), padding, scale);
      ofPoint b = GetPoint(candidate, padding, scale);
      DrawLine(a, a + (b - a) * display_size * 100, clip);
    }
    ofSetLineWidth(1);
  }

  if (is_straight) ofSetColor(0, 0, 150);
  else ofSetColor(0, 150, 0);
  // ofDrawCircle(GetPoint(candidate, padding, scale), 0.01 * display_size);
  DrawPoint(candidate, padding, scale, clip);
}

void ofApp::draw(){
  //
  // Normal
  //
  R minX, maxX, minY, maxY;
  minX = maxX = coords[0].x;
  minY = maxY = coords[0].y;
  rep (i, num_points) {
    minX = min(minX, coords[i].x); maxX = max(maxX, coords[i].x);
    minY = min(minY, coords[i].y); maxY = max(maxY, coords[i].y);
  }

  R w = max(maxX - minX, maxY - minY);
  R scale = 1 / w * 9 / 10;
  P padding = P(-minX * scale + R(1, 20) + (w - (maxX - minX)) / 2,
              -minY * scale + R(1, 20) + (w - (maxY - minY)) / 2);

  Draw(padding, scale,
       ofRectangle(ofPoint(0, 0), ofPoint(display_size, display_size)));


  //
  // Magnifier
  //
  {
    int center_i = path.empty() ? candidate : path.back();
    P center_p = coords[center_i];

    R nearest_d = 1;
    for (const auto &e : adj_rational[center_i]) nearest_d = min(nearest_d, e.second);

    R scale2 = 1 / nearest_d * 1 / 5;
    P padding2 = -1 * center_p * scale2 + P(R(1, 2), R(1, 2));

    Draw(P(1, 0) + padding2, scale2,
         ofRectangle(ofPoint(display_size, 0), ofPoint(display_size * 2, display_size)));
  }



  /*
  for (int y = 0; y <= N; ++y) ofDrawLine(0, y * scale, N * scale, y * scale);
  for (int x = 0; x <= N; ++x) ofDrawLine(x * scale, 0, x * scale, N * scale);

  for (int y = 0; y < N; ++y) {
    for (int x = 0; x < N; ++x) {
      ofSetColor(ofColor::fromHsb(int((100 - A[y][x]) * 1.7), 255, 255));
      ofDrawRectangle(x * scale, y * scale, scale, scale);
      ofSetColor(0, 0, 0);
      font.drawString(to_string(A[y][x]), x * scale, (y + 1) * scale);
    }
  }

  gui.draw();
  */
}

void ofApp::windowResized(int w, int h){
  display_size = min(w / 2, h);
  //  scale = min((int)((w - gui.getWidth()) / N), h / N);
  // gui.setPosition(N * scale, 0);
  font.load("verdana.ttf", display_size / 2, true, true);
}

void ofApp::keyPressed(int key) {
  // Starting point selection mode
  switch (key) {
    case OF_KEY_RIGHT:
      if (path.empty()) {
        candidate = NextCandidate(path, candidate);
      } else {
        candidate = NextCandidate(path, candidate);
      }
      break;

    case OF_KEY_LEFT:
      if (path.empty()) {
        candidate = PrevCandidate(path, candidate);
      } else {
        candidate = PrevCandidate(path, candidate);
      }
      break;

    case ' ':
      path.emplace_back(candidate);
      cout << PathSummary(path) << endl;

      candidate = NextCandidate(path, -1);
      break;

    case OF_KEY_RETURN:
      ext_event::PushSaveHintAndExecuteSolver();
      return;

    case OF_KEY_BACKSPACE:
      if (!path.empty()) {
        candidate = path.back();
        path.pop_back();

        cout << PathSummary(path) << endl;
      }
      break;

    default:
      return;
  }

  {
    vector<int> tmp(path);
    tmp.emplace_back(candidate);
    cerr << "Candidate: " << candidate << endl;
    PrintPathDistance(tmp, cerr);
  }
}

void ofApp::keyReleased(int key){}
void ofApp::mouseMoved(int x, int y){}
void ofApp::mouseDragged(int x, int y, int button){}
void ofApp::mousePressed(int x, int y, int button){}
void ofApp::mouseReleased(int x, int y, int button){}
void ofApp::mouseEntered(int x, int y){}
void ofApp::mouseExited(int x, int y){}
void ofApp::gotMessage(ofMessage msg){}
void ofApp::dragEvent(ofDragInfo dragInfo){}
