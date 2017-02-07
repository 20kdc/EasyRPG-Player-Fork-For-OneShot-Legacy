// Oneshot Hacks File
#ifndef _ONESHOT_H_
#define _ONESHOT_H_

#include <string>

#define ONESHOT_VAR_FUNC 1
#define ONESHOT_VAR_ARG1 2
#define ONESHOT_VAR_ARG2 3
#define ONESHOT_VAR_ARG3 4
#define ONESHOT_VAR_ARG4 5
#define ONESHOT_VAR_RETURN 6
#define ONESHOT_VAR_ENDING 7
#define ONESHOT_VAR_SAFE_CODE 29
#define ONESHOT_VAR_GEORGE 47

void oneshot_preinit();
void oneshot_func_init();
// If this returns true, the script continues,
//  otherwise, the command will be repeated.
// This allows for "freezing" the game.
bool oneshot_func_exec();
const char * oneshot_titlescreen();
const char * oneshot_titlebgm();
const char * oneshot_exitgameprompt();

std::string oneshot_process_text(const std::string & inp);

int oneshot_override_closing();
// used to patch over an inconsistency(?) between RPG Maker 2003 and EasyRPG
void oneshot_fake_quit_handler();

extern int oneshot_global_messagebox_count;

// Game_Variables
// Game_Switches
// ^ These global vars are important!

#endif
