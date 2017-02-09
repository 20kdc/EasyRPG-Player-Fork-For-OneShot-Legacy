// Oneshot Hacks File
#ifndef _ONESHOT_H_
#define _ONESHOT_H_

// ---- CONFIGURATION

// -- The following configure the oneshot_ser module:

// Messageboxes in SDL2 available?
#define ONESHOT_SDL2_MSGBOX

// Is <stdio.h> FILE* available? (If not, the saved game is held in RAM.)
#define ONESHOT_DATA_PERSISTENCE

#ifdef ONESHOT_DATA_PERSISTENCE
	// This is configuration for saving (oneshot_ser)
	// The defaults right now are awful, but... better ideas, anyone?
	// (Under some circumstances this could be changed.)
	#ifdef WINDOWS

		#define ONESHOT_PRESAVE_COMMAND system("mkdir %appdata%/Oneshot");
		#define ONESHOT_WIPESAVE_COMMAND _unlink(buf);
		#define ONESHOT_SAVE_PATH "%s/Oneshot/save", getenv("APPDATA")
		#define ONESHOT_ENDING_PATH "%s/Oneshot/ending", getenv("APPDATA")

		#define ONESHOT_DOCUMENT_PATH "%s/My Documents/DOCUMENT.oneshot.txt", getenv("HOMEPATH")

		#define ONESHOT_USERNAME_GUESSER getenv("USERNAME")
		#define ONESHOT_HOSTNAME_GUESSER getenv("COMPUTERNAME")

	#else

		// for unlink
		#include <unistd.h>

		#define ONESHOT_PRESAVE_COMMAND system("mkdir -p ~/.config/Oneshot");
		#define ONESHOT_WIPESAVE_COMMAND unlink(buf);
		#define ONESHOT_SAVE_PATH "%s/.config/Oneshot/save", getenv("HOME")
		#define ONESHOT_ENDING_PATH "%s/.config/Oneshot/ending", getenv("HOME")

		#define ONESHOT_DOCUMENT_PATH "%s/DOCUMENT.oneshot.txt", getenv("HOME")

		#define ONESHOT_USERNAME_GUESSER getenv("USER")
		#define ONESHOT_HOSTNAME_GUESSER getenv("HOSTNAME")

	#endif
#else
	// This is required no matter what, but with data persistence
	//  it uses getenv anyway so it can use it for these.
	// Change the way things are handled there (like a sdcard .oneshot folder for Wii)
	//  if you want data persistence but your platform can't work with the defines,
	//  which is probably any platform other than the main quadrilateral of Mac/BSD/Linux/Windows.
	// The HOSTNAME_GUESSER is deliberately randomized so VAR_GEORGE stuff comes out randomly.
	#define ONESHOT_USERNAME_GUESSER "Pancake"
#endif

// Use SDL for messageboxes. (NYI)
// #define ONESHOT_SDL_MESSAGEBOXES

// -- The following options affect gameplay to handle lacking parts of platform support (oneshot_ser).

// Note! You shouldn't have to use this on any device with ONESHOT_DATA_PERSISTENCE working.

// This option will replace the Game Browser title with the safe code when the document is supposed to appear.
// The following define (ENTITY_AWARE_OF_EMULATION) assumes you have engaged this,
//  hence it asks that the player sleep to allow them to exit the 
// #define ONESHOT_GAME_BROWSER_SHOWS_SAFE_CODE

// This option will replace STR_STILL_HAVING_TROUBLE with an altered message that hints to the location
//  of the new safe code in the Game Browser. This is as best as I can do without altering game event-code.
// (By "best" I mean the best balance between avoiding modifying the work and making it actually usable.)
// #define ONESHOT_ENTITY_AWARE_OF_EMULATION

// This option will replace STR_DO_YOU_UNDERSTAND with an absolute spoiler to the safe code,
//  without having to sleep in order to see it in the Game Browser.
// (SOME EDITS LATER: Basically necessary because the Entity
//   refuses to let Niko sleep until a progression point.
//  Fun for debugging. Not.)
// (FURTHER EDITS LATER: Actually by the time they reach the safe
//   they are 100% guaranteed to be capable of sleep.
//  If they know about the bed in the first place.)
//#define ONESHOT_ABSOLUTELY_CHEAT_SINCE_WE_CANT_META

// ----

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

#define MESSAGE_INFO 0
#define MESSAGE_WARNING 1
#define MESSAGE_ERROR 2
#define MESSAGE_QUESTION 3

void oneshot_preinit();
void oneshot_func_init();
// If this returns true, the script continues,
//  otherwise, the command will be repeated.
// This allows for "freezing" the game.
bool oneshot_func_exec();
const char * oneshot_titlescreen();
void oneshot_titlescreen_special_ready();
const char * oneshot_titlebgm();

// Note: This is not the close window prompt, and that's only used internally
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
