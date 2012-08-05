#ifndef Cfg_h
#define Cfg_h

#define NONE	0
#define KAOSS	1
#define PADS	2
#define MIXER	3
#define SLIDERS	4

#define QUARTER		4
#define SIXTH		6
#define EIGTH		8
#define TWELVE		12
#define SIXTEEN		16
#define TWENTYFOUR	24
#define THIRTYTWO	32
#define FORTYEIGHT	48

#include <list>
using namespace std;

struct CCParam{
	u8		channel;
	u8		cc;
	u8		max;
	u8		min;
	bool	toggle;
};

class CCRec{
	public:
		bool				sync;
		bool				waitForPlaying;
		bool				waitForRecording;
		bool				playing;
		bool				recording;
		
		list<u8>			values;
		u32					length;
		list<u8>::iterator	current;

		CCRec(bool sync);
		void				addValue(u8 value);
		u8					nextValue(void);
		void				start();
		void				stop();
		void				clear(void);
		~CCRec(void);
};

struct KaossParams{
	u8				bank;
	u8				previousBank;
	bool			record;
	bool			clear;
	CCParam*		x;
	CCRec***		xRec;
	CCParam*		y;
	CCRec***		yRec;
	CCParam*		z;
	CCRec***		zRec;
};

struct MixerParams{
	s8			held;
	CCParam*	ccs;
	CCParam*	mutes;
	CCParam*	solos;
	CCParam*	buttons;
};

struct SlidersParams{
	u8			bank;
	u8			previousBank;
	s8			held;
	CCParam**	ccs;
	CCParam**	buttons;
	u8**		colors;
};

struct PadsParams{
	u8			channel;
	u8			baseNote;
	s8			held;
	bool		updated;
	bool		oct_updated;
	bool		shift;
	bool		hold;
	bool		repeat;
	u8			repeatRate;
	u8			doubleTriple;
};

class Cfg{
	public:
		static u8					mode;
		static bool					modeSelector;
		static bool					editParams;
		static u8					previousMode;
		static u8					midiConnected;
		static bool					wifi;
		static bool					receiveMidi;
		static bool					debug;
		static bool					debugIn;
		static bool					debugOut;
		static bool					debugClock;
		static bool					saveCcs;
		static bool					midiInClock;
		static bool					midiInMessage;

		static u8**					notesStatus;	//2ko (s8 * 16 * 128 = 2048 octets)
		static u8**					ccValues;		//2ko (s8 * 16 * 128 = 2048 octets)
		static u16**				notesRepeat;	//2ko (s8 * 16 * 128 = 2048 octets)
		static bool**				ccChangedVblank;
		static bool**				ccChangedMblank;

		static struct KaossParams	kaoss;
		static struct PadsParams	pads;
		static struct SlidersParams	sliders;
		static struct MixerParams	mixer;

		static char*				fileBuffer;

		static void					init(void);
};

#endif