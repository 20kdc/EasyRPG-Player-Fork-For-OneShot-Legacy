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

#include "oneshot.h"
#include "oneshot_ser.h"

static int oneshot_ser_ending = 0;

int oneshot_ser_loadEnding() {
	return oneshot_ser_ending;
}

void oneshot_ser_saveEnding(int ending) {
	oneshot_ser_ending = ending;
}

void oneshot_ser_wipeSave() {
}

// Header Format: 1 byte for the size of the username.
// Followed by that many bytes for it.
// 200 bytes of switches.
// 100 variables, each 4 bytes, little-endian.
void oneshot_ser_saveBegin(uint8_t * switches, int32_t * vars, char * username, uint32_t usernameSize) {
	// NYI
}
// Item format is essentially a string - it's zero-terminated.
// Writing is controlled mostly by the game script.
// When a zero is written, the file is closed.
void oneshot_ser_saveItem(int item) {
	// NYI
}

// 0: No save file exists.
// 1: A save file exists. It has a player name.
// 2: A save file exists. It has no player name.
// The calling function is responsible for updating the "oneshot" and "ending" internal variables,
//  along with the cached on-quit message.
int oneshot_ser_loadBegin(uint8_t * switches, int32_t * vars, char * username, uint32_t * usernameSize) {
	// NYI
	return 0;
}
int oneshot_ser_loadItem() {
	// NYI
	return 0;
}

void oneshot_ser_document(char * string) {
	// Nowhere to output the document to :(
}