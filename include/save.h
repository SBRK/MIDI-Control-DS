#ifndef Save_h
#define Save_h

#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class Save{
	public:
		FILE*	nitro_file;
		FILE*	fat_file;
		FILE*	file;
		char*	file_content;
		s8		currentPart;
		bool	nitroOk;
		bool	imfat;
		bool	fat_file_new;

		Save(void);
		bool	selectFile();
		void	loadFromNitroFs();
		bool	load();
		void	getLine(char* line);
		void	ignoreEndOfLine();
		void	getPart();

		bool	getKeysAndValue(char* line, char** keys, u8* value);

		bool	parseKaoss(char** keys, u8 value);
		bool	parseMixer(char** keys, u8 value);
		bool	parseSliders(char** keys, u8 value);
		bool	parseGeneral(char* key, u8 value);
		bool	parseDebug(char* key, u8 value);
		bool	parseMidiIn(char* key, u8 value);
		
		void	writeKaoss();
		void	writeMixer();
		void	writeSliders();
		void	writeGeneral();
		void	writeDebug();
		void	writeMidiIn();
		
		bool	write();
		bool	writeCcs();
		bool	loadCcs();
		~Save(void);
};

#endif