#ifndef Midi_h
#define Midi_h

#include "cfg.h"
#include "input.h"
#include "display.h"

#define CHANNEL_MASK 0x0F
#define MESSAGE_MASK 0xF0

#define SYNC_CLOCK		0xF8
#define SYNC_START		0xFA
#define SYNC_POINTER	0xF2
#define SYNC_CONTINUE	0xFB
#define SYNC_STOP		0xFC

#define CHANNEL_1  (u8) 0
#define CHANNEL_2  (u8)  1
#define CHANNEL_3  (u8)  2
#define CHANNEL_4  (u8)  3
#define CHANNEL_5  (u8)  4
#define CHANNEL_6  (u8)  5
#define CHANNEL_7  (u8)  6
#define CHANNEL_8  (u8)  7
#define CHANNEL_9  (u8)  8
#define CHANNEL_10 (u8)  9
#define CHANNEL_11 (u8) 10
#define CHANNEL_12 (u8) 11
#define CHANNEL_13 (u8) 12
#define CHANNEL_14 (u8) 13
#define CHANNEL_15 (u8) 14
#define CHANNEL_16 (u8) 15

class Midi {
	public:
		s8				channel;
		u8				status;
		u16				frame;
		u16				tempo;
		u8				tempoLed;
		u8				quarters;
		u8				beats;
		u16				bars;
		static u16		msTicks;
		bool			releaseNote;

		Midi();
		static void		addTick(void);
		void			update();
		void			connect(void);
		void			receiveMidi(void);

		//CCs
		void			sendCC(u8 channel, u8 cc, u8 cc_value);
		void			sendCC(CCParam ccParam, u8 cc_value);
		u8				calculateCcValue(CCParam ccparam, u16 val, u16 val_max, bool invert);
		u8				getCcValue(CCParam ccParam);
		u8				getCcValue(u8 channel, u8 cc);
		void			setCcValue(CCParam ccParam, u8 value);
		void			setCcValue(u8 channel, u8 cc, u8 value);
		void			sendCcValues(void);

		//Notes
		void			sendNoteOn(u8 note_number, u8 note_velocity, u8 note_channel);
		void			sendNoteOff(u8 note_number, u8 note_channel);
		u8				getNote(u8 note_number, u8 note_channel);
		static char*	getNoteName(u8 note);
		u8				getNoteVelocity(u16 x, u16 y);
		u8				getNotePressed(u16 x, u16 y);

		//Kaoss
		void			kaossUpdate(u8 bank);
		void			updateKaossRecs(void);
		void			updateKaossRec(CCRec** cc, u8 bank, u8 loop);
		void			playKaossLoops();
		void			recordKaossLoops(u8 bank);

		//Pads
		void			padsUpdate();
		
		//Mixer
		void			mixerUpdate();

		//Sliders
		void			slidersUpdate(u8 bank);
		u8				getSliderCCVal(u8 slider, u16 y);
};

#endif