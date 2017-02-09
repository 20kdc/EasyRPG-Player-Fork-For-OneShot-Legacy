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

#ifndef _SCENE_OSMB_H_
#define _SCENE_OSMB_H_

// Headers
#include "scene.h"
#include "window_command.h"
#include "window_help.h"
#include "async_handler.h"

/**
 * Scene OSMB class.
 * Used by OneShot as a replacement for message boxes, for lack of a better way.
 */
class Scene_OSMB : public Scene {

public:
	/**
	 * Constructor.
	 */
	Scene_OSMB(const char * s, const char * s2, int yn, std::shared_ptr<Scene> const& next_scene);

	void Start() override;
	void Update() override;

	/**
	 * Creates the Window displaying the yes and no option.
	 */
	void CreateCommandWindow();

	/**
	 * Creates the Window displaying the confirmation
	 * text.
	 */
	void CreateHelpWindow();

private:
	/** Help window showing the confirmation text. */
	std::unique_ptr<Window_Help> help_window;
	/** Command window containing the yes and no option. */
	std::unique_ptr<Window_Command> command_window;
	FileRequestBinding request_id;
	std::string str, str2;
	int yesNo;
	void * tryPtr;
	// These 3 variables are safety checks to prevent pretty major issues
	bool hasInputControl;
	bool answered;
	int creationMBC;
	// Used to chain scenes without the danger of 2-pushes-1-frame
	std::shared_ptr<Scene> nextBox;

	void OnWindowskinReady(FileRequestResult* result);
	void FinishMsgbox();
};

#endif
