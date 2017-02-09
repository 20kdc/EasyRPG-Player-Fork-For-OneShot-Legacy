/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

// Headers
#include <vector>
#include "audio.h"
#include "baseui.h"
#include "cache.h"
#include "game_system.h"
#include "game_variables.h"
#include "game_map.h"
#include "input.h"
#include "scene_osmb.h"
#include "scene_menu.h"
#include "scene_title.h"
#include "util_macro.h"
#include "bitmap.h"
#include "oneshot.h"
#include "output.h"

Scene_OSMB::Scene_OSMB(const char * text, bool yn, std::shared_ptr<Scene> const& next_scene) {
	Scene::type = Scene::End;
	str = text;
	yesNo = yn;
	nextBox = next_scene;
	hasInputControl = false;
	creationMBC = ++oneshot_global_messagebox_count;
	answered = false;
}

void Scene_OSMB::Start() {
	CreateCommandWindow();
	CreateHelpWindow();
	FileRequestAsync* request = AsyncHandler::RequestFile("System", "title");
	request_id = request->Bind(&Scene_OSMB::OnWindowskinReady, this);
	request->Start();
}

void Scene_OSMB::OnWindowskinReady(FileRequestResult* result) {
	help_window->ForceWindowskin(result->file);
	command_window->ForceWindowskin(result->file);
}

void Scene_OSMB::Update() {
	command_window->Update();

	if (answered)
		return;

	bool topStack = oneshot_global_messagebox_count == creationMBC;
	if (!topStack)
		hasInputControl = false;
	if (!hasInputControl) {
		if (topStack)
			hasInputControl = true;
		return;
	}

	if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		switch (command_window->GetIndex()) {
		case 0: // Yes
			Game_Variables[ONESHOT_VAR_RETURN] = 1;
			break;
		case 1: // No
			Game_Variables[ONESHOT_VAR_RETURN] = 0;
		}
		Game_Map::SetNeedRefresh(Game_Map::Refresh_All);
		oneshot_global_messagebox_count--;
		answered = true;
		if (nextBox) {
			// This may have the side effect of destroying this object.
			// (The worrying thing is that I'm not sure *when*.)
			Scene::Push(nextBox, true);
			return;
		} else {
			if (oneshot_global_messagebox_count)
				Output::Warning("Messagebox-count inconsistent with scene linked list.");
			Scene::Pop();
		}
	}
}

void Scene_OSMB::CreateCommandWindow() {
	// Create Options Window
	std::vector<std::string> options;
	options.push_back(Data::terms.yes);
	if (yesNo)
		options.push_back(Data::terms.no);

	command_window.reset(new Window_Command(options));
	command_window->SetX((SCREEN_TARGET_WIDTH/2) - command_window->GetWidth() / 2);
	command_window->SetY(72 + 48);
	command_window->SetIndex(1);
}

void Scene_OSMB::CreateHelpWindow() {
	Rect text_size = Text::GetSize(Font::Default(), str);
	// 32 + 72 = 104
	// 14 + 18 = 32
	int h = text_size.height + 18;
	
	help_window.reset(new Window_Help((SCREEN_TARGET_WIDTH/2) - (text_size.width + 16) / 2,
									  104 - h, text_size.width + 16, h));
	help_window->SetText(str);

	command_window->SetHelpWindow(help_window.get());
}
