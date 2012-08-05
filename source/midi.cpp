#include <nds.h>
#include <libdsmi.h>

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "midi.h"

extern Input*	input;
extern Display* display;

u16 Midi::msTicks = 0;

// Init---------------------------------------------------------
Midi::Midi(){
	channel = CHANNEL_1;

	frame			= 0;
	tempoLed		= 0;

	beats			= 1;
	bars			= 1;
	quarters		= 1;
}
//--------------------------------------------------------------


// Connection to the server-------------------------------------
void	Midi::connect(){
	Cfg::midiConnected = dsmi_connect();
	oamClear(&oamMain, 0, 1);
}
//--------------------------------------------------------------


// Get/Set CC value---------------------------------------------
u8 Midi::getCcValue(CCParam ccParam){
	return getCcValue(ccParam.channel, ccParam.cc);
}
u8 Midi::getCcValue(u8 channel, u8 cc){
	return (Cfg::ccValues[channel][cc]);
}

void Midi::setCcValue(CCParam ccParam, u8 value){
	setCcValue(ccParam.channel, ccParam.cc, value);
}

void Midi::setCcValue(u8 channel, u8 cc, u8 value){
	Cfg::ccValues[channel][cc] = value;
}

u8	Midi::calculateCcValue(CCParam ccParam, u16 val, u16 val_max, bool invert){
	u8 range = ccParam.max - ccParam.min;
	if (invert)
		return ccParam.min + (range - (val * range) / val_max);
	return ccParam.min + ((val * range) / val_max);
}

void Midi::sendCcValues(){
	for (u8 channel = 0; channel < 16; ++channel){
		for (u8 cc = 0; cc < 128; ++cc){
			if (Cfg::ccChangedMblank[channel][cc]){
				dsmi_write(MIDI_CC|channel, cc, getCcValue(channel, cc));
				if (Cfg::debug && Cfg::debugOut){
					consoleSelect(&display->subConsole);
					iprintf("\e[42mSENT \e[43mCH \e[47m%2d \e[43mCC \e[47m%3d \e[43mVAL \e[47m%3d\n", channel+1, cc, getCcValue(channel, cc));
					consoleSelect(&display->mainConsole);
				}
				Cfg::ccChangedMblank[channel][cc] = false;
			}
		}
	}
}
//--------------------------------------------------------------


// Notes functions----------------------------------------------
u8 Midi::getNotePressed(u16 x, u16 y){
	return Cfg::pads.baseNote + (x / 64) + 4*((192 - y)/64);
}

u8 Midi::getNoteVelocity(u16 x, u16 y){
	u8 distance;
	x = (x%64);
	x = (x < 32 ? 32 - x : x - 32);
	y = (y%64);
	y = (y < 32 ? 32 - y : y - 32);
	distance = 42 - sqrt32(x*x + y*y);
	return (46 - ((46 - distance)*(46 - distance))/46) * 127 / 46;
}

u8 Midi::getNote(u8 note_number, u8 note_channel = -1){
	if (note_channel == -1)
		note_channel = channel;
	return (Cfg::notesStatus[note_channel][note_number]);
}

char* Midi::getNoteName(u8 note_number){
	char*		ret = new char[3];
	s8 octave	= (note_number + 3)/12 - 2;
	u8 note		= note_number%12;

	switch(note){
		case  0: ret[0] = 'C'; break;
		case  1: ret[0] = '!'; break;
		case  2: ret[0] = 'D'; break;
		case  3: ret[0] = '"'; break;
		case  4: ret[0] = 'E'; break;
		case  5: ret[0] = 'F'; break;
		case  6: ret[0] = '$'; break;
		case  7: ret[0] = 'G'; break;
		case  8: ret[0] = '%'; break;
		case  9: ret[0] = 'A'; break;
		case 10: ret[0] = '&'; break;
		case 11: ret[0] = 'B'; break;
	}
	switch(octave){
		case -2: ret[1] ='+'; break;
		case -1: ret[1] ='*'; break;
		case  0: ret[1] ='0'; break;
		case  1: ret[1] ='1'; break;
		case  2: ret[1] ='2'; break;
		case  3: ret[1] ='3'; break;
		case  4: ret[1] ='4'; break;
		case  5: ret[1] ='5'; break;
		case  6: ret[1] ='6'; break;
		case  7: ret[1] ='7'; break;
		case  8: ret[1] ='8'; break;
	}
	ret[2] = '\0';
	return ret;
}
//--------------------------------------------------------------


// Midi send note/cc--------------------------------------------
void Midi::sendCC(CCParam ccParam, u8 value){
	sendCC(ccParam.channel, ccParam.cc, value);
}
void Midi::sendCC(u8 channel, u8 cc, u8 value){
	if (getCcValue(channel, cc) != value){
		Cfg::ccChangedMblank[channel][cc]	= true;
		Cfg::ccChangedVblank[channel][cc]	= true;
		setCcValue(channel, cc, value);
	}
}

void Midi::sendNoteOn(u8 note_number, u8 note_velocity, u8 note_channel){
	releaseNote = false;
	if (Cfg::notesStatus[note_channel][note_number] == 0){
		if (!Cfg::pads.repeat)
			dsmi_write(NOTE_ON|note_channel, note_number, note_velocity);
		else
			Cfg::notesRepeat[note_channel][note_number] = note_velocity;
		if (Cfg::debug && Cfg::debugOut){
			consoleSelect(&display->subConsole);
			iprintf("\e[42mSENT \e[43mNOTE ON  \e[47m%3d \e[43mCH \e[47m%2d \e[43mVEL \e[47m%3d\n", note_number, note_channel+1, note_velocity);
			consoleSelect(&display->mainConsole);
		}
		Cfg::notesStatus[note_channel][note_number] = 1;

	} else if (Cfg::pads.hold){
		releaseNote = true;
	}
}

void Midi::sendNoteOff(u8 note_number, u8 note_channel){
	if (Cfg::notesStatus[note_channel][note_number] == 1 && (!Cfg::pads.hold || (Cfg::pads.hold && releaseNote))){
		dsmi_write(NOTE_OFF|note_channel, note_number, 0);
		if (Cfg::debug && Cfg::debugOut){
			consoleSelect(&display->subConsole);
			iprintf("\e[42mSENT \e[43mNOTE OFF \e[47m%3d \e[43mCH \e[47m%2d\n", note_number, note_channel+1);
			consoleSelect(&display->mainConsole);
		}
		Cfg::notesStatus[note_channel][note_number] = 0;
		if (Cfg::pads.repeat)
			Cfg::notesRepeat[note_channel][note_number] = 128;
	}
}
//--------------------------------------------------------------

// Treatement of received Midi messages-------------------------
void Midi::receiveMidi(){
	u8 message, number, value;
	u16 midiPointer;
	if(Midi::msTicks%3 == 0 && dsmi_read(&message, &number, &value)){
		if (Cfg::midiInClock){
			if (message == SYNC_START){
				if (Cfg::debug && Cfg::debugIn){
					consoleSelect(&display->subConsole);
					iprintf("\e[41mRECEIVED \e[43mSYNC \e[47mSTART\n");
					consoleSelect(&display->mainConsole);
				}
				Midi::msTicks	= 0;
				frame			= 0;
				beats			= 1;
				bars			= 1;
				quarters		= 1;
			}
			if (message == SYNC_STOP){
				if (Cfg::debug && Cfg::debugIn){
					consoleSelect(&display->subConsole);
					iprintf("\e[41mRECEIVED \e[43mSYNC \e[47mSTOP\n");
					consoleSelect(&display->mainConsole);
				}
				Midi::msTicks	= 0;
				frame			= 0;
				beats			= 1;
				bars			= 1;
				quarters		= 1;
				display->greyTempoLeds();
			}
			if (message == SYNC_POINTER){
				if (Cfg::debug && Cfg::debugIn){
					consoleSelect(&display->subConsole);
					iprintf("\e[41mRECEIVED \e[43mSYNC \e[47mPOINTER\n");
					consoleSelect(&display->mainConsole);
				}
				midiPointer = 128*value + number;

				Midi::msTicks	= 0;
				frame			= midiPointer * 6;
				bars			= (midiPointer/16) + 1;
				beats			= (midiPointer/4)%4 + 1;
				quarters		= midiPointer%4 + 1;
			}
			if (message == SYNC_CLOCK){
				if (Cfg::debug && Cfg::debugIn && Cfg::debugClock){
					consoleSelect(&display->subConsole);
					iprintf("\e[41mRECEIVED \e[43mSYNC \e[47mCLOCK\n");
					consoleSelect(&display->mainConsole);
				}
				if (++frame % 6 == 0){
					quarters		= quarters%4 + 1;
				}
				if (frame % 24 == 0){
					tempoLed		= 6;
					tempo			= (60 * 1024) / Midi::msTicks;
					beats			= beats%4 + 1;
					bars			= beats == 1 ? bars + 1 : bars;
					Midi::msTicks = 0;
				}

				if (frame % (96 / Cfg::pads.repeatRate) == 0){
					for (u8 c = 0; c < 16; ++c)
						for (u8 note = 0; note < 128; ++note)
							if (Cfg::notesStatus[c][note] == 1 && Cfg::notesRepeat[c][note] != 128)
								dsmi_write(NOTE_ON|c, note, Cfg::notesRepeat[c][note]);
				}			
				if (frame % (96 / (2 * Cfg::pads.repeatRate)) == 0 && frame % (24 / Cfg::pads.repeatRate) != 0){
					for (u8 c = 0; c < 16; ++c)
						for (u8 note = 0; note < 128; ++note)
							if (Cfg::notesStatus[c][note] == 1 && Cfg::notesRepeat[c][note] != 128)
								dsmi_write(NOTE_OFF|c, note, 0);
				}

			
				if (frame%6 == 0 && beats == 1 && quarters == 1)
					updateKaossRecs();
				if (Cfg::mode == KAOSS)
					recordKaossLoops(Cfg::kaoss.bank);
				playKaossLoops();
			} 
		}
		channel = message & CHANNEL_MASK;
		message = message & MESSAGE_MASK;

		if (Cfg::midiInMessage){
			if (message == MIDI_CC){
				if (Cfg::debug && Cfg::debugIn){
					consoleSelect(&display->subConsole);
					iprintf("\e[41mRECEIVED \e[43mCC \e[47m%3d \e[43mCH \e[47m%2d \e[43mVAL \e[47m%3d\n", number, channel+1, value);
					consoleSelect(&display->mainConsole);
				}
				setCcValue(channel, number, value);
				Cfg::ccChangedVblank[channel][number] = true;
			}
			if (message == NOTE_ON){
				if (Cfg::debug && Cfg::debugIn){
					consoleSelect(&display->subConsole);
					iprintf("\e[41mRECEIVED \e[43mNOTE ON  \e[47m%3d \e[43mCH \e[47m%2d \e[43mVEL \e[47m%3d\n", number, channel+1, value);
					consoleSelect(&display->mainConsole);
				}
				Cfg::notesStatus[channel][number] = 1;
				Cfg::pads.updated = true;
			}
			if (message == NOTE_OFF){
				if (Cfg::debug && Cfg::debugIn){
					consoleSelect(&display->subConsole);
					iprintf("\e[41mRECEIVED \e[43mNOTE OFF \e[47m%3d \e[43mCH \e[47m%2d\n", number, channel+1);
					consoleSelect(&display->mainConsole);
				}
				Cfg::pads.updated = true;
				Cfg::notesStatus[channel][number] = 0;
			}
		}
	}
}
//--------------------------------------------------------------

// Midi update function. Called every ms------------------------
void Midi::update(){
	if (input->updated && !input->paused){
		switch(Cfg::mode){
			case KAOSS:
				kaossUpdate(Cfg::kaoss.bank);
				break;
			case PADS:
				padsUpdate();
				break;
			case MIXER:
				mixerUpdate();
				break;
			case SLIDERS:
				slidersUpdate(Cfg::sliders.bank);
				break;
		}
		input->updated = false;
	}
	if (Cfg::receiveMidi)
		receiveMidi();
	sendCcValues();
}
//--------------------------------------------------------------


// KAOSS--------------------------------------------------------
void Midi::kaossUpdate(u8 bank){
	CCParam	x = Cfg::kaoss.x[bank];
	CCParam	y = Cfg::kaoss.y[bank];
	CCParam	z = Cfg::kaoss.z[bank];

	if (input->touch->press() == HELD || input->touch->press() == DOWN){
		if (input->touch->press() == DOWN)
			sendCC(z, 127);
		sendCC(x, calculateCcValue(x, input->touch->pos.px, 256, false));
		sendCC(y, calculateCcValue(y, input->touch->pos.py, 192, true));
	}
	if (input->touch->press() == RELEASED)
		sendCC(z, 0);
}

void Midi::updateKaossRecs(){
	CCRec** x;
	CCRec** y;
	CCRec** z;

	for (u8 bank = 0; bank < 4; ++bank){
		x = Cfg::kaoss.xRec[bank];
		y = Cfg::kaoss.yRec[bank];
		z = Cfg::kaoss.zRec[bank];
		for (u8 loop = 0; loop < 4; ++loop){
			updateKaossRec(x, bank, loop);
			updateKaossRec(y, bank, loop);
			updateKaossRec(z, bank, loop);
		}
	}
}

void Midi::updateKaossRec(CCRec** cc, u8 bank, u8 loop){
	if (cc[loop]->waitForRecording){
		if (bank == Cfg::kaoss.bank && Cfg::mode == KAOSS){
			for (u8 i = 0; i < 4; ++i){
				cc[i]->recording		= false;
				cc[i]->playing			= false;
				cc[i]->waitForRecording	= false;
				cc[i]->waitForPlaying	= false;
			}
			cc[loop]->recording = true;
		} else {
			cc[loop]->recording = false;
		}
		cc[loop]->waitForRecording	= false;
		cc[loop]->waitForPlaying	= false;
	}
	if (cc[loop]->waitForPlaying){
		for (u8 i = 0; i < 4; ++i){
			cc[i]->playing	= false;
		}
		cc[loop]->start();
		cc[loop]->waitForPlaying = false;
	}
}

void Midi::recordKaossLoops(u8 bank){
	u8 value;
	for (u8 loop = 0; loop < 4; ++loop){
		if (Cfg::kaoss.xRec[bank][loop]->recording){
			if (Cfg::kaoss.xRec[bank][loop]->length < 96){
				value = getCcValue(Cfg::kaoss.x[bank]);
				Cfg::kaoss.xRec[bank][loop]->addValue(value);
			} else {
				Cfg::kaoss.xRec[bank][loop]->recording = false;
				Cfg::kaoss.xRec[bank][loop]->start();
			}
		}
		if (Cfg::kaoss.yRec[bank][loop]->recording){
			if (Cfg::kaoss.yRec[bank][loop]->length < 96){
				value = getCcValue(Cfg::kaoss.y[bank]);
				Cfg::kaoss.yRec[bank][loop]->addValue(value);
			} else {
				Cfg::kaoss.yRec[bank][loop]->recording = false;
				Cfg::kaoss.yRec[bank][loop]->start();
			}
		}
		if (Cfg::kaoss.zRec[bank][loop]->recording){
			if (Cfg::kaoss.zRec[bank][loop]->length < 96){
				value = getCcValue(Cfg::kaoss.z[bank]);
				Cfg::kaoss.zRec[bank][loop]->addValue(value);
			} else {
				Cfg::kaoss.zRec[bank][loop]->recording = false;
				Cfg::kaoss.zRec[bank][loop]->start();
			}
		}
	}
}

void Midi::playKaossLoops(){
	u8 value;
	bool kaossPressed;

	for (u8 bank = 0; bank < 4; ++bank){
		for (u8 loop = 0; loop < 4; ++loop){
			kaossPressed = Cfg::mode == KAOSS && Cfg::kaoss.bank == bank && (input->touch->press() == DOWN || input->touch->press() == HELD);
			if (Cfg::kaoss.xRec[bank][loop]->playing){
				value = Cfg::kaoss.xRec[bank][loop]->nextValue();
				if (!kaossPressed)
					sendCC(Cfg::kaoss.x[bank], value);
			}
			if (Cfg::kaoss.yRec[bank][loop]->playing){
				value = Cfg::kaoss.yRec[bank][loop]->nextValue();
				if (!kaossPressed)
					sendCC(Cfg::kaoss.y[bank], value);
			}
			if (Cfg::kaoss.zRec[bank][loop]->playing){
				value = Cfg::kaoss.zRec[bank][loop]->nextValue();
				if (!kaossPressed)
					sendCC(Cfg::kaoss.z[bank], value);
			}
		}
	}
}
//--------------------------------------------------------------


// PADS---------------------------------------------------------
void Midi::padsUpdate(){
	if (input->touch->press() == DOWN){
		Cfg::pads.updated = true;
		Cfg::pads.held = getNotePressed(input->touch->pos.px, input->touch->pos.py);
		sendNoteOn(Cfg::pads.held, getNoteVelocity(input->touch->pos.px, input->touch->pos.py),Cfg::pads.channel);
	}
	if (input->touch->press() == RELEASED){
		Cfg::pads.updated = true;
		sendNoteOff(Cfg::pads.held, Cfg::pads.channel);
		Cfg::pads.held = -1;
	}
}
//--------------------------------------------------------------

// MIXER--------------------------------------------------------
void Midi::mixerUpdate(){
	u8		value;
	CCParam mute;
	CCParam	mixer;

	if (input->touch->press() == DOWN && input->touch->pos.py > 162 && input->touch->pos.py < 193){
		mute = Cfg::mixer.mutes[input->touch->pos.px/32];
		value = getCcValue(mute.channel, mute.cc) == 0 ? 127 : 0;
		sendCC(mute, value);
	}
	if(input->touch->press() == DOWN || input->touch->press() == HELD){
		if (input->touch->press() == DOWN && input->touch->pos.py <= 162)
			Cfg::mixer.held = (input->touch->pos.px/32);
		if (Cfg::mixer.held != -1){
			mixer = Cfg::mixer.ccs[Cfg::mixer.held];
			value = input->touch->pos.py > 155 ? 0 : input->touch->pos.py < 8 ? 127 : (149 - (input->touch->pos.py - 7)) * 127/149;
			sendCC(mixer, value);
		}
	} else {
		Cfg::mixer.held = -1;
	}
}
//--------------------------------------------------------------

// SLIDERS------------------------------------------------------
void Midi::slidersUpdate(u8 bank){
	u8		value;
	CCParam	slider;

	if(input->touch->press() == DOWN || input->touch->press() == HELD){
		if (input->touch->press() == DOWN)
			Cfg::sliders.held = (input->touch->pos.px/32 + 8*(input->touch->pos.py/96));
		slider = Cfg::sliders.ccs[bank][Cfg::sliders.held];
		value = getSliderCCVal(Cfg::sliders.held, input->touch->pos.py);
		sendCC(slider, value);
	} else {
		Cfg::sliders.held = -1;
	}
}

u8 Midi::getSliderCCVal(u8 slider, u16 y){
	if (slider < 8){
		if(y >= 89)
			return 0;
		else if (y <= 7)
			return 127;
		else
			return ((96 - y) - 7)*127/82;
	}
	else if (slider >= 8){
		if(y >= 185)
			return 0;
		else if (y <= 103)
			return 127;
		else
			return (96 - y%96 - 7)*127/82;
	}
	return 0;
}
//--------------------------------------------------------------