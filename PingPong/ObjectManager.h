
#ifndef DYENGINE_OBJECT_MANAGER_H
#define DYENGINE_OBJECT_MANAGER_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <memory>
#include <vector>
#include <list>
#include <cmath>
#include <iostream>
#include "Component.h"
#include "Object.h"
#include "Shape.h"
#include "Collision.h"

using namespace std;
using namespace cv;

class Scene;
class Component;
class Object;
class DynamicObject;
class SceneManager;
class Collision;

class ObjectManager
{
public: 
	//! Scene that manages this objMgr
	Scene* m_Scene;

	vector<unique_ptr<Object>> StaticObjectsList;
	vector<unique_ptr<DynamicObject>> DynamicObjectsList;

	//! Delete all obj managed by this manager
	void ReleaseObjects();

	//! Create Object
	Object* CreateStaticObject();
	Object* CreateStaticObject(const Object& other);
	Object* CreateStaticObject(const Vec2f& pos, int shapeType);

	//! Create DynamicObject
	DynamicObject* CreateDynamicObject();
	DynamicObject* CreateDynamicObject(const DynamicObject& other);
	DynamicObject* CreateDynamicObject(const Object& other);
	DynamicObject* CreateDynamicObject(const Vec2f& pos, const Vec2f& vec, int shapeType);

	//! Find Object with Name
	Object* FindObject(std::string name);
	Object* FindStaticObject(std::string name);
	DynamicObject* FindDynamicObject(std::string name);

	// Delete Object with Pointer
	static void DestroyObject(Object* obj);

	// Start -> Update -> LateUpdate

	//! Useless Currently
	void Start();
	//! Update object if active
	void Update(float timeStep);
	//! LateUpdate object if active
	void LateUpdate(float timeStep);
	//! Useless Currently
	void CheckCollision(DynamicObject& obj);
};


#endif