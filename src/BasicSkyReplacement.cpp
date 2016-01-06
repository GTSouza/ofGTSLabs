#include "BasicSkyReplacement.h"

/* fix aspect ratio */
float _w;
float _x;
float alphaColor = 255.0f;

ofLoopType videoLoopType = OF_LOOP_NORMAL;

//--------------------------------------------------------------
void BasicSkyReplacement::setup(){
    
    /* attempts to set the frame rate to a given target by sleeping a certain amount per frame */
    ofSetFrameRate(60.0);
    
    /* syncs the refresh-rate with your video-cards refresh-rate */
    ofSetVerticalSync(true);
    
    /* enables anti-aliasing (smoothing) for lines */
    // ofEnableAntiAliasing();
    
    /* turns on alpha blending */
    ofEnableAlphaBlending();
    
    /* texture to draw on FBO */
    skyBackgroundImage.load("BasicSkyReplacement/Sky_Background.jpg");
    
    /* video */
    videoPlayer.load("BasicSkyReplacement/Motorcycle_Footage.mov");
    videoPlayer.setLoopState(videoLoopType);
    
    /* FBO's */
    fbo.allocate(desiredWidth, desiredHeight);
    maskFbo.allocate(desiredWidth, desiredHeight);
    
    /* Let's clear the FBO's
    * otherwise it will bring some junk with it from the memory */
    maskFbo.begin();
    ofClear(0, 0, 0, 0);
    maskFbo.end();
    
    fbo.begin();
    ofClear(0, 0, 0, 0);
    fbo.end();
    
#ifdef TARGET_OPENGLES
    maskShader.load("BasicSkyReplacement/Shaders/GLES/alpha_mask");
    chromaKeyShader.load("BasicSkyReplacement/Shaders/GLES/chroma_key");
#else
    if(ofIsGLProgrammableRenderer()){
        maskShader.load("BasicSkyReplacement/Shaders/GLSL150/alpha_mask");
        chromaKeyShader.load("BasicSkyReplacement/Shaders/GLSL150/chroma_key");
    } else {
        maskShader.load("BasicSkyReplacement/Shaders/GLSL120/alpha_mask");
        chromaKeyShader.load("BasicSkyReplacement/Shaders/GLSL120/chroma_key");
    }
#endif
    
    /* set the texture parameters for the maks shader. just do this at the beginning */
    setupSkyLinearWipeMask();
    
    /* openCV Images */
    colorImg.allocate(desiredWidth, desiredHeight);
    grayImage.allocate(desiredWidth, desiredHeight);
    
    /* GUI Listener */
    linearWipe.addListener(this, &BasicSkyReplacement::linearWipeChanged);
    
    /* GUI Setup */
    gui.setup(); // most of the time you don't need a name

    gui.add(fps.setup("fps", ofToString(ofGetFrameRate())));
    gui.add(screenSize.setup("screen size", ofToString(ofGetWidth())+"x"+ofToString(ofGetHeight())));
    gui.add(colorKey.setup("color key", ofFloatColor(0.55, 0.6, 0.8), ofFloatColor(0, 0), ofFloatColor(1,1)));
    gui.add(colorKeyTolerance.setup("color key tolerance", 0.25f, 0.0f, 1.0f));
    gui.add(linearWipe.setup("linear wipe", 0.25f, 0.0f, 1.0f));
    gui.add(layer1.setup("video", true));
    gui.add(layer2.setup("sky", true));
    gui.add(layer3.setup("video chromakey", true));
    
    bHide = false;
}

//--------------------------------------------------------------
void BasicSkyReplacement::linearWipeChanged(float & linearWipe){
    setupSkyLinearWipeMask();
}

void BasicSkyReplacement::setupSkyLinearWipeMask(){
    
    /* MASK (frame buffer object) */
    maskFbo.begin();
    {
        ofClear(0, 0, 0, 0);
        
        glBegin(GL_QUAD_STRIP);
        
        glColor3f(1.0, 1.0, 1.0);
        glVertex3f( 0, 0, 0 );
        glVertex3f( fbo.getWidth(), 0, 0);
        
        glColor3f(linearWipe, linearWipe, linearWipe);
        glVertex3f( 0, fbo.getHeight(), 0);
        glVertex3f( fbo.getWidth(), fbo.getHeight(),0);
        
        glEnd();
        
    } maskFbo.end();
    
    /* HERE the shader-masking happends */
    fbo.begin();
    {
        ofClear(0, 0, 0, 0);
        
        maskShader.begin();
        maskShader.setUniformTexture("maskTex", maskFbo.getTexture(), 1 );
        {
            ofPushMatrix();
            {
                ofPushStyle();
                {
                    skyBackgroundImage.draw( 0, 0, fbo.getWidth(), fbo.getHeight() );
                } ofPopStyle();
                
            } ofPopMatrix();
            
        } maskShader.end();
    } fbo.end();
    
}

//--------------------------------------------------------------
void BasicSkyReplacement::update(){
    
    /* aspect ratio of video */
    _w = (ofGetHeight() * videoPlayer.getWidth()) / videoPlayer.getHeight();
    _x = (ofGetWidth() - _w) / 2.0f;
    
    bool bNewFrame = false;
    
    videoPlayer.update();
    bNewFrame = videoPlayer.isFrameNew();
    
    if (bNewFrame){
        float videoLength = videoPlayer.getDuration();
        float videoElapsedTime = videoPlayer.getPosition()*videoPlayer.getDuration();
        float videoTimeRemaining = videoLength - videoElapsedTime;
        float fadeTime = 1.0;
        
        alphaColor = 255.0f;
        
        /* fade IN/OUT if video play once */
        if(videoLoopType == OF_LOOP_NONE){
            if (videoElapsedTime < fadeTime) { /* fade in */
                alphaColor = 255.0f * videoElapsedTime / fadeTime;
            }
            else if (videoTimeRemaining < fadeTime) { /* fade out */
                alphaColor = 255.0f * videoTimeRemaining / fadeTime;
                
                /* perform the timmer when video reach the end */
                if (alphaColor <= 0.0f) {
                    videoPlayer.stop();
                    videoPlayer.setPosition(0.0f);
                }
            }
        }
        
        /* motion track*/
        colorImg.setFromPixels(videoPlayer.getPixels());
        grayImage = colorImg;
        
        if(subjectIsDefined)
        {
            // TODO: performance implementation
            // grayImage.setROI(subjectFrame);
            // ROI width; ROI height;
            IplImage * result = cvCreateImage(cvSize(desiredWidth - subjectImg.width + 1, desiredHeight - subjectImg.height + 1), 32, 1);
            
            cvMatchTemplate(grayImage.getCvImage(), subjectImg.getCvImage(), result, CV_TM_SQDIFF);
            // grayImage.resetROI();
            
            double minVal, maxVal;
            CvPoint minLoc, maxLoc;
            cvMinMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, 0);
            
            subjectLocation.x = minLoc.x;
            subjectLocation.y = minLoc.y;
            
            cvReleaseImage( & result );
        }
    }
    
}

//--------------------------------------------------------------
void BasicSkyReplacement::draw(){

    drawSkyReplacement();
    
    drawUI();
    
}

void BasicSkyReplacement::drawVideo(){
    
    /* draw */
    ofPushMatrix();
    {
        ofPushStyle();
        {
            /* draw black background */
            ofFill();
            ofSetColor(255.0f, 255.0f, 255.0f, alphaColor);
            ofDrawRectangle(ofRectangle(0.0f, 0.0f, ofGetWidth(), ofGetHeight()));
            
            /* draw current video frame */
            ofNoFill();
            ofSetColor(255.0f, 255.0f, 255.0f, alphaColor);
            videoPlayer.draw(_x, 0, _w, ofGetHeight());
            
        } ofPopStyle();
        
    } ofPopMatrix();
    
}

void BasicSkyReplacement::drawSkyReplacement(){
    
    float trackDeltaX = subjectInitialLocation.x - subjectLocation.x;
    float trackDeltaY = subjectInitialLocation.y - subjectLocation.y;
    
    /* prevent draw if fully transparenty */
    if(alphaColor <= 0.0){ return; }
    
    if(layer1){
        drawVideo();
    }
    
    /* draw sky with linear wipe */
    /* draw */
    if(layer2){
        ofPushMatrix();
        {
            ofPushStyle();
            {
                ofNoFill();
                ofSetColor(255.0f, 255.0f, 255.0f, alphaColor);
                fbo.draw(-trackDeltaX + _x - 100.0, -trackDeltaY + -100.0, _w + 200.0, ofGetHeight() + 200.0);
                
            } ofPopStyle();
            
        } ofPopMatrix();
    }
    
    /* draw video chromakey */
    if(layer3){
        chromaKeyShader.begin();
        chromaKeyShader.setUniform4f("keyColor", colorKey);
        chromaKeyShader.setUniform1f("tolerance", colorKeyTolerance);
        {
            /* draw */
            ofPushMatrix();
            {
                ofPushStyle();
                {
                    /* draw current video frame */
                    ofNoFill();
                    ofSetColor(255.0f, 255.0f, 255.0f, alphaColor);
                    videoPlayer.draw(_x, 0, _w, ofGetHeight());
                    
                } ofPopStyle();
                
            } ofPopMatrix();
            
        } chromaKeyShader.end();
    }
}

void BasicSkyReplacement::drawUI(){
    
    string info = "";
    info += "Press [space] to stop/play video\n";
    info += "Hold [c] and click to define a color key\n";
    info += "Hold [t] and click then drag to define object to track\n";
    
    ofPushMatrix();
    {
        ofPushStyle();
        {
            if(subjectIsDefined)
            {
                ofNoFill();
                ofSetHexColor(0xFFFFFF);
                ofDrawRectangle(subjectLocation.x, subjectLocation.y, subjectFrame.width, subjectFrame.height);
                
                info += "White outline is tracked object\n";
            }
            
            if(isSelectingTrackingRegion)
            {
                ofSetColor(255, 255, 255, 127);
                ofDrawRectangle(subjectFrame);
            }
            
            ofSetColor(ofColor::white);
            ofDrawBitmapString(info, ofVec2f(20, desiredHeight - 60));
            
            // auto draw?
            // should the gui control hiding?
            if(!bHide){
                fps = ofToString(ofGetFrameRate());
                gui.draw();
            }
            
        } ofPopStyle();
        
    } ofPopMatrix();
    
}

//--------------------------------------------------------------
void BasicSkyReplacement::keyPressed(int key){
    
    keyIsDown[key] = true;
    
    if(key == 'h'){
        bHide = !bHide;
    }
    else if(key == 's'){
        gui.saveToFile("settings.xml");
    }
    else if(key == 'l'){
        gui.loadFromFile("settings.xml");
    }
    else if(key == ' '){
        if(videoPlayer.isPlaying()){
            videoPlayer.setPosition(0.0);
            videoPlayer.stop();
        } else {
            videoPlayer.play();
        }
    }
    
}

//--------------------------------------------------------------
void BasicSkyReplacement::keyReleased(int key){
    keyIsDown[key] = false;
}

//--------------------------------------------------------------
void BasicSkyReplacement::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void BasicSkyReplacement::mouseDragged(int x, int y, int button){
    
    if (keyIsDown['c']){
        colorKey = ofFloatColor(colorImg.getPixels().getColor(x, y));
    } else if(isSelectingTrackingRegion) {
        subjectFrame.width = x - subjectFrame.x;
        subjectFrame.height = y - subjectFrame.y;
    }
    
}

//--------------------------------------------------------------
void BasicSkyReplacement::mousePressed(int x, int y, int button){
    
    if (keyIsDown['t']){
        subjectIsDefined = false;
        subjectFrame.x = x;
        subjectFrame.y = y;
        subjectFrame.width = 0;
        subjectFrame.height = 0;
        isSelectingTrackingRegion = true;
    }
    
}

//--------------------------------------------------------------
void BasicSkyReplacement::mouseReleased(int x, int y, int button){
    
    if (keyIsDown['c']){
        colorKey = ofFloatColor(colorImg.getPixels().getColor(x, y));
    } else if(subjectFrame.width != 0) {
        //End tracking and normalize subject frame
        if(subjectFrame.width < 0)
        {
            subjectFrame.x += subjectFrame.width;
            subjectFrame.width *= -1;
        }
        
        if(subjectFrame.height < 0)
        {
            subjectFrame.y += subjectFrame.height;
            subjectFrame.height *= -1;
        }
        isSelectingTrackingRegion = false;
        subjectInitialLocation.x = subjectLocation.x = subjectFrame.x;
        subjectInitialLocation.y = subjectLocation.y = subjectFrame.y;
        
        colorImg.setFromPixels(videoPlayer.getPixels());
        grayImage = colorImg;
        
        //Copy selected portion of the image to the subject image;
        subjectImg.allocate(subjectFrame.width, subjectFrame.height);
        grayImage.setROI(subjectFrame);
        subjectImg = grayImage;
        grayImage.resetROI();
        subjectIsDefined = true;
    } else {
        subjectIsDefined = false;
        subjectInitialLocation.x = subjectLocation.x = subjectFrame.x = 0;
        subjectInitialLocation.y = subjectLocation.y = subjectFrame.y = 0;
        subjectFrame.width = 0;
        subjectFrame.height = 0;
        isSelectingTrackingRegion = false;
    }
    
}

//--------------------------------------------------------------
void BasicSkyReplacement::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void BasicSkyReplacement::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void BasicSkyReplacement::windowResized(int w, int h){
    screenSize = ofToString(w) + "x" + ofToString(h);
}

//--------------------------------------------------------------
void BasicSkyReplacement::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void BasicSkyReplacement::dragEvent(ofDragInfo dragInfo){
    
}
