// Ending RAM-thing to keep things working
// This code is a derivative of the oneshot-legacy code.
// The license to that code is:
/* The MIT License
 * 
 * Copyright (c) 2015 Mathew Velasquez (http://mathew.link/)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string.h>
#include "oneshot.h" // Includes configuration.
#include "oneshot_ser.h"
#include "output.h"

#ifndef ONESHOT_DATA_PERSISTENCE
static int oneshot_ser_ending = 0;

static unsigned char oneshot_ser_hasSave = 0;

static char oneshot_ser_username[256];
static unsigned char oneshot_ser_usernameSize;
static unsigned char oneshot_ser_items[256];
static unsigned char * oneshot_ser_iptr;

static uint8_t oneshot_ser_switches[200];
static int32_t oneshot_ser_vars[100];
#else

#include <stdio.h>

static FILE * oneshot_ser_workingfile = 0;

#define ONESHOT_SER_ALERTFILEFAIL FILE * a = fopen(buf, mode); if (!a) Output::Debug("Couldn't open %s mode %s", buf, mode); return a;
#define ONESHOT_SER_SEQS char buf[2048]; snprintf(buf, 2048,
#define ONESHOT_SER_SEQE ); ONESHOT_SER_ALERTFILEFAIL
#define ONESHOT_SER_SEQET );

static FILE * open_ending_with_mode(char * mode) {
	ONESHOT_PRESAVE_COMMAND
	ONESHOT_SER_SEQS ONESHOT_ENDING_PATH ONESHOT_SER_SEQE
}
static FILE * open_save_with_mode(char * mode) {
	ONESHOT_PRESAVE_COMMAND
	ONESHOT_SER_SEQS ONESHOT_SAVE_PATH ONESHOT_SER_SEQE
}
static FILE * open_document_with_mode(char * mode) {
	ONESHOT_SER_SEQS ONESHOT_DOCUMENT_PATH ONESHOT_SER_SEQE
}
static void wipe_save() {
	ONESHOT_SER_SEQS ONESHOT_SAVE_PATH ONESHOT_SER_SEQET
	ONESHOT_WIPESAVE_COMMAND
}
#undef ONESHOT_SER_ALERTFILEFAIL
#undef ONESHOT_SER_SEQS
#undef ONESHOT_SER_SEQE
#undef ONESHOT_SER_SEQET

#endif

int oneshot_ser_loadEnding() {
#ifndef ONESHOT_DATA_PERSISTENCE
	return oneshot_ser_ending;
#else
	FILE * f = open_ending_with_mode("rb");
	if (f) {
		int i = fgetc(f);
		if (i < 0)
			i = 0;
		fclose(f);
		return i;
	}
	return 0;
#endif
}

void oneshot_ser_saveEnding(int ending) {
#ifndef ONESHOT_DATA_PERSISTENCE
	oneshot_ser_ending = ending;
#else
	FILE * f = open_ending_with_mode("wb");
	if (f) {
		fputc(ending, f);
		fclose(f);
	}
#endif
}

void oneshot_ser_wipeSave() {
#ifndef ONESHOT_DATA_PERSISTENCE
	oneshot_ser_hasSave = 0;
#else
	wipe_save();
#endif
}

// Header Format: 1 byte for the size of the username.
// Followed by that many bytes for it.
// 200 bytes of switches.
// 100 variables, each 4 bytes, little-endian.
void oneshot_ser_saveBegin(uint8_t * switches, int32_t * vars, char * username, uint32_t usernameSize) {
#ifndef ONESHOT_DATA_PERSISTENCE
	oneshot_ser_usernameSize = usernameSize;
	memcpy(oneshot_ser_username, username, usernameSize);

	oneshot_ser_iptr = oneshot_ser_items;

	memcpy(oneshot_ser_switches, switches, 200);
	memcpy(oneshot_ser_vars, vars, 100 * sizeof(int32_t));
#else
	oneshot_ser_workingfile = open_save_with_mode("wb");
	if (oneshot_ser_workingfile) {
		fputc(usernameSize, oneshot_ser_workingfile);
		fwrite(username, usernameSize, 1, oneshot_ser_workingfile);
		fwrite(switches, 200, 1, oneshot_ser_workingfile);
		for (int i = 0; i < 100; i++) {
			// For portability.
			fputc(vars[i] & 0xFF, oneshot_ser_workingfile);
			fputc((vars[i] >> 8) & 0xFF, oneshot_ser_workingfile);
			fputc((vars[i] >> 16) & 0xFF, oneshot_ser_workingfile);
			fputc((vars[i] >> 24) & 0xFF, oneshot_ser_workingfile);
		}
	}
#endif
}
// Item format is essentially a string - it's zero-terminated.
// Writing is controlled mostly by the game script.
// When a zero is written, the file is closed.
// (Note: there are no safety checks on any of this)
void oneshot_ser_saveItem(int item) {
#ifndef ONESHOT_DATA_PERSISTENCE
	*(oneshot_ser_iptr++) = item;
	if (item == 0)
		oneshot_ser_hasSave = 1;
#else
	if (oneshot_ser_workingfile) {
		fputc(item, oneshot_ser_workingfile);
		if (item == 0) {
			fclose(oneshot_ser_workingfile);
			oneshot_ser_workingfile = 0;
		}
	}
#endif
}

// 0: No save file exists.
// 1: A save file exists.
// (The 1/2 disambiguation has been moved to oneshot.cpp to make life easier here)
// The calling function is responsible for updating the "oneshot" and "ending" internal variables,
//  along with the cached on-quit message.
int oneshot_ser_loadBegin(uint8_t * switches, int32_t * vars, char * username, uint32_t * usernameSize) {
#ifndef ONESHOT_DATA_PERSISTENCE
	if (!oneshot_ser_hasSave)
		return 0;

	*usernameSize = oneshot_ser_usernameSize;
	memcpy(username, oneshot_ser_username, oneshot_ser_usernameSize);
	username[oneshot_ser_usernameSize] = 0;

	oneshot_ser_iptr = oneshot_ser_items;

	memcpy(switches, oneshot_ser_switches, 200);
	memcpy(vars, oneshot_ser_vars, 100 * sizeof(int32_t));
	return 1;
#else
	oneshot_ser_workingfile = open_save_with_mode("rb");
	if (oneshot_ser_workingfile) {
		int us = fgetc(oneshot_ser_workingfile);
		if (us < 0) {
			// file is clearly corrupt / has been blanked out by a bad save-wipe routine
			fclose(oneshot_ser_workingfile);
			return 0;
		}
		*usernameSize = us;
		if (*usernameSize) {
			// just hope the file is valid from this point on *gulp*
			fread(username, us, 1, oneshot_ser_workingfile);
			username[us] = 0;
			return 1;
		}
	} else {
		return 0;
	}
#endif
}
int oneshot_ser_loadItem() {
#ifndef ONESHOT_DATA_PERSISTENCE
	return *(oneshot_ser_iptr++);
#else
	if (oneshot_ser_workingfile) {
		int i = fgetc(oneshot_ser_workingfile);
		if (i < 1) {
			fclose(oneshot_ser_workingfile);
			oneshot_ser_workingfile = 0;
			return 0;
		}
		return i;
	} else {
		return 0;
	}
#endif
}

void oneshot_ser_document(char * str) {
#ifndef ONESHOT_DATA_PERSISTENCE
	// For debugging
	Output::Debug("Document: ||%s||", str);
#else
	FILE * doc = open_document_with_mode("wb");
	if (doc) {
		fputs(str, doc);
		fclose(doc);
	}
#endif
}

void oneshot_ser_setwp(bool enabled) {
	// For debugging
	if (enabled)
		Output::Debug("Wallpaper: YES");
	else
		Output::Debug("Wallpaper: NO");
}

void oneshot_ser_shakewindow() {
	// For debugging
	Output::Debug("Shake window!");
}

void oneshot_ser_leavewindow() {
	// For debugging
	Output::Debug("Niko walks off the screen...");
}

bool oneshot_ser_trymsgbox(const char * text, const char * title, int msg_type, void (*cb)(void * a, bool result), void * userdata) {
	// This basic backend is enough to run the game, but meta stuff doesn't work here.
	// (let's just hope you remember that wallpaper 'kay)
	return false;
}