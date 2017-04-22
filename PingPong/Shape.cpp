#include "Shape.h"

bool Circle::CheckOverlayWithRect(const Rectangle& rect, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision)
{
	// to be fixed
	VECTOR::vector2f tCenter(mCenter);
	tCenter += Offset;
	VECTOR::vector2f oCenter(otherCenter);

	// First check if the circle is inside the rectangle
	// TO DO

	// Check if intersected
	vector<VECTOR::vector2f> points;
	points.push_back(VECTOR::vector2f(oCenter + VECTOR::vector2f(rect.Width / 2.0f, 0) + VECTOR::vector2f(0, rect.Height / 2.0f)));
	points.push_back(VECTOR::vector2f(oCenter - VECTOR::vector2f(rect.Width / 2.0f, 0) + VECTOR::vector2f(0, rect.Height / 2.0f)));
	points.push_back(VECTOR::vector2f(oCenter - VECTOR::vector2f(0, rect.Height / 2.0f) - VECTOR::vector2f(0, rect.Height / 2.0f)));
	points.push_back(VECTOR::vector2f(oCenter + VECTOR::vector2f(0, rect.Height / 2.0f) - VECTOR::vector2f(0, rect.Height / 2.0f)));

	int counter = 0;
	for (int i = 0; i < 4; i++)
	{
		// compute distance from circle center to four lines
		VECTOR::vector2f begin = points[i];
		VECTOR::vector2f end = (i + 1 < 3) ? points[i + 1] : points[0];
		VECTOR::vector2f center = tCenter;

		float distance = center.distanceToLine(begin, end);

		if (distance < Radius)
			counter++;
	}

	return (counter >= 2);
}

bool Circle::CheckOverlayWithCircle(const Circle& circle, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision)
{
	VECTOR::vector2f tCenter(mCenter);
	VECTOR::vector2f oCenter(otherCenter);

	VECTOR::vector2f diff = oCenter - tCenter;
	float magnitude = diff.length();

	// inside
	if (magnitude < circle.Radius + Radius)
	{
		VECTOR::vector2f meanCenter = tCenter + (diff) / 2;
		collision.hitPoint = meanCenter;
		collision.hitNormal = -diff;
		return true;
	}
	else
	{
		return false;
	}
}

bool Circle::CheckIfHitByRect(const Rectangle& rect, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision)
{
	return false;
}

bool Circle::CheckIfHitByCircle(const Circle& circle, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision)
{
	VECTOR::vector2f tCenter(mCenter);
	tCenter += Offset;
	VECTOR::vector2f oCenter(otherCenter);

	VECTOR::vector2f diff = oCenter - tCenter;
	float magnitude = diff.length();

	// inside
	if (magnitude < circle.Radius + Radius)
	{
		VECTOR::vector2f meanCenter = tCenter + (diff) / 2;
		collision.hitPoint = meanCenter;
		collision.hitNormal = diff;
		return true;
	}
	else
	{
		return false;
	}
}

// TO DO
bool Rectangle::CheckOverlayWithRect(const Rectangle& rect, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision)
{
	return false;
}

bool Rectangle::CheckOverlayWithCircle(const Circle& circle, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision)
{
	return false;
}


bool Rectangle::CheckIfHitByRect(const Rectangle& rect, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision)
{
	return false;
}

bool Rectangle::CheckIfHitByCircle(const Circle& circle, const Vec2f& otherCenter, const Vec2f& mCenter, Collision& collision)
{
	return false;
}