#ifndef DYENGINE_COLLISION_H
#define DYENGINE_COLLISION_H

#include "Vector2.h"
#include "Object.h"

using namespace std;

class Object;

struct Collision
{
	Object* collider = nullptr;
	VECTOR::vector2f hitPoint;
	VECTOR::vector2f hitNormal;
};

#endif