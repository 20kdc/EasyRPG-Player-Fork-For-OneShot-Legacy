#include "oneshot.h"
#include "oneshot_ser.h"

// Ending RAM-thing to keep things working
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
void oneshot_ser_saveBegin() {
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
int oneshot_ser_loadBegin() {
	// NYI
	return 0;
}
int oneshot_ser_loadItem() {
	// NYI
	return 0;
}