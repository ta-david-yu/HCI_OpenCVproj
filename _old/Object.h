
#ifndef DYENGINE_OBJECT_H
#define DYENGINE_OBJECT_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <memory>
#include <vector>
#include <cmath>
#include <iostream>
#include "Component.h"
#include "ObjectManager.h"
#include "Shape.h"

using namespace std;
using namespace cv;

class Shape;
class ObjectManager;
class Component;

enum ColliderType
{
	Collider,
	Trigger,
	None
};

class Object
{

public:
	ObjectManager* ObjManager = nullptr;
	vector<unique_ptr<Component>> components;

	Vec2f Position;
	float Rotation;			


	Vec2f ColliderScale;		// legacy
	ColliderType ColType;		// Collider, Trigger, or None
	shared_ptr<Shape> ColShape; // Rect, Circle

	bool IsActive = true;


private:
	Vec2i m_NativeSize;			// NativeSize is the size to Image Set
	Vec2f Scale;				// Scale of the object, real size is Scale * NativeSize

public:

	Object(ObjectManager* objMgr);
	Object(const Object& other, ObjectManager* objMgr);
	Object(const Vec2f& pos, int shapeType, ObjectManager* objMgr);
	~Object();

	Vec2i GetNativeSize();
	void SetNativeSize(const Vec2i& size);

	virtual void Start();
	virtual void Update(float timeStep);					// called every frame for game loop
	virtual void LateUpdate(float timeStep);				// called every frame for game loop after Update being called
	virtual void OnCollision(Collision& col);				// called on collision if this is a collider
	virtual void OnTrigger(Collision& col);					// called on collision if this is a trigger

	template<class ComponentName, class...Parameters>
	Component* AddComponent(Parameters&&... param)	// add new component to this object
	{
		components.push_back( make_unique<ComponentName>(std::forward<Parameters>(param)...) );
		components[components.size()-1]->AttachTo(this);

		return components[components.size() - 1].get();
	}

	template <class ComponentName>							// get certain type of component attached to this object
	ComponentName* GetComponent()
	{
		const type_info& desiredCompID = typeid(ComponentName);

		for (auto &compPtr : components)
		{
			const type_info& compID = typeid(*compPtr);
			if (compID == desiredCompID)
				return static_cast<ComponentName*>(compPtr.get());
		}
		
		return nullptr;
	}
};

class DynamicObject : public Object
{
public:
	DynamicObject(ObjectManager* objMgr);
	DynamicObject(const DynamicObject& other, ObjectManager* objMgr);
	DynamicObject(const Object& other, ObjectManager* objMgr);
	DynamicObject(const Vec2f& pos, const Vec2f& vec, int shapeType, ObjectManager* objMgr);

	Vec2i Velocity;
	void Update(float timeStep) override;
	void OnCollision(Collision& col) override;
	void OnTrigger(Collision& col) override;
};

#endif