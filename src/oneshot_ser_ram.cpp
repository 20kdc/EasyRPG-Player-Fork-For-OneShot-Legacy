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
#include "oneshot.h"
#include "oneshot_ser.h"
#include "output.h"

static int oneshot_ser_ending = 0;

static unsigned char oneshot_ser_hasSave = 0;

static char oneshot_ser_username[256];
static unsigned char oneshot_ser_usernameSize;
static unsigned char oneshot_ser_items[256];
static unsigned char * oneshot_ser_iptr;

static uint8_t oneshot_ser_switches[200];
static int32_t oneshot_ser_vars[100];

int oneshot_ser_loadEnding() {
	return oneshot_ser_ending;
}

void oneshot_ser_saveEnding(int ending) {
	oneshot_ser_ending = ending;
}

void oneshot_ser_wipeSave() {
	oneshot_ser_hasSave = 0;
}

// Header Format: 1 byte for the size of the username.
// Followed by that many bytes for it.
// 200 bytes of switches.
// 100 variables, each 4 bytes, little-endian.
void oneshot_ser_saveBegin(uint8_t * switches, int32_t * vars, char * username, uint32_t usernameSize) {
	oneshot_ser_usernameSize = usernameSize;
	memcpy(oneshot_ser_username, username, usernameSize);

	oneshot_ser_iptr = oneshot_ser_items;

	memcpy(oneshot_ser_switches, switches, 200);
	memcpy(oneshot_ser_vars, vars, 100 * sizeof(int32_t));
}
// Item format is essentially a string - it's zero-terminated.
// Writing is controlled mostly by the game script.
// When a zero is written, the file is closed.
// (Note: there are no safety checks on any of this)
void oneshot_ser_saveItem(int item) {
	*(oneshot_ser_iptr++) = item;
	if (item == 0)
		oneshot_ser_hasSave = 1;
}

// 0: No save file exists.
// 1: A save file exists. It has a player name.
// 2: A save file exists. It has no player name.
// The calling function is responsible for updating the "oneshot" and "ending" internal variables,
//  along with the cached on-quit message.
int oneshot_ser_loadBegin(uint8_t * switches, int32_t * vars, char * username, uint32_t * usernameSize) {
	if (!oneshot_ser_hasSave)
		return 0;

	*usernameSize = oneshot_ser_usernameSize;
	memcpy(username, oneshot_ser_username, oneshot_ser_usernameSize);
	username[oneshot_ser_usernameSize] = 0;

	oneshot_ser_iptr = oneshot_ser_items;

	memcpy(switches, oneshot_ser_switches, 200);
	memcpy(vars, oneshot_ser_vars, 100 * sizeof(int32_t));
	return 1;
}
int oneshot_ser_loadItem() {
	return *(oneshot_ser_iptr++);
}

void oneshot_ser_document(char * string) {
	// For debugging
	Output::Debug("Document: ||%s||", string);
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
