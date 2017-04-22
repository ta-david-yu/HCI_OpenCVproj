#ifndef DYENGINE_SHAPE_H
#define DYENGINE_SHAPE_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <memory>
#include <vector>
#include <cmath>
#include "Vector2.h"
#include "Collision.h"

using namespace std;
using namespace cv;

class Circle;
class Rectangle;

class Shape
{
public:
	VECTOR::vector2f Offset;
	virtual void SetSize(const Vec2f& size) {}
	virtual int Area() { return 0; }
	virtual int Perimeter() { return 0; }
	virtual bool CheckOverlay(const Vec2i& pointTBChecked, const Vec2i& mCenter) { return false; }
	virtual bool CheckOverlayWithRect(const Rectangle& rect, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision) { return false; }
	virtual bool CheckOverlayWithCircle(const Circle& circle, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision) { return false; }
	virtual bool CheckIfHitByRect(const Rectangle& circle, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision) { return false; }
	virtual bool CheckIfHitByCircle(const Circle& circle, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision) { return false; }
	template<class ShapeType>
	ShapeType* IdentifyShapePtr()
	{
		return dynamic_cast<ShapeType*>(this);
	}
};

class Circle : public Shape
{
public:
	float Radius;

	Circle(float radius) : Radius(radius) {}

	void SetSize(const Vec2f& size) override
	{
		Radius = (size[0] > size[1])? size[1]/2 : size[0]/2;
	}

	int Area() override
	{
		float rad = (float)Radius - .5f;
		return ceil(rad * rad * 3.14f);
	}

	int Perimeter() override
	{
		return 2.0f * 3.14f * Radius;
	}

	bool CheckOverlay(const Vec2i& pointTBChecked, const Vec2i& mCenter) override
	{
		int hammingDis = abs(pointTBChecked[0] - mCenter[0]) + abs(pointTBChecked[1] - mCenter[1]);
		return (hammingDis <= Radius);
	}

	bool CheckOverlayWithRect(const Rectangle& rect, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision) override;
	bool CheckOverlayWithCircle(const Circle& circle, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision) override;

	bool CheckIfHitByRect(const Rectangle& circle, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision) override;
	bool CheckIfHitByCircle(const Circle& circle, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision) override;
};

class Rectangle : public Shape
{
public:
	float Width;
	float Height;

	Rectangle(float width, float height) : Width(width), Height(height) {}

	void SetSize(const Vec2f& size) override
	{
		Width = size[0];
		Height = size[1];
	}

	int Area() override
	{
		return Width * Height;
	}

	int Perimeter() override
	{
		return (Width + Height) * 2 - 4;
	}

	bool CheckOverlay(const Vec2i& pointTBChecked, const Vec2i& mCenter) override
	{
		int diffX = abs(pointTBChecked[0] - mCenter[0]);
		int diffY = abs(pointTBChecked[1] - mCenter[1]);
		return (diffX <= Width) && (diffY <= Height);
	}

	bool CheckOverlayWithRect(const Rectangle& rect, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision) override;
	bool CheckOverlayWithCircle(const Circle& circle, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision) override;

	bool CheckIfHitByRect(const Rectangle& circle, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision) override;
	bool CheckIfHitByCircle(const Circle& circle, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision) override;
};

#endif