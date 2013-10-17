#include "testApp.h"



//--------------------------------------------------------------
void testApp::setup(){
	BezierSurface.setup();
	fullscreenStsatus = 0;
	
}

//--------------------------------------------------------------
void testApp::update(){
	BezierSurface.update();

}

//--------------------------------------------------------------
void testApp::draw(){
	BezierSurface.draw();	
	
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	
	BezierSurface.keyPressed(key);
	if(key == 'f' && fullscreenStsatus == 0) {
		fullscreenStsatus = 1;
		ofToggleFullscreen();
	}
	
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	
	fullscreenStsatus = 0;
	if(key == 'f') {
		BezierSurface.defaults();
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	BezierSurface.mouseMoved(x);
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	BezierSurface.mouseDragged(x, y);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	BezierSurface.mousePressed(x, y, button);
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	BezierSurface.mouseReleased();
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}