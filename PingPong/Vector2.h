/****************************************
* By Will Perone
* Original: 9-16-2002
* Revised: 19-11-2003
*          18-12-2003
*          06-06-2004
****************************************/

#ifndef DYENGINE_VECTOR2_H
#define DYENGINE_VECTOR2_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h> 

namespace VECTOR
{

	template <typename T>
	struct vector2
	{
		T x, y;

		//! trivial ctor
		vector2<T>() {}

		//! setting ctor
		vector2<T>(const T x0, const T y0) : x(x0), y(y0) {}

		//! array indexing
		T &operator [](unsigned int i)
		{
			return *(&x + i);
		}

		//! array indexing
		const T &operator [](unsigned int i) const
		{
			return *(&x + i);
		}

		//! function call operator
		void operator ()(const T x0, const T y0)
		{
			x = x0; y = y0;
		}

		//! test for equality
		bool operator==(const vector2<T> &v)
		{
			return (x == v.x && y == v.y);
		}

		//! test for inequality
		bool operator!=(const vector2<T> &v)
		{
			return (x != v.x || y != v.y);
		}

		//! set to value
		const vector2<T> &operator =(const vector2<T> &v)
		{
			x = v.x; y = v.y;
			return *this;
		}

		//! negation
		const vector2<T> operator -(void) const
		{
			return vector2<T>(-x, -y);
		}

		//! addition
		const vector2<T> operator +(const vector2<T> &v) const
		{
			return vector2<T>(x + v.x, y + v.y);
		}

		//! subtraction
		const vector2<T> operator -(const vector2<T> &v) const
		{
			return vector2<T>(x - v.x, y - v.y);
		}

		//! uniform scaling
		const vector2<T> operator *(const T num) const
		{
			vector2<T> temp(*this);
			return temp *= num;
		}

		//! uniform scaling
		const vector2<T> operator /(const T num) const
		{
			vector2<T> temp(*this);
			return temp /= num;
		}

		//! addition
		const vector2<T> &operator +=(const vector2<T> &v)
		{
			x += v.x;	y += v.y;
			return *this;
		}

		//! subtraction
		const vector2<T> &operator -=(const vector2<T> &v)
		{
			x -= v.x;	y -= v.y;
			return *this;
		}

		//! uniform scaling
		const vector2<T> &operator *=(const T num)
		{
			x *= num;	y *= num;
			return *this;
		}

		//! uniform scaling
		const vector2<T> &operator /=(const T num)
		{
			x /= num;	y /= num;
			return *this;
		}

		//! dot product
		T operator *(const vector2<T> &v) const
		{
			return x*v.x + y*v.y;
		}
	};


	struct vector2i : public vector2<int>
	{
		vector2i() {}
		vector2i(const vector2<int> &v) : vector2<int>(v.x, v.y) {}
		vector2i(int x0, int y0) : vector2<int>(x0, y0) {}
		vector2i(const cv::Vec2i& vec) : vector2<int>(vec[0], vec[1]) {}
		operator cv::Vec2i() const { return cv::Vec2i(x, y); }
	};


	struct vector2ui : public vector2<unsigned int>
	{
		vector2ui() {}
		vector2ui(const vector2<unsigned int> &v) : vector2<unsigned int>(v.x, v.y) {}
		vector2ui(unsigned int x0, unsigned int y0) : vector2<unsigned int>(x0, y0) {}
	};


	struct vector2f : public vector2<float>
	{
		vector2f() {}
		vector2f(const vector2<float> &v) : vector2<float>(v.x, v.y) {}
		vector2f(float x0, float y0) : vector2<float>(x0, y0) {}
		vector2f(const cv::Vec2f& vec) : vector2<float>(vec[0], vec[1]) {}
		vector2f(const cv::Point2f& vec) : vector2<float>(vec.x, vec.y) {}

		//! gets the length of this vector squared
		float length_squared() const
		{
			return (float)(*this * *this);
		}

		//! gets the length of this vector
		float length() const
		{
			return (float)sqrt(*this * *this);
		}

		//! normalizes this vector
		void normalize()
		{
			*this /= length();
		}

		//! returns the normalized vector
		vector2f normalized() const
		{
			return  *this / length();
		}

		//! reflects this vector about n
		void reflect(const vector2f &n)
		{
			vector2f orig(*this);
			project(n);
			*this = *this * 2 - orig;
		}

		//! projects this vector onto v
		void project(const vector2f &v)
		{
			*this = v * (*this * v) / (v*v);
		}

		//! returns this vector projected onto v
		vector2f projected(const vector2f &v)
		{
			return v * (*this * v) / (v*v);
		}

		double dotProduct(const vector2f &v) const
		{
			return v.x * x + v.y * y;
		}

		double cross(const vector2f &v)
		{
			return this->x * v.y - v.x * this->y;
		}

		double distanceToLine(const vector2f &begin, const vector2f &end)
		{
			// compute distance from circle center to four lines
			VECTOR::vector2f tbegin = begin;
			VECTOR::vector2f tend = end;
			VECTOR::vector2f center = *this;

			tend -= tbegin;
			center -= tbegin;

			double area = center.cross(tend);
			return 2.0f * area / tend.length();
		}

		vector2f truncate(float _length)
		{
			return (length() > _length) ? normalized() * length() : *this;
		}

		vector2f clamp(float _min, float _max)
		{
			if (length() > _max)
				return normalized() * _max;
			else if (length() < _min)
				return normalized() * _min;
			else
				return *this;
		}

		vector2f toMatSpace(int height)
		{
			return vector2f(x, height - y - 1);
		}

		//! computes the value of line equation form by p1 and p2 on _p
		static float evaluateEquation(const VECTOR::vector2f& _p, const VECTOR::vector2f& p1, const VECTOR::vector2f& p2)
		{
			return (p2.y - p1.y) * (_p.x - p1.x) - (p2.x - p1.x) * (_p.y - p1.y);
		}

		//! computes the value of line equation form by p1 and p2 on _p and identitfy it is at the right or left space
		//! true if right, false is left
		static bool evaluatePointSpace(const VECTOR::vector2f& _p, const VECTOR::vector2f& p1, const VECTOR::vector2f& p2)
		{
			float deltaX = p1.x - p2.x;
			float deltaY = p1.y - p2.y;

			// vertical, coord x is the answer
			if (deltaY - 0.0f < 0.01f)
				return (_p.x > 0.0f);
			// horizontal, coord y is the answer
			else if (deltaX - 0.0f < 0.01f)
				return (_p.y > 0.0f);

			float slope = deltaY / deltaX;
			float value = evaluateEquation(_p, p1, p2);

			if (slope > 0)
			{
				return (value > 0);
			}
			else
			{
				return (value < 0);
			}
		}

		//dot = x1*x2 + y1*y2      # dot product
		//	det = x1*y2 - y1*x2      # determinant
		//	angle = atan2(det, dot)  # atan2(y, x) or atan2(sin, cos)

		//! computes the angle between 2 arbitrary vectors
		static inline float angle(const vector2f &v1, const vector2f &v2)
		{
			float dot = v1.dotProduct(v2);
			float det = v2.x * v1.y - v2.y * v1.x;
			return atan2(det, dot);
		}

		//! computes the angle between 2 normalized arbitrary vectors
		static inline float angle_normalized(const vector2f &v1, const vector2f &v2)
		{
			return acosf(v1*v2);
		}

		static inline vector2f lerp(const vector2f & start, const vector2f & end, float interval)
		{
			if (interval > 1.0f)
				interval = 1.0f;
			else if (interval < 0.0f)
				interval = 0.0f;

			return (start * (1.0f - interval) + end * interval);
		}

		static vector2f rotate(const vector2f & vec, float degree)
		{
			float sin = std::sin(degree * 3.1415 / 180);
			float cos = std::cos(degree * 3.1415 / 180);

			float tx = vec.x;
			float ty = vec.y;

			return vector2f(cos * tx - sin * ty, sin * tx - cos * ty);
		}

		//! conversion to opencv Vec2f
		operator cv::Vec2f() const { return cv::Vec2f(x, y); }

		//! conversion to opencv Point
		operator cv::Point2f() const { return cv::Point2f(x, y); }

		static float lerpF(float a, float b, float interval)
		{
			return a * (1 - interval) + b * interval;
		}
	};
}



#endif