// OneShot Serialization Helper

#ifndef _ONESHOT_SER_H_
#define _ONESHOT_SER_H_

// For standard-sized integer holders
#include <stdint.h>

int oneshot_ser_loadEnding();
void oneshot_ser_saveEnding(int ending);
void oneshot_ser_wipeSave();
// Header Format: 1 byte for the size of the username.
// Followed by that many bytes for it.
// 200 bytes of switches.
// 100 variables, each 4 bytes, little-endian.
void oneshot_ser_saveBegin(uint8_t * switches, int32_t * vars, char * username, uint32_t usernameSize);
// Item format is essentially a string - it's zero-terminated.
// Writing is controlled mostly by the game script.
// When a zero is written, the file is closed.
void oneshot_ser_saveItem(int item);

// 0: No save file exists.
// 1: A save file exists. It has a player name.
// 2: A save file exists. It has no player name.
// The calling function is responsible for updating the "oneshot" and "ending" internal variables,
//  along with the cached on-quit message.
int oneshot_ser_loadBegin(uint8_t * switches, int32_t * vars, char * username, uint32_t * usernameSize);
// Reads one item from the file.
// When this returns zero, the file has been closed.
int oneshot_ser_loadItem();

// -- META-ELEMENT HELPERS --
// These are optional so long as the right defines are set in oneshot.cpp
// (and you remember what the wallpaper's supposed to look like, until I add something for that),
// but are preferred.

// Output the Document with a given string.
void oneshot_ser_document(char * string);
void oneshot_ser_setwp(bool enabled);
void oneshot_ser_shakewindow();
void oneshot_ser_leavewindow();
// At some point, the messagebox stuff should go here.
// Notably, oneshot_global_messagebox_count should still be in oneshot.cpp -
//  the "decrement value and modify a return variable" system still applies.
// A possible idea may be to have scene_osmb query if a messagebox is possible,
//  and if not use old behavior, otherwise pass on handling.
// Notably this should be done in update & render, so that the correct sequence occurs.
// (remember that message boxes are pushed in reverse order)

// The function would be:
bool oneshot_ser_trymsgbox(const char * text, const char * title, int msg_type, void (*cb)(void * a, bool result), void * userdata);
// The callback would be used to set a killswitch in scene_osmb, and potentially set the question return value.
#endif
