/*
 *  sprites.h
 *  MyEmpty
 *
 *  Created by patrick on 11-12-27.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _BezierWarp_
#define _BezierWarp_

#include "ofMain.h"
#include "ofxQTKitVideoPlayer.h"
#include "ofxOsc.h"
#include "ofxFileDialogOSX.h"

#define PORT_RECEIVE 13000
#define NUM_MSG_STRINGS 20

class BezierWarp{
	
	public:
	// globales
	int mousePosX;
	int mousePosY;
	int nbSprites;
	int nbControlPoints;
	float x_pos[4];
	float y_pos[4];
	float x_ctrl[8];
	float y_ctrl[8];
	int selectedSprite[4];
	int selectedControlPoint[8];
	int anchorControl;
	int mouseON;
	int spritesON;
	int rad;
	int lastKey;
	int clef;
	bool keyBoardStatus, lastKeyBoardStatus, mouseStatus, lastMouseStatus;
	int width;
	int height;
	int gridRes;
	string URL;

	BezierWarp() {  }
	
	void setup();
	void update();
	void draw();
	void defaults();
	void save();
	void recall();
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y);
	void mouseReleased();
	void keyPressed(int clef);
	void mouseMoved(int x);
	void openFile();
	
	ofxQTKitVideoPlayer testMovie;
	ofTexture ourOwnTexture;
	int mode; //define a mode to interact with the movie
	
	ofxOscSender sender;
	ofxOscReceiver receiver;
	int				current_msg_string;
	string		msg_strings[NUM_MSG_STRINGS];
	float			timers[NUM_MSG_STRINGS];
	
	ofFbo fbomask, fbocalib;
	unsigned char 	* colorPixels;
	ofTexture		texColor;
	
	bool blendLeft, blendRight, blendUp, blendDown;
	float blendAmount;
	float blendGamma[3];
	float ccRed, ccGreen, ccBlue;
	bool maskChanged;
	
	
	int nbOverlapPixels;
	
	string output;
};


#endif

