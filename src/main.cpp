#include "ofMain.h"
#include "BasicSkyReplacement.h"

//========================================================================
int main( ){
    ofAppGlutWindow window;
    window.setGlutDisplayString("rgb double depth>=16 alpha samples<=16h");
    
    ofSetupOpenGL(&window, 720, 480, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new BasicSkyReplacement());

}
