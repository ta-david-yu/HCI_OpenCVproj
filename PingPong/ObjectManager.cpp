
#include "ObjectManager.h"

using namespace std;
using namespace cv;

void ObjectManager::ReleaseObjects()
{
	cout << "Releasing Objects" << endl;
	StaticObjectsList.clear();
	DynamicObjectsList.clear();
}

Object* ObjectManager::CreateStaticObject()
{
	StaticObjectsList.push_back(make_unique<Object>(this));
	StaticObjectsList[StaticObjectsList.size() - 1].get()->Identity = StaticObjectsList.size() - 1;
	return StaticObjectsList[StaticObjectsList.size() - 1].get();
}
Object* ObjectManager::CreateStaticObject(const Object& other)
{
	StaticObjectsList.push_back(make_unique<Object>(other, this));
	StaticObjectsList[StaticObjectsList.size() - 1].get()->Identity = StaticObjectsList.size() - 1;
	return StaticObjectsList[StaticObjectsList.size() - 1].get();
}
Object* ObjectManager::CreateStaticObject(const Vec2f& pos, int shapeType = 0)
{
	StaticObjectsList.push_back(make_unique<Object>(pos, shapeType, this));
	StaticObjectsList[StaticObjectsList.size() - 1].get()->Identity = StaticObjectsList.size() - 1;
	return StaticObjectsList[StaticObjectsList.size() - 1].get();
}

DynamicObject* ObjectManager::CreateDynamicObject()
{
	DynamicObjectsList.push_back(make_unique<DynamicObject>(this));
	DynamicObject* obj = DynamicObjectsList[DynamicObjectsList.size() - 1].get();
	obj->Identity = DynamicObjectsList.size() - 1;
	obj->IsDynamic = true;
	return obj;

}
DynamicObject* ObjectManager::CreateDynamicObject(const DynamicObject& other)
{
	DynamicObjectsList.push_back(make_unique<DynamicObject>(other, this));
	DynamicObject* obj = DynamicObjectsList[DynamicObjectsList.size() - 1].get();
	obj->Identity = DynamicObjectsList.size() - 1;
	obj->IsDynamic = true;
	return obj;
}
DynamicObject* ObjectManager::CreateDynamicObject(const Object& other)
{
	DynamicObjectsList.push_back(make_unique<DynamicObject>(other, this));
	DynamicObject* obj = DynamicObjectsList[DynamicObjectsList.size() - 1].get();
	obj->Identity = DynamicObjectsList.size() - 1;
	obj->IsDynamic = true;
	return obj;
}
DynamicObject* ObjectManager::CreateDynamicObject(const Vec2f& pos, const Vec2f &vec, int shapeType)
{
	DynamicObjectsList.push_back(make_unique<DynamicObject>(pos, vec, shapeType, this));
	DynamicObject* obj = DynamicObjectsList[DynamicObjectsList.size() - 1].get();
	obj->Identity = DynamicObjectsList.size() - 1;
	obj->IsDynamic = true;
	return obj;
}

Object* ObjectManager::FindObject(std::string name)
{
	for (auto &obj : StaticObjectsList)
	{
		if (obj->Name == name)
			return obj.get();
	}
	for (auto &obj : DynamicObjectsList)
	{
		if (obj->Name == name)
			return obj.get();
	}
	return nullptr;
}

Object* ObjectManager::FindStaticObject(std::string name)
{
	for (auto &obj : StaticObjectsList)
	{
		if (obj->Name == name)
			return obj.get();
	}
	return nullptr;
}

DynamicObject* ObjectManager::FindDynamicObject(std::string name)
{
	for (auto &obj : DynamicObjectsList)
	{
		if (obj->Name == name)
			return obj.get();
	}
	return nullptr;
}

void ObjectManager::DestroyObject(Object* obj)
{
	ObjectManager* objMgr = obj->ObjManager;
	if (obj->IsDynamic)
	{
		objMgr->DynamicObjectsList.erase(objMgr->DynamicObjectsList.begin() + obj->Identity);
	}
	else
	{
		objMgr->StaticObjectsList.erase(objMgr->StaticObjectsList.begin() + obj->Identity);
	}
}

void ObjectManager::Start()
{
	/*
	for (auto &obj : StaticObjectsList)
	{
		if (obj->IsActive()) obj->Start();
	}
	for (auto &obj : DynamicObjectsList)
	{
		if (obj->IsActive()) obj->Start();
	}
	*/
}

void ObjectManager::Update(float timeStep)
{
	for (auto &obj : StaticObjectsList)
	{
		if (obj->IsActive()) obj->Update(timeStep);
	}
	for (auto &obj : DynamicObjectsList)
	{
		if (obj->IsActive()) obj->Update(timeStep);
	}
}

void ObjectManager::LateUpdate(float timeStep)
{
	for (auto &obj : StaticObjectsList)
	{
		if (obj->IsActive()) obj->LateUpdate(timeStep);
	}
	for (auto &obj : DynamicObjectsList)
	{
		if (obj->IsActive()) obj->LateUpdate(timeStep);
	}
}

void ObjectManager::CheckCollision(DynamicObject& obj)
{
	
	for (auto &otherObj : StaticObjectsList)
	{
		if (otherObj->IsActive() && otherObj->ColType != None)
		{
			Collision coll;
			coll.collider = &obj;

			Vec2f& otherPos = otherObj->Position();
			// TO DO: fix structure, currently only with circle
			if (otherObj->ColShape->IdentifyShapePtr<Circle>() != nullptr)
			{
				if (obj.ColShape->CheckOverlayWithCircle(*dynamic_cast<Circle*>(otherObj->ColShape.get()), otherPos, obj.Position(), coll))
				{
					// add the collision to list
					obj.OnCollision(coll);
				}
			}
		}
	}

	for (auto &otherObj : DynamicObjectsList)
	{
		// escape collision checking with itself
		if (otherObj.get() == &obj)
			continue;

		if (otherObj->IsActive() && otherObj->ColType != None)
		{
			Collision coll;
			coll.collider = &obj;

			Vec2f& otherPos = otherObj->Position();
			// TO DO: fix structure, currently only with circle
			if (otherObj->ColShape->IdentifyShapePtr<Circle>() != nullptr)
			{
				if (obj.ColShape->
					CheckOverlayWithCircle(*dynamic_cast<Circle*>(otherObj->ColShape.get()), otherPos, obj.Position(), coll))
				{
					// add the collision to list
					obj.OnCollision(coll);
				}
			}
		}
	}
	
}