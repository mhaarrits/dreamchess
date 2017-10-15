/*  DreamChess
**
**  DreamChess is the legal property of its developers, whose names are too
**  numerous to list here. Please refer to the COPYRIGHT file distributed
**  with this source distribution.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DREAMCHESS_SCENE_H
#define DREAMCHESS_SCENE_H

#include <vector>

class DreamChess;
class Object;

class Scene {
public:
	Scene(DreamChess *d) {_game = d; _is3D = false;}
	DreamChess *getGame() {return _game;}
	void addObject(Object *o);
	void removeObject(Object *o);
	int getObjectCount() {return _objects.size();}

	void render();

	virtual void init() = 0;
	virtual void update() = 0;
protected:
	bool _is3D;
	DreamChess *_game;
	std::vector<Object*> _objects;
};

#endif
