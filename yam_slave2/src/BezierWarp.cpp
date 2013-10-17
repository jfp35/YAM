/*
 *  sprites.cpp
 *  MyEmpty
 *
 *  Created by patrick on 11-12-27.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "BezierWarp.h"
#include <iostream>

string ExtractFilename( const std::string& path )
{
	return path.substr( path.find_last_of( '\/' ) +1 );
}

void bezierDraw(ofTexture textur, float x_coordss[11][11], float y_coordss[11][11], int gridRes) {
	for(int i = 0; i < gridRes; i++) {
		for(int j = 0; j < gridRes; j++) {
			textur.bind();
			glBegin(GL_QUADS);   
			
			glTexCoord2f((i) * (textur.getWidth()/gridRes), (j) * (textur.getHeight()/gridRes));
			glVertex2f(x_coordss[i][j], y_coordss[i][j]);  
			
			glTexCoord2f((i+1) * (textur.getWidth()/gridRes), (j) * (textur.getHeight()/gridRes));  
			glVertex2f(x_coordss[i+1][j], y_coordss[i+1][j]);  
			
			glTexCoord2f((i+1) * (textur.getWidth()/gridRes), (j+1) * (textur.getHeight()/gridRes)); 
			glVertex2f(x_coordss[i+1][j+1], y_coordss[i+1][j+1]);  
			
			glTexCoord2f((i) * (textur.getWidth()/gridRes), (j+1) * (textur.getHeight()/gridRes)); 
			glVertex2f(x_coordss[i][j+1], y_coordss[i][j+1]);  
			
			glEnd();  
			textur.unbind();  			
		}
	}
}
	

float bezierPoint(float x0, float x1, float x2, float x3, float t) {
	float   ax, bx, cx;
	float   ay, by, cy;
	float   t2, t3;
	float   x;
	
	// polynomial coefficients
	cx = 3.0f * (x1 - x0);
	bx = 3.0f * (x2 - x1) - cx;
	ax = x3 - x0 - cx - bx;
	
	t2 = t * t;
	t3 = t2 * t;
	x = (ax * t3) + (bx * t2) + (cx * t) + x0;
	
	return x;
}


//--------------------------------------------------------------
void BezierWarp::openFile(){
	// first, create a string that will hold the URL
	URL;
	
	// openFile(string& URL) returns 1 if a file was picked
	// returns 0 when something went wrong or the user pressed 'cancel'
	int response = ofxFileDialogOSX::openFile(URL);
	if(response){
		// now you can use the URL 
		output = "URL to open: \n "+URL;
		testMovie.loadMovie(URL, mode);
		testMovie.pause();
	}else {
		output = "OPEN canceled. ";
	}
}

void BezierWarp::setup() {
	nbSprites = 4;
	nbControlPoints = 8;
	for(int i = 0; i < nbSprites; i++) {
		x_pos[i] = 0;
	}
	for(int i = 0; i < nbSprites; i++) {
		y_pos[i] = 0;
	}
	for(int i = 0; i < nbSprites; i++) {
		selectedSprite[i] = 0;
	}
	for(int i = 0; i < nbControlPoints; i++) {
		x_ctrl[i] = 0;
	}
	for(int i = 0; i < nbControlPoints; i++) {
		y_ctrl[i] = 0;
	}
	for(int i = 0; i < nbControlPoints; i++) {
		selectedControlPoint[i] = 0;
	}
	mouseON = 0;
	spritesON = 1;
	anchorControl = 0;
	rad = 15;
	lastKey  = 0;
	width = ofGetWidth();
	height = ofGetHeight();
	gridRes = 10;
	defaults();

	output ="";
	ofBackground(0,0,0);	
	ofSetVerticalSync(true);
	ofEnableAlphaBlending();

	mode = OFXQTVIDEOPLAYER_MODE_TEXTURE_ONLY;

	receiver.setup(PORT_RECEIVE);

	ofFbo::Settings settings;    
    settings.width = 1920;    
    settings.height = 1080;    
    settings.internalformat = GL_RGB;    
    settings.numSamples = 0;    
    settings.useDepth = false;    
    settings.useStencil = false;    
	fbomask.allocate(settings);
	
	ofFbo::Settings settings2;    
    settings2.width = 1920;    
    settings2.height = 1080;    
    settings2.internalformat = GL_RGBA;    
    settings2.numSamples = 0;    
    settings2.useDepth = false;    
    settings2.useStencil = false;    
	fbocalib.allocate(settings);
	
	fbocalib.begin();
	ofClear(0, 0, 0, 255);
	fbocalib.end();
	
	texColor.allocate(fbomask.getWidth(), fbomask.getHeight(), GL_RGB);

	// color pixels, use w and h to control red and green
	int w = fbomask.getWidth();
	int h = fbomask.getHeight();
	colorPixels	= new unsigned char [w*h*3];
	ccRed = 255;
	ccGreen = 255;
	ccBlue = 255;
	
	for (int i = 0; i < w; i++){
		for (int j = 0; j < h; j++){
			colorPixels[(j*w+i)*3 + 0] = ccRed;		// r
			colorPixels[(j*w+i)*3 + 1] = ccGreen;	// g
			colorPixels[(j*w+i)*3 + 2] = ccBlue;	// b
		}
	}
	maskChanged = true;
	blendAmount = 1;
		
	testMovie.loadMovie("calibrate.mov", mode);
	testMovie.pause();
}

void BezierWarp::update() {
	
	testMovie.update();
	
	if(maskChanged) {
		maskChanged = false;
		
		
		// color pixels, use w and h to control red and green
		int w = fbomask.getWidth();
		int h = fbomask.getHeight();
		
		for (int i = 0; i < w; i++){
			for (int j = 0; j < h; j++){
				colorPixels[(j*w+i)*3 + 0] = ccRed;	// r
				colorPixels[(j*w+i)*3 + 1] = ccGreen;	// g
				colorPixels[(j*w+i)*3 + 2] = ccBlue;	// b
			}
		}
		
		if(blendLeft) {
			//printf("marde");
			for (int i = 0; i < blendAmount; i++){
				for (int j = 0; j < h; j++){
					float base = float(i) / blendAmount;
					colorPixels[(j*w+i)*3 + 0] = pow(base, blendGamma[0]) * ccRed;	// r
					colorPixels[(j*w+i)*3 + 1] = pow(base, blendGamma[1]) * ccGreen;	// g
					colorPixels[(j*w+i)*3 + 2] = pow(base, blendGamma[2]) * ccBlue;	// b
				}
			}
		}
		if(blendRight) {
			for (int i = (w-blendAmount); i < w; i++){
				for (int j = 0; j < h; j++){
					float base = (w - float(i)-1) / blendAmount;
					colorPixels[(j*w+i)*3 + 0] = pow(base, blendGamma[0]) * ccRed;	// r
					colorPixels[(j*w+i)*3 + 1] = pow(base, blendGamma[1]) * ccGreen;	// g
					colorPixels[(j*w+i)*3 + 2] = pow(base, blendGamma[2]) * ccBlue;	// b
				}
			}
		}
		if(blendUp) {
			for(int i = 0; i < w; i++){
				for (int j = 0; j < blendAmount; j++){
					float base = float(j) / blendAmount;
					if(colorPixels[(j*w+i)*3 + 0] != ccRed) {
						colorPixels[(j*w+i)*3 + 0] *= pow(base, blendGamma[0]);	// r
						colorPixels[(j*w+i)*3 + 1] *= pow(base, blendGamma[1]);	// g
						colorPixels[(j*w+i)*3 + 2] *= pow(base, blendGamma[2]);	// b
					} else {
						colorPixels[(j*w+i)*3 + 0] = pow(base, blendGamma[0]) * ccRed;	// r
						colorPixels[(j*w+i)*3 + 1] = pow(base, blendGamma[1]) * ccGreen;	// g
						colorPixels[(j*w+i)*3 + 2] = pow(base, blendGamma[2]) * ccBlue;	// b
					}
				}
			}
		}
		if(blendDown) {
			for(int i = 0; i < w; i++){
				for (int j = h-blendAmount; j < h; j++){
					float base = (h - float(j)-1) / blendAmount;
					if(colorPixels[(j*w+i)*3 + 0] != ccRed) {
						colorPixels[(j*w+i)*3 + 0] *= pow(base, blendGamma[0]);	// r
						colorPixels[(j*w+i)*3 + 1] *= pow(base, blendGamma[1]);	// g
						colorPixels[(j*w+i)*3 + 2] *= pow(base, blendGamma[2]);	// b
					} else {
						colorPixels[(j*w+i)*3 + 0] = pow(base, blendGamma[0]) * ccRed;	// r
						colorPixels[(j*w+i)*3 + 1] = pow(base, blendGamma[1]) * ccGreen;	// g
						colorPixels[(j*w+i)*3 + 2] =pow(base, blendGamma[2]) * ccBlue;	// b
					}
				}
			}
		}
		texColor.loadData(colorPixels, w,h, GL_RGB);
		
		fbomask.begin();
		texColor.draw(0, 0, fbomask.getWidth(), fbomask.getHeight());
		fbomask.end();
		
	}
		
	//osc receiver update stuff
	// hide old messages
	for ( int i=0; i<NUM_MSG_STRINGS; i++ )
	{
		if ( timers[i] < ofGetElapsedTimef() )
			msg_strings[i] = "";
	}
	// check for waiting messages
	while( receiver.hasWaitingMessages() )
	{
		// get the next message
		ofxOscMessage m;
		receiver.getNextMessage( &m );
		
		// route les messages ici
		if ( m.getAddress() == "/speed" )
		{
			// both the arguments are int32's
			float speed = m.getArgAsFloat( 0 );
			testMovie.setSpeed(speed);
		}
		if ( m.getAddress() == "/play" )
		{
			// both the arguments are int32's
			int play = m.getArgAsInt32( 0 );
			if(play == 1) {	testMovie.play(); };
			if(play == 0) {	testMovie.pause(); };
		}
		if ( m.getAddress() == "/frame" )
		{
			// both the arguments are int32's
			int frame = m.getArgAsInt32( 0 );
			//testMovie.play();
			testMovie.setFrame(frame);
			//testMovie.pause();
		}
		
		if ( m.getAddress() == "/showControls" )
		{
			// both the arguments are int32's
			spritesON = m.getArgAsInt32( 0 );
			if (spritesON == 1) {
				ofShowCursor();
			} else {
				ofHideCursor();
			}

		}
		
		
		if ( m.getAddress() == "/blend" )
		{
			int lefty = m.getArgAsInt32( 0 );
			int uppy = m.getArgAsInt32( 1 );
			int righty = m.getArgAsInt32( 2 );
			int downy = m.getArgAsInt32( 3 );
			
			if(lefty == 0) {
				blendLeft = false;
			} else {
				blendLeft = true;
			}
			if(uppy == 0) {
				blendUp = false;
			} else {
				blendUp = true;
			}
			if(righty == 0) {
				blendRight = false;
			} else {
				blendRight = true;
			}
			if(downy == 0) {
				blendDown = false;
			} else {
				blendDown = true;
			}
			
			maskChanged = true;
		
		}
				
		if ( m.getAddress() == "/blendAmount" )	{
			// both the arguments are int32's
			blendAmount = m.getArgAsFloat( 0 );
			maskChanged = true;
			
			fbocalib.begin();
			ofClear(0, 0, 0, 255);
			
			ofPushStyle();
			ofSetColor(255, 255, 0);
			//ofSetLineWidth(2);
			ofLine(blendAmount, 0, blendAmount, fbocalib.getHeight());
			ofLine(fbocalib.getWidth() - blendAmount, 0, fbocalib.getWidth() - blendAmount, fbocalib.getHeight());
			ofLine(0, blendAmount, fbocalib.getWidth(), blendAmount);
			ofLine(0, fbocalib.getHeight() - blendAmount, fbocalib.getWidth(), fbocalib.getHeight() - blendAmount);
			ofPopStyle();
			
			ofPushStyle();
			ofNoFill();
			ofSetColor(255, 0, 0);
			ofCircle(fbocalib.getWidth() * 0.333333333, 0, 15);
			ofCircle(fbocalib.getWidth() * 0.666666666, 0, 15);
			ofCircle(fbocalib.getWidth() * 0.666666666, fbocalib.getHeight(), 15);
			ofCircle(fbocalib.getWidth() * 0.333333333, fbocalib.getHeight(), 15);
			ofCircle(0, fbocalib.getHeight() * 0.333333333, 15);
			ofCircle(0, fbocalib.getHeight() * 0.666666666, 15);
			ofCircle(fbocalib.getWidth(), fbocalib.getHeight() * 0.333333333, 15);
			ofCircle(fbocalib.getWidth(), fbocalib.getHeight() * 0.666666666, 15);
			ofPopStyle();
			
			ofPushStyle();
			ofNoFill();
			ofSetColor(0, 255, 0);
			ofCircle(fbocalib.getWidth() * 0.333333333, blendAmount, 20);
			ofCircle(fbocalib.getWidth() * 0.666666666, blendAmount, 20);
			ofCircle(fbocalib.getWidth() * 0.666666666, fbocalib.getHeight() - blendAmount, 20);
			ofCircle(fbocalib.getWidth() * 0.333333333, fbocalib.getHeight() - blendAmount, 20);
			ofCircle(blendAmount, fbocalib.getHeight() * 0.333333333, 20);
			ofCircle(blendAmount, fbocalib.getHeight() * 0.666666666, 20);
			ofCircle(fbocalib.getWidth() - blendAmount, fbocalib.getHeight() * 0.333333333, 20);
			ofCircle(fbocalib.getWidth() - blendAmount, fbocalib.getHeight() * 0.666666666, 20);
			ofPopStyle();
			fbocalib.end();
		}
		
		if ( m.getAddress() == "/gamma" )
		{
			// both the arguments are int32's
			blendGamma[0] = m.getArgAsFloat( 0 );
			blendGamma[1] = m.getArgAsFloat( 1 );
			blendGamma[2] = m.getArgAsFloat( 2 );
			maskChanged = true;
		}
		
		if ( m.getAddress() == "/ccR" )
		{
			// both the arguments are int32's
			ccRed = m.getArgAsFloat( 0 );
			maskChanged = true;
		}
		if ( m.getAddress() == "/ccG" )
		{
			// both the arguments are int32's
			ccGreen = m.getArgAsFloat( 0 );
			maskChanged = true;
		}
		if ( m.getAddress() == "/ccB" )
		{
			// both the arguments are int32's
			ccBlue = m.getArgAsFloat( 0 );
			maskChanged = true;
		}
		
		if ( m.getAddress() == "/open" ) {
			string path = m.getArgAsString(0);
			testMovie.closeMovie();
			testMovie.loadMovie(path, mode);
			testMovie.pause();
		}
		
		if ( m.getAddress() == "/volume" ) {
			float vol = m.getArgAsFloat( 0 );
			testMovie.setVolume(vol);
		}
		
		if ( m.getAddress() == "/default" ) {
			defaults();
		}
		if ( m.getAddress() == "/preset" ) {
			recall();
		}
		if ( m.getAddress() == "/fullscreen" ) {
			// both the arguments are int32's
			int booldeMarde = m.getArgAsInt32( 0 );
			if(booldeMarde == 1 && ofGetWindowMode() != OF_FULLSCREEN) {ofToggleFullscreen();};
			if(booldeMarde == 0 && ofGetWindowMode() == OF_FULLSCREEN) {ofToggleFullscreen();};
			
		}
		
		if ( m.getAddress() == "/loop" ) {
			int loop = m.getArgAsInt32( 0 );
			if(loop == 0) {
				testMovie.setLoopState(false);
			} else {
				testMovie.setLoopState(true);
			}
		}
		
		
	}
}
	
// draws the sprites
void BezierWarp::draw() {
    float x_coords[11][11];
	float y_coords[11][11];
	
	
	for(int i = 0; i <= gridRes; i++) {
		for(int j = 0; j <= gridRes; j++) {
			float start_x = bezierPoint(x_pos[0], x_ctrl[0], x_ctrl[7], x_pos[3], (float)j/gridRes);
			float end_x = bezierPoint(x_pos[1], x_ctrl[3], x_ctrl[4], x_pos[2], (float)j/gridRes);
			//printf("%f \n", start_x);
			float start_y = bezierPoint(y_pos[0], y_ctrl[0], y_ctrl[7], y_pos[3], (float)j/gridRes);
			float end_y = bezierPoint(y_pos[1], y_ctrl[3], y_ctrl[4], y_pos[2], (float)j/gridRes);
			
			float x = bezierPoint(start_x, ((x_ctrl[1] - x_ctrl[6]) * (1.0f - (float)j/gridRes)) + x_ctrl[6], ((x_ctrl[2] - x_ctrl[5]) * (1.0f - (float)j/gridRes)) + x_ctrl[5], end_x, (float)i/gridRes);
			float y = bezierPoint(start_y, ((y_ctrl[1] - y_ctrl[6]) * (1.0f - (float)j/gridRes)) + y_ctrl[6], ((y_ctrl[2] - y_ctrl[5]) * (1.0f - (float)j/gridRes)) + y_ctrl[5], end_y, (float)i/gridRes);
			
			x_coords[i][j] = x;
		 	y_coords[i][j] = y;
		} 
		//printf("%f ,%f    %f ,%f    %f ,%f    %f ,%f \n", x_coords[0][0], y_coords[0][0], x_coords[1][0], y_coords[1][0], x_coords[1][1], y_coords[1][1], x_coords[0][1], y_coords[0][1]);
	} 	
	
	
    testMovie.draw2(x_coords, y_coords, gridRes);

	glEnable(GL_BLEND);	
	if(spritesON == 1) {
		// additive blending
		glEnable (GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE); 
		bezierDraw(fbocalib.getTextureReference(), x_coords, y_coords, gridRes);
		// normal blending
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
		
	}

	//  multiply blending
	glBlendFunc(GL_ZERO,GL_SRC_COLOR);
	
	bezierDraw(fbomask.getTextureReference(), x_coords, y_coords, gridRes);
	glDisable(GL_BLEND);
	
	
	if(spritesON == 1) {
		ofEnableSmoothing();
		for(int i = 0; i < nbSprites; i++) {
			if(selectedSprite[i] == 1) {
				x_pos[i] = mousePosX; 
				y_pos[i] = mousePosY; 
				ofEnableAlphaBlending();
				ofPushStyle();
				ofSetColor(0, 255, 0, 100);
				ofFill();
				ofCircle(x_pos[i], y_pos[i], rad);
				ofDisableAlphaBlending();
				ofPopStyle();
				ofPushStyle();
				ofSetColor(255, 0, 0);
				ofNoFill();
				ofCircle(x_pos[i], y_pos[i], rad);
				ofLine(x_pos[i], y_pos[i] - (rad) - 5, x_pos[i], y_pos[i] + (rad) + 5);
				ofLine(x_pos[i] - (rad) - 5, y_pos[i], x_pos[i] + (rad) + 5, y_pos[i]);
				ofPopStyle();
			} else {
				ofPushStyle();
				ofSetColor(255, 0, 0);
				ofNoFill();
				ofCircle(x_pos[i], y_pos[i], rad);
				ofLine(x_pos[i], y_pos[i] - (rad) - 5, x_pos[i], y_pos[i] + (rad) + 5);
				ofLine(x_pos[i] - (rad) - 5, y_pos[i], x_pos[i] + (rad) + 5, y_pos[i]);
				ofPopStyle();
			}		      
		}
		for(int i = 0; i < nbControlPoints; i++) {		    
		    if(selectedControlPoint[i] == 1) {
				x_ctrl[i] = mousePosX; 
				y_ctrl[i] = mousePosY; 
				ofEnableAlphaBlending();
				ofPushStyle();
				ofSetColor(0, 255, 0, 100);
				ofFill();
				ofCircle(x_ctrl[i], y_ctrl[i], rad/2);
				ofPopStyle();
				ofDisableAlphaBlending();
				ofPushStyle();
				ofNoFill();
				ofSetColor(255, 255, 0);
				ofCircle(x_ctrl[i], y_ctrl[i], rad/2);
				if((i% 2) == 0) {
			    	ofBezier(x_pos[i/2], y_pos[i/2], x_ctrl[(i+1) % nbControlPoints], y_ctrl[(i+1) % nbControlPoints], x_ctrl[(i+2) % nbControlPoints], y_ctrl[(i+2) % nbControlPoints], x_pos[((i/2)+1) % nbSprites], y_pos[((i/2)+1) % nbSprites]); 
				}
				ofLine(x_pos[i/2], y_pos[i/2], x_ctrl[i], y_ctrl[i]);
				ofPopStyle();
		    } else {
				ofPushStyle();
				ofNoFill();
				ofSetColor(255, 255, 0);
				ofCircle(x_ctrl[i], y_ctrl[i], rad/2);
				if((i% 2) == 0) {
			    	ofBezier(x_pos[i/2], y_pos[i/2], x_ctrl[(i+1) % nbControlPoints], y_ctrl[(i+1) % nbControlPoints], x_ctrl[(i+2) % nbControlPoints], y_ctrl[(i+2) % nbControlPoints], x_pos[((i/2)+1) % nbSprites], y_pos[((i/2)+1) % nbSprites]); 
				}
				ofLine(x_pos[i/2], y_pos[i/2], x_ctrl[i], y_ctrl[i]);
				ofPopStyle();
			}
			

		}
		ofDisableSmoothing();
	}
		
}; 

//restore to defaults
void BezierWarp::defaults() {
	for(int i = 0; i < nbSprites; i++) {
		selectedSprite[i] = 0;
	}
	for(int i = 0; i < nbControlPoints; i++) {
		selectedControlPoint[i] = 0;
	}
	
	x_pos[0] = ofGetWidth() * 0.2f;
	x_pos[1] = ofGetWidth() * 0.8f;
	x_pos[2] = ofGetWidth() * 0.8f;
	x_pos[3] = ofGetWidth() * 0.2f;  
	
	y_pos[0] = ofGetHeight() * 0.2f;
	y_pos[1] = ofGetHeight() * 0.2f;
	y_pos[2] = ofGetHeight() * 0.8f;
	y_pos[3] = ofGetHeight() * 0.8f;
	
	x_ctrl[0] = ofGetWidth() * 0.2f;
	x_ctrl[1] = ofGetWidth() * 0.4f;
	x_ctrl[2] = ofGetWidth() * 0.6f;
	x_ctrl[3] = ofGetWidth() * 0.8f;
	x_ctrl[4] = ofGetWidth() * 0.8f;
	x_ctrl[5] = ofGetWidth() * 0.6f;
	x_ctrl[6] = ofGetWidth() * 0.4f;
	x_ctrl[7] = ofGetWidth() * 0.2f;
	
	y_ctrl[0] = ofGetHeight() * 0.4f;
	y_ctrl[1] = ofGetHeight() * 0.2f;
	y_ctrl[2] = ofGetHeight() * 0.2f;
	y_ctrl[3] = ofGetHeight() * 0.4f;
	y_ctrl[4] = ofGetHeight() * 0.6f;
	y_ctrl[5] = ofGetHeight() * 0.8f;
	y_ctrl[6] = ofGetHeight() * 0.8f;
	y_ctrl[7] = ofGetHeight() * 0.6f;
};

//saves sprites positions
void BezierWarp::save() {
		
	// open and write data to the file
	std::fstream ofs( "presets.bin", std::ios::out | std::ios::binary );
	for(int i = 0; i < 4; i++) {
		ofs.write( (const char*) &x_pos[i], sizeof(x_pos[i]) );
	}
	for(int i = 0; i < 4; i++) {
		ofs.write( (const char*) &y_pos[i], sizeof(y_pos[i]) );
	}
	for(int i = 0; i < 8; i++) {
		ofs.write( (const char*) &x_ctrl[i], sizeof(x_ctrl[i]) );
	}
	for(int i = 0; i < 8; i++) {
		ofs.write( (const char*) &y_ctrl[i], sizeof(y_ctrl[i]) );
	}
	ofs.close();
		
}

//reload last saved sprites positions
void BezierWarp::recall() {
	// re-open the file, but this time to read from it
	std::fstream ifs( "presets.bin", std::ios::in | std::ios::binary );
	for(int i = 0; i < 4; i++) {
		ifs.read( (char*) &x_pos[i], sizeof(x_pos[i]) );
	}
	for(int i = 0; i < 4; i++) {
		ifs.read( (char*) &y_pos[i], sizeof(y_pos[i]) );
	}
	for(int i = 0; i < 8; i++) {
		ifs.read( (char*) &x_ctrl[i], sizeof(x_ctrl[i]) );
	}
	for(int i = 0; i < 8; i++) {
		ifs.read( (char*) &y_ctrl[i], sizeof(y_ctrl[i]) );
	}
	ifs.close();
}


//handles mouse events (mouse button pressed
void BezierWarp::mousePressed(int x, int y, int button) {
	mousePosX = x;
	mousePosY = y;

	for(int i = 0; i < nbSprites; i++) {
		selectedSprite[i] = 0;
	}
	for(int i = 0; i < nbControlPoints; i++) {
		selectedControlPoint[i] = 0;
	}
	
	if(spritesON == 1) {
		for(int i = 0; i < nbSprites; i++) {
			if(selectedSprite[i] == 0 && anchorControl == 0) {
				
				if((mousePosX > x_pos[i] - (rad)) && (mousePosX < x_pos[i] + (rad)) && (mousePosY >  y_pos[i] - (rad)) && (mousePosY <  y_pos[i] + (rad))) {
					selectedSprite[i] = 1;
				} else {
					selectedSprite[i] = 0;
				}
				
			}
			
		}
		for(int i = 0; i < nbControlPoints; i++) {
			if(selectedControlPoint[i] == 0 && anchorControl == 1) {
				
				if((mousePosX > x_ctrl[i] - (rad/2)) && (mousePosX < x_ctrl[i] + (rad/2)) && (mousePosY >  y_ctrl[i] - (rad/2)) && (mousePosY <  y_ctrl[i] + (rad/2))) {
					selectedControlPoint[i] = 1;
				} else {
					selectedControlPoint[i] = 0;
				}
				
			}
			
		}
	}
};

//handles mouse events
void BezierWarp::mouseReleased() {
	mouseStatus = 0;
	lastMouseStatus = 0;
};

//handles mouse events
void BezierWarp::mouseDragged(int x, int y) {
	mousePosX = x;
	mousePosY = y;
};

//handles mouse events
void BezierWarp::mouseMoved(int x) {
};


//handles mouse events (mouse button pressed
void BezierWarp::keyPressed(int key) {
	switch(key) {
		case 99:
		switch(spritesON) {
			case 1:
			spritesON = 0;
			ofHideCursor();
			break;
			case 0:
			spritesON = 1;
				ofShowCursor();
			break;
		}
		break;
			
		case 'o':
			testMovie.closeMovie();
			openFile();
			break;
			
		case 100:
		defaults();
		break;
			
		case 112:
		recall();
		break;
			
		case 97:
		switch(anchorControl) {
			case 1:
				anchorControl = 0;
				break;
			case 0:
				anchorControl = 1;
				break;
		}
		break;
			
		case 115:
		save();
		break;
            
        case 32:
			if(testMovie.isPlaying()) {
				testMovie.pause();
			} else {
				testMovie.play();
			}
			break;
	}
};

