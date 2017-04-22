#include "Object.h"

using namespace std;
using namespace cv;

Object::Object(ObjectManager* objMgr)
{
	ObjManager = objMgr;
	ColShape = nullptr;
}

Object::Object(const Object& other, ObjectManager* objMgr) :
m_LocalPosition(other.m_LocalPosition),
Rotation(other.Rotation),
m_Scale(other.m_Scale),
ColType(other.ColType)
{
	ObjManager = objMgr;
	ColShape = make_unique<Shape>(*other.ColShape);
}

Object::Object(const Vec2f& pos, int shapeType, ObjectManager* objMgr) :
m_LocalPosition(pos),
Rotation(0),
m_Scale(Vec2f(1, 1)),
m_NativeSize(Vec2i(1, 1)),
ColliderScale(Vec2f(1, 1)),
ColType(Trigger)
{
	ObjManager = objMgr;
	
	if (shapeType == 0)
		ColShape = make_unique<Rectangle>(1.0f, 1.0f);
	else
		ColShape = make_unique<Circle>(1.0f);
}

Object::~Object()
{
	components.clear();
}

bool Object::IsActive()
{
	if (Parent != nullptr)
		return m_IsActive && Parent->IsActive();
	else
		return m_IsActive;
}


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
	ColShape->SetSize(Vec2f(size[0] * m_Scale[0], size[1] * m_Scale[1]));
	// std::cout << "RADIUS: " << dynamic_cast<Circle*>(ColShape.get())->Radius << std::endl;
}

void Object::SetScale(const Vec2f &_scale)
{
	m_Scale = _scale;
	ColShape->SetSize(Vec2f(m_NativeSize[0] * _scale[0], m_NativeSize[1] * _scale[1]));
}

Vec2i Object::GetNativeSize()
{
	return m_NativeSize;
}

DynamicObject::DynamicObject(ObjectManager* objMgr) : Object(objMgr) {}

DynamicObject::DynamicObject(const DynamicObject& other, ObjectManager* objMgr) :
Object(other, objMgr)
{

}

DynamicObject::DynamicObject(const Object& other, ObjectManager* objMgr) :
Object(other, objMgr)
{

}

DynamicObject::DynamicObject(const Vec2f& pos, const Vec2f& vec, int shapeType, ObjectManager* objMgr) :
Object(pos, shapeType, objMgr)
{
	Velocity = vec;
}


void DynamicObject::Update(float timeStep)
{
	Object::Update(timeStep);

	cv::Vec2f oldPos = m_LocalPosition;
	m_LocalPosition = Vec2f(m_LocalPosition[0] + Velocity[0] * timeStep, m_LocalPosition[1] + Velocity[1] * timeStep);
}

void DynamicObject::OnCollision(Collision& col)
{
	Object::OnCollision(col);
}

void DynamicObject::OnTrigger(Collision& col)
{
	Object::OnTrigger(col);
}