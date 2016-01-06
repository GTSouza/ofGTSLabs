#pragma once

#include "ofMain.h"

#include "ofxGui.h"
#include "ofxOpenCv.h"

const int desiredWidth = 720;
const int desiredHeight = 480;

class BasicSkyReplacement : public ofBaseApp {

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    ofImage skyBackgroundImage;
    ofVideoPlayer videoPlayer;
    
    ofShader maskShader;
    ofShader chromaKeyShader;
    
    ofFbo       maskFbo;
    ofFbo       fbo;
    
    void setupSkyLinearWipeMask();
    
    void drawVideo();
    void drawSkyReplacement();
    void drawUI();
    
    /* */
    ofxCvColorImage			colorImg;
    ofxCvGrayscaleImage 	grayImage;
    
    ofxCvGrayscaleImage subjectImg;
    ofRectangle subjectFrame;
    
    bool subjectIsDefined;
    bool isSelectingTrackingRegion;
    
    ofPoint subjectInitialLocation;
    ofPoint subjectLocation;
    
    /* */
    void linearWipeChanged(float & linearWipe);
    
    /* */
    bool bHide;
    
    /* */
    ofxLabel fps;
    ofxLabel screenSize;
    ofxFloatColorSlider colorKey;
    ofxFloatSlider colorKeyTolerance;

    ofxFloatSlider linearWipe;
    
    ofxToggle layer1;
    ofxToggle layer2;
    ofxToggle layer3;
    
    /* */
    ofxPanel gui;
    
    /* */
    bool keyIsDown[255];
		
};
