#include <nds.h>
#include "input.h"

extern Display* display;

Input::Input(){
	touch	= new InputTouch(&keys_down, &keys_held, &keys_released);
	pad 	= new InputPad(&keys_down, &keys_held, &keys_released);
	paused	= false;
}

// Kaoss Loops handling----------------------------------------------
void Input::kaossLoopPress(u8 loop){	
	kaossLoopPressCc(Cfg::kaoss.xRec[Cfg::kaoss.bank], loop);
	kaossLoopPressCc(Cfg::kaoss.yRec[Cfg::kaoss.bank], loop);
	kaossLoopPressCc(Cfg::kaoss.zRec[Cfg::kaoss.bank], loop);
}
void Input::kaossLoopPressCc(CCRec** cc, u8 loop){
	if (Cfg::kaoss.record){
		// Preparing the loop for recording
		for (u8 i=0; i<4; ++i){
			cc[i]->waitForRecording = false;
		}
		cc[loop]->waitForRecording = true;
		display->orangeTempoLeds();
	}
	if (Cfg::kaoss.clear || Cfg::kaoss.record){
		// Clearing the loop if clear or record is on
		cc[loop]->clear();
	} else {
		// Play/Stop loop
		if (cc[loop]->playing)
			cc[loop]->playing = false;
		else
			cc[loop]->waitForPlaying = cc[loop]->length != 0;
	}
}

void Input::kaossSendX(){
	CCParam	x = Cfg::kaoss.x[Cfg::kaoss.bank];

	Cfg::ccChangedMblank[x.channel][x.cc] = true;
}

void Input::kaossSendY(){
	CCParam	y = Cfg::kaoss.y[Cfg::kaoss.bank];

	Cfg::ccChangedMblank[y.channel][y.cc] = true;
}


void Input::editKaossParams(){
	update();
	CCParam* ccParam;
	if (touch->press() == DOWN){
		u16 x = touch->pos.px;
		u16 y = touch->pos.py;
		if (x < 88 && x > 56)
			ccParam = &Cfg::kaoss.x[Cfg::kaoss.bank];
		else if (x < 152 && x > 120)
			ccParam = &Cfg::kaoss.y[Cfg::kaoss.bank];
		else if (x < 216 && x > 184)
			ccParam = &Cfg::kaoss.z[Cfg::kaoss.bank];
		else
			return;
		
		if (y < 80 && y > 72 && (*ccParam).cc < 127)
			++(*ccParam).cc;
		else if (y < 96 && y > 88 && (*ccParam).cc > 0)
			--(*ccParam).cc;
		else if (y < 120 && y > 112 && (*ccParam).channel < 15)
			++(*ccParam).channel;
		else if (y < 136 && y > 128 && (*ccParam).channel > 0)
			--(*ccParam).channel;
	}
}
//-------------------------------------------------------------------


// Buttons press handling--------------------------------------------
void Input::padUpdate(){
	switch(Cfg::mode){
		case KAOSS :
			// A and B : Record and Clear Loop----------------
			Cfg::kaoss.record	= pad->a() == DOWN || pad->a() == HELD;
			Cfg::kaoss.clear	= pad->b() == DOWN || pad->b() == HELD;
			//------------------------------------------------

			// X and Y : Send X and Y single midi CC----------
			if (pad->x() == DOWN) {
				kaossSendX();
			}
			if (pad->y() == DOWN) {
				kaossSendY();
			}
			//------------------------------------------------

			// D-Pad : Loops (play, record, clear)------------
			if (pad->down()  == DOWN){
				kaossLoopPress(0);
			}
			if (pad->up()	 == DOWN){
				kaossLoopPress(1);
			}
			if (pad->left()  == DOWN){
				kaossLoopPress(2);
			}
			if (pad->right() == DOWN){
				kaossLoopPress(3);
			}
			//------------------------------------------------


			// R and L : Kaoss Bank Up and Down---------------
			if (pad->r() == DOWN && Cfg::kaoss.bank < 3)
				Cfg::kaoss.bank = Cfg::kaoss.bank + 1;
			if (pad->l() == DOWN && Cfg::kaoss.bank != 0)
					Cfg::kaoss.bank = Cfg::kaoss.bank - 1;
			//------------------------------------------------
			break;

		case PADS :
			// Y, A, B, X : Shift, Hold, Note Repeat, Dble/Trple
			Cfg::pads.shift		= pad->y() == DOWN || pad->y() == HELD;
			Cfg::pads.hold		= (Cfg::pads.hold || (pad->a() == DOWN || pad->a() == HELD)) && !(pad->a() == RELEASED && !Cfg::pads.shift);
			if (pad->b() == DOWN)
				Cfg::pads.repeat = !Cfg::pads.repeat;
			if (pad->x() == DOWN){
				Cfg::pads.doubleTriple = (Cfg::pads.doubleTriple == 2 ? 3 : 2);
				switch(Cfg::pads.repeatRate){
					case QUARTER:
						Cfg::pads.repeatRate = SIXTH;
						break;
					case EIGTH:
						Cfg::pads.repeatRate = TWELVE;
						break;
					case SIXTEEN:
						Cfg::pads.repeatRate = TWENTYFOUR;
						break;
					case THIRTYTWO:
						Cfg::pads.repeatRate = FORTYEIGHT;
						break;
					case SIXTH:
						Cfg::pads.repeatRate = QUARTER;
						break;
					case TWELVE:
						Cfg::pads.repeatRate = EIGTH;
						break;
					case TWENTYFOUR:
						Cfg::pads.repeatRate = SIXTEEN;
						break;
					case FORTYEIGHT:
						Cfg::pads.repeatRate = THIRTYTWO;
						break;
					default:
						Cfg::pads.repeatRate = QUARTER;
						break;
				}
			}
			//------------------------------------------------


			// R and L : Channel Up and Down------------------
			if (pad->r() == DOWN && Cfg::pads.channel < 15){
				++Cfg::pads.channel;
				Cfg::pads.updated = true;
			}
			if (pad->l() == DOWN && Cfg::pads.channel != 0){
				--Cfg::pads.channel;
				Cfg::pads.updated = true;
			}
			//------------------------------------------------
			

			// UP and DOWN : Octave Up and Down---------------
			if (pad->up() == DOWN && Cfg::pads.baseNote < 116){
				if (Cfg::pads.shift){
					Cfg::pads.baseNote = (Cfg::pads.baseNote/12 + 1)*12;
					if (Cfg::pads.baseNote > 116)
						Cfg::pads.baseNote = 116;
				} else {
					Cfg::pads.baseNote += 4;
				}
				Cfg::pads.oct_updated = true;
				Cfg::pads.updated = true;
			}
			if (pad->down() == DOWN && Cfg::pads.baseNote >= 4){
				if (Cfg::pads.shift){
					Cfg::pads.baseNote = (Cfg::pads.baseNote/12 - (Cfg::pads.baseNote%12 == 0))*12;
				} else {
					Cfg::pads.baseNote -= 4;
				}
				Cfg::pads.oct_updated = true;
				Cfg::pads.updated = true;
			}
			//------------------------------------------------


			// LEFT and RIGHT : Note Repeat Rate + or - ------
			if (pad->left() == DOWN && Cfg::pads.repeatRate < Cfg::pads.doubleTriple * 16)
				Cfg::pads.repeatRate = Cfg::pads.repeatRate * 2;
			if (pad->right() == DOWN && Cfg::pads.repeatRate != Cfg::pads.doubleTriple * 2)
				Cfg::pads.repeatRate = Cfg::pads.repeatRate / 2;
			//------------------------------------------------
			break;

		case SLIDERS :
			// R and L : Slider Bank Up and Down--------------
			if (pad->r() == DOWN && Cfg::sliders.bank < 2)
				Cfg::sliders.bank = Cfg::sliders.bank + 1;
			if (pad->l() == DOWN && Cfg::sliders.bank != 0)
				Cfg::sliders.bank = Cfg::sliders.bank - 1;
			//------------------------------------------------
			break;

		case MIXER :
			break;
	}
}
//-------------------------------------------------------------------


// Pad update--------------------------------------------------------
void Input::update(){
	scanKeys();

	keys_down		= keysDown();
	keys_released	= (keys_held | keysHeld())^keysHeld();
	keys_held		= keysHeld();

	updated = true;
	touch->update();
	if (!paused)
		padUpdate();
}

u8 Input::modeSelektor(){
	update();
	Cfg::modeSelector = true;
	if (touch->press() == DOWN){
		Cfg::mode = 1 + ((touch->pos.px > 128) ? 1 : 0) + (2* ((touch->pos.py > 96) ? 1 : 0)); 
		return 1;
	}
	return 0;
}

void Input::editSliderParams(){	
	CCParam*	ccParam;
	u8*			color;

	update();
	if (touch->press() == DOWN){
		u16 x = touch->pos.px;
		u16 y = touch->pos.py;
		ccParam = &Cfg::sliders.ccs[Cfg::sliders.bank][(x/32) + 8*(y/96)];
		color = &Cfg::sliders.colors[Cfg::sliders.bank][(x/32) + 8*(y/96)];
		
		y = y%96;

		if (y < 16 && y > 8 && (*ccParam).cc < 127)
			++(*ccParam).cc;
		else if (y < 32 && y > 24 && (*ccParam).cc > 0)
			--(*ccParam).cc;
		else if (y < 48 && y > 40 && (*ccParam).channel < 15)
			++(*ccParam).channel;
		else if (y < 64 && y > 56 && (*ccParam).channel > 0)
			--(*ccParam).channel;
		else if (y < 104 && y > 72)
			*color = (*color + 1) % 6;
	}
};

void Input::editMixerParams(){
	CCParam*	ccParam;

	update();
	if (touch->press() == DOWN){
		u16 x = touch->pos.px;
		u16 y = touch->pos.py;

		if (y > 96)
			ccParam = &Cfg::mixer.mutes[(x/32)];
		else
			ccParam = &Cfg::mixer.ccs[(x/32)];

		y = y%96;

		if (y < 32 && y > 24 && (*ccParam).cc < 127)
			++(*ccParam).cc;
		else if (y < 48 && y > 40 && (*ccParam).cc > 0)
			--(*ccParam).cc;
		else if (y < 64 && y > 56 && (*ccParam).channel < 15)
			++(*ccParam).channel;
		else if (y < 80 && y > 72 && (*ccParam).channel > 0)
			--(*ccParam).channel;
	}
};


InputPad::InputPad(int* keys_down, int* keys_held, int* keys_released){
	this->keys_released = keys_released;
	this->keys_down = keys_down;
	this->keys_held = keys_held;
}

int InputPad::get_press(int key){return RELEASED*((*keys_released & key) > 0) + DOWN*((*keys_down & key) > 0) + HELD*(!((*keys_down & key) > 0) && ((*keys_held & key) > 0));}

int InputPad::a()	  {return get_press(KEY_A);}
int InputPad::b()	  {return get_press(KEY_B);}
int InputPad::x()	  {return get_press(KEY_X);}
int InputPad::y()	  {return get_press(KEY_Y);}
int InputPad::l()	  {return get_press(KEY_L);}
int InputPad::r()	  {return get_press(KEY_R);}
int InputPad::start() {return get_press(KEY_START);}
int InputPad::select(){return get_press(KEY_SELECT);}
int InputPad::up()	  {return get_press(KEY_UP);}
int InputPad::down()  {return get_press(KEY_DOWN);}
int InputPad::left()  {return get_press(KEY_LEFT);}
int InputPad::right() {return get_press(KEY_RIGHT);}
//-------------------------------------------------------------------


// Touchscreen update------------------------------------------------
InputTouch::InputTouch(int *keys_down, int *keys_held, int* keys_released){
	this->keys_down = keys_down;
	this->keys_held = keys_held;
	this->keys_released = keys_released;
}

int InputTouch::press(){return RELEASED*((*keys_released & KEY_TOUCH) > 0) +DOWN*((*keys_down & KEY_TOUCH) > 0) + HELD*(!((*keys_down & KEY_TOUCH) > 0) && ((*keys_held & KEY_TOUCH) > 0));}

void InputTouch::update(){
	if (press() != HELD && press() != DOWN){
		x_movement = 0;
		y_movement = 0;
	} else {
		if (press() != DOWN)
			previous_x = pos.px;
			previous_y = pos.py;
		touchRead(&pos);
		if (press() == HELD){
			x_movement = pos.px - previous_x;
			y_movement = pos.py - previous_y;
		}
	}
}
//-------------------------------------------------------------------