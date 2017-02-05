#include <stdlib.h>
#include "oneshot.h"
#include "oneshot_ser.h" // For serialization
#include "game_variables.h"
#include "game_switches.h"
#include "scene.h"
#include "scene_osmb.h" // messageboxes!
#include "scene_end.h" // alt-f4 override
#include "player.h" // For access to safe code

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

#define STR_ONESHOT                 "You only have one shot, %hs."
#define STR_DO_YOU_UNDERSTAND       "Do you understand what this means?"
#ifndef ENTITY_AWARE_OF_EMULATION
#define STR_STILL_HAVING_TROUBLE    "Still having trouble? Want me to spell it out for you?"
#else
// This isn't great but it'll help ease out the UX differences required.
// Further forks could make their own platform-specific ways of handling this.
// The current method is to show the safe code in the Game Browser.
#define STR_STILL_HAVING_TROUBLE    "Still having trouble? Maybe you should sleep for a bit...?"
#endif
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

#define ENDING_BEGINNING 0
#define ENDING_DEJAVU 1
#define ENDING_DEAD 2
#define ENDING_ESCAPED 3
#define ENDING_TRAPPED 4

#define QUIT_MESSAGE            "But you only just started playing!"
#define QUIT_MESSAGE_ONESHOT        "  Niko won't make it out alive."

static int oneshot = 0;
static int ending = ENDING_BEGINNING;

static int forceEnding = 0;

static int gameStarted = 0;
static int isInGame = 0;

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
	// (usually set to -1 on the line before the call here)
	Player::safe_code = 1;
}

void oneshot_func_init() {
	//puts("We are doing ONESHOT init!");
	for (int i = 0; i < 7; i++)
		Game_Variables[i + 1] = 0;
	// Unsure on this
	for (int i = 0; i < 20; i++)
		Game_Switches[i + 1] = false;
	// Computer-player-name-thing needs doing
	Game_Variables[ONESHOT_VAR_ENDING] = ending;
	Game_Variables[ONESHOT_VAR_GEORGE] = ((rand() & 0x7FFF) % 6) + 1; // not quite accurate but ok
	gameStarted = 1;
	isInGame = 1;
}

static void func_GuessName() {
	// NYI
}
static void func_SetName() {
	// NYI
}
static void func_SetNameEntry() {
	// NYI
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
		sprintf(buff, STR_ONESHOT, "<FIX>");
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
		util_messagebox(STR_STILL_HAVING_TROUBLE, "", MESSAGE_QUESTION);
		break;
	case 3:
#ifdef ABSOLUTELY_CHEAT_SINCE_WE_CANT_META
		sprintf(buff, "(It's %06d. Could've just slept, the Game Browser shows it)", Player::safe_code);
		util_messagebox(buff, "", MESSAGE_INFO);
#else
		util_messagebox(STR_SURELY_YOU_KNOW, "", MESSAGE_QUESTION);
#end
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
	oneshot_ser_saveBegin();
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
	Game_Variables[ONESHOT_VAR_RETURN] = oneshot_ser_loadBegin();
	oneshot = 1;
	util_updateQuitMessage();
	ending = ENDING_DEAD;
}
static void func_ReadItem() {
	if ((Game_Variables[ONESHOT_VAR_RETURN] = oneshot_ser_loadItem()) == 0) {
		// End of save.
		oneshot_ser_wipeSave();
	}
}
static void func_Document() {
	// you're welcome
	Player::safe_code = Game_Variables[ONESHOT_VAR_SAFE_CODE];
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
    func_GuessName,
    func_SetName,
    func_SetNameEntry,
    func_ShakeWindow,
    func_SetWallpaper,
    func_MessageBox,
    func_LeaveWindow,
    func_Save,
    func_WriteItem,
    func_Load,
    func_ReadItem,
    func_Document,
    func_End,
    func_SetCloseEnabled,
};

void oneshot_func_exec() {
	funcs[Game_Variables[ONESHOT_VAR_FUNC]]();
}

const char * oneshot_titlescreen() {
	//puts("TitleScreen hit");
	// Due to the incompatibility between OneShot's "the game closes",
	//  and EasyRPG's "back to menu",
	// this seems like the best solution until I have save/load implemented
	ending = util_loadEnding();
	if (ending <= ENDING_DEJAVU) {
		if (oneshot)
			ending = ENDING_DEAD;
		else if (gameStarted)
			ending = ENDING_DEJAVU;
		util_saveEnding();
	}
	isInGame = 0;
	oneshot = 0;
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
	// (Kind of relied upon anyway by oneshot_override_closing)
	const char * t = oneshot_closewindowprompt();
	if (t)
		return t;
	return "But you've only just started playing!";
}
int oneshot_override_closing() {
	if (Scene::Find(Scene::Title)) {
		const char * ccc = oneshot_closewindowprompt();
		if (ccc == STR_YOU_KILLED_NIKO) {
			Scene::PopUntil(Scene::Title);
			Scene::Pop();
			Scene::Push(std::make_shared<Scene_OSMB>(ccc, false));
			// Just do this now
			oneshot_fake_quit_handler();
			return 1;
		}
		if (ccc) {
			Scene::PopUntil(Scene::Title);
			Scene::Pop();
			Scene::Push(std::make_shared<Scene_End>());
			return 1;
		}
	}
	return 0;
}
void oneshot_fake_quit_handler() {
	// ---Pre-quit-stuffdoings (process detach handler)---
	// (Given this tends to accidentally prevent death from working: Does this even get run???)
	//util_saveEnding();
	// ---Post-quit-reset. This should reset everything as if the Player had been newly restarted.---
	oneshot = 0;
	ending = ENDING_BEGINNING;
	forceEnding = 0;
	gameStarted = 0;
	isInGame = 0;
	quitMsgPtr = QUIT_MESSAGE;
}