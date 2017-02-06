#include <string.h>
#include <stdlib.h>
#include "oneshot.h"
#include "oneshot_ser.h" // For serialization
#include "oneshot_info.h" // For text
#include "game_variables.h"
#include "game_switches.h"
#include "game_system.h"
#include "scene.h"
#include "scene_osmb.h" // messageboxes!
#include "scene_end.h" // alt-f4 override
#include "player.h" // For access to safe code
#include "output.h" // Output::Debug()

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

// This option will replace the Game Browser title with the safe code when the document is supposed to appear.
// The following define (ENTITY_AWARE_OF_EMULATION) assumes you have engaged this,
//  hence it asks that the player sleep to allow them to exit the 
#define GAME_BROWSER_SHOWS_SAFE_CODE

// This option will replace STR_STILL_HAVING_TROUBLE with an altered message that hints to the location
//  of the new safe code in the Game Browser. This is as best as I can do without altering game event-code.
// #define ENTITY_AWARE_OF_EMULATION

// This option will replace STR_SURELY_YOU_KNOW with an absolute spoiler to the safe code,
//  without having to sleep in order to see it in the Game Browser.
// Use only if there are significant uptake difficulties preventing things
//  (Assuming a player goes in one direction and sticks with it,
//   they'll either run into the safe or the metal rod first.
//   If I recall correctly, anyway.
//   Since running into the safe will trigger the conversation,
//    though the player isn't stuck there,
//    they are still not going to appreciate it
//    if the code isn't actually visible anywhere.)
// #define ABSOLUTELY_CHEAT_SINCE_WE_CANT_META

// -- Start translatables --

#define STR_ONESHOT                 "You only have one shot, %hs."
#define STR_DO_YOU_UNDERSTAND       "Do you understand what this means?"
#define STR_STILL_HAVING_TROUBLE    "Still having trouble? Want me to spell it out for you?"
// Used on platforms where "read that text document" isn't so practical via ENTITY_AWARE_OF_EMULATION.
// The idea is this will convince the player to sleep, which will take them to the Game Browser,
// which in the case of GAME_BROWSER_SHOWS_SAFE_CODE will lead them to the solution.
// Notably, this has to happen NOW, before the Entity has started feeding the player
//  what will turn out to be misinformation (the actual ingame scripts don't have a clue about any of this).
// (NOTE: This still isn't great as the player might not know of the bed yet. Could read flags???)
#define STR_STILL_HAVING_TROUBLE_EXTENDED1 "Still having trouble? Want me to spell it out for you?"
#define STR_STILL_HAVING_TROUBLE_EXTENDED2 "Perhaps you should let Niko sleep for a while, as you ponder the issue."
//
#define STR_SURELY_YOU_KNOW         "Surely you know where to find it now?"
#define STR_STILL_PLANNING          "You're still planning on saving the world, aren't you?"
#define STR_SUFFERING               "The world is suffering."
#define STR_GONE                    "The savior is gone. All hope for the world is lost."
#define STR_WOULDA_REALIZED         "Well, you would have realized it sooner or later."
#define STR_HAD_ENOUGH              "I've had enough of this world. Haven't you?"
#define STR_DESTROY_IT              "Either that lightbulb will be destroyed, or Niko will be killed."
#define STR_QUIT_NOW                "I'll make sure you never reach the end.\nQuit now, and save yourself the effort."
#define STR_BAD_DREAM               "If Niko smashes the bulb and leaves, it will just be like waking up from a bad dream."
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

#define MESSAGE_INFO 0
#define MESSAGE_WARNING 0
#define MESSAGE_ERROR 0
#define MESSAGE_QUESTION 1

static void util_messagebox(const char * text, const char * title, int type) {
	Scene::Push(std::make_shared<Scene_OSMB>(text, type == MESSAGE_QUESTION));
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
	//puts("OneShot preinit function called,");
	//puts("if you are running this on the wrong game: this is your chance to back out.");
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
	// For now use a test string that everybody testing will recognize
	strcpy(username, "Pancake");
	usernameSize = 8;
	// Final bits & pieces
	Game_Variables[ONESHOT_VAR_ENDING] = ending;
	Game_Variables[ONESHOT_VAR_GEORGE] = ((rand() & 0x7FFF) % 6) + 1; // not quite accurate but ok
	gameStarted = 1;
	isInGame = 1;
}

static void func_DeliberatelyNYI() {
}
static void func_SetNameEntry() {
	// Changes our internal record of the username to the C string "\\N[2]",
	//  so that when we replace it in future or save it, that will be used.
	// (I am not entirely sure how this 'escape-flag' method is meant to be saved.)
	strcpy(username, "\\N[2]");
	usernameSize = 5;
}
static void func_ShakeWindow() {
	// NYI
}
static void func_SetWallpaper() {
	// NYI
}

static void func_MessageBox() {
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
		util_messagebox(STR_DO_YOU_UNDERSTAND, "", MESSAGE_QUESTION);
		break;
	case 2:
#ifndef ENTITY_AWARE_OF_EMULATION
		util_messagebox(STR_STILL_HAVING_TROUBLE, "", MESSAGE_QUESTION);
#else
// This isn't great but it'll help ease out the UX differences required.
// Further forks could make their own platform-specific ways of handling this.
// The current method is to show the safe code in the Game Browser.
		util_messagebox(STR_STILL_HAVING_TROUBLE_EXTENDED1, "", MESSAGE_INFO);
		util_messagebox(STR_STILL_HAVING_TROUBLE_EXTENDED2, "", MESSAGE_INFO);
#endif
		util_messagebox(STR_STILL_HAVING_TROUBLE, "", MESSAGE_QUESTION);
		break;
	case 3:
#ifdef ABSOLUTELY_CHEAT_SINCE_WE_CANT_META
		sprintf(buff, "(It's %06d. Could've just slept, the Game Browser shows it)", Player::safe_code);
		util_messagebox(buff, "", MESSAGE_INFO);
#else
		util_messagebox(STR_SURELY_YOU_KNOW, "", MESSAGE_QUESTION);
#endif
		break;
	case 4:
		util_messagebox(STR_STILL_PLANNING, "", MESSAGE_INFO);
		util_messagebox(STR_SUFFERING, "", MESSAGE_INFO);
		break;
	case 5:
		util_messagebox(STR_GONE, "Fatal Error", MESSAGE_ERROR);
		// oh, this "just" quits the game
		Scene::PopUntil(Scene::Title);
		Scene::Pop();
		break;
	case 6:
		util_messagebox(STR_WOULDA_REALIZED, "", MESSAGE_INFO);
		util_messagebox(STR_HAD_ENOUGH, "", MESSAGE_WARNING);
		util_messagebox(STR_DESTROY_IT, "", MESSAGE_WARNING);
		util_messagebox(STR_QUIT_NOW, "", MESSAGE_ERROR);
		break;
	case 7:
		util_messagebox(STR_BAD_DREAM, "", MESSAGE_INFO);
		util_messagebox(STR_MISERABLE, "", MESSAGE_INFO);
		util_messagebox(STR_YOU_MONSTER, "", MESSAGE_INFO);
		break;
	default:
		util_messagebox("messagebox nyi, fixme", "", MESSAGE_INFO);
		break;
	}
}
static void func_LeaveWindow() {
	ending = ENDING_ESCAPED;
	// NYI
}
static void func_Save() {
	uint8_t switches[200];
	for (int i = 0; i < 200; i++)
		switches[i] = Game_Switches[i + 1] ? 1 : 0;
	int32_t vars[200];
	for (int i = 0; i < 100; i++)
		vars[i] = Game_Variables[i + 1] ? 1 : 0;

	if (usernameSize == 5)
		if (!strcmp(username, "\\N[0]"))
			usernameSize = 0;
	oneshot_ser_saveBegin(switches, vars, username, usernameSize);
}
static void func_WriteItem() {
	int i = Game_Variables[ONESHOT_VAR_ARG1];
	oneshot_ser_saveItem(i);
	if (i == 0) {
		// Don't kill Niko
		ending = ENDING_DEJAVU;
		forceEnding = 1;
		// Write the ending here in case it crashes
		util_saveEnding();
	}
}
static void func_Load() {
	uint8_t switches[200];
	int32_t vars[200];
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

	oneshot = 1;
	util_updateQuitMessage();
	ending = ENDING_DEAD;
}
static void func_ReadItem() {
	if ((Game_Variables[ONESHOT_VAR_RETURN] = oneshot_ser_loadItem()) == 0) {
		// End of save-read, automatically wipes.
		// oneshot_ser has no reason to need to know these quirks.
		oneshot_ser_wipeSave();
	}
}

static void func_Document() {
	// This is very big for hopefully obvious reasons.
	char document_building_buffer[0x10000];
	// you're welcome
#ifdef GAME_BROWSER_SHOWS_SAFE_CODE
	Player::safe_code = Game_Variables[ONESHOT_VAR_SAFE_CODE];
#endif
	sprintf(document_building_buffer, "%s%06d%s", oneshot_text_1, Game_Variables[ONESHOT_VAR_SAFE_CODE], oneshot_text_2);
	oneshot_ser_document(document_building_buffer);
}
static void func_End() {
	ending = Game_Variables[ONESHOT_VAR_ARG1];
	forceEnding = 1;
	util_saveEnding();
	Scene::PopUntil(Scene::Title);
	Scene::Pop();
}
static void func_SetCloseEnabled() {
	// NYI
}

static void (*const funcs[])(void) = {
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
};

void oneshot_func_exec() {
	funcs[Game_Variables[ONESHOT_VAR_FUNC]]();
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
	if ((ending == ENDING_DEAD) && (!gameStarted)) {
		return "title_dead";
	} else if (ending == ENDING_TRAPPED) {
		return "title_stay";
	}
	return "title";
}
const char * oneshot_titlebgm() {
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
	// (Kind of relied upon anyway by oneshot_override_closing,
	//  because Scene_End always uses this no matter what.
	//  An idea would be to work on Scene_End to make it like Scene_OSMB, taking a const char * ptr.)
	const char * t = oneshot_closewindowprompt();
	if (t)
		return t;
	return "But you've only just started playing!";
}

// This function has the ability to stop the game from closing on request.
// Use with care.
int oneshot_override_closing() {
	if (Scene::Find(Scene::Title)) {
		const char * ccc = oneshot_closewindowprompt();
		if (ccc == STR_YOU_KILLED_NIKO) {
			Scene::PopUntil(Scene::Title);
			Scene::Pop();
			// Commence the guilt-tripping.
			Scene::Push(std::make_shared<Scene_OSMB>(ccc, false));
			// and do this while we're waiting.
			oneshot_fake_quit_handler();
			return 1;
		}
		if (ccc) {
			Scene::Push(std::make_shared<Scene_End>());
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
}