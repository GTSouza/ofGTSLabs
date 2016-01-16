#include "LightGlow.h"

float rotation = 110.0f;
float _rotation = 0.0f;

//--------------------------------------------------------------
void LightGlow::setup(){
    
    stoneWall.load("LightGlow/Stone_Wall.jpg");
    stoneWall.setAnchorPercent(0.5f, 0.5f);
    
    plane.set(stoneWall.getWidth(), stoneWall.getHeight());
    
    fbo.allocate(desiredWidth, desiredHeight);
    
#ifdef TARGET_OPENGLES
    shaderBlurX.load("LightGlow/Shaders/ES2/blurX");
    shaderBlurY.load("LightGlow/Shaders/ES2/blurY");
#else
    if(ofIsGLProgrammableRenderer()){
        shaderBlurX.load("LightGlow/Shaders/GL3/blurX");
        shaderBlurY.load("LightGlow/Shaders/GL3/blurY");
    } else {
        shaderBlurX.load("LightGlow/Shaders/GL2/blurX");
        shaderBlurY.load("LightGlow/Shaders/GL2/blurY");
    }
#endif
    
    fboBlurOnePass.allocate(desiredWidth, desiredHeight);
    fboBlurTwoPass.allocate(desiredWidth, desiredHeight);
}

//--------------------------------------------------------------
void LightGlow::update(){
    
    float elapsedTime = ofGetElapsedTimef();
    
    fbo.begin();
    {
        ofEnableDepthTest();
     
        ofClear(0.0, 0.0, 0.0, 0.0);
        
        camera.begin();
        {
            
            ofPushMatrix();
            {
                ofPushStyle();
                {
                    ofRotateX(rotation);
                    
                    ofSetColor(0.0, 0.0, 0.0, 0.0);
                    plane.draw();
                    
                } ofPopStyle();
                
            } ofPopMatrix();
            
            float radius = 50.0f;
            
            float yPos = ofMap(cos(elapsedTime), 0.0f, 1.0f, 0.0f, (radius * 2.0f));
            
            ofSetColor(ofColor::magenta);
            ofDrawCircle(-(radius * 2.5f), yPos, radius);
            
            ofSetColor(ofColor::cyan);
            ofDrawCircle(0.0f, yPos, radius);
            
            ofSetColor(ofColor::greenYellow);
            ofDrawCircle((radius * 2.5f), yPos, radius);
            
        } camera.end();
        
        ofDisableDepthTest();
        
    } fbo.end();
    
}

//--------------------------------------------------------------
void LightGlow::draw(){
    
    float elapsedTime = ofGetElapsedTimef();
    
    ofSetColor(ofColor::white);
    ofBackground(ofColor::black);
    
    camera.begin();
    {
        
        ofPushMatrix();
        {
            ofRotateX(rotation);
            
            stoneWall.draw(0, 0);
            
        } ofPopMatrix();
        
    } camera.end();
    
    /* orbs */
    drawOrbs();
    
    ofSetColor(ofColor::white);
    ofDrawBitmapString(ofToString(ofGetFrameRate()), 20.0f, 20.0f);
    
}

//--------------------------------------------------------------
void LightGlow::drawOrbs(){
    
    float blur = 3.0f;
    
    //----------------------------------------------------------
    fboBlurOnePass.begin();
    {
        ofClear(0.0, 0.0, 0.0, 0.0);
        
        shaderBlurX.begin();
        shaderBlurX.setUniform1f("blurAmnt", blur);
        {
            /* orbs */
            ofSetColor(ofColor::white);
            fbo.draw(0.0f, 0.0f);
            
        } shaderBlurX.end();
        
    } fboBlurOnePass.end();
    
    //----------------------------------------------------------
    fboBlurTwoPass.begin();
    {
        ofClear(0.0, 0.0, 0.0, 0.0);
        
        shaderBlurY.begin();
        shaderBlurY.setUniform1f("blurAmnt", blur);
        {
            fboBlurOnePass.draw(0, 0);
            
        } shaderBlurY.end();
        
    } fboBlurTwoPass.end();
    
    //----------------------------------------------------------
    
    /* orbs */
    ofSetColor(ofColor::white);
    fboBlurTwoPass.draw(0, 0);
    
    /* orbs reflection - 10% opacity */
    ofSetColor(255.0f, 255.0f, 255.0f, (255.0f * 0.1f));
    fboBlurTwoPass.draw(0.0f, fbo.getHeight(), fbo.getWidth(), -fbo.getHeight());
    
    ofEnableAlphaBlending();
    
}

//--------------------------------------------------------------
void LightGlow::keyPressed(int key){
    if(key == 'a'){
        _rotation++;
    } else if(key == 'b'){
        _rotation--;
    }
}

//--------------------------------------------------------------
void LightGlow::keyReleased(int key){
    
}

//--------------------------------------------------------------
void LightGlow::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void LightGlow::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void LightGlow::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void LightGlow::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void LightGlow::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void LightGlow::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void LightGlow::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void LightGlow::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void LightGlow::dragEvent(ofDragInfo dragInfo){
    
}
