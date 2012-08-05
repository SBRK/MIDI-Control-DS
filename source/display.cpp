#include <nds.h>
#include <time.h>
#include <input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//Sprite headers
#include <kaossBrickSprite.h>
#include <padSprite.h>
#include <modeSprite.h>
#include <muteSprite.h>
#include <ledSprite.h>
#include <dotSprite.h>
#include <sliderCursorSprite.h>
#include <editParamsSprite.h>
#include <banksSprite.h>
#include <octaveSelectSprite.h>

//Background headers
#include <bgSplash.h>
#include <bgMain.h>
#include <kaossBgSub.h>
#include <mixerBgSub.h>
#include <padsBgMain.h>
#include <greyedBgSub.h>
#include <slidersBg2Sub.h>
#include <slidersBg3Sub.h>

//Font header
#include <font.h>

#include "display.h"

extern Input*	input;
extern Midi*	midi;
extern Save*	save;

Display::Display(){

// INIT
//-------------------------------------------------------------------------
	frame = 0;
	srand((int)NULL);

	// Main Init
	videoSetMode(MODE_5_2D | DISPLAY_BG_EXT_PALETTE);
	oamInit(&oamMain, SpriteMapping_1D_128, true);

	// Sub Init
	videoSetModeSub(MODE_5_2D | DISPLAY_BG_EXT_PALETTE);
	oamInit(&oamSub, SpriteMapping_1D_128, true);
//-------------------------------------------------------------------------
	

// FONT LOADING
//-------------------------------------------------------------------------
	ConsoleFont font;
	font.gfx				= (u16*)fontTiles;
	font.pal				= (u16*)fontPal;
	font.numChars			= 95;
	font.numColors			=  16;
	font.bpp				= 4;
	font.asciiOffset		= 32;
	font.convertSingleColor = true;
//-------------------------------------------------------------------------


// MAIN BACKGROUNDS
//-------------------------------------------------------------------------
	//BG Palettes
	vramSetBankE(VRAM_E_LCD);
	dmaCopy(fontPal,	VRAM_E_EXT_PALETTE[0], 512);
	dmaCopy(bgMainPal,	VRAM_E_EXT_PALETTE[2], 512);
	dmaCopy(padsBgMainPal,	VRAM_E_EXT_PALETTE[3], 512);
	vramSetBankE(VRAM_E_BG_EXT_PALETTE);

	//BG0 : Text
	mainbg0 = bgInit(0, BgType_Text4bpp, BgSize_T_256x256, 1, 0);
	consoleInit(&this->mainConsole, 0, BgType_Text4bpp, BgSize_T_256x256, 1, 0, true, false);
	consoleSetFont(&this->mainConsole, &font);
	bgSetPriority(this->mainbg0, 0);

	//BG2
	mainbg2 = bgInit(2, BgType_Rotation, BgSize_R_256x256, 7,	1);
	dmaCopy(bgMainTiles,	bgGetGfxPtr(mainbg2), bgMainTilesLen);
	dmaCopy(bgMainMap,		bgGetMapPtr(mainbg2), bgMainMapLen);
	bgSetPriority(this->mainbg2, 3);
	bgHide(mainbg2);

	//BG3
	mainbg3 = bgInit(3, BgType_Rotation, BgSize_R_256x256, 12,	2);
	dmaCopy(padsBgMainTiles,	bgGetGfxPtr(mainbg3), padsBgMainTilesLen);
	dmaCopy(padsBgMainMap,		bgGetMapPtr(mainbg3), padsBgMainMapLen);
	bgSetPriority(this->mainbg3, 2);
	bgHide(mainbg3);
//-------------------------------------------------------------------------
	

// MAIN SPRITES
//-------------------------------------------------------------------------
	//Palettes
	vramSetBankF(VRAM_F_LCD);
	dmaCopy(dotSpritePal,			VRAM_F_EXT_SPR_PALETTE[0], 512);
	dmaCopy(ledSpritePal,			VRAM_F_EXT_SPR_PALETTE[1], 512);
	dmaCopy(banksSpritePal,			VRAM_F_EXT_SPR_PALETTE[2], 512);
	dmaCopy(octaveSelectSpritePal,	VRAM_F_EXT_SPR_PALETTE[3], 512);
	vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
	
	//Sprites loading
	dotSprite			= new Sprite(1, dotSpriteTiles,				0, SpriteSize_64x32, 3);
	ledSprite			= new Sprite(1, ledSpriteTiles,				1, SpriteSize_16x16, 4);
	banksSprite			= new Sprite(1, banksSpriteTiles,			2, SpriteSize_16x16, 8);
	octaveSelectSprite	= new Sprite(1, octaveSelectSpriteTiles,	3, SpriteSize_32x16, 1);
//-------------------------------------------------------------------------


// SUB BACKGROUNDS
//-------------------------------------------------------------------------
	//BG Palettes
	vramSetBankH(VRAM_H_LCD);
	dmaCopy(fontPal, VRAM_H_EXT_PALETTE[0], 512);
	dmaCopy(slidersBg2SubPal, VRAM_H_EXT_PALETTE[2], 512);
	dmaCopy(slidersBg3SubPal, VRAM_H_EXT_PALETTE[3], 512);
	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE);

	//BG0 : Text
	subbg0 = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 4, 0);
	consoleInit(&subConsole, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, false, true);
	consoleSetFont(&subConsole, &font);
	bgSetPriority(subbg0, 1);

	//BG2
	subbg2 = bgInitSub(2, BgType_Rotation, BgSize_R_256x256, 5,1);
	bgSetPriority(subbg2, 3);

	//BG3
	subbg3 = bgInitSub(3, BgType_Rotation, BgSize_R_256x256, 6,2);
	bgSetPriority(subbg3, 2);
	bgHide(subbg3);
//-------------------------------------------------------------------------
	

// SUB SPRITES
//-------------------------------------------------------------------------
	//Palettes
	vramSetBankI(VRAM_I_LCD);
	dmaCopy(kaossBrickSpritePal,	VRAM_I_EXT_SPR_PALETTE[0], 512);
	dmaCopy(padSpritePal,			VRAM_I_EXT_SPR_PALETTE[1], 512);
	dmaCopy(modeSpritePal,			VRAM_I_EXT_SPR_PALETTE[2], 512);
	dmaCopy(sliderCursorSpritePal,	VRAM_I_EXT_SPR_PALETTE[3], 512);
	dmaCopy(muteSpritePal,			VRAM_I_EXT_SPR_PALETTE[4], 512);
	dmaCopy(editParamsSpritePal,	VRAM_I_EXT_SPR_PALETTE[5], 512);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);

	//Sprites loading	
	kaossBrickSprite	= new Sprite(0, kaossBrickSpriteTiles,		0, SpriteSize_32x32, 16);
	padSprite			= new Sprite(0, padSpriteTiles,				1, SpriteSize_64x64, 2);
	modesSprite			= new Sprite(0, modeSpriteTiles,			2, SpriteSize_64x64, 4);
	sliderCursorSprite	= new Sprite(0, sliderCursorSpriteTiles,	3, SpriteSize_32x16, 6);
	muteSprite			= new Sprite(0, muteSpriteTiles,			4, SpriteSize_32x32, 2);
	paramsEditSprite	= new Sprite(0, editParamsSpriteTiles,		5, SpriteSize_16x16, 8);
//-------------------------------------------------------------------------


// STRUCTURES INITIALISATION
//-------------------------------------------------------------------------
	kaossBricksState = new u8**[4];
	for (u8 b = 0; b < 4; ++b){
		kaossBricksState[b] = new u8*[8];
		for (u8 i = 0; i < 8; ++i){
			kaossBricksState[b][i] = new u8[8];
			for (u8 j = 0; j < 8; ++j)
				kaossBricksState[b][i][j] = 0;
		}
	}
	slidersBg2Map	= new unsigned short[768];
	slidersBg3Map	= new unsigned short[768];
//-------------------------------------------------------------------------
	consoleSelect(&mainConsole);
	consoleClear();

	dmaCopy(bgSplashTiles,	bgGetGfxPtr(subbg2), bgSplashTilesLen);
	dmaCopy(bgSplashMap,	bgGetMapPtr(subbg2), bgSplashMapLen);

	echo(0, GREY, 12, 10, "Connecting...");
	echo(0, WHITE, 23, 6, "midicontrolds.blogspot.com");
}

void Display::connected(){
	clearGfx();
	bgShow(mainbg2);
	greyTempoLeds();
}

void Display::pre_connect(){
	if (Midi::msTicks%500 == 0)
		dotSprite->display(0, 96,80, 3, ((Midi::msTicks%1500)/500)%3);
	oamUpdate(&oamMain);
}

void Display::connection_error(){
	oamClear(&oamMain, 0, 0);
	oamClear(&oamSub, 0, 0);
	echo(1, RED, 10,8, "Could not connect");
	echo(1, RED, 11,9, "to DSMI Server !");
}

// KAOSS Functions
//-------------------------------------------------------------------------
void Display::kaoss(){
	u16 i;

	clearGfx();

	dmaCopy(kaossBgSubTiles,	bgGetGfxPtr(subbg2), kaossBgSubTilesLen);
	dmaCopy(kaossBgSubMap,		bgGetMapPtr(subbg2), kaossBgSubMapLen);

	for (i = 0; i < 64; ++i)
		setKaossSpriteFrame(i, 0);
	
	bgHide(mainbg3);

	echo(1, LIGHT_GREY, 1, 2, "BANK:");
	save->writeCcs();
}

void Display::kaossUpdate(){
	u8		x, y;
	bool	z;
	u8		line, row;
	u8*		brick;
	u8		spriteId;
	bool	surroundingBrick;

	x = midi->getCcValue(Cfg::kaoss.x[Cfg::kaoss.bank]) / 16;
	y = 7 - (midi->getCcValue(Cfg::kaoss.y[Cfg::kaoss.bank]) / 16);
	z = input->touch->press() == HELD;//midi->getCcValue(Cfg::kaoss.z[Cfg::kaoss.bank]) >= 64;

	for (row = 0; row < 8; ++row)
		for (line = 0; line < 8; ++line){
			brick		= &kaossBricksState[Cfg::kaoss.bank][row][line];
			spriteId	= row + 8*line;
			surroundingBrick = (line == y && (row == x-1 || row == x+1)) || ( row == x && (line == y-1 || line == y+1));
			if ((!z && row == x && line == y) || (z && surroundingBrick && (frame % 3) == 0))
				*brick = *brick + 2*(*brick == 0) + (*brick != 0 && *brick < 3);
			else if (frame % 6 == 0)
				*brick = *brick - (*brick > 0);
			setKaossSpriteFrame(spriteId, *brick);
		}
	kaossButtonsStatus();
	showBank(Cfg::kaoss.bank, 4);
}

void Display::setKaossSpriteFrame(u8 sprite, u8 frame){
		kaossBrickSprite->display(sprite, 32 * (sprite % 8), 24 * (sprite / 8), 3, (Cfg::kaoss.bank*4) + frame);
}

void Display::kaossButtonsStatus(){
	consoleSelect(&mainConsole);
	echo(1, Cfg::kaoss.record ? RED : WHITE, 13, 24, "RECORD");
	echo(1, Cfg::kaoss.clear ? RED : WHITE, 17, 18, "CLEAR");
	echo(1, WHITE, 6, 3, "BANK-");
	echo(1, WHITE, 6, 19, "BANK;");
	echo(1, WHITE, 9, 4, "1");
	echo(1, WHITE, 13, 11, "2");
	echo(1, WHITE, 17, 7, "3");
	echo(1, WHITE, 13, 0, "4");
	ledSprite->display(17, 39, 132,	OBJPRIORITY_0, kaossGetLoopFrame(0));
	ledSprite->display(18, 40,  68, OBJPRIORITY_0, kaossGetLoopFrame(1));
	ledSprite->display(19,  7, 100, OBJPRIORITY_0, kaossGetLoopFrame(2));
	ledSprite->display(20, 72, 100, OBJPRIORITY_0, kaossGetLoopFrame(3));
}

u8 Display::kaossGetLoopFrame(u8 loop){
	return (2*Cfg::kaoss.xRec[Cfg::kaoss.bank][loop]->waitForRecording + 3*Cfg::kaoss.xRec[Cfg::kaoss.bank][loop]->playing + 1*Cfg::kaoss.xRec[Cfg::kaoss.bank][loop]->recording);
}

//-------------------------------------------------------------------------



// PADS Functions
//-------------------------------------------------------------------------
void Display::pads(){
	u8		noteStatus;
	char*	noteName;

	clearGfx();

	for (u8 i = 0; i < 12; ++i){
		noteName	= midi->getNoteName(i + Cfg::pads.baseNote);
		noteStatus	= Cfg::notesStatus[Cfg::pads.channel][i + Cfg::pads.baseNote];
		echo(0, 40, 22 - (i/4)*8, (i%4)*8 + 3, noteName);
		setPadSpriteFrame(i,noteStatus);
	}
	bgShow(mainbg3);
	echo(1, LIGHT_GREY, 1, 2, "CHANNEL:");
	save->writeCcs();
}

void Display::padsUpdate(){	
	u8		noteStatus;
	char*	noteName;

	if (Cfg::pads.updated){
		if (Cfg::pads.oct_updated){
			consoleSelect(&subConsole);
			consoleClear();
		}
		for (u8 i = 0; i < 12; ++i){
			noteStatus = Cfg::notesStatus[Cfg::pads.channel][i + Cfg::pads.baseNote];
			if (Cfg::pads.oct_updated){
				noteName = midi->getNoteName(i + Cfg::pads.baseNote);
				echo(0, 40, 22 - (i/4)*8, (i%4)*8+3, noteName);
			}
			setPadSpriteFrame(i,noteStatus);
		}
		Cfg::pads.updated		= false;
		Cfg::pads.oct_updated	= false;
	}

	octaveSelectSprite->display(25, 231, 144 - Cfg::pads.baseNote, OBJPRIORITY_0, 0);
	consoleSelect(&mainConsole);
	iprintf("\x1b[1;10H%2d", Cfg::pads.channel + 1);

	padsButtonsStatus();
}

void Display::padsButtonsStatus(){
	const char* rate;

	consoleSelect(&mainConsole);
	echo(1, Cfg::pads.hold ? RED : WHITE, 13, 24, "HOLD");
	echo(1, Cfg::pads.shift ? RED : WHITE, 13, 13, "SHFT");
	echo(1, WHITE, 9, 18, (Cfg::pads.doubleTriple == 2 ? "DOUBLE" : "TRIPLE"));
	echo(1, Cfg::pads.repeat ? RED : WHITE, 17, 13, "NOTE REPEAT");
	echo(1, Cfg::pads.shift ? RED : WHITE, 13, 9, "NR;");
	echo(1, Cfg::pads.shift ? RED : WHITE, 13, 0, "NR-");
	switch(Cfg::pads.repeatRate){
		case QUARTER:
			rate = "1/4 ";
			break;
		case EIGTH:
			rate = "1/8 ";
			break;
		case SIXTEEN:
			rate = "1/16";
			break;
		case THIRTYTWO:
			rate = "1/32";
			break;
		case SIXTH:
			rate = "1/6 ";
			break;
		case TWELVE:
			rate = "1/12";
			break;
		case TWENTYFOUR:
			rate = "1/24";
			break;
		case FORTYEIGHT:
			rate = "1/48";
			break;
		default:
			rate = "";
			break;
	}
	echo(1, Cfg::pads.repeat ? RED : WHITE, 17, 25, rate);
	echo(1, WHITE, 6, 2, "CHANNEL-");
	echo(1, WHITE, 6, 18, "CHANNEL;");
	echo(1, WHITE, 17, 1, Cfg::pads.shift ? "OCTAVE DOWN" : "NOTES DOWN ");
	echo(1, WHITE,  9, 2, Cfg::pads.shift ? "OCTAVE UP" : "NOTES UP ");
}

void Display::setPadSpriteFrame(u8 sprite, u8 frame){
	padSprite->display(sprite, 64 * (sprite % 4), 128 - 64 * (sprite / 4), OBJPRIORITY_3, frame);
}
//-------------------------------------------------------------------------


// MIXER Functions
//-------------------------------------------------------------------------
void Display::mixer(){
	clearGfx();

	DC_FlushAll();
	dmaCopy(mixerBgSubTiles,	bgGetGfxPtr(subbg2), mixerBgSubTilesLen);
	dmaCopy(mixerBgSubMap,		bgGetMapPtr(subbg2), mixerBgSubMapLen);
	
	for (u8 i = 0; i < 8; ++i)
		muteSprite->display(i, i*32 , 163, 1, 1);
	
	bgHide(mainbg3);
	save->writeCcs();
}

void Display::mixerUpdate(){
	CCParam mixer;
	CCParam mute;
	u8		mixerValue;
	u8		muteValue;

	for (u8 i = 0; i < 8; ++i){
		mixer	= Cfg::mixer.ccs[i];
		mute	= Cfg::mixer.mutes[i];

		mixerValue	= midi->getCcValue(mixer);
		muteValue	= midi->getCcValue(mute);

		muteSprite->display(i, i*32 , 163, OBJPRIORITY_2, muteValue == 127 ? 1 : 0);
		sliderCursorSprite->display(8+i, 32*i, 2 + (149 - mixerValue * 149/127), OBJPRIORITY_2, 0);
	}
}
//-------------------------------------------------------------------------


// SLIDERS Functions
//-------------------------------------------------------------------------
void Display::sliders(){
	clearGfx();

	for(u8 i = 0; i<16; ++i){
		this->updateSliderMap(i);
		this->updateSliderUpMap(i);
	}

	DC_FlushAll();	
	dmaCopy(slidersBg2SubTiles,		bgGetGfxPtr(subbg2), slidersBg2SubTilesLen);
	dmaCopy(slidersBg3SubTiles,		bgGetGfxPtr(subbg3), slidersBg3SubTilesLen);

	DC_FlushAll();
	dmaCopy(slidersBg2Map,		bgGetMapPtr(subbg2), 1536);
	dmaCopy(slidersBg3Map,		bgGetMapPtr(subbg3), 1536);
	
	echo(1, LIGHT_GREY, 1, 2, "BANK:");
	echo(1, WHITE, 6, 3, "BANK-");
	echo(1, WHITE, 6, 19, "BANK;");

	bgHide(mainbg3);
	save->writeCcs();
}

void Display::slidersUpdate(){
	bool change = false;
	if (Cfg::sliders.bank != Cfg::sliders.previousBank){
		sliders();
		Cfg::sliders.previousBank = Cfg::sliders.bank;
		change = true;
	}

	CCParam*	sliderCC;

	for (u8 s = 0; s < 16; ++s){
		sliderCC = &Cfg::sliders.ccs[Cfg::sliders.bank][s];
		if (Cfg::ccChangedVblank[(*sliderCC).channel][(*sliderCC).cc]){
			updateSliderUpMap(s);
			change = true;
		}
	}
	
	DC_FlushAll();
	dmaCopy(this->slidersBg2Map,			bgGetMapPtr(subbg2), 1536);
	if (change){
		DC_FlushAll();
		dmaCopy(this->slidersBg3Map,	bgGetMapPtr(subbg3), 1536);
	}
	showBank(Cfg::sliders.bank, 3);
}

void Display::updateSliderMap(u8 slider){
	u16	offset = (384 * (slider >= 8)) + (slider%8) * 4;
	u8 color = Cfg::sliders.colors[Cfg::sliders.bank][slider];

	for(u16 i = offset; i < offset + 4; ++i){
		slidersBg2Map[i]	= (color*9)		+ ((i%4 == 0) ? 1 : ((i%4 != 3) ? 2 : 3));
	}
	for(u8 j=0; j<10; ++j){
		offset += 32;
		for(u16 i = offset; i < offset + 4; ++i){
			slidersBg2Map[i]	= (color*9)		+ ((i%4 == 0) ? 4 : ((i%4 != 3) ? 5 : 6));
		}
	}
	for(u16 i = offset + 32; i < offset + 36; ++i){
		slidersBg2Map[i]	= (color*9)		+ ((i%4 == 0) ? 7 : ((i%4 != 3) ? 8 : 9));
	}
}

void Display::updateSliderUpMap(u8 slider){
	CCParam		ccParam = Cfg::sliders.ccs[Cfg::sliders.bank][slider];
	u16			offset = (384 * (slider >= 8)) + (slider%8) * 4;
	u8			color = Cfg::sliders.colors[Cfg::sliders.bank][slider];
	u8			val = (midi->getCcValue(ccParam)*82)/127;

	sliderCursorSprite->display(slider, 32*(slider%8), ((slider >= 8) ? 96 : 0) + 2 + (82 - val), OBJPRIORITY_2, color);

	for(u16 i = offset; i < offset + 4; ++i){
		slidersBg3Map[i] = (val > 85) ? ((color*24)	+ ((i%4 == 0) ? 0 : ((i%4 != 3) ? 8 : 16)) + (val > 90 ? 5 : val - 85)) : 0;
	}
	for(u8 j=0; j<10; ++j){
		offset += 32;
		for(u16 i = offset; i < offset + 4; ++i){
			slidersBg3Map[i] = (val > 5 + (72 - j*8)) ?	((color*24)	+ ((i%4 == 0) ? 0 : ((i%4 != 3) ? 8 : 16)) + (val > 5 + (80 - j*8) ? 8 : val - (5 + (72 - j*8)))) : 0;
		}
	}
	for(u16 i = offset + 32; i < offset + 36; ++i)
		slidersBg3Map[i]		= (val != 0) ?	144 + ((color*24) + ((i%4 == 0) ? 0 : ((i%4 != 3) ? 8 : 16)) + (val > 5 ? 5 : val)) : 0;
}
//-------------------------------------------------------------------------


// DISPLAY UPDATE FUNCTION
//-------------------------------------------------------------------------
void Display::update(){
	consoleSelect(&mainConsole);
	iprintf("\e[37m\x1b[1;20H%4d.%1d.%1d", midi->bars, midi->beats, midi->quarters);
	consoleSelect(&subConsole);

	tempoLeds();
	switch (Cfg::mode){
		case KAOSS:
			if (Cfg::previousMode != KAOSS){
				Cfg::previousMode = KAOSS;
				kaoss();
			}
			if (!input->paused)
				kaossUpdate();
			break;
		case PADS:
			if (Cfg::previousMode != PADS){
				Cfg::previousMode = PADS;
				pads();
			}
			padsUpdate();
			break;
		case MIXER:
			if (Cfg::previousMode != MIXER){
				Cfg::previousMode = MIXER;
				mixer();
			}
			mixerUpdate();
			break;
		case SLIDERS:
			if (Cfg::previousMode != SLIDERS){
				Cfg::previousMode = SLIDERS;
				sliders();
			}
			slidersUpdate();
			break;
	}

	for (u8 c = 0; c < 16; ++c){
		for (u8 v = 0; v < 128; ++v)
			Cfg::ccChangedVblank[c][v] = false;
	}
	oamUpdate(&oamSub);
	oamUpdate(&oamMain);
	++frame;
}
//-------------------------------------------------------------------------


void Display::tempoLeds(){
	u8 tempoLedFrame;
	u8 redLed = 4 * (midi->beats - 1) + (midi->quarters - 1);

	for (u8 i = 0; i < redLed; ++i)
		ledSprite->display(1 + i, 16 * i, 174, OBJPRIORITY_3, 0);
	ledSprite->display(1 + redLed, 16 * redLed, 174, OBJPRIORITY_3, 1);
	if (redLed == 0)
		ledSprite->display(16, 16 * 15, 174, OBJPRIORITY_3, 0);
	
	if (midi->tempoLed > 0){
		tempoLedFrame = 1 + (midi->beats == 1 ? 1 : 0);
		--midi->tempoLed;
	} else {
		tempoLedFrame = 0;
	}
	ledSprite->display(0, 230, 4, OBJPRIORITY_3, tempoLedFrame);
}

void Display::orangeTempoLeds(){
	u8 startLed = 4 * (midi->beats - 1) + (midi->quarters - 1) + 1;	
	for (u8 i = startLed; i < 16; ++i)
		ledSprite->display(1 + i, 16 * i, 174, OBJPRIORITY_3, 2);

}
void Display::greyTempoLeds(){
	for (u8 i = 0; i < 16; ++i)
		ledSprite->display(1 + i, 16 * i, 174, OBJPRIORITY_3, 0);
}

void Display::modeSelektor(){
	//Displaying bg3, and mode sprites
	bgSetPriority(this->subbg3, 1);
	DC_FlushAll();
	dmaCopy(greyedBgSubTiles,	bgGetGfxPtr(this->subbg3), greyedBgSubTilesLen);
	dmaCopy(greyedBgSubMap,		bgGetMapPtr(this->subbg3), greyedBgSubMapLen);
	bgShow(subbg3);
	s8 i;
	
	
	for (i = 0; i < 4; ++i){
		modesSprite->display(64 + i, 32 +  128 * (i % 2), 16 + 96 * (i / 2), OBJPRIORITY_0, i);
	}
	oamUpdate(&oamSub);
	input->paused = true;
	while(!(Cfg::modeSelector && input->pad->select() == DOWN) && input->modeSelektor() != 1){
		swiWaitForVBlank();
	}
	Cfg::modeSelector = false;
	oamClear(&oamSub, 64, 4);
	if (Cfg::mode == SLIDERS){
		dmaCopy(slidersBg3SubTiles,		bgGetGfxPtr(this->subbg3), slidersBg3SubTilesLen);
		dmaCopy(this->slidersBg3Map,	bgGetMapPtr(this->subbg3), 1536);
	} else {
		bgSetPriority(this->subbg3, 2);
		bgHide(this->subbg3);
	}
}

void Display::editSliderParams(){
	CCParam ccParam;
	u8		color;

	bgSetPriority(this->subbg3, 1);
	DC_FlushAll();
	dmaCopy(greyedBgSubTiles,	bgGetGfxPtr(this->subbg3), greyedBgSubTilesLen);
	dmaCopy(greyedBgSubMap,		bgGetMapPtr(this->subbg3), greyedBgSubMapLen);
	bgShow(this->subbg3);

	while(!(Cfg::editParams && input->pad->start() == DOWN)){
		for (u8 i = 0; i<16; ++i){
			ccParam = Cfg::sliders.ccs[Cfg::sliders.bank][i];
			color = Cfg::sliders.colors[Cfg::sliders.bank][i];

			echo(0, WHITE, 12*(i/8) , 1 + 4*(i%8), "CC");
			paramsEditSprite->display(16+i, 8 + 32*(i%8), 8 + 96*(i/8), OBJPRIORITY_0, 0);
			iprintf("\x1b[%d;%dH%3d", 2 + 12*(i/8) , 4*(i%8), ccParam.cc);
			paramsEditSprite->display(32+i, 8 + 32*(i%8), 24 + 96*(i/8), OBJPRIORITY_0, 1);
			
			echo(0, WHITE, 4+12*(i/8) , 1 + 4*(i%8), "CH");
			paramsEditSprite->display(48+i, 8 + 32*(i%8), 40 + 96*(i/8), OBJPRIORITY_0, 0);
			iprintf("\x1b[%d;%dH%3d", 6 + 12*(i/8) , 4*(i%8), 1 + ccParam.channel);
			paramsEditSprite->display(64+i, 8 + 32*(i%8), 56 + 96*(i/8), OBJPRIORITY_0, 1);

			paramsEditSprite->display(80+i, 8 + 32*(i%8), 72 + 96*(i/8), OBJPRIORITY_0, color + 2);
		}
		oamUpdate(&oamSub);
		input->editSliderParams();
		Cfg::editParams = true;
		swiWaitForVBlank();
	}
	save->write();
	Cfg::editParams = false;
	oamClear(&oamSub, 16, 80);	
	consoleClear();
	sliders();
}

void Display::editMixerParams(){
	CCParam ccParam;

	bgSetPriority(this->subbg3, 1);
	DC_FlushAll();
	dmaCopy(greyedBgSubTiles,	bgGetGfxPtr(this->subbg3), greyedBgSubTilesLen);
	dmaCopy(greyedBgSubMap,		bgGetMapPtr(this->subbg3), greyedBgSubMapLen);
	bgShow(this->subbg3);

	echo(0, RED, 0, 13, "MIXER");
	echo(0, RED, 12, 13, "MUTES");
	while(!(Cfg::editParams && input->pad->start() == DOWN)){
		for (u8 i = 0; i<8; ++i){
			ccParam = Cfg::mixer.ccs[i];
			
			echo(0, WHITE, 2, 1 + 4*(i%8), "CC");
			paramsEditSprite->display(16+i, 8 + 32*(i%8), 24, OBJPRIORITY_0, 0);
			iprintf("\x1b[%d;%dH%3d", 4, 4*(i%8), ccParam.cc);
			paramsEditSprite->display(32+i, 8 + 32*(i%8), 40, OBJPRIORITY_0, 1);
			
			echo(0, WHITE, 6, 1 + 4*(i%8), "CH");
			paramsEditSprite->display(48+i, 8 + 32*(i%8), 56, OBJPRIORITY_0, 0);
			iprintf("\x1b[%d;%dH%3d", 8, 4*(i%8), 1 + ccParam.channel);
			paramsEditSprite->display(64+i, 8 + 32*(i%8), 72, OBJPRIORITY_0, 1);
			
			ccParam = Cfg::mixer.mutes[i];

			echo(0, WHITE, 14, 1 + 4*(i%8), "CC");
			paramsEditSprite->display(24+i, 8 + 32*(i%8), 120, OBJPRIORITY_0, 0);
			iprintf("\x1b[%d;%dH%3d", 16, 4*(i%8), ccParam.cc);
			paramsEditSprite->display(40+i, 8 + 32*(i%8), 136, OBJPRIORITY_0, 1);
			
			echo(0, WHITE, 18, 1 + 4*(i%8), "CH");
			paramsEditSprite->display(56+i, 8 + 32*(i%8), 152, OBJPRIORITY_0, 0);
			iprintf("\x1b[%d;%dH%3d", 20, 4*(i%8), 1 + ccParam.channel);
			paramsEditSprite->display(72+i, 8 + 32*(i%8), 168, OBJPRIORITY_0, 1);
		}
		oamUpdate(&oamSub);
		input->editMixerParams();
		Cfg::editParams = true;
		swiWaitForVBlank();
	}
	save->write();
	Cfg::editParams = false;
	oamClear(&oamSub, 16, 80);	
	consoleClear();
}


void Display::editKaossParams(){
	CCParam* ccParam = NULL;

	bgSetPriority(this->subbg3, 1);
	DC_FlushAll();
	dmaCopy(greyedBgSubTiles,	bgGetGfxPtr(this->subbg3), greyedBgSubTilesLen);
	dmaCopy(greyedBgSubMap,		bgGetMapPtr(this->subbg3), greyedBgSubMapLen);
	bgShow(this->subbg3);
	while(!(Cfg::editParams && input->pad->start() == DOWN)){
		for (u8 i = 0; i<3; ++i){
			if (i == 0){
				echo(0, RED, 5, 7 + 8*(i%8), "X");
				ccParam = &Cfg::kaoss.x[Cfg::kaoss.bank];
			}
			if (i == 1){
				echo(0, RED, 5, 7 + 8*(i%8), "Y");
				ccParam = &Cfg::kaoss.y[Cfg::kaoss.bank];
			}
			if (i == 2){
				echo(0, RED, 5, 7 + 8*(i%8), "Z");
				ccParam = &Cfg::kaoss.z[Cfg::kaoss.bank];
			}
			

			echo(0, WHITE, 8, 7 + 8*(i%8), "CC");
			paramsEditSprite->display(16+i, 56 + 64*(i%8), 72, OBJPRIORITY_0, 0);
			iprintf("\x1b[%d;%dH%3d", 10, 6 + 8*(i%8), (*ccParam).cc);
			paramsEditSprite->display(32+i, 56 + 64*(i%8), 88, OBJPRIORITY_0, 1);
			
			echo(0, WHITE, 13, 7 + 8*(i%8), "CH");
			paramsEditSprite->display(48+i, 56 + 64*(i%8), 112, OBJPRIORITY_0, 0);
			iprintf("\x1b[%d;%dH%3d", 15, 6 + 8*(i%8), 1 + (*ccParam).channel);
			paramsEditSprite->display(64+i, 56 + 64*(i%8), 128, OBJPRIORITY_0, 1);
		}
		oamUpdate(&oamSub);
		input->editKaossParams();
		Cfg::editParams = true;
		swiWaitForVBlank();
	}
	Cfg::editParams = false;
	save->write();
	oamClear(&oamSub, 16, 80);	
	consoleClear();
	if (Cfg::mode == SLIDERS){
		dmaCopy(slidersBg3SubTiles,	bgGetGfxPtr(this->subbg3), slidersBg3SubTilesLen);
		dmaCopy(slidersBg3Map,		bgGetMapPtr(this->subbg3), 1536);
	} else {
		bgSetPriority(this->subbg3, 2);
		bgHide(this->subbg3);
	}
}

/////////////
//HELPERS
////////////
void Display::echo(u8 screen, u8 color, u8 line, u8 column, const char* text){
	consoleSelect(screen == 1 ? &mainConsole : &subConsole);
	iprintf("\e[%dm\x1b[%d;%dH%s", color, line, column,  text);
}

void Display::showBank(u8 bank, u8 max){
	for (u8 i = 0; i < max; ++i)
		banksSprite->display(26 + i, 62 + i*8, 6, 3, 2*i + (i == bank ? 1 : 0));
}

void Display::clearGfx(){
	consoleSelect(&subConsole);
	consoleClear();
	consoleSelect(&mainConsole);
	consoleClear();
	oamClear(&oamSub, 0, 0);
	oamClear(&oamMain, 17, 110);
}

// SPRITE CLASS
//-------------------------------------------------------------------------

// Loading a sprite
Sprite::Sprite(u8 screen, const unsigned int* tiles, u8 palette, SpriteSize spriteSize, u8 frames){
	u16			memorySize = 0;

	switch(spriteSize){
		case SpriteSize_8x8 :	memorySize = 64;
			break;
		case SpriteSize_8x16 :
		case SpriteSize_16x8 :	memorySize = 128;
			break;
		case SpriteSize_32x8 :
		case SpriteSize_8x32 :
		case SpriteSize_16x16:	memorySize = 256;
			break;
		case SpriteSize_32x16: 
		case SpriteSize_16x32:	memorySize = 512;
			break;
		case SpriteSize_32x32:	memorySize = 1024;
			break;
		case SpriteSize_64x32:
		case SpriteSize_32x64:	memorySize = 2048;
			break;
		case SpriteSize_64x64:	memorySize = 4096;
			break;
	}
	oam					= (screen == 1 ? &oamMain : &oamSub);
	palNumber			= palette;
	this->frames		= frames;
	gfxMem				= new u16*[frames];
	this->spriteSize	= spriteSize;
	for (u8 i = 0; i < frames; ++i){
		gfxMem[i] = oamAllocateGfx(oam, spriteSize, SpriteColorFormat_256Color);
		dmaCopy((u8*)tiles + (i * memorySize), gfxMem[i], memorySize);
	}
}

//Displaying a sprite
void Sprite::display(u8 id, int x, int y, u8 priority, u8 frame){
	oamSet(oam, 
			id,
			x, y,  
			priority, palNumber, 
			spriteSize, SpriteColorFormat_256Color, 
			gfxMem[frame], 
			-1, false, false, 
			false, false, 
			false);
}
//-------------------------------------------------------------------------