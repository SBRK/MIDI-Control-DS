#include <nds.h>
#include <filesystem.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fat.h>

#include "save.h"
#include "cfg.h"

// Initialisation. Opening files on SD card-------------------
// and on NitroFS (directly in the rom) ----------------------
Save::Save(){
	fat_file		= NULL;
	nitro_file		= NULL;
	fat_file_new	= false;
	
	loadFromNitroFs();

	imfat = fatInitDefault();
	if (imfat){
		fat_file = fopen("midicontrol.cfg", "r+");
		if (fat_file == NULL)
			write();
		else 
			load();
		fclose(fat_file);
		fat_file = NULL;
		loadCcs();
	}

	currentPart = -1;
}

void Save::loadFromNitroFs(){
	nitroOk = nitroFSInit();
	if (nitroOk)
		nitro_file = fopen("midicontrol.cfg", "r");
	load();
	fclose(nitro_file);
	nitro_file = NULL;
}
//------------------------------------------------------------

// File Selection for loading---------------------------------
bool Save::selectFile(){
	file = NULL;
	if (fat_file != NULL)
		file = fat_file;
	else if (nitro_file != NULL)
		file = nitro_file;
	if (file == NULL)
		return false;
	rewind(this->file);
	return true;
}
//------------------------------------------------------------

// Getting the current line-----------------------------------
void Save::getLine(char* line){
	u16		i;
	char	c = fgetc(file);

	for( i = 0; !feof(file) && c != '\n'; ++i){
		if (c == '['){
			getPart();
			ignoreEndOfLine();
			line = NULL;
			i = 0;
			break;
		}
		if (c == '#'){
			ignoreEndOfLine();
			line = NULL;
			i = 0;
			break;
		}
		line[i] = c;
		c = fgetc(file);
	}
	line[i] = '\0';
}
//------------------------------------------------------------


// Ignoring end of line (for line beginning with "#")---------
void Save::ignoreEndOfLine(){
	while (fgetc(file) != '\n');
}
//------------------------------------------------------------

// Changing the current part (Kaoss, Pads, Mixer, Sliders)----
// For lines beginning with a "["-----------------------------
void Save::getPart(){
	u8		i;
	char	c			= fgetc(file);
	char*	partName	= new char[128];

	for (i = 0; i < 127 && c != '\n' && c != ']'; ++i){
		partName[i] = c;
		c = fgetc(file);
	}
	partName[i] = '\0';
	char parts[][8] = {"kaoss", "pads", "mixer", "sliders", "general", "debug", "midiin"};
	i = 0;
	while (i<7 && strcmp(parts[i], partName) != 0)
		++i;
	if (i < 7)
		currentPart = i;
	delete partName;
}
//------------------------------------------------------------


// Getting keys and values from the line----------------------
// They must have this schema : "key1:key2:key3=value" -------
bool Save::getKeysAndValue(char* line, char** keys, u8* value){
	u16		i;
	char*	token;
	char**	tokens = new char*[2];

	token = strtok(line, "=");
	for(i = 0; i < 2 && token != NULL; ++i){
		tokens[i] = token;
		token = strtok(NULL, "=");
	}
	if (token != NULL)
		return false;
	*value = atoi(tokens[1]);
	token = strtok(tokens[0], " :");
	for(i = 0; i<3 && token != NULL; ++i){
		keys[i] = token;
		token = strtok(NULL, " :");
	}
	delete tokens;
	if (token == NULL){
		delete token;
		return true;
	} else {
		delete token;
		if (currentPart > 3){
			keys[0] = tokens[0];
			return true;
		} else {
			return false;
		}
	}
}
//------------------------------------------------------------


// Line Parsing-----------------------------------------------
bool Save::parseKaoss(char** keys, u8 value){
	CCParam*	ccParam;
	u8			bank = atoi(keys[0]) - 1;

	if (bank >= 4)
		return false;

	if (value > 127)
		value = 127;
	
	if (keys[1][0] ==		'x')
		ccParam = &Cfg::kaoss.x[bank];
	else if (keys[1][0] ==	'y')
		ccParam = &Cfg::kaoss.y[bank];
	else if (keys[1][0] ==	'z')
		ccParam = &Cfg::kaoss.z[bank];
	else 
		return false;
		
	if (strcmp(keys[2],			"cc") == 0)
		(*ccParam).cc		= value;
	else if (strcmp(keys[2],	"ch") == 0)
		(*ccParam).channel = (value - 1) < 16 ? (value - 1) : 0;
	else
		return false;

	return true;
}

bool Save::parseMixer(char** keys, u8 value){
	CCParam*	ccParam;
	u8			index = atoi(keys[1]) - 1;

	if (index >= 8)
		return false;

	if (value > 127)
		value = 127;

	if (strcmp(keys[0],			"mi") == 0)
		ccParam = &Cfg::mixer.ccs[index];
	else if (strcmp(keys[0],	"mu") == 0)
		ccParam = &Cfg::mixer.mutes[index];
	else
		return false;

	if (strcmp(keys[2],			"cc") == 0)
		(*ccParam).cc		= value;
	else if (strcmp(keys[2],	"ch") == 0)
		(*ccParam).channel	= (value - 1) < 16 ? (value - 1) : 0;
	else
		return false;

	return true;
}

bool Save::parseSliders(char** keys, u8 value){
	CCParam*	ccParam;
	u8			bank	= atoi(keys[0]) - 1;
	u8			slider	= atoi(keys[1]) - 1;

	if (slider >= 16 || bank >= 3)
		return false;

	if (value > 127)
		value = 127;

	ccParam = &Cfg::sliders.ccs[bank][slider];

	if (strcmp(keys[2],			"cc") == 0)
		(*ccParam).cc						= value;
	else if (strcmp(keys[2],	"ch") == 0)
		(*ccParam).channel					= (value - 1) < 16 ? (value - 1) : 0;
	else if (strcmp(keys[2],	"co") == 0)
		Cfg::sliders.colors[bank][slider]	= (value - 1) < 6 ? (value - 1) : 0;
	else
		return false;

	return true;
}

bool Save::parseGeneral(char* key, u8 value){
	if (strcmp(key,			"wifi") == 0)
		Cfg::wifi = (value == 1);
	else if (strcmp(key,	"midiinput") == 0)
		Cfg::receiveMidi = (value == 1);
	else if (strcmp(key,	"debug") == 0)
		Cfg::debug = (value == 1);
	else if (strcmp(key,	"saveccs") == 0)
		Cfg::saveCcs = (value == 1);
	else
		return false;

	return true;
}

bool Save::parseDebug(char* key, u8 value){
	if (strcmp(key,			"in") == 0)
		Cfg::debugIn = (value == 1);
	else if (strcmp(key,	"out") == 0)
		Cfg::debugOut = (value == 1);
	else if (strcmp(key,	"clock") == 0)
		Cfg::debugClock = (value == 1);
	else
		return false;

	return true;
}

bool Save::parseMidiIn(char* key, u8 value){
	if (strcmp(key,			"clock") == 0)
		Cfg::midiInClock = (value == 1);
	else if (strcmp(key,	"message") == 0)
		Cfg::midiInMessage = (value == 1);
	else
		return false;

	return true;
}
//------------------------------------------------------------


// Config Loading---------------------------------------------
bool Save::load(){
	char*	line		= new char[256];
	char**	keys		= new char*[3];
	u8		value;

	if (!selectFile())
		return false;
	while(!feof(file)){
		getLine(line);
		if (strlen(line) > 1 && getKeysAndValue(line, keys, &value)){
			switch(currentPart){
				// KAOSS
				case 0:
					parseKaoss(keys, value);
					break;
				// MIXER
				case 2:
					parseMixer(keys, value);
					break;
				//SLIDERS
				case 3:
					parseSliders(keys, value);
					break;
				//GENERAL
				case 4:
					parseGeneral(keys[0], value);
					break;
				//DEBUG
				case 5:
					parseDebug(keys[0], value);
					break;
				//MIDIIN
				case 6:
					parseMidiIn(keys[0], value);
					break;
			}
		}
	}
	delete line;
	delete keys;
	return true;
}
//------------------------------------------------------------

Save::~Save(){
	fclose(fat_file);
	fclose(nitro_file);
}

bool Save::write(){
	if (imfat){
		file = fopen("midicontrol.cfg", "w");

		writeGeneral();
		writeDebug();
		writeMidiIn();
		writeKaoss();
		writeMixer();
		writeSliders();

		fclose(file);
		return true;
	} else {
		return false;
	}
}
bool Save::writeCcs(){
	if (Cfg::saveCcs && imfat){
		file = fopen("midicontrol.bin", "wb");
		
		for(u8 i = 0; i < 16; ++i)
			fwrite(Cfg::ccValues[i], sizeof(u8), 128, file);

		fclose(file);
		return true;
	} else {
		return false;
	}
}

bool Save::loadCcs(){
	if (Cfg::saveCcs && imfat){
		file = fopen("midicontrol.bin", "rb");
		if (file != NULL){
			for(u8 i = 0; i < 16; ++i)
				fread(Cfg::ccValues[i], sizeof(u8), 128, file);

			fclose(file);
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}
// Line Writing-----------------------------------------------
void Save::writeKaoss(){
	fprintf(file, "[kaoss]\r\n\r\n");
	for(u8 bank = 0; bank < 3; ++bank){
		fprintf(file, "%1d:x:ch = %d\r\n", bank + 1, Cfg::kaoss.x[bank].channel + 1);
		fprintf(file, "%1d:x:cc = %d\r\n\r\n", bank + 1, Cfg::kaoss.x[bank].cc);

		fprintf(file, "%1d:y:ch = %d\r\n", bank + 1, Cfg::kaoss.y[bank].channel + 1);
		fprintf(file, "%1d:y:cc = %d\r\n\r\n", bank + 1, Cfg::kaoss.y[bank].cc);

		fprintf(file, "%1d:z:ch = %d\r\n", bank + 1, Cfg::kaoss.z[bank].channel + 1);
		fprintf(file, "%1d:z:cc = %d\r\n\r\n", bank + 1, Cfg::kaoss.z[bank].cc);
	}
}

void Save::writeMixer(){
	fprintf(file, "[mixer]\r\n\r\n");
	for(u8 slider = 0; slider < 8; ++slider){
		fprintf(file, "mi:%1d:ch = %d\r\n", slider + 1, Cfg::mixer.ccs[slider].channel + 1);
		fprintf(file, "mi:%1d:cc = %d\r\n\r\n", slider + 1, Cfg::mixer.ccs[slider].cc);

		fprintf(file, "mu:%1d:ch = %d\r\n", slider + 1, Cfg::mixer.mutes[slider].channel + 1);
		fprintf(file, "mu:%1d:cc = %d\r\n\r\n", slider + 1, Cfg::mixer.mutes[slider].cc);
	}
}

void Save::writeSliders(){
	fprintf(file, "[sliders]\r\n\r\n");
	for(u8 bank = 0; bank < 3; ++bank){
		for(u8 slider = 0; slider < 16; ++slider){
			fprintf(file, "%1d:%1d:ch = %d\r\n", bank + 1, slider + 1, Cfg::sliders.ccs[bank][slider].channel + 1);
			fprintf(file, "%1d:%1d:cc = %d\r\n", bank + 1, slider + 1, Cfg::sliders.ccs[bank][slider].cc);
			fprintf(file, "%1d:%1d:co = %d\r\n\r\n", bank + 1, slider + 1, Cfg::sliders.colors[bank][slider] + 1);
		}
	}
}

void Save::writeGeneral(){
	fprintf(file, "[general]\r\n\r\n");
	fprintf(file, "wifi = %1d\r\n", (Cfg::wifi ? 1 : 0));
	fprintf(file, "debug = %1d\r\n", (Cfg::debug ? 1 : 0));
	fprintf(file, "saveccs = %1d\r\n\r\n", (Cfg::saveCcs ? 1 : 0));
}

void Save::writeDebug(){
	fprintf(file, "[debug]\r\n\r\n");
	fprintf(file, "in = %1d\r\n", (Cfg::debugIn ? 1 : 0));
	fprintf(file, "out = %1d\r\n", (Cfg::debugOut ? 1 : 0));
	fprintf(file, "clock = %1d\r\n\r\n", (Cfg::debugClock ? 1 : 0));
}

void Save::writeMidiIn(){
	fprintf(file, "[midiin]\r\n\r\n");
	fprintf(file, "clock = %1d\r\n", (Cfg::midiInClock ? 1 : 0));
	fprintf(file, "message = %1d\r\n\r\n", (Cfg::midiInMessage ? 1 : 0));
}