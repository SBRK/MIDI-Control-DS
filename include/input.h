#ifndef Input_h
#define Input_h

#include "cfg.h"
#include "midi.h"
#include "display.h"

#define OFF 		0
#define DOWN 		1
#define HELD 		2
#define RELEASED	3


class InputPad {
	private:
		int*	keys_down;
		int*	keys_held;
		int*	keys_released;
		
		int		get_press(int key);
		
	public:
		InputPad(int* down, int* held, int* released);
		int		a(void);
		int		b(void);
		int		x(void);
		int		y(void);
		int		l(void);
		int		r(void);
		int		start(void);
		int		select(void);
		int		up(void);
		int		down(void);
		int		left(void);
		int		right(void);
};

class InputTouch {
	private :
		int*			keys_down;
		int*			keys_held;
		int*			keys_released;
	
	public:
		touchPosition 	pos;
		int16 			previous_x;
		int16 			previous_y;
		int16 			x_movement;
		int16 			y_movement;
	
		InputTouch(int* down, int* held, int* released);
		void			update(void);
		int				press(void);
};

class Input {
	private:
		int			keys_down;
		int			keys_held;
		int			keys_released;
	public:
		bool		paused;
		bool		updated;
		InputTouch	*touch;
		InputPad	*pad;
		
		Input(void);
		void		kaossSendX();
		void		kaossSendY();
		void		padUpdate();
		void		kaossLoopPress(u8 loop);
		void		kaossLoopPressCc(CCRec** cc, u8 loop);
		void		update(void);
		u8			modeSelektor(void);
		void		editSliderParams(void);
		void		editKaossParams(void);
		void		editMixerParams();
};

#endif