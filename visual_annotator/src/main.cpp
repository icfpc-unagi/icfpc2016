#include "ofMain.h"
#include "ofApp.h"
#include "common.h"

//========================================================================
int main(int argc, char **argv){
  CHECK(argc > 1);

  {
    ifstream ifs(argv[1]);
    CHECK_PERROR(ifs);
    SetInput(string((std::istreambuf_iterator<char>(ifs)),
                     std::istreambuf_iterator<char>()));

    {
      string path(argv[1]);
      path = path.substr(path.find_last_of('/') + 1);
      path = path.substr(0, path.find('.'));
      problem_id = path;
    }
  }
  if (argc > 2) {
    solver_command = argv[2];
  }
  if (argc > 3) {
    submit_command = argv[3];
  }

  {
    char dir[1024];
    getcwd(dir, 1024);
    output_directory = dir;
  }

  ofSetupOpenGL(1024, 768, OF_WINDOW);			// <-------- setup the GL context

  // this kicks off the running of my app
  // can be OF_WINDOW or OF_FULLSCREEN
  // pass in width and height too:
  ofRunApp(new ofApp());
}
