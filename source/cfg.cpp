#include <nds.h>
#include <stdio.h>

#include <nds.h>
#include <stdio.h>

#include "cfg.h"

CCRec::CCRec(bool sync){
	sync				= sync;
	waitForPlaying		= false;
	waitForRecording	= false;
	playing				= false;
	recording			= false;
	length				= 0;
}

void CCRec::addValue(u8 value){
	values.push_back(value);
	++length;
}

void CCRec::start(){
	playing = true;
	current = values.begin();
}

void CCRec::stop(){
	playing = false;
}

u8 CCRec::nextValue(){
	u8 ret = *current;
	++current;
	if (current == values.end())
		current = values.begin();
	return ret;
}

void CCRec::clear(){
	stop();
	values.clear();
	length  = 0;
	current = values.begin();
}

void Cfg::init(){
	// General------------------------------------------------
	Cfg::midiConnected			= 2;
	Cfg::wifi					= true;
	Cfg::receiveMidi			= true;
	Cfg::modeSelector			= false;
	Cfg::editParams				= false;
	Cfg::debug					= false;
	Cfg::debugIn				= false;
	Cfg::debugOut				= false;
	Cfg::debugOut				= false;
	Cfg::saveCcs				= true;
	Cfg::midiInClock			= true;
	Cfg::midiInMessage			= true;

	Cfg::mode					= KAOSS;
	Cfg::previousMode			= NONE;	
	
	Cfg::notesStatus				= new u8*[16];
	Cfg::notesRepeat				= new u16*[16];
	Cfg::ccValues					= new u8*[16];
	Cfg::ccChangedVblank			= new bool*[16];
	Cfg::ccChangedMblank			= new bool*[16];

	for (u8 i = 0; i < 16; ++i){
		Cfg::notesStatus[i]			= new u8[128];
		Cfg::notesRepeat[i]			= new u16[128];
		Cfg::ccValues[i]			= new u8[128];
		Cfg::ccChangedVblank[i]		= new bool[128];
		Cfg::ccChangedMblank[i]		= new bool[128];
		for (u8 j = 0; j < 128; ++j){
			Cfg::notesStatus[i][j]		= 0;
			Cfg::notesRepeat[i][j]		= 128;
			Cfg::ccValues[i][j]			= 0;
			Cfg::ccChangedVblank[i][j]	= false;
			Cfg::ccChangedMblank[i][j]	= false;
		}
	}
	//--------------------------------------------------------


	// KAOSS Params-------------------------------------------
	Cfg::kaoss.x		= new CCParam[4];
	Cfg::kaoss.y		= new CCParam[4];
	Cfg::kaoss.z		= new CCParam[4];

	Cfg::kaoss.xRec		= new CCRec**[4];
	Cfg::kaoss.yRec		= new CCRec**[4];
	Cfg::kaoss.zRec		= new CCRec**[4];
	
	Cfg::kaoss.clear	= false;
	Cfg::kaoss.record	= false;

	for(u8 i = 0; i < 4; ++i){
		Cfg::kaoss.x[i].min = 0;
		Cfg::kaoss.x[i].max = 127;
		Cfg::kaoss.y[i].min = 0;
		Cfg::kaoss.y[i].max = 127;
		Cfg::kaoss.z[i].min = 0;
		Cfg::kaoss.z[i].max = 127;
		Cfg::kaoss.xRec[i] = new CCRec*[4];
		Cfg::kaoss.yRec[i] = new CCRec*[4];
		Cfg::kaoss.zRec[i] = new CCRec*[4];
		for (u8 j = 0; j < 4; ++j){
			Cfg::kaoss.xRec[i][j] = new CCRec(true);
			Cfg::kaoss.yRec[i][j] = new CCRec(true);
			Cfg::kaoss.zRec[i][j] = new CCRec(true);
		}
	}	
	Cfg::kaoss.bank				= 0;
	Cfg::kaoss.previousBank		= 0;
	//--------------------------------------------------------


	// MIXER Params-------------------------------------------
	Cfg::mixer.ccs		= new CCParam[8];
	Cfg::mixer.mutes	= new CCParam[8];
	Cfg::mixer.solos	= new CCParam[8];
	Cfg::mixer.buttons	= new CCParam[4];
	//--------------------------------------------------------


	// SLIDERS Params-----------------------------------------
	Cfg::sliders.ccs		= new CCParam*[3];
	Cfg::sliders.buttons	= new CCParam*[3];
	Cfg::sliders.colors		= new u8*[3];
	for (u8 i = 0; i < 3; ++i){
		Cfg::sliders.ccs[i]		= new CCParam[16];
		Cfg::sliders.buttons[i]	= new CCParam[4];
		Cfg::sliders.colors[i]	= new u8[16];
	}
	Cfg::sliders.held			= -1;
	Cfg::sliders.bank			= 0;
	Cfg::sliders.previousBank	= 0;
	//--------------------------------------------------------


	// PADS Params--------------------------------------------
	Cfg::pads.baseNote			= 36;
	Cfg::pads.channel			= 0;
	Cfg::pads.updated			= true;
	Cfg::pads.hold				= false;
	Cfg::pads.repeat			= false;
	Cfg::pads.repeatRate		= QUARTER;
	Cfg::pads.doubleTriple		= 2;
	//--------------------------------------------------------
}

u8						Cfg::midiConnected;
	
u8						Cfg::mode;
u8						Cfg::previousMode;

bool					Cfg::modeSelector;
bool					Cfg::editParams;
bool					Cfg::wifi;
bool					Cfg::receiveMidi;
bool					Cfg::debug;
bool					Cfg::debugIn;
bool					Cfg::debugOut;
bool					Cfg::debugClock;
bool					Cfg::saveCcs;
bool					Cfg::midiInClock;
bool					Cfg::midiInMessage;

struct KaossParams		Cfg::kaoss;
struct PadsParams		Cfg::pads;
struct MixerParams		Cfg::mixer;
struct SlidersParams	Cfg::sliders;

u8**	Cfg::ccValues;
u8**	Cfg::notesStatus;
u16**	Cfg::notesRepeat;
bool**	Cfg::ccChangedVblank;
bool**	Cfg::ccChangedMblank;

char*	Cfg::fileBuffer			= new char[2048];