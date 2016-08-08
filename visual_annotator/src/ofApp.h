#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include <fstream>
using namespace std;

extern string solver_command;
extern string submit_command;
extern string output_directory;
extern string problem_id;

void SetInput(const string &txt);
// void set_solution(const string &txt);
// void set_solution_builtin(const string &name);

struct ext_event_handler;

class ofApp : public ofBaseApp{
 public:
  void setup();
  void update();
  void draw();

  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y);
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void mouseEntered(int x, int y);
  void mouseExited(int x, int y);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);

 private:
  friend ext_event_handler;

  ofTrueTypeFont font;
  ofxPanel gui;
  ofxFloatSlider step_slider;
  ofxFloatSlider speed_slider;
  ofxIntSlider history_slider;
};
