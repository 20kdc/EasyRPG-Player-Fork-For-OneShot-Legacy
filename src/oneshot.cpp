#include <string.h>
#include <stdlib.h>
//#include <iostream> // For text debug
#include <sstream>
#include <string>
#include "oneshot.h"
#include "oneshot_ser.h" // For serialization
#include "oneshot_info.h" // For text
#include "game_variables.h"
#include "game_switches.h"
#include "game_system.h"
#include "game_map.h" // Used to nudge the Game_Map event stuff after load finishes.
#include "scene.h"
#include "scene_osmb.h" // messageboxes!
#include "scene_end.h" // alt-f4 override
#include "scene_title.h" // If the bulb is broken, scene_title has to be kicked to cause gone().
#include "player.h" // For access to safe code
#include "output.h" // Output::Debug()

#include "main_data.h" // TPlayerNameT

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

// These configuration options could be perceived as technically altering game canon.
// However, they could smooth out the experience because we can't do meta-things on 3DS.
// Your choice, `whoami`.

// -- Start translatables --

#define STR_ONESHOT                 "You only have one shot, %hs."
#define STR_DO_YOU_UNDERSTAND       "Do you understand what this means?"
#define STR_STILL_HAVING_TROUBLE    "Still having trouble? Want me to spell it out for you?"
// Used on platforms where "read that text document" isn't so practical via ONESHOT_ENTITY_AWARE_OF_EMULATION.
// The idea is this will convince the player to sleep, which will take them to the Game Browser,
// which in the case of ONESHOT_GAME_BROWSER_SHOWS_SAFE_CODE will lead them to the solution.
// Notably, this has to happen NOW, before the Entity has started feeding the player
//  what will turn out to be misinformation (the actual ingame scripts don't have a clue about any of this).
// (NOTE: This still isn't great as the player might not know of the bed yet. Could read flags???)
#define STR_STILL_HAVING_TROUBLE_EXTENDED1 "Still having trouble? Want me to spell it out for you?"
#define STR_STILL_HAVING_TROUBLE_EXTENDED2 "Perhaps you should let Niko sleep for a while, as you ponder the issue."
//
#define STR_SURELY_YOU_KNOW         "Surely you know where to find it now?"
#define STR_STILL_PLANNING          "You're still planning on saving the world,\naren't you?"
#define STR_SUFFERING               "The world is suffering."
#define STR_GONE                    "The savior is gone.\nAll hope for the world is lost."
#define STR_WOULDA_REALIZED         "Well, you would have realized it sooner or later."
#define STR_HAD_ENOUGH              "I've had enough of this world. Haven't you?"
// some newline revisements here because there isn't much screenspace.
#define STR_DESTROY_IT              "Either that lightbulb will be destroyed,\nor Niko will be killed."
#define STR_QUIT_NOW                "I'll make sure you never reach the end.\nQuit now, and save yourself the effort."
#define STR_BAD_DREAM               "If Niko smashes the bulb and leaves,\nit will just be like waking up from a bad dream."
#define STR_MISERABLE               "Niko will be miserable in this world."
#define STR_YOU_MONSTER             "You do care about Niko, don't you?"
#define STR_NIKO_WILL_DIE           "If you quit now, Niko will die. Continue?"
#define STR_YOU_KILLED_NIKO         "You killed Niko."

#define QUIT_MESSAGE            "But you only just started playing!"
#define QUIT_MESSAGE_ONESHOT        "  Niko won't make it out alive."

// -- End translatables --

#define ENDING_BEGINNING 0
#define ENDING_DEJAVU 1
#define ENDING_DEAD 2
#define ENDING_ESCAPED 3
#define ENDING_TRAPPED 4

static int oneshot = 0;
static int ending = ENDING_BEGINNING;

static int forceEnding = 0;

static int gameStarted = 0;
static int isInGame = 0;

// Notably, this is apparently both zero-terminated *and* size-restricted.
// The following is a valid initialization for reference:

// strcpy(username, "Pancake"); // 7-char string + 1 zero byte = 8
// usernameSize = 8;

// Notably, the player name entry isn't persisted.
// Memory of the player's name will be lost when the player sleeps, if name entry is used.
// (Let's just pretend Niko's forgetful.)

static char username[256];
static uint32_t usernameSize = sizeof(username);

// The quit message pointer.
static const char * quitMsgPtr = QUIT_MESSAGE;

// Used to freeze/unfreeze the Script Interpreter during message boxes
// Notably the value of this is 100% controlled by scene_OSMB outside of a reset
int oneshot_global_messagebox_count = 0;
// Used as a flag by func_MessageBox to see if this is a re-execution
static bool messagebox_running = false;
// Used to wait an additional frame to try and prevent the script from messing up the scene-stack
static bool messagebox_terminating = false;
// OneShot performs an additional (and invalid) LoadItem for some reason
// Let's just pretend that didn't happen
static bool performing_load = false;

// Which messagebox was last pushed (that is, which is the first)?
// This is just temporary, and used to build a messagebox chain.
static std::shared_ptr<Scene> messagebox_current;

#define MESSAGE_INFO 0
#define MESSAGE_WARNING 0
#define MESSAGE_ERROR 0
#define MESSAGE_QUESTION 1

static void util_messagebox(const char * text, const char * title, int type) {
	std::shared_ptr<Scene> m = std::make_shared<Scene_OSMB>(text, type == MESSAGE_QUESTION, messagebox_current);
	messagebox_current = m;
}

// Confirm that the last messagebox pushed should be the one to appear,
// and clear the messagebox_current pointer.
static void util_messagebox_end() {
	Scene::Push(messagebox_current);
	messagebox_current.reset();
}

int util_loadEnding() {
	return oneshot_ser_loadEnding();
}
void util_saveEnding() {
	if (!forceEnding) {
		if (oneshot && (ending <= ENDING_DEJAVU))
			ending = ENDING_DEAD;
		if (gameStarted && (ending == ENDING_BEGINNING))
			ending = ENDING_DEJAVU;
	}
	if (ending != ENDING_BEGINNING) {
		int endingOld = util_loadEnding();
		if ((ending == ENDING_DEAD) && (endingOld == ENDING_DEAD)) {
			oneshot_ser_wipeSave();
			ending = ENDING_DEJAVU;
		}
		//printf("'saving' val %i\n", ending);
		oneshot_ser_saveEnding(ending);
	}
}

void util_updateQuitMessage() {
	quitMsgPtr = QUIT_MESSAGE;
	if ((ending == ENDING_BEGINNING) || (ending == ENDING_DEJAVU))
		quitMsgPtr = QUIT_MESSAGE_ONESHOT;
}

void oneshot_preinit() {
	Output::Debug("Oneshot preinit function called. If not playing Oneshot, I suggest you stop now.");
#ifdef ONESHOT_DATA_PERSISTENCE
	Output::Debug("Data persistence is enabled. That is, you only have one shot.");
#else
	Output::Debug("Data persistence is disabled.");
#endif
	// To test safe_code logic
	// (usually set to -1 on the line before the call here, which disables it.)
	// Player::safe_code = 1;
}

void oneshot_func_init() {
	//puts("We are doing ONESHOT init!");
	for (int i = 0; i < 7; i++)
		Game_Variables[i + 1] = 0;
	// Unsure on this
	for (int i = 0; i < 20; i++)
		Game_Switches[i + 1] = false;

	// Computer-player-name-thing needs doing

	char computer[256]; // For George

	oneshot_ser_getcomputer(username, computer);
	usernameSize = strlen(username) + 1;

	unsigned int george = 0;

	char * computerptr = computer;

	// One of those cases where the only right thing to do is use oneshot-legacy code (more or less)
	// Though this will return inaccurate results for UTF-8'd names.
	// ...I really do hope that Windows actually gives UTF-8 here instead of some nonsense.
	while (*computerptr) {
		unsigned char c = *(computerptr++);
		george += c;
	}

	george %= 6;
	Game_Variables[ONESHOT_VAR_GEORGE] = george + 1;

	// TPlayerNameT stuff happens now
	std::stringstream unstr;
	unstr << username;
	Main_Data::game_data.actors[1].name = unstr.str();
	// Final bits & pieces
	Game_Variables[ONESHOT_VAR_ENDING] = ending;
	gameStarted = 1;
	isInGame = 1;
}

static bool func_DeliberatelyNYI() {
	return true;
}
static bool func_SetNameEntry() {
	// Changes our internal record of the username to the C string "\\N[2]",
	//  so that when we replace it in future or save it, that will be used.
	// (I am not entirely sure how this 'escape-flag' method is meant to be saved.)
	strcpy(username, "\\N[2]");
	usernameSize = 5;
	return true;
}
static bool func_ShakeWindow() {
	oneshot_ser_shakewindow();
	return true;
}
static bool func_SetWallpaper() {
	if (Game_Variables[ONESHOT_VAR_ARG1] == 1) {
		oneshot_ser_setwp(true);
	} else {
		oneshot_ser_setwp(false);
	}
	return true;
}

// Run util_messagebox_end() at some point after this
static void gone() {
	util_messagebox(STR_GONE, "Fatal Error", MESSAGE_ERROR);
}

static bool func_MessageBox() {
	// Usually unused.
	std::shared_ptr<Scene> scene;
	// Keep the script in limbo till all messageboxes have been confirmed.
	if (messagebox_running) {
		messagebox_running = oneshot_global_messagebox_count != 0;
		// Even after the messagebox is dealt with,
		//  wait another frame to prevent messagebox "overload".
		messagebox_terminating = true;
		return false;
	}
	if (messagebox_terminating) {
		messagebox_terminating = false;
		return true;
	}

	char buff[512];
	switch (Game_Variables[ONESHOT_VAR_ARG1]) {
	case 0:
		sprintf(buff, STR_ONESHOT, username);
		util_messagebox(buff, "", MESSAGE_INFO);
		// Technically this happens a tad too early now but, whatever
		oneshot = 1;
		util_updateQuitMessage();
		// Save a dead ending NOW, in case the program improperly terminates
		ending = ENDING_DEAD;
		util_saveEnding();
		break;
	case 1:
#ifdef ONESHOT_ABSOLUTELY_CHEAT_SINCE_WE_CANT_META
		sprintf(buff, "(Meta-element NYI, Code is %06d. --20kdc)", Player::safe_code);
		util_messagebox(buff, "", MESSAGE_INFO);
#else
		util_messagebox(STR_DO_YOU_UNDERSTAND, "", MESSAGE_QUESTION);
#endif
		break;
	case 2:
#ifndef ONESHOT_ENTITY_AWARE_OF_EMULATION
		util_messagebox(STR_STILL_HAVING_TROUBLE, "", MESSAGE_QUESTION);
#else
// This isn't great but it'll help ease out the UX differences required.
// Further forks could make their own platform-specific ways of handling this.
// The current method is to show the safe code in the Game Browser.
		util_messagebox(STR_STILL_HAVING_TROUBLE_EXTENDED2, "", MESSAGE_INFO);
		util_messagebox(STR_STILL_HAVING_TROUBLE_EXTENDED1, "", MESSAGE_INFO);
#endif
		break;
	case 3:
		util_messagebox(STR_SURELY_YOU_KNOW, "", MESSAGE_QUESTION);
		break;
	case 4:
		util_messagebox(STR_SUFFERING, "", MESSAGE_INFO);
		util_messagebox(STR_STILL_PLANNING, "", MESSAGE_INFO);
		break;
	case 5:
		if (scene = Scene::Find(Scene::Title)) {
			// First go back to title,
			// and make sure it won't display anything
			Scene::PopUntil(Scene::Title);
			(static_cast<Scene_Title *>(scene.get()))->Kick();
			oneshot_fake_quit_handler();
		} else {
			// How did this happen?
			Scene::PopUntil(Scene::Null);
		}
		return false;
	case 6:
		util_messagebox(STR_QUIT_NOW, "", MESSAGE_ERROR);
		util_messagebox(STR_DESTROY_IT, "", MESSAGE_WARNING);
		util_messagebox(STR_HAD_ENOUGH, "", MESSAGE_WARNING);
		util_messagebox(STR_WOULDA_REALIZED, "", MESSAGE_INFO);
		break;
	case 7:
		util_messagebox(STR_YOU_MONSTER, "", MESSAGE_INFO);
		util_messagebox(STR_MISERABLE, "", MESSAGE_INFO);
		util_messagebox(STR_BAD_DREAM, "", MESSAGE_INFO);
		break;
	default:
		util_messagebox("messagebox nyi, fixme", "", MESSAGE_INFO);
		break;
	}
	// By this point a messagebox must have been pushed.
	// They should be pushed in reverse order -
	// the magical linked list stuff *should* do all the work
	util_messagebox_end();
	messagebox_running = true;
	return false;
}
static bool func_LeaveWindow() {
	// bye bye Niko...
	ending = ENDING_ESCAPED;
	oneshot_ser_leavewindow();
	return true;
}
static bool func_Save() {
	uint8_t switches[200];
	for (int i = 0; i < 200; i++)
		switches[i] = Game_Switches[i + 1] ? 1 : 0;
	int32_t vars[100];
	for (int i = 0; i < 100; i++)
		vars[i] = Game_Variables[i + 1];

	if (usernameSize == 5)
		if (!strcmp(username, "\\N[0]"))
			usernameSize = 0;
	oneshot_ser_saveBegin(switches, vars, username, usernameSize);
	return true;
}
static bool func_WriteItem() {
	int i = Game_Variables[ONESHOT_VAR_ARG1];
	oneshot_ser_saveItem(i);
	if (i == 0) {
		// Don't kill Niko
		ending = ENDING_DEJAVU;
		forceEnding = 1;
		// Write the ending here in case it crashes
		util_saveEnding();
		// now we exit
		Scene::PopUntil(Scene::Title);
		Scene::Pop();
		oneshot_fake_quit_handler();
		return false;
	}
	return true;
}
static bool func_Load() {
	uint8_t switches[200];
	int32_t vars[100];
	int val = oneshot_ser_loadBegin(switches, vars, username, &usernameSize);
	if (val) {
		for (int i = 0; i < 200; i++)
			Game_Switches[i + 1] = switches[i] != 0;
		for (int i = 0; i < 100; i++)
			Game_Variables[i + 1] = vars[i];
		// Now clean up for what happens in func_Save
		if (usernameSize == 0) {
			func_SetNameEntry();
			val = 2; // special return for this case.
		}
	}
	Game_Variables[ONESHOT_VAR_RETURN] = val;

	performing_load = true;
	oneshot = 1;
	util_updateQuitMessage();
	ending = ENDING_DEAD;
	return true;
}
static bool func_ReadItem() {
	if (!performing_load) {
		// Just in case.
		Game_Variables[ONESHOT_VAR_RETURN] = 0;
		return true;
	}
	if ((Game_Variables[ONESHOT_VAR_RETURN] = oneshot_ser_loadItem()) == 0) {
		// End of save-read, automatically wipes.
		// oneshot_ser has no reason to need to know these quirks.
		oneshot_ser_wipeSave();
		performing_load = false;
	}
	return true;
}

static bool func_Document() {
	// This is very big for hopefully obvious reasons.
	char document_building_buffer[0x10000];
	// you're welcome
#ifdef ONESHOT_GAME_BROWSER_SHOWS_SAFE_CODE
	Player::safe_code = Game_Variables[ONESHOT_VAR_SAFE_CODE];
#endif
	sprintf(document_building_buffer, "%s%06d%s", oneshot_text_1, Game_Variables[ONESHOT_VAR_SAFE_CODE], oneshot_text_2);
	oneshot_ser_document(document_building_buffer);
	return true;
}
static bool func_End() {
	ending = Game_Variables[ONESHOT_VAR_ARG1];
	forceEnding = 1;
	util_saveEnding();
	return true;
}
static bool func_SetCloseEnabled() {
	// NYI
	return true;
}

static bool (*const funcs[])(void) = {
	// GuessName
	// Replaces the particular area of memory used for the 'initial guess' (ProphetBot/etc. assume this)
	//  of the player's name. Notably, this does NOT include most uses of player name.
	// Presumably the idea here was that as this is implemented as a rough memory-scan,
	//  the program should try to avoid potentially using the earlier name in a later slot or vice versa.
	func_DeliberatelyNYI,
	// SetName
	// Replaces the particular area of memory used for the 'final version' of the player's name.
	// If it's replaced with a player name or a player name entry tag depends on if SetNameEntry is used.
	func_DeliberatelyNYI,
	func_SetNameEntry,
	func_ShakeWindow, // NYI
	func_SetWallpaper, // NYI
	func_MessageBox,
	func_LeaveWindow, // part-NYI
	func_Save,
	func_WriteItem,
	func_Load,
	func_ReadItem,
	func_Document,
	func_End,
	func_SetCloseEnabled, // NYI
	// Function 14 would go here(?)
};

bool oneshot_func_exec() {
	bool r = funcs[Game_Variables[ONESHOT_VAR_FUNC]]();
	Game_Map::SetNeedRefresh(Game_Map::Refresh_All);
	return r;
}

static void oneshot_titlescreen_init() {
	// Due to the incompatibility between OneShot's "the game closes",
	//  and EasyRPG's "back to menu",
	// this seems like the best solution until I have save/load implemented
	// (Actually it's the best solution I can think of given later work on meta-stuff)
	ending = util_loadEnding();
	if (ending <= ENDING_DEJAVU) {
		if (oneshot)
			ending = ENDING_DEAD;
		else if (gameStarted)
			ending = ENDING_DEJAVU;
		util_saveEnding();
	}
	Output::Debug("Titlescreen code was hit. Loaded ending as %i.", ending);
	isInGame = 0;
	oneshot = 0;
}

const char * oneshot_titlescreen() {
	oneshot_titlescreen_init();
	if (ending == ENDING_ESCAPED)
		return 0; // Magic value that gets replaced with "" in the titlescreen code and sets a flag to run oneshot_titlescreen_special_ready()
	if ((ending == ENDING_DEAD) && (!gameStarted)) {
		return "title_dead";
	} else if (ending == ENDING_TRAPPED) {
		return "title_stay";
	}
	return "title";
}

void oneshot_titlescreen_special_ready() {
	// Standard message box procedure can't work here, do something special
	gone();
	Scene::PopUntil(Scene::Null);
	Scene::Push(messagebox_current);
	messagebox_current.reset();
}

const char * oneshot_titlebgm() {
	if (ending == ENDING_ESCAPED)
		return ""; // Set to blank - do override later
	if ((ending == ENDING_DEAD) && (!gameStarted)) {
		return "MyBurdenIsDead";
	} else if (ending == ENDING_TRAPPED) {
		return "Distant";
	}
	return "MyBurdenIsLight";
}
const char * oneshot_closewindowprompt() {
	if (isInGame && (ending == ENDING_DEAD)) {
		if (oneshot) {
			return STR_NIKO_WILL_DIE;
		} else {
			return STR_YOU_KILLED_NIKO;
		}
	}
	return 0;
}
const char * oneshot_exitgameprompt() {
	// Since 'exit game' now == 'close game' to prevent title caching issues,
	// it's probably best just to do this.
	return quitMsgPtr;
}

std::string oneshot_process_text(const std::string & inp) {
	// Used to quickly test name substitution.
	// std::string work = "The person _PlayerName_xxxxxxxxxxxxxxxxxxxx is a tester.";
	std::string work = inp;
	std::string un = username;
	//std::cout << work << '\n';
	size_t found;
	// _PlayerName_guess_xxxxxxxxxxxxxx == 
	// 12341234123412341234123412341234
	//    1   2   3   4   5   6   7   8 ==
	// 32
	while ((found = work.find("_PlayerName_guess_xxxxxxxxxxxxxx")) != std::string::npos) {
		work.replace(found, 32, un);
	}
	while ((found = work.find("_PlayerName_xxxxxxxxxxxxxxxxxxxx")) != std::string::npos) {
		work.replace(found, 32, un);
	}
	return work;
}

// This function has the ability to stop the game from closing on request.
// Use with care.
int oneshot_override_closing() {
	if (Scene::Find(Scene::End)) {
		// Outright refuse for End or OSMB.
		return 1;
	}
	if (Scene::Find(Scene::Title)) {
		const char * ccc = oneshot_closewindowprompt();
		if (ccc == STR_YOU_KILLED_NIKO) {
			// This is better than pop-till-gamebrowser,
			//  because the game browser may not exist.
			Scene::PopUntil(Scene::Title);
			Scene::Pop();
			// Do this while we're waiting
			oneshot_fake_quit_handler();
			// Commence the guilt-tripping.
			// (Has to happen after the quit because it'll nudge the mbc by 1)
			util_messagebox(ccc, "", MESSAGE_INFO);
			return 1;
		}
		if (ccc) {
			Scene::Push(std::make_shared<Scene_End>(ccc, true));
			// If the user ends the game from Scene_End, it should exit-to-gamebrowser.
			return 1;
		}
	}
	return 0;
}
void oneshot_fake_quit_handler() {
	// ---Pre-quit-stuffdoings (process detach handler)---
	// (Given this tends to accidentally prevent death from working: Does this even get run???)
	// (Ok, now some things are fixed this actually needs to be added again)
	// (Ok, it seems this *is required* for the proper sequence of:
	//  *kill niko* -> dead, says "you killed niko" ->
	//  *magically comes back to life only after you've been informed that you murdered Niko*)
	util_saveEnding();
	// ---Post-quit-reset. This should reset everything as if the Player had been newly restarted.---
	oneshot = 0;
	ending = ENDING_BEGINNING;
	forceEnding = 0;
	gameStarted = 0;
	isInGame = 0;
	quitMsgPtr = QUIT_MESSAGE;
	usernameSize = sizeof(username);
	oneshot_global_messagebox_count = 0;
	messagebox_running = false;
	messagebox_terminating = false;
	performing_load = false;
	// Deallocate all messageboxes.
	messagebox_current.reset();
}
