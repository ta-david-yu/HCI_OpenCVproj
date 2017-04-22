#include "Object.h"

using namespace std;
using namespace cv;

Object::Object(ObjectManager* objMgr)
{
	ObjManager = objMgr;
	ColShape = nullptr;
}

Object::Object(const Object& other, ObjectManager* objMgr) :
Position(other.Position),
Rotation(other.Rotation),
Scale(other.Scale),
ColType(other.ColType)
{
	ObjManager = objMgr;
	shared_ptr<Shape> colShape = make_shared<Shape>(*other.ColShape);
	ColShape = colShape;
}

Object::Object(const Vec2f& pos, int shapeType, ObjectManager* objMgr) :
Position(pos),
Rotation(0),
Scale(Vec2f(1, 1)),
m_NativeSize(Vec2i(1, 1)),
ColliderScale(Vec2f(1, 1)),
ColType(Trigger)
{
	ObjManager = objMgr;
	if (shapeType == 0)
		ColShape = make_shared<Rectangle>(1, 1);
	else
		ColShape = make_shared<Circle>(1);
}

Object::~Object() {}

void Object::Start()
{
	for (auto &compPtr : components)
	{
		if (compPtr->IsEnabled) compPtr->Start();
	}
}

void Object::Update(float timeStep)
{
	for (auto &compPtr : components)
	{
		if (compPtr->IsEnabled) compPtr->Update(timeStep);
	}
}

void Object::LateUpdate(float timeStep)
{
	for (auto &compPtr : components)
	{
		if (compPtr->IsEnabled) compPtr->LateUpdate(timeStep);
	}
}

void Object::OnCollision(Collision& col)
{
	for (auto &compPtr : components)
	{
		if (compPtr->IsEnabled) compPtr->OnCollision(col);
	}
}

void Object::OnTrigger(Collision& col)
{
	for (auto &compPtr : components)
	{
		if (compPtr->IsEnabled) compPtr->OnTrigger(col);
	}
}


void Object::SetNativeSize(const Vec2i& size)
{
	m_NativeSize = size;
	ColShape->SetSize(Vec2f(size[0] * Scale[0], size[1] * Scale[1]));
	std::cout << "RADIUS: " << dynamic_cast<Circle*>(ColShape.get())->Radius << std::endl;
}

Vec2i Object::GetNativeSize()
{
	return m_NativeSize;
}

DynamicObject::DynamicObject(ObjectManager* objMgr) : Object(objMgr) {}

DynamicObject::DynamicObject(const DynamicObject& other, ObjectManager* objMgr) :
Object(other, objMgr),
Velocity(other.Velocity)
{

}

DynamicObject::DynamicObject(const Object& other, ObjectManager* objMgr) :
Object(other, objMgr),
Velocity(Vec2i(0, 0))
{

}

DynamicObject::DynamicObject(const Vec2f& pos, const Vec2f& vec, int shapeType, ObjectManager* objMgr) :
Object(pos, shapeType, objMgr),
Velocity(vec)
{
	
}


void DynamicObject::Update(float timeStep)
{
	///////////////////////////////////
	Object::Update(timeStep);

	cv::Vec2f oldPos = Position;
	Position = Vec2f(Position[0] + Velocity[0] * timeStep, Position[1] + Velocity[1] * timeStep);

	// TO DO: check collision, check collision, call OnCollision if triggerred
	/*
	if (this->ColType != None)
	{
		bool isCollided = false;
		vector<Collision> hitList = ObjManager->CheckCollision(*this);
		for (auto& col : hitList)
		{
			// TO DO: collision happen only when normal and velocity dot product <= 0
			if (this->ColType == Collider)
			{
				if (VECTOR::vector2f(Velocity).dotProduct(col.hitNormal) < 0)
				{
					OnCollision(col);
					isCollided = true;
					Position = oldPos;
				}
			}
			else if (this->ColType == Trigger)
			{
				OnTrigger(col);
			}
		}
	}*/
}

void DynamicObject::OnCollision(Collision& col)
{
	Object::OnCollision(col);

	// TO DO: change velocity based on hit normal
	std::cout << "Collide: Original Vec:" << Velocity[0] << ":" << Velocity[1];

	VECTOR::vector2f inVec(Velocity);
	inVec.reflect(col.hitNormal);
	Velocity = cv::Vec2f(inVec);

	std::cout << " Reflected Vec:" << Velocity[0] << ":" << Velocity[1] << std::endl;
}

void DynamicObject::OnTrigger(Collision& col)
{
	Object::OnTrigger(col);
}