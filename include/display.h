#ifndef Display_h
#define Display_h

#include "save.h"
#include "cfg.h"
#include "input.h"
#include "midi.h"

#define RED			41
#define GREEN		42
#define BLUE		44
#define GREY		40
#define LIGHT_GREY	37
#define WHITE		47

class Sprite{
public:
		u16**		gfxMem;
		u16			frames;
		u8			palNumber;
		OamState*	oam;
		SpriteSize	spriteSize;

				Sprite(u8 screen, const unsigned int* tiles, u8 palette, SpriteSize spriteSize, u8 frames);
		void	display(u8 id, int x, int y, u8 priority, u8 frame);
};

class Display {
	private: 
		u32				frame;
		u32				msFrame;

		//Sprites
		Sprite*			padSprite;
		Sprite*			kaossBrickSprite;
		Sprite*			modesSprite;
		Sprite*			ledSprite;
		Sprite*			dotSprite;
		Sprite*			sliderCursorSprite;
		Sprite*			muteSprite;
		Sprite*			paramsEditSprite;
		Sprite*			banksSprite;
		Sprite*			octaveSelectSprite;

		//Backgrounds
		int				subbg0;
		int				subbg1;
		int				subbg2;
		int				subbg3;
		int				mainbg0;
		int				mainbg1;
		int				mainbg2;
		int				mainbg3;

	public:
		s8				channel;
		u8***			kaossBricksState;
		unsigned short*	slidersBg2Map;
		unsigned short*	slidersBg3Map;

		//Consoles
		PrintConsole	subConsole;
		PrintConsole	mainConsole;

						Display();
		void			nextFrame(void);

		//Connection
		void			pre_connect(void);
		void			connected();
		void			connection_error(void);

		//Update
		void			update(void);

		//Kaoss
		void			kaoss(void);
		void			kaossUpdate(void);
		void			setKaossSpriteFrame(u8 sprite, u8 frame);
		void			kaossButtonsStatus();
		u8				kaossGetLoopFrame(u8 loop);
		void			editKaossParams();

		//Pads
		void			pads(void);
		void			padsUpdate(void);
		void			setPadSpriteFrame(u8 sprite, u8 frame);
		void			padsButtonsStatus();

		//Mixer
		void			mixer(void);
		void			mixerUpdate(void);
		void			editMixerParams(void);

		//Sliders
		void			sliders(void);
		void			slidersUpdate(void);
		void			editSliderParams(void);
		void			updateSliderMap(u8 slider);
		void			updateSliderUpMap(u8 slider);

		//Pause Menus
		void			modeSelektor(void);

		//Tempo leds
		void			tempoLeds();
		void			orangeTempoLeds();
		void			greyTempoLeds();

		//Helpers
		void			echo(u8 screen, u8 color, u8 line, u8 column, const char* text);
		void			showBank(u8 bank, u8 max);
		void			clearGfx();
};

#endif