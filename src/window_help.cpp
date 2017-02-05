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
#include "window_help.h"
#include "bitmap.h"
#include "font.h"
#include "scene.h"

Window_Help::Window_Help(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight),
	align(Text::AlignLeft) {

	SetContents(Bitmap::Create(width - 16, height - 16));

	contents->Clear();
}

void Window_Help::SetText(std::string text,	Text::Alignment align) {
	if (this->text != text || this->align != align) {
		contents->Clear();

		this->text = text;
		this->align = align;

		// NOTE: OneShot-specific hackery. Who wrote the font colour selection system? Why?
		// In any case, this works around the fact that for some reason text goes black for no good reason,
		// on ENDING_DEAD. This way is arguably much better because it means the player sees the "you killed niko" screen.
		// Which they wouldn't get to before.
		if (Scene::Find(Scene::End)) {
			Color c(255, 255, 255, 255);
			contents->TextDraw(0, 2, c, text);
		} else {
			contents->TextDraw(0, 2, Font::ColorDefault, text, align);
		}
	}
}
