#include <nds.h>
#include <dswifi9.h>

#include <stdio.h>

#include "display.h"
#include "cfg.h"
#include "midi.h"
#include "input.h"
#include "save.h"

Display*	display;
Midi*		midi;
Input*		input;
Save*		save;

void pauseButtons(){
	if (input->pad->select() == DOWN)
		display->modeSelektor();
	if (input->pad->start() == DOWN)
		switch(Cfg::mode){
			case KAOSS:
				display->editKaossParams();
				break;
			case MIXER:
				display->editMixerParams();
				break;
			case SLIDERS:
				display->editSliderParams();
				break;
		}
	if (input->paused && (input->touch->press() == RELEASED || input->pad->select() == RELEASED || input->pad->start() == RELEASED))
		input->paused = false;
}

void nextframe(){
	swiWaitForVBlank();
}

void timerCallback(){
	/*if (Cfg::midiConnected == 2){
		display->pre_connect();
	}*/
	if (Cfg::midiConnected == 1){
		++Midi::msTicks;
		midi->update();
	}
}

int main(void){
	Cfg::init();
	input	=	new Input();
	midi	=	new Midi();
	display =	new Display();
	save	=	new Save();

	save->loadCcs();

	midi->connect();
	
	if (Cfg::midiConnected == 0){
		display->connection_error();
		while(1)
			nextframe();
	} else {
		timerStop(3);
		timerStart(3, ClockDivider_1024, TIMER_FREQ_1024(1000), timerCallback);
		display->connected();
		while(1){
			input->update();
			display->update();
			pauseButtons();
			nextframe();
		}
	}
	return (0);
}