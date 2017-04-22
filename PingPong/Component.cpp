#include "Component.h"
#include "XmlParser\rapidxml.hpp"
#include <cmath>
typedef rapidxml::xml_node<> XmlNode;

//////////////////////////////////////////////////////////////////////////////////////
Component::Component() {}

Component::~Component() {}

void Component::AttachTo(Object* obj)
{
	m_Transform = obj;
}

//////////////////////////////////////////////////////////////////////////////////////
cv::Scalar ImageComponent::GlobalMaskColor(255, 255, 255, 255);

ImageComponent::ImageComponent(std::string _imageFileName)
{
	m_Image = cv::imread(_imageFileName, -1);
	m_UseMask = false;
	// m_Mask = _image;
}

ImageComponent::ImageComponent(Mat _image)
{
	m_UseMask = false;
	// m_Mask = _image;
	m_Image = _image;
}

ImageComponent::ImageComponent(Mat _image, Mat _mask, bool _useMask) :
Component(),
m_UseMask(_useMask)
{
	m_Image = _image;
	m_Mask = _mask;
}

ImageComponent::~ImageComponent()
{
	/*
	cout << "Image Component Destroy!" << endl;
	m_ResizeImage.release();
	m_ResizeMask.release();

	m_ResizeRotImage.release();
	m_ResizeRotMask.release();

	m_OutputImage.release();
	m_OutputMask.release();

	m_Image.release();
	m_Mask.release();
	*/
	/*
	m_ResizeImage = Mat();
	m_ResizeMask = Mat();

	m_ResizeRotImage = Mat();
	m_ResizeRotMask = Mat();

	m_OutputImage = Mat();
	m_OutputMask = Mat();

	m_Image = Mat();
	m_Mask = Mat();
	*/
}

void ImageComponent::Start()
{

}

void ImageComponent::Update(float timeStep)
{

}

void ImageComponent::LateUpdate(float timeStep)
{
	if (m_Transform->ObjManager == nullptr)
	{
		cout << "OBJMGR is null" << endl;
		return;
	}
	if (m_Transform->ObjManager->m_Scene == nullptr)
	{
		cout << "m_Scene is null" << endl;
		return;
	}
	if (m_Transform->ObjManager->m_Scene->GetTargetFrame() == nullptr)
	{
		cout << "GetTargetFrame is null" << endl;
		return;
	}
	Mat* target = m_Transform->ObjManager->m_Scene->GetTargetFrame();
	
	drawOnTarget(target);
	
}

void ImageComponent::AttachTo(Object* obj)
{
	Component::AttachTo(obj);
	m_Transform->SetNativeSize(Vec2i(m_Image.size().width, m_Image.size().height));

}
/*
void ImageComponent::calculateResizeImage(VECTOR::vector2f _size)
{
	
	// Resize image if scale has been changed
	m_PreScale = m_Transform->Scale();
	// Resize Image!
	cv::resize(m_Image, m_ResizeImage, cv::Size(), m_PreScale.x, m_PreScale.y);
	// Resize Mask!
	if (m_UseMask && m_Mask.size().area() > 0)
		cv::resize(m_Mask, m_ResizeMask, cv::Size(), m_PreScale.x, m_PreScale.y);


	m_PreRotAngle = (int)m_Transform->Rotation;
	// 2D Rotation Matrix
	Mat rot_mat = cv::getRotationMatrix2D(Point2f(m_ResizeImage.cols / 2.0f, m_ResizeImage.rows / 2.0f), m_PreRotAngle, 1.0f);

	// Rotate Image!
	cv::warpAffine(m_ResizeImage, m_ResizeRotImage, rot_mat, m_ResizeImage.size());
	// Rotate Mask!
	if (m_UseMask && m_Mask.size().area() > 0)
		cv::warpAffine(m_ResizeMask, m_ResizeRotMask, rot_mat, m_ResizeMask.size());
	
}

void ImageComponent::calculateRotateImage(float _angle)
{
	

	m_PreRotAngle = (int)m_Transform->Rotation;
	// 2D Rotation Matrix
	Mat rot_mat = cv::getRotationMatrix2D(Point2f(m_ResizeImage.cols / 2.0f, m_ResizeImage.rows / 2.0f), m_PreRotAngle, 1.0f);

	// Rotate Image!
	cv::warpAffine(m_ResizeImage, m_ResizeRotImage, rot_mat, m_ResizeImage.size());
	// Rotate Mask!
	if (m_UseMask && m_Mask.size().area() > 0)
		cv::warpAffine(m_ResizeMask, m_ResizeRotMask, rot_mat, m_ResizeMask.size());
	
}
*/
void ImageComponent::calculateRotateImage(Mat& dst, Mat& mskDst)
{
	m_PreRotAngle = (int) m_Transform->Rotation;
	// 2D Rotation Matrix
	Mat rot_mat = cv::getRotationMatrix2D(Point2f(m_Image.cols / 2.0f, m_Image.rows / 2.0f), m_PreRotAngle, 1.0f);

	// Rotate Image!
	cv::warpAffine(m_Image, dst, rot_mat, m_Image.size());
	// Rotate Mask!
	if (m_UseMask && m_Mask.size().area() > 0)
		cv::warpAffine(m_Mask, mskDst, rot_mat, m_Mask.size());

	rot_mat.release();
}

void ImageComponent::drawOnTarget(Mat* target)
{
	if (RotationEnabled)
	{
		if (std::abs((float)m_PreRotAngle - m_Transform->Rotation) > 1.0f || m_OutputImage.size().area() == 0 || ReqestedUpdate)
		{
			m_PreRotAngle = m_Transform->Rotation;
			// 2D Rotation Matrix
			Mat rot_mat = cv::getRotationMatrix2D(Point2f(m_Image.cols / 2.0f, m_Image.rows / 2.0f), m_PreRotAngle, 1.0f);
			// Rotate Image!
			cv::warpAffine(m_Image, m_OutputImage, rot_mat, m_Image.size());
			// Rotate Mask!
			if (m_UseMask && m_Mask.size().area() > 0)
				cv::warpAffine(m_Mask, m_OutputMask, rot_mat, m_Mask.size());
			ReqestedUpdate = false;
		}
	}
	else
	{
		m_OutputImage = m_Image;
		m_OutputMask = m_Mask;
	}

	// draw image on render target
	int tWidth = m_OutputImage.size().width;
	int tHeight = m_OutputImage.size().height;

	int wWidth = target->size().width;
	int wHeight = target->size().height;

	// check if out of target
	// first get four edge points
	Vec2f UpperLeft = m_Transform->Position() + Vec2f(-tWidth / 2, tHeight / 2);
	Vec2f UpperRight = m_Transform->Position() + Vec2f(tWidth / 2, tHeight / 2);
	Vec2f LowerLeft = m_Transform->Position() + Vec2f(-tWidth / 2, -tHeight / 2);
	Vec2f LowerRight = m_Transform->Position() + Vec2f(tWidth / 2, -tHeight / 2);

	cv::Rect targetRect(0, 0, wWidth, wHeight);

	// If all outside of window, skip
	if (targetRect.contains(Point2f(UpperLeft[0], UpperLeft[1])) ||
		targetRect.contains(Point2f(UpperRight[0], UpperRight[1])) ||
		targetRect.contains(Point2f(LowerLeft[0], LowerLeft[1])) ||
		targetRect.contains(Point2f(LowerRight[0], LowerRight[1])))
	{
		Vec2f position = m_Transform->Position();

		double internalAlpha = ColorAlpha / 255.0;
		cv::Scalar internalColor = Color / 255.0;
		cv::Scalar globalMaskColor = (UseGlobalMask) ? (GlobalMaskColor / 255.0) : cv::Scalar(1, 1, 1, 1);

		for (int offsetX = 0; offsetX < tHeight; offsetX++)
		{
			for (int offsetY = 0; offsetY < tWidth; offsetY++)
			{
				int posI = wHeight - position[1] - tHeight / 2 + offsetX - 1;
				int posJ = position[0] - tWidth / 2 + offsetY;

				if (posI >= 0 && posJ >= 0 && posI < wHeight && posJ < wWidth)
				{
					// calculate alpha value
					if (m_UseMask && m_Mask.size().area() > 0)
					{
						double globalMaskAlpha = globalMaskColor[3];
						double alpha = (m_OutputImage.at<Vec4b>(offsetX, offsetY)[3]) / 255.0;
						double maskAlpha = (m_OutputMask.at<Vec3b>(offsetX, offsetY)[0]) / 255.0;

						double finalAlpha = alpha * internalAlpha * maskAlpha * globalMaskAlpha;
						for (int c = 0; c < 3; c++)
						{
							target->at<Vec3b>(posI, posJ)[c] =
								m_OutputImage.at<Vec4b>(offsetX, offsetY)[c] * internalColor[c] * globalMaskColor[c] * finalAlpha +
								target->at<Vec3b>(posI, posJ)[c] * (1.0 - finalAlpha);
						}
					}
					else
					{
						double globalMaskAlpha = globalMaskColor[3];
						double alpha = (m_OutputImage.at<Vec4b>(offsetX, offsetY)[3]) / 255.0;

						double finalAlpha = alpha * internalAlpha * globalMaskAlpha;
						for (int c = 0; c < 3; c++)
						{
							target->at<Vec3b>(posI, posJ)[c] =
								m_OutputImage.at<Vec4b>(offsetX, offsetY)[c] * internalColor[c] * globalMaskColor[c] * finalAlpha +
								target->at<Vec3b>(posI, posJ)[c] * (1.0 - finalAlpha);
						}
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void TextComponent::LateUpdate(float timeStep)
{
	Mat* TargetFrame = m_Transform->ObjManager->m_Scene->GetTargetFrame();

	int base;
	cv::Size textSize = getTextSize(Text, FontType, Scale, Thickness, &base);

	Point2f offset(textSize.width * Anchor.x, textSize.height * Anchor.y);

	Point2f worldPos(m_Transform->Position()[0] - offset.x, m_Transform->Position()[1] - offset.y);

	Point2f imgPos(worldPos.x, TargetFrame->size().height - worldPos.y - 1);

	cv::putText(*TargetFrame, Text, imgPos, FontType, Scale, Color, Thickness);
}
//////////////////////////////////////////////////////////////////////////////////////

void ImageLerper::Update(float timeStep)
{
	if (IsLerping)
	{
		LerpTimer -= timeStep;
		if (LerpTimer > 0.0f)
		{
			float ch0 = TargetImage->Color[0];
			float ch1 = TargetImage->Color[1];
			float ch2 = TargetImage->Color[2];
			float a = TargetImage->ColorAlpha;

			float value = timeStep * 1 / LerpTimer;
			if (value > 1.0f)
				value = 1.0f;

			TargetImage->Color[0] = VECTOR::vector2f::lerpF(ch0, TargetColor[0], value);
			TargetImage->Color[1] = VECTOR::vector2f::lerpF(ch1, TargetColor[1], value);
			TargetImage->Color[2] = VECTOR::vector2f::lerpF(ch2, TargetColor[2], value);
			TargetImage->ColorAlpha = VECTOR::vector2f::lerpF(a, TargetAlpha, value);

			if (TargetImage->Color[0] > 255.0f)
				TargetImage->Color[0] = 255.0f;
			if (TargetImage->Color[1] > 255.0f)
				TargetImage->Color[1] = 255.0f;
			if (TargetImage->Color[2] > 255.0f)
				TargetImage->Color[2] = 255.0f;
			if (TargetImage->ColorAlpha > 255.0f)
				TargetImage->ColorAlpha = 255.0f;
		}
		else
		{
			TargetImage->Color[0] = TargetColor[0];
			TargetImage->Color[1] = TargetColor[1];
			TargetImage->Color[2] = TargetColor[2];
			TargetImage->ColorAlpha = TargetAlpha;
			IsLerping = false;
		}
	}
}

void ImageLerper::LerpColorTo(cv::Scalar col, float time, float alpha)
{
	IsLerping = true;
	if (alpha < 0.0f)
		TargetAlpha = TargetImage->ColorAlpha;
	else
		TargetAlpha = alpha;
	LerpTimer = time;
	TargetColor = col;
}

//////////////////////////////////////////////////////////////////////////////////////
void Rotator::Update(float timeStep) 
{
	m_Transform->Rotation = Target->Rotation;
}

//////////////////////////////////////////////////////////////////////////////////////
void CircleIndicator::LateUpdate(float timeStep)
{
	cv::Point center
		(m_Transform->Position()[0], m_Transform->ObjManager->m_Scene->GetTargetFrame()->size().height - m_Transform->Position()[1] - 1);
	cv::circle(*m_Transform->ObjManager->m_Scene->GetTargetFrame(), center, Radius, Color, Thickness);
}

//////////////////////////////////////////////////////////////////////////////////////
Mat BallTrackerComponent::thresholdTemp;

BallTrackerComponent::BallTrackerComponent(int maxR, int minR, cv::Scalar maxCol, cv::Scalar minCol)
: MaxRadius(maxR), MinRadius(minR), MaxHSVColor(maxCol), MinHSVColor(minCol)
{
}

void BallTrackerComponent::Update(float timeStep)
{	
	// update center and radius
	IsTracked = calculateTargetCenter(*m_Transform->ObjManager->m_Scene->GetTargetFrame(), TargetCenter, TargetRadius);

	if (FollowingEnabled)
	{
		if (LerpToPosition)
		{
			if (VECTOR::vector2f(m_Transform->LocalPosition() - TargetCenter).length_squared() > 0.1f)
				m_Transform->SetLocalPosition(cv::Vec2f(VECTOR::vector2f::lerp(m_Transform->LocalPosition(), TargetCenter, timeStep * LerpSpeed)));
		}
		else
		{
			m_Transform->SetLocalPosition(TargetCenter);
		}
	}
}

void BallTrackerComponent::LateUpdate(float timeStep) 
{
	if (IndicatorEnabled)
	{
		cv::Point center
			(m_Transform->Position()[0], m_Transform->ObjManager->m_Scene->GetTargetFrame()->size().height - m_Transform->Position()[1]);
		cv::circle(*m_Transform->ObjManager->m_Scene->GetTargetFrame(), center, DrawRadius, DrawColor, DrawThickness);
	}
}

bool BallTrackerComponent::calculateTargetCenter(Mat& frame, cv::Vec2f &_center, float &_radius)
{
	cv::Size originalSize = frame.size();
	// do blur
	int blurMul = 0;
	Mat blur_frame;
	if (blurMul > 1)
	{
		cv::resize(frame, blur_frame, cv::Size(frame.size().width / blurMul, frame.size().height / blurMul));
		cv::resize(blur_frame, blur_frame, originalSize);
	}
	else
	{
		blur_frame = frame;
	}
	// convert from BGR channel to HSV channel space
	Mat hsv_frame;
	cv::cvtColor(blur_frame, hsv_frame, cv::COLOR_BGR2HSV);

	// construct a mask according to the range, use erosion and dilation to remove
	// small blobs noises
	Mat mask;
	cv::inRange(hsv_frame, MinHSVColor, MaxHSVColor, mask);

	cv::erode(mask, mask, Mat(), Point(-1, -1), 2);
	cv::dilate(mask, mask, Mat(), Point(-1, -1), 2);

	thresholdTemp = mask;

	// find contours of the connected components
	// list of points for a contour
	Mat conMask = mask.clone();
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(conMask, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

	if (contours.size() > 0)
	{
		// find contour componenet with max pixel, which is the largest contour
		int index = 0;
		int maxArea = 0;
		int contourCount = contours.size();
		for (int i = 0; i < contourCount; i++)
		{
			int tmpArea = cv::contourArea(contours[i]);
			if (tmpArea > maxArea)
			{
				maxArea = tmpArea;
				index = i;
			}
		}

		std::vector<cv::Point> &maxContour = contours[index];
		// calculate minimum enclosing circle of the largest connected component
		cv::Point2f center;
		float radius;
		cv::minEnclosingCircle(maxContour, center, radius);

		// return center if the radius is qualified, return true
		if (radius >= MinRadius && radius <= MaxRadius)
		{
			cv::Vec2f ctr = cv::Vec2f(center.x, frame.size().height - center.y);
			float r = radius;

			_center = cv::Vec2f(center.x, frame.size().height - center.y);
			_radius = radius;
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////
DuoBallTracker::DuoBallTracker(int maxR, int minR, cv::Scalar maxCol, cv::Scalar minCol)
: MaxRadius(maxR), MinRadius(minR), MaxHSVColor(maxCol), MinHSVColor(minCol)
{
}

void DuoBallTracker::AttachTo(Object* obj)
{
	Component::AttachTo(obj);
	init();
}

void DuoBallTracker::init()
{
	rightIndicator = m_Transform->ObjManager->CreateStaticObject(VECTOR::vector2f(540, 240), 1);
	rightIndicator->AddComponent<CircleIndicator>(CircleIndicator());
	

	leftInidicator = m_Transform->ObjManager->CreateStaticObject(VECTOR::vector2f(100, 240), 1);
	leftInidicator->AddComponent<CircleIndicator>(CircleIndicator());
}

void DuoBallTracker::Update(float timeStep)
{
	// update center and radius
	int ballFound = calculateTargetCenter(*m_Transform->ObjManager->m_Scene->GetTargetFrame(), BallList);

	if (ballFound == 2)
	{
		Ball b1 = BallList[0];
		Ball b2 = BallList[1];

		VECTOR::vector2f midPos = b1.Center * 0.5 + b2.Center * 0.5;
		VECTOR::vector2f midNormalPos = midPos + DivisionAxis;

		bool IsRight = VECTOR::vector2f::evaluatePointSpace(b1.Center, midPos, midNormalPos);
		if (IsRight)
		{
			// b1 is right, b2 is left
			if (LerpInidicatorToPosition)
			{
				VECTOR::vector2f rightDif = VECTOR::vector2f(rightIndicator->LocalPosition()) - b1.Center;
				if (rightDif.length_squared() > 0.1f)
				{
					rightIndicator->SetLocalPosition(
						cv::Vec2f(
								VECTOR::vector2f::lerp(rightIndicator->LocalPosition(), b1.Center, timeStep * 20.0f)
							)
						);
				}

				VECTOR::vector2f leftDif = VECTOR::vector2f(leftInidicator->LocalPosition()) - b2.Center;
				if (leftDif.length_squared() > 0.1f)
				{
					leftInidicator->SetLocalPosition(
						cv::Vec2f(
								VECTOR::vector2f::lerp(leftInidicator->LocalPosition(), b2.Center, timeStep * 20.0f)
							)
						);
				}
			}
			else
			{
				rightIndicator->SetLocalPosition(cv::Vec2f(b1.Center));
				leftInidicator->SetLocalPosition(cv::Vec2f(b2.Center));
			}
		}
		
	}
	else if (ballFound == 1)
	{
		Ball b1 = BallList[0];
		// only b1, is right
		if (LerpInidicatorToPosition)
		{
			VECTOR::vector2f rightDif = VECTOR::vector2f(rightIndicator->LocalPosition()) - b1.Center;
			if (rightDif.length_squared() > 0.1f)
			{
				rightIndicator->SetLocalPosition(
					cv::Vec2f(
							VECTOR::vector2f::lerp(rightIndicator->LocalPosition(), b1.Center, timeStep * 20.0f)
						)
					);
			}
		}
		else
		{
			rightIndicator->SetLocalPosition(cv::Vec2f(b1.Center));
		}

	}
}

int DuoBallTracker::calculateTargetCenter(Mat frame, std::vector<Ball>& balls)
{
	cv::Size originalSize = frame.size();

	int blurMul = 2;
	Mat blur_frame;
	if (blurMul > 1)
	{
		cv::resize(frame, blur_frame, cv::Size(frame.size().width / blurMul, frame.size().height / blurMul));
		cv::resize(blur_frame, blur_frame, originalSize);
	}
	else
	{
		blur_frame = frame;
	}

	// convert from BGR channel to HSV channel space
	Mat hsv_frame;
	cv::cvtColor(blur_frame, hsv_frame, cv::COLOR_BGR2HSV);

	// construct a mask according to the range, use erosion and dilation to remove
	// small blobs noises
	Mat mask;
	cv::inRange(hsv_frame, MinHSVColor, MaxHSVColor, mask);

	cv::erode(mask, mask, Mat(), Point(-1, -1), 2, 1, 1);
	cv::dilate(mask, mask, Mat(), Point(-1, -1), 2, 1, 1);

	// find contours of the connected components
	// list of points for a contour
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(mask, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

	int ballFoundCount = 0;

	if (contours.size() > 0)
	{
		// find contour componenet with max pixel, which is the largest contour
		// biggest index
		int index = 0;
		// second big index
		int sec_index = -1;

		int maxArea = 0;
		int contourCount = contours.size();
		for (int i = 0; i < contourCount; i++)
		{
			int tmpArea = cv::contourArea(contours[i]);
			if (tmpArea > maxArea)
			{
				maxArea = tmpArea;
				sec_index = index;
				index = i;
			}
		}

		
		// calculate Max Ball first

		std::vector<cv::Point> &maxContour = contours[index];
		// calculate minimum enclosing circle of the largest connected component
		cv::Point2f center;
		float radius;
		cv::minEnclosingCircle(maxContour, center, radius);
		// return center if the radius is qualified, add it to the list
		if (radius >= MinRadius && radius <= MaxRadius)
		{
			Ball ball;
			ball.Center = VECTOR::vector2f(center.x, frame.size().height - center.y - 1);
			ball.Radius = radius;
			if (balls.size() < 1)
				balls.push_back(ball);
			else
				balls[0] = ball;

			ballFoundCount++;
			
		}

		// calculate Sec-Max Ball if there's one
		if (sec_index != -1)
		{
			std::vector<cv::Point> &secContour = contours[sec_index];
			// calculate minimum enclosing circle of the largest connected component
			cv::Point2f center;
			float radius;
			cv::minEnclosingCircle(secContour, center, radius);
			// return center if the radius is qualified, add it to the list
			if (radius >= MinRadius && radius <= MaxRadius)
			{
				Ball ball;
				ball.Center = VECTOR::vector2f(center.x, frame.size().height - center.y - 1);
				ball.Radius = radius;
				if (balls.size() < 2)
					balls.push_back(ball);
				else 
					balls[1] = ball;

				ballFoundCount++;
			}
		}
	}

	return ballFoundCount;
}
//////////////////////////////////////////////////////////////////////////////////////
void ColorSelector::Update(float timeStep)
{
	//
	if (State == Moving)
	{
		VECTOR::vector2f currentPos = m_Transform->LocalPosition();
		VECTOR::vector2f diff = currentPos - m_NextPosition;
		// not yet reached, keep moving
		if (diff.length_squared() > 1.0f)
			m_Transform->SetLocalPosition(VECTOR::vector2f::lerp(currentPos, m_NextPosition, timeStep * 10.0f));
		// reached, go counting
		else
		{
			m_Transform->SetLocalPosition(m_NextPosition);
			State = Counting;
		}
	}
	else if (State == Counting)
	{
		// not yet selected, keep counting
		if (m_TimerToSelectColor > 0.0f)
		{
			m_TimerToSelectColor -= timeStep;
		}
		// time up, cache color, to idle
		else
		{
			cacheColor();
			State = Idle;
		}
	}

}

void ColorSelector::SelectNewColor(const VECTOR::vector2f& _atPos, float _countDownAfterReached)
{
	State = Moving;
	m_NextPosition = _atPos;
	m_TimerToSelectColor = _countDownAfterReached;
}

void ColorSelector::cacheColor()
{
	Mat* frame = m_Transform->ObjManager->m_Scene->GetTargetFrame();
	Mat hsvFrame;
	// BGR to HSV

	cv::cvtColor(*frame, hsvFrame, CV_BGR2HSV);
	/*
	cv::Vec3f vec3Color = hsvFrame.at<cv::Vec3b>(Point(m_NextPosition.x, frame->size().height - m_NextPosition.y - 1));
	cv::Vec3f vec3Color1 = hsvFrame.at<cv::Vec3b>(Point(m_NextPosition.x - 5, frame->size().height - (m_NextPosition.y) - 1));
	cv::Vec3f vec3Color2 = hsvFrame.at<cv::Vec3b>(Point(m_NextPosition.x + 5, frame->size().height - (m_NextPosition.y) - 1));
	cv::Vec3f vec3Color3 = hsvFrame.at<cv::Vec3b>(Point(m_NextPosition.x, frame->size().height - (m_NextPosition.y + 5) - 1));
	cv::Vec3f vec3Color4 = hsvFrame.at<cv::Vec3b>(Point(m_NextPosition.x, frame->size().height - (m_NextPosition.y - 5) - 1));
	cv::Vec3f vec3Color5 = hsvFrame.at<cv::Vec3b>(Point(m_NextPosition.x - 3, frame->size().height - (m_NextPosition.y) - 1));
	cv::Vec3f vec3Color6 = hsvFrame.at<cv::Vec3b>(Point(m_NextPosition.x + 3, frame->size().height - (m_NextPosition.y) - 1));
	cv::Vec3f vec3Color7 = hsvFrame.at<cv::Vec3b>(Point(m_NextPosition.x, frame->size().height - (m_NextPosition.y + 3) - 1));
	cv::Vec3f vec3Color8 = hsvFrame.at<cv::Vec3b>(Point(m_NextPosition.x, frame->size().height - (m_NextPosition.y - 3) - 1));
	*/
	/*
	cv::Vec3f vec3ColorFinal =
		vec3Color + vec3Color1 + vec3Color2 + vec3Color3 + vec3Color4 + vec3Color5 + vec3Color6 + vec3Color7 + vec3Color8;
	vec3ColorFinal /= 9;
	*/

	cv::Vec3f vec3ColorFinal;
	int width = 5;
	int begin = 0 - width / 2;
	int end = 0 + (5 - width);
	for (int i = begin; i < end; i++)
	{
		for (int j = begin; j < end; j++)
		{
			vec3ColorFinal += hsvFrame.at<cv::Vec3b>(Point(m_NextPosition.x + i, frame->size().height - (m_NextPosition.y + j) - 1));
		}
	}
	int divider = end - begin;
	vec3ColorFinal /= divider * divider;

	///////
	// vec3ColorFinal = vec3Color;

	cv::Scalar color(vec3ColorFinal[0], vec3ColorFinal[1], vec3ColorFinal[2]);

	// first color
	if (AllColors.size() == 0)
	{
		MinColor = color;
		MinColor = color;
		AverageColor = color;
		AllColors.push_back(color);
	}
	else
	{
		// get maximum and minimum

		if (color[0] > MaxColor[0])
			MaxColor[0] = color[0];
		else if (color[0] < MinColor[0])
			MinColor[0] = color[0];

		if (color[1] > MaxColor[1])
			MaxColor[1] = color[1];
		else if (color[1] < MinColor[1])
			MinColor[1] = color[1];

		if (color[2] > MaxColor[2])
			MaxColor[2] = color[2];
		else if (color[2] < MinColor[2])
			MinColor[2] = color[2];

		// get average color

		int count = AllColors.size();
		cv::Scalar avg = AverageColor * count / (count + 1);
		cv::Scalar ncol = color / (count + 1);
		AverageColor = (avg + ncol);

		// add to the list

		AllColors.push_back(color);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void SteeringController::Update(float timeStep) 
{  
	// Get Steering according to behaviour
	VECTOR::vector2f accel(0, 0);

	if (Behaviour == Seeking)
		accel = Seek(m_Target) * timeStep;
	else if (Behaviour == Fleeing)
		accel = Flee(m_Target) * timeStep;

	VECTOR::vector2f vec = VECTOR::vector2f(m_Transform->Velocity) + accel;

	// truncate velocity if its over max speed
	if (vec.length() > MaxSpeed)
		vec = vec.normalized() * MaxSpeed;

	m_Transform->Velocity = cv::Vec2f(vec);

	
	// Update Rotation if enabled according to the velocity
	if (UpdateRotationEnabled && vec.length_squared() > 0.01f)
	{
		float rot = std::atan2f(vec.y, vec.x) * 180.0f / 3.1415f;
		m_Transform->Rotation = RotationOffset + rot;
	}
	else
	{
		m_Transform->Rotation = 0.0f;
	}
	
}

VECTOR::vector2f SteeringController::Seek(Object* Target) 
{ 
	VECTOR::vector2f targetPos = Target->Position();
	VECTOR::vector2f mPos = m_Transform->Position();

	// calculate desired velocity
	VECTOR::vector2f desiredVec = targetPos - mPos;
	desiredVec = desiredVec.normalized() * MaxSpeed;

	// calculate acceleration
	VECTOR::vector2f steering = desiredVec - VECTOR::vector2f(m_Transform->Velocity);

	if (steering.length() > MaxAcceleration)
		steering = steering.normalized() * MaxAcceleration;

	return steering;
}

VECTOR::vector2f SteeringController::Flee(Object* Target)
{
	VECTOR::vector2f targetPos = Target->Position();
	VECTOR::vector2f mPos = m_Transform->Position();

	// calculate desired velocity
	VECTOR::vector2f desiredVec = mPos - targetPos;
	desiredVec = desiredVec.normalized() * MaxSpeed;

	// calculate acceleration
	VECTOR::vector2f steering = desiredVec - VECTOR::vector2f(m_Transform->Velocity);

	if (steering.length() > MaxAcceleration)
		steering = steering.normalized() * MaxAcceleration;

	return steering;
}

//////////////////////////////////////////////////////////////////////////////////////
MotionQuantifiler::MotionQuantifiler(Object* ball) : TargetObj(ball)
{
	// m_HistoryPosition.push_back(TargetObj->Position());
}

void MotionQuantifiler::Update(float timeStep)
{
	VECTOR::vector2f newPos = VECTOR::vector2f(TargetObj->Position());
	VECTOR::vector2f difference = newPos - m_PrePosition;
	Motion = difference;
	// difference /= timeStep;

	if (difference.length_squared() < 0.1f)
		Value = 0;
	else
	{
		if (ProjectOnAxis)
			difference = difference.projected(Axis);

		double dot = difference.dotProduct(Axis);
		if (dot > 0)
			Value = -difference.length();
		else
			Value = difference.length();
	}
	// std::cout << "VEC: " << difference.x << " : " << difference.y << std::endl;
	// std::cout << "Motion Value: " << Value << std::endl;
	m_PrePosition = newPos;
}

//////////////////////////////////////////////////////////////////////////////////////
void BalancingRod::Update(float timeStep)
{
}

void BalancingRod::SetAngle(const VECTOR::vector2f &p_left, const VECTOR::vector2f & p_right)
{
	if (!IsControllable)
	{
		VECTOR::vector2f slideVec = VECTOR::vector2f(1, 0);
		m_Transform->Rotation = std::atan2f(slideVec.y, slideVec.x) * 180.0f / 3.1415f;
		if (m_Transform->Rotation > RotationLimit)
			m_Transform->Rotation = RotationLimit;
		else if (m_Transform->Rotation < -RotationLimit)
			m_Transform->Rotation = -RotationLimit;
		return;
	}

	VECTOR::vector2f slideVec = p_right - p_left;
	m_Transform->Rotation = std::atan2f(slideVec.y, slideVec.x) * 180.0f / 3.1415f;
	if (m_Transform->Rotation > RotationLimit)
		m_Transform->Rotation = RotationLimit;
	else if (m_Transform->Rotation < -RotationLimit)
		m_Transform->Rotation = -RotationLimit;
}

VECTOR::vector2f BalancingRod::GetSlidingForce()
{
	// Project Gravity on sliding vector
	VECTOR::vector2f slopeVector = VECTOR::vector2f::rotate(VECTOR::vector2f(1, 0), m_Transform->Rotation);
	VECTOR::vector2f force = Gravity.projected(slopeVector);

	return force;
}

VECTOR::vector2f BalancingRod::GetPointAt(float value, bool clamp)
{
	if (clamp)
	{
		if (value > 1.0f)
			value = 1.0f;
		else if (value < -1.0f)
			value = -1.0f;
	}

	VECTOR::vector2f slopeVector = VECTOR::vector2f::rotate(VECTOR::vector2f(1, 0), m_Transform->Rotation);

	slopeVector *= Radius;

	VECTOR::vector2f point1 = VECTOR::vector2f(m_Transform->Position()) + slopeVector;
	VECTOR::vector2f point2 = VECTOR::vector2f(m_Transform->Position()) - slopeVector;

	value = (value + 1) / 2.0f;
	return point2 * (1 - value) + point1 * value;
}

VECTOR::vector2f BalancingRod::GetDirectionVector()
{
	VECTOR::vector2f slopeVector = VECTOR::vector2f::rotate(VECTOR::vector2f(1, 0), m_Transform->Rotation);
	return slopeVector;
}

void BalancingRod::drawRod()
{
	Mat* renderTarget = m_Transform->ObjManager->m_Scene->GetTargetFrame();
	VECTOR::vector2f slopeVector = GetDirectionVector();

	slopeVector *= Radius;

	VECTOR::vector2f point1 = VECTOR::vector2f(m_Transform->Position()) + slopeVector;
	VECTOR::vector2f point2 = VECTOR::vector2f(m_Transform->Position()) - slopeVector;

	Point p1(point1.x, renderTarget->size().height - point1.y - 1);
	Point p2(point2.x, renderTarget->size().height - point2.y - 1);

	cv::line(*renderTarget, p2, p1, LineColor, LineThickness);
	cv::line(*renderTarget, p2, p1, InnerLineColor, LineThickness - 2);
}

///////////////////////////////////////////////////////////////////////////

void BalancingCharacter::Update(float timeStep) 
{
	if (IsDead)
	{

	}
	else
	{
		// Update Position in 1D Space
		Value += Speed * timeStep;
		m_Transform->Rotation -= Speed * RotationMultiplier * timeStep;

		// Check if hit edge, if so, bounce off based on bouncing loss param
		if (Value > Rod->Radius)
		{
			Value = Rod->Radius;
			Speed = -Speed * (1 - BouncingLoss * BouncingLossMultiplier);
		}
		else if (Value < -Rod->Radius)
		{
			Value = -Rod->Radius;
			Speed = -Speed * (1 - BouncingLoss * BouncingLossMultiplier);
		}

		// map Value to -1 ~ 1
		float relative1DPosX = Value / Rod->Radius;

		// Update Real Position
		m_Transform->SetLocalPosition(cv::Vec2f(Rod->GetPointAt(relative1DPosX)) + cv::Vec2f(PosOffset));

		// Update Speed
		float accel = GetForce() / Mass * MassMultiplier;
		Speed += timeStep * accel;
		Speed *= (1 - FrictionLoss * timeStep);

		// Change Face
		if (abs(Speed) > 70.0f)
			Surprised();
		else
			Normal();
	}
}

float BalancingCharacter::GetForce()
{
	VECTOR::vector2f slidingForce = Rod->GetSlidingForce();
	VECTOR::vector2f slopeVector = Rod->GetDirectionVector();

	float dot = slidingForce.dotProduct(slopeVector);

	return (dot > 0.0f)? Rod->GetSlidingForce().length() : -Rod->GetSlidingForce().length();
}

void BalancingCharacter::Normal()
{
	if (ImageComp == nullptr)
		ImageComp = m_Transform->GetComponent<ImageComponent>();
	if (ImageComp != nullptr)
		ImageComp->m_Image = NormalImg;
}

void BalancingCharacter::Surprised()
{
	if (ImageComp == nullptr)
		ImageComp = m_Transform->GetComponent<ImageComponent>();
	if (ImageComp != nullptr)
		ImageComp->m_Image = SurprisedImg;
}

void BalancingCharacter::Dead()
{
	IsDead = true;

	if (ImageComp == nullptr)
		ImageComp = m_Transform->GetComponent<ImageComponent>();

	if (ImgLerper == nullptr)
		ImgLerper = m_Transform->GetComponent<ImageLerper>();

	if (ImageComp != nullptr)
	{
		ImageComp->m_Image = DeadImg;
		ImageComp->ReqestedUpdate = true;

		if (ImgLerper != nullptr)
			ImgLerper->LerpColorTo(cv::Scalar(0, 0, 0), DeathAnimationTime, 0.0f);
	}
}

////////////////////////////////////////////////////////////////////////////

void BalancingRodController::Update(float timeStep)
{
	Rod->SetAngle(Cursor2->m_Transform->Position(), Cursor1->m_Transform->Position());
}
////////////////////////////////////////////////////////////////////////////

SlidingMapController::SlidingMapController(TileMapManager* map, Object* player, MotionQuantifiler* rmotion, MotionQuantifiler* lmotion)
: SlidingMap(map), Player(player), RightMotionIndicator(rmotion), LeftMotionIndicator(lmotion)
{}

void SlidingMapController::Update(float timeStep)
{
	if (SlidingMap->Mode == TileMapManager::GameMode::Eagle)
	{
		coolDownTimer -= timeStep;
		if (coolDownTimer > 0.0f)
		{
			flickrTimer -= timeStep;
			if (flickrTimer < 0.0f)
			{
				if (imageComp != nullptr)
				{
					imageComp->IsEnabled = !imageComp->IsEnabled;
				}
				flickrTimer = FlickrTime;
			}
		}
		else
		{
			if (imageComp != nullptr)
			{
				imageComp->IsEnabled = true;
			}
		}
	}
	caculateInputMotion(timeStep);
	updateVelocity(timeStep);
	// VECTOR::vector2f posOnMap = SlidingMap->PositionOnMap(Player->Position());
	// cout << "Eagle on Map: " << posOnMap.x << ":" << posOnMap.y << endl;
}

void SlidingMapController::AddSpeed(float speed)
{
	if (speed < 0.0f)
	{
		if (coolDownTimer < 0.0f)
		{
			coolDownTimer = HitCoolDown;
			Speed += speed;
			cout << "SLOW" << endl;
		}
		// PLAY HIT ANIMATION
	}
	else
	{
		Speed += speed;
	}
}

void SlidingMapController::AddScore(float score)
{
	SlidingMap->Score += score;
}

void SlidingMapController::caculateInputMotion(float timeStep)
{
	if (!IsControllable)
		return;

	if (LeftMotionIndicator == nullptr)
	{
		std::cout << "Error: No Left Input" << std::endl;
		return;
	}

	VECTOR::vector2f leftPos = LeftMotionIndicator->m_Transform->Position();
	VECTOR::vector2f rightPos = RightMotionIndicator->m_Transform->Position();

	// middle Point
	VECTOR::vector2f middlePos = leftPos * 0.5f + rightPos * 0.5f;

	// calculate normal direction of left -> right 
	VECTOR::vector2f left2Right = rightPos - leftPos;
	VECTOR::vector2f grav(0, 1);

	VECTOR::vector2f tangent = grav.projected(left2Right);
	
	VECTOR::vector2f normal = grav - tangent;

	// Rotate player to fit velocity
	float targetRotation = VECTOR::vector2f::angle(normal, VECTOR::vector2f(0, 1));
	Player->Rotation = VECTOR::vector2f::lerpF(Player->Rotation, targetRotation * 180 / 3.1415, timeStep * 10.0f);

	float leftMotionValue = LeftMotionIndicator->Value;
	float rightMotionValue = RightMotionIndicator->Value;

	float value = leftMotionValue * 0.5 + rightMotionValue * 0.5;
	// std::max<float>(leftMotionValue, rightMotionValue);

	// Calculate Speed
	Speed = Speed * (1 - SpeedLoss * timeStep);

	float Accel;
	if (value > 0)
	{
		if (value > 50)
			value = 50;

		float Accel = value * AccelMultiplier;

		//! the higher the speed is, the higher the resistance of air is
		Speed += Accel; // * (MaxSlidingSpeed - Speed) / (MaxSlidingSpeed - MinSlidingSpeed);
	}

	if (Speed > MaxSlidingSpeed)
		Speed = MaxSlidingSpeed;
	else if (Speed < MinSlidingSpeed)
		Speed = MinSlidingSpeed;
	
	// calculate map sliding velocity based on player facing and speed
	VECTOR::vector2f direction = VECTOR::vector2f::rotate(VECTOR::vector2f(1, 0), Player->Rotation - 90);
	SlidingVelocity = direction.normalized() * Speed;

	// change motion axis
	RightMotionIndicator->Axis = normal.normalized();
	LeftMotionIndicator->Axis = normal.normalized();
}

void SlidingMapController::updateVelocity(float timeStep)
{
	SlidingMap->m_Transform->Velocity = SlidingVelocity;
}

void SlidingMapController::LateUpdate(float timeStep)
{
	if (IndicatorEnabled)
		drawCompass();
}

void SlidingMapController::drawCompass()
{
	Mat* frame = m_Transform->ObjManager->m_Scene->GetTargetFrame();
	int height = frame->size().height;

	VECTOR::vector2f p1 = RightMotionIndicator->TargetObj->Position();
	VECTOR::vector2f p2 = LeftMotionIndicator->TargetObj->Position();
	VECTOR::vector2f direction = (p1 - p2);
	direction.normalize();

	VECTOR::vector2f rightP = VECTOR::vector2f(Player->Position()) + direction * 150;
	VECTOR::vector2f leftP = VECTOR::vector2f(Player->Position()) - direction * 150;

	VECTOR::vector2f rightMidP = VECTOR::vector2f(Player->Position()) + direction * 50;
	VECTOR::vector2f leftMidP = VECTOR::vector2f(Player->Position()) - direction * 50;

	Point rp = Point2f(rightP.toMatSpace(height));
	Point lp = Point2f(leftP.toMatSpace(height));
	Point rmp = Point2f(rightMidP.toMatSpace(height));
	Point lmp = Point2f(leftMidP.toMatSpace(height));

	cv::line(*frame, rp, rmp, cv::Scalar(255, 255, 255), 2);

	cv::line(*frame, lp, lmp, cv::Scalar(255, 255, 255), 2);
	
}

VECTOR::vector2f SlidingMapController::InputMiddlePoint()
{
	VECTOR::vector2f p1 = RightMotionIndicator->TargetObj->Position();
	VECTOR::vector2f p2 = LeftMotionIndicator->TargetObj->Position();

	return p1 * 0.5 + p2 * 0.5;
}

float SlidingMapController::InputAngle()
{
	VECTOR::vector2f p1 = RightMotionIndicator->TargetObj->Position();
	VECTOR::vector2f p2 = LeftMotionIndicator->TargetObj->Position();

	VECTOR::vector2f direction = p1 - p2;
	float degree = VECTOR::vector2f::angle(direction, VECTOR::vector2f(1, 0)) * 180 / 3.1415;
	return degree;
}

////////////////////////////////////////////////////////////////////////////

TileMapManager::TileMapManager(std::map<char, TileInform> mapping, std::map<std::string, ItemInform> itemList, int _offset, GameMode _mode)
: TileSet(mapping), ItemSet(itemList), TileOffset(_offset)
{
	Mode = _mode;
	// cout << "TILE OFFSET " << _offset << endl;
}

void TileMapManager::Update(float timeStep)
{
	// cv::Vec2f slidingVec = SlidingVelocity;
	// m_Transform->SetLocalPosition(m_Transform->LocalPosition() + cv::Vec2f(slidingVec * timeStep));

	if (Mode == Eagle)
	{
		if (UseRegionMode)
			handleRegionMode(timeStep);
		UpdateEagleMode(timeStep);
	}
	else if (Mode == Rock)
	{
		UpdateRockMode(timeStep);
	}
		
}

void TileMapManager::UpdateEagleMode(float timeStep)
{
	if (State == Preparing)
	{
		if (!ItemPoolSpawned)
		{
			for (int i = 0; i < 50; i++)
			{
				getRing();
				getRock();
			}
			ItemPoolSpawned = true;
		}

		VECTOR::vector2f pos = Player->m_Transform->LocalPosition();
		VECTOR::vector2f diff = pos - StartPlayerPosition;
		if (StartDelay > 0.0f)
		{
			StartDelay -= timeStep;
		}
		else if (diff.length_squared() > 0.01f)
		{
			Player->m_Transform->SetLocalPosition(VECTOR::vector2f::lerp(pos, StartPlayerPosition, timeStep * 3.0f));
		}
		else
		{
			State = Playing;
			Timer = MapTime;
			Player->IsControllable = true;
			ScoreBoard->IsEnabled = true;
			TimerText->IsEnabled = true;
			SpeedText->IsEnabled = true;
			ProgressBar->IsEnabled = true;
			ProgressPointer->IsEnabled = true;
			startPosY = MapSpacePosition(Player->m_Transform->LocalPosition()).y;
		}
	}
	else if (State == Playing)
	{
		if (IsGameOver())
		{
			State = GameOver;
			Player->IsControllable = false;
			ProgressBar->IsEnabled = false;
			ProgressPointer->IsEnabled = false;
			SpeedText->IsEnabled = false;
			tempSlidingSpeed = Player->SlidingVelocity;
		}
		else
		{

			// update text
			Timer -= timeStep;
			if (ScoreBoard != nullptr)
				ScoreBoard->Text = std::string("SCORE " + std::to_string((int)Score));
			if (TimerText != nullptr)
				TimerText->Text = std::string(std::to_string((int)Timer));
			SpeedText->Text = std::to_string((int)Player->Speed);

			updateProgressBar(timeStep);
			updateItemPoolStateEagle(timeStep);
		}
	}
	else if (State == GameOver)
	{
		// Play Animation
		if (Player->SlidingVelocity.length() > 20.0f)
		{
			Player->SlidingVelocity -= tempSlidingSpeed * timeStep * 0.1f;
			VECTOR::vector2f playerVec = VECTOR::vector2f(Player->m_Transform->Velocity) - tempSlidingSpeed * timeStep * 0.3f;
			Player->m_Transform->Velocity = playerVec;
		}
		else
		{
			State = Result;
			if (Timer < 0.0f)
				overTime = true;
			finalScore = Score + Timer * ScorePerSec;
			// Player->SlidingVelocity = VECTOR::vector2f(0, 0);
		}
	}
	else
	{
		if (ScoreBoard != nullptr)
			ScoreBoard->Text = std::string("SCORE " + std::to_string((int)Score));
		if (TimerText != nullptr)
			TimerText->Text = std::string(std::to_string((int)Timer));

		Mat* frame = m_Transform->ObjManager->m_Scene->GetTargetFrame();
		VECTOR::vector2f window(frame->size().width, frame->size().height);
		VECTOR::vector2f boardPos = ScoreBoard->m_Transform->LocalPosition();
		VECTOR::vector2f diff = boardPos - window / 2;
		if (diff.length_squared() > 0.1f)
		{
			ScoreBoard->Anchor.x = VECTOR::vector2f::lerpF(ScoreBoard->Anchor.x, 0.5, timeStep * 20.0f);
			boardPos = VECTOR::vector2f::lerp(boardPos, window / 2, timeStep * 10.0f);
			ScoreBoard->m_Transform->SetLocalPosition(boardPos);
		}
		else
		{
			if (!finishScoreCounting)
			{
				// lose score
				if (overTime)
				{
					if (Timer < 0.0f)
					{
						Timer += timeStep * 10.0;
						Score -= timeStep * ScorePerSec * 10.0;
					}
					else
					{
						Timer = 0.0f;
						Score = finalScore;
						finishScoreCounting = true;
					}
				}
				// gain score
				else
				{
					if (Timer > 0.0F)
					{
						Timer -= timeStep * 10.0;
						Score += timeStep * ScorePerSec * 10.0f;
					}
					else
					{
						Timer = 0.0f;
						Score = finalScore;
						finishScoreCounting = true;
					}
				}
			}
			else
			{
				if (resultDelay > 0.0f)
				{
					resultDelay -= timeStep;
				}
				else
				{
					// load to title
					m_Transform->ObjManager->m_Scene->ScnManager->LoadScene(1);
				}
			}
		}
	}
}

void TileMapManager::UpdateRockMode(float timeStep)
{
	if (State == Preparing)
	{
		if (!ItemPoolSpawned)
		{
			for (int i = 0; i < 50; i++)
			{
				getHole(5);
			}
			ItemPoolSpawned = true;
			beginRandomHoleSpawn(beginHoleYMin, beginHoleYMax, beginHoleYOffset);
			MapController->IsControllable = false;
			slidingSpeed = minSlidingSpeed;
			MapController->SlidingVelocity = Vec2f(0, -slidingSpeed);
		}

		if (StartDelay > 0.0f)
		{
			StartDelay -= timeStep;
		}
		else
		{
			State = Playing;
			Timer = 0.0f;
			PlayerRod->IsControllable = true;
			RockPlayer->DeathAnimationTime = DeathAnimationTime;
			startPosY = MapSpacePosition(PlayerRod->m_Transform->LocalPosition()).y;
			currentHoleDistance = holeMaxSpawnDistance;
			distanceCounter = currentHoleDistance;
		}
	}
	else if (State == Playing)
	{
		if (IsGameOver())
		{
			State = GameOver;
			RockPlayer->IsDead = true;
			PlayerRod->IsControllable = false;
		}
		else
		{
			// TO DO, speed up
			// update text
			Timer += timeStep;
			if (ScoreBoard != nullptr)
				ScoreBoard->Text = std::string(std::to_string((int) DistanceWalked));

			slidingSpeed *= 1.0f + (0.005f * timeStep);
			if (slidingSpeed > maxSlidingSpeed)
				slidingSpeed = maxSlidingSpeed;

			MapController->SlidingVelocity = Vec2f(0, -slidingSpeed);

			updateItemPoolStateRock(timeStep);
		}
	}
	else if (State == GameOver)
	{
		// Play Animation
		if (DeathAnimationTime > 0.0f)
		{
			DeathAnimationTime -= timeStep;
		}
		else
		{
			State = Result;
		}
	}
	else
	{
		SpeedText->IsEnabled = true;
		Mat* frame = m_Transform->ObjManager->m_Scene->GetTargetFrame();
		VECTOR::vector2f window(frame->size().width, frame->size().height);
		VECTOR::vector2f boardPos = SpeedText->m_Transform->LocalPosition();
		VECTOR::vector2f diff = boardPos - window / 2;
		if (diff.length_squared() > 0.1f)
		{
			boardPos = VECTOR::vector2f::lerp(boardPos, window / 2, timeStep * 2.0f);
			SpeedText->m_Transform->SetLocalPosition(boardPos);
		}
		else
		{
			if (rockResultDelay > 0.0f)
			{
				rockResultDelay -= timeStep;
			}
			else
			{
				// load to title
				m_Transform->ObjManager->m_Scene->ScnManager->LoadScene(1);
			}
		}
	}
}

VECTOR::vector2f TileMapManager::MapSpacePosition(VECTOR::vector2f worldPos)
{
	VECTOR::vector2f mapPos = m_Transform->Position();
	return worldPos - mapPos;
}

VECTOR::vector2i TileMapManager::MapSpaceAtRegion(VECTOR::vector2f mapPos)
{
	// TO DO
	if (mapPos.x < 0)
		mapPos.x -= TileOffset;
	if (mapPos.y < 0)
		mapPos.y -= TileOffset;

	VECTOR::vector2i atGrid(mapPos.x / TileOffset, mapPos.y / TileOffset);
	VECTOR::vector2i regionSize = GetRegionSize();

	if (atGrid.x < 0)
		atGrid.x -= regionSize.x;
	if (atGrid.y < 0)
		atGrid.y -= regionSize.y;

	VECTOR::vector2i relative(atGrid.x / regionSize.x, atGrid.y / regionSize.y);
	VECTOR::vector2i absolute(abs(relative.x) % 2, abs(relative.y) % 2);

	return absolute;
}

VECTOR::vector2i TileMapManager::WorldSpaceAtRegion(VECTOR::vector2f worldPos)
{
	VECTOR::vector2f mapPos = MapSpacePosition(worldPos);
	return MapSpaceAtRegion(mapPos);
}

VECTOR::vector2i TileMapManager::GetRegionSize() 
{ 
	return VECTOR::vector2i(Regions[0][0]->Width, Regions[0][0]->Height); 
}

VECTOR::vector2f TileMapManager::GetRegionRealSize()
{
	return VECTOR::vector2f(GetRegionSize().x * TileOffset, GetRegionSize().y * TileOffset);
}

void TileMapManager::LoadMap(std::string _mapName)
{
	CurrentMapName = _mapName;
	std::vector<std::vector<char>> charMap = parseMap(CurrentMapName);
	InstantiateTiles(charMap);
}

void TileMapManager::InstantiateTiles(std::vector<std::vector<char>>& _charMap)
{
	std::cout << "Instantiate Map!" << std::endl;

	ObjectManager* objMgr = m_Transform->ObjManager;

	if (UseRegionMode)
	{
		// how many regions in x direction divided
		int regionDivideX = 2;	// fixed, mul
		int regionDivideY = 2;	// fixed, div

		int regionWidth = Width;
		int regionHeight = MapLength / 2;

		// printCharMap(_charMap);


		// generate a new char Map with y extended or cut
		std::vector<std::vector<char>> loopCharMap = _charMap;

		// double the width
		for (int i = 0; i < Height; i++)
		{
			loopCharMap[i].resize(Width * 2);
			for (int j = 0; j < Width; j++)
			{
				loopCharMap[i][Width + j] = loopCharMap[i][j];
			}
		}

		// expand Height to Map Length
		loopCharMap.resize(MapLength);
		for (int i = Height; i < MapLength; i++)
		{
			// copy to expand
			loopCharMap[i] = loopCharMap[i - Height];
		}


		Regions.resize(2);
		Regions[0].resize(2);
		Regions[1].resize(2);
		
		// Instantiante Regions
		for (int X = 0; X < regionDivideX; X++)
		{
			for (int Y = 0; Y < regionDivideY; Y++)
			{

				// Region at (X, Y)
				VECTOR::vector2i regionOriginGrid = VECTOR::vector2i(regionWidth * X, regionHeight * Y);
				VECTOR::vector2f mapSpacePosition(regionOriginGrid.x * TileOffset, regionOriginGrid.y * TileOffset);

				// init region
				Object* regionObj = objMgr->CreateStaticObject(mapSpacePosition, 0);
				MapRegion* region = regionObj->AddComponent<MapRegion>(MapRegion());
				region->Init(this, VECTOR::vector2i(X, Y));
				region->Width = regionWidth;
				region->Height = regionHeight;
				region->TileMap.resize(regionHeight);

				for (int y = 0; y < regionHeight; y++)
				{
					region->TileMap[y].resize(regionWidth);
					for (int x = 0; x < regionWidth; x++)
					{
						// std::cout << "Instantiate Tile: " << x << ":" << y << std::endl;
						cv::Vec2f tilePos = cv::Vec2f(Origin) + cv::Vec2f(x * TileOffset, y * TileOffset);

						char code = loopCharMap[regionOriginGrid.y + y][regionOriginGrid.x + x];
						TileInform tileInform = TileSet[code];

						Object* tileObj = objMgr->CreateStaticObject(tilePos, 0);
						tileObj->Parent = regionObj;
						// RX_Y-name x:y
						tileObj->Name = 
							"R" + std::to_string(X) + "_" + std::to_string(Y) + "-" + 
							tileInform.Name + " " + std::to_string(x) + ":" + std::to_string(y);

						Tile* tile = tileObj->AddComponent<Tile>(Tile(this, code, tileInform.ScorePerSecond));
						tile->Coordinate = VECTOR::vector2i(x, y);
						tile->ImageType = tileInform.TileType;

						// TO DO: Add animation, 9 sliced
						std::vector<cv::Ptr<Mat>> imageSets = tileInform.TileImages;
						if (imageSets.size() > 0)
						{
							if (tile->ImageType == Sliced)
							{
								int typeCode = slicedCode(loopCharMap, VECTOR::vector2i(regionWidth * X + x, regionHeight * Y + y));
								// Disable Render Rotation
								tileObj->AddComponent<ImageComponent>(ImageComponent(*imageSets[typeCode]))->RotationEnabled = false;
							}
							else
							{
								// Disable Render Rotation
								tileObj->AddComponent<ImageComponent>(ImageComponent(*imageSets[0]))->RotationEnabled = false;
							}
						}
						region->TileMap[y][x] = tile;
					}
				}
				Regions[Y][X] = region;
			}
		}
	}
	// normal map mode
	else
	{
		//////////////////////
		TileMap.resize(Height);
		for (int y = 0; y < Height; y++)
		{
			TileMap[y].resize(Width);
			for (int x = 0; x < Width; x++)
			{
				// std::cout << "Instantiate Tile: " << x << ":" << y << std::endl;
				cv::Vec2f tilePos = cv::Vec2f(Origin) + cv::Vec2f(x * TileOffset, y * TileOffset);

				char code = _charMap[y][x];
				TileInform tileInform = TileSet[code];

				Object* tileObj = objMgr->CreateStaticObject(tilePos, 0);
				tileObj->Parent = this->m_Transform;
				tileObj->Name = tileInform.Name + " " + std::to_string(x) + ":" + std::to_string(y);

				Tile* tile = tileObj->AddComponent<Tile>(Tile(this, code, tileInform.ScorePerSecond));
				tile->Coordinate = VECTOR::vector2i(x, y);
				tile->ImageType = tileInform.TileType;

				// TO DO: Add animation, 9 sliced
				std::vector<cv::Ptr<Mat>> imageSets = tileInform.TileImages;
				if (imageSets.size() > 0)
				{
					if (tile->ImageType == Sliced)
					{
						int typeCode = slicedCode(_charMap, VECTOR::vector2i(x, y));
						// Disable Render Rotation
						tileObj->AddComponent<ImageComponent>(ImageComponent(*imageSets[typeCode]))->RotationEnabled = false;
					}
					else
					{
						// Disable Render Rotation
						tileObj->AddComponent<ImageComponent>(ImageComponent(*imageSets[0]))->RotationEnabled = false;
					}
				}
				TileMap[y][x] = tile;
			}
		}
	}
}

std::vector<std::vector<char>> TileMapManager::parseMap(std::string _mapName)
{
	std::ifstream mapFile(_mapName);
	std::string line;

	// read width of map
	std::getline(mapFile, line);
	if (line == "xl")
		LoopXEnabled = true;
	else
		LoopXEnabled = false;

	// read width of map
	std::getline(mapFile, line);
	Width = std::stoi(line);

	// read height of map
	std::getline(mapFile, line);
	Height = std::stoi(line);

	// read length of map
	std::getline(mapFile, line);
	MapLength = std::stoi(line);

	// read time of map
	std::getline(mapFile, line);
	MapTime = std::stof(line);

	if (Mode == Rock)
		return std::vector<std::vector<char>>();

	// initialize vector size
	std::vector<std::vector<char>> char_map;
	char_map.resize(Height);
	for (auto &char_array : char_map)
	{
		char_array.resize(Width);
	}

	// read char array from file
	for (int i = 0; i < Height; i++)
	{
		std::getline(mapFile, line);
		for (int j = 0; j < Width; j++)
		{
			char_map[Height - i - 1][j] = line[j];
		}
	}

	mapFile.close();
	return char_map;
}

void TileMapManager::handleRegionMode(float timeStep)
{
	// Check new Region and Sector

	// Frame Center Position
	Mat* frame = m_Transform->ObjManager->m_Scene->GetTargetFrame();
	int CamWidth = frame->size().width;
	int CamHeight = frame->size().height;
	VECTOR::vector2f camPos(CamWidth / 2, CamHeight / 2);

	// Check new Region and Sector
	VECTOR::vector2f camMapSpace = MapSpacePosition(camPos);
	VECTOR::vector2i newRegion = MapSpaceAtRegion(camMapSpace);
	// Regions.at(newRegion.y).at(newRegion.x);
	VECTOR::vector2f newRegionCenter = MapSpacePosition(Regions[newRegion.y][newRegion.x]->CenterWorldPosition());

	VECTOR::vector2i newSector;
	newSector.x = (camMapSpace.x < newRegionCenter.x) ? 0 : 1;
	newSector.y = (camMapSpace.y < newRegionCenter.y) ? 0 : 1;

	// Update
	// new Sector, move other region to fit and if needed, restore
	currentRegion = newRegion;
	
	if (newSector != currentSectorInRegion)
	{
		MapRegion* region = Regions[currentRegion.y][currentRegion.x];
		VECTOR::vector2i newSectorDir(newSector.x * 2 - 1, newSector.y * 2 - 1);
		cv::Vec2f regionPos = region->m_Transform->LocalPosition();

		// bottom left 00
		if (newSector == VECTOR::vector2i(0, 0))
		{
			Regions[currentRegion.y][currentRegion.x ^ 1]->m_Transform
				->SetLocalPosition(cv::Vec2f(regionPos - cv::Vec2f(GetRegionRealSize().x, 0)));

			Regions[currentRegion.y ^ 1][currentRegion.x]->m_Transform
				->SetLocalPosition(cv::Vec2f(regionPos - cv::Vec2f(0, GetRegionRealSize().y)));

			Regions[currentRegion.y ^ 1][currentRegion.x ^ 1]->m_Transform
				->SetLocalPosition(cv::Vec2f(regionPos - cv::Vec2f(GetRegionRealSize().x, 0) - cv::Vec2f(0, GetRegionRealSize().y)));

		}
		// bottom right 10
		else if (newSector == VECTOR::vector2i(1, 0))
		{
			Regions[currentRegion.y][currentRegion.x ^ 1]->m_Transform
				->SetLocalPosition(cv::Vec2f(regionPos + cv::Vec2f(GetRegionRealSize().x, 0)));

			Regions[currentRegion.y ^ 1][currentRegion.x]->m_Transform
				->SetLocalPosition(cv::Vec2f(regionPos - cv::Vec2f(0, GetRegionRealSize().y)));

			Regions[currentRegion.y ^ 1][currentRegion.x ^ 1]->m_Transform
				->SetLocalPosition(cv::Vec2f(regionPos + cv::Vec2f(GetRegionRealSize().x, 0) - cv::Vec2f(0, GetRegionRealSize().y)));
		}
		// top left 01
		else if (newSector == VECTOR::vector2i(0, 1))
		{
			Regions[currentRegion.y][currentRegion.x ^ 1]->m_Transform
				->SetLocalPosition(cv::Vec2f(regionPos - cv::Vec2f(GetRegionRealSize().x, 0)));

			Regions[currentRegion.y ^ 1][currentRegion.x]->m_Transform
				->SetLocalPosition(cv::Vec2f(regionPos + cv::Vec2f(0, GetRegionRealSize().y)));

			Regions[currentRegion.y ^ 1][currentRegion.x ^ 1]->m_Transform
				->SetLocalPosition(cv::Vec2f(regionPos - cv::Vec2f(GetRegionRealSize().x, 0) + cv::Vec2f(0, GetRegionRealSize().y)));
		}
		// top right 11
		else
		{
			Regions[currentRegion.y][currentRegion.x ^ 1]->m_Transform
				->SetLocalPosition(cv::Vec2f(regionPos + cv::Vec2f(GetRegionRealSize().x, 0)));

			Regions[currentRegion.y ^ 1][currentRegion.x]->m_Transform
				->SetLocalPosition(cv::Vec2f(regionPos + cv::Vec2f(0, GetRegionRealSize().y)));

			Regions[currentRegion.y ^ 1][currentRegion.x ^ 1]->m_Transform
				->SetLocalPosition(cv::Vec2f(regionPos + cv::Vec2f(GetRegionRealSize().x, 0) + cv::Vec2f(0, GetRegionRealSize().y)));
		}
		currentSectorInRegion = newSector;
	}
}

void TileMapManager::updateProgressBar(float timeStep)
{
	int currentY = MapSpacePosition(Player->m_Transform->Position()).y - startPosY;
	int finalY = MapLength * TileOffset;
	ProgressValue = (float) currentY / finalY;
	float targetPosY = 50 + 385 * ProgressValue;

	ProgressPointer->m_Transform->SetLocalPosition(cv::Vec2f(ProgressPointer->m_Transform->LocalPosition()[0], targetPosY));
}

void TileMapManager::updateItemPoolStateEagle(float timeStep)
{
	float mul = (Player->Speed - Player->MinSlidingSpeed + 50.0f) / (Player->MaxSlidingSpeed - Player->MinSlidingSpeed);
	ringTimer -= timeStep * mul * 1.5f;
	rockTimer -= timeStep * mul * 1.5f;

	// Spawn Rock
	if (rockTimer < 0.0f)
	{
		rockTimer = rockDuration;

		// get random pos from a 45 ~ 135 degree circle
		float degree = minDegree + (((float)rand()) / (float)RAND_MAX) * (maxDegree - minDegree);
		float rad = degree * 3.1415 / 180;

		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = maxRockRadius - minRockRadius;
		float radius = minRockRadius + random * diff + (20 * Player->Speed / 100);

		VECTOR::vector2f posOffset(std::cos(rad) * radius, std::sin(rad) * radius);

		// get a random anchor position
		VECTOR::vector2f playerPos(Player->m_Transform->Position()[0], Player->m_Transform->Position()[1]);
		VECTOR::vector2f mapPos = MapSpacePosition(playerPos + posOffset);

		Item* rock = getRock();
		spawnItemAt(rock, mapPos);
	}

	// Spawn Ring
	if (ringTimer < 0.0f)
	{
		ringTimer = ringDuration;

		// get random pos from a 45 ~ 135 degree circle
		float degree = minRingDegree + (((float)rand()) / (float)RAND_MAX) * (maxRingDegree - minRingDegree);
		float rad = degree * 3.1415 / 180;

		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = maxRingRadius - minRingRadius;
		float radius = minRingRadius + random * diff + (20 * Player->Speed / 100);

		VECTOR::vector2f posOffset(std::cos(rad) * radius, std::sin(rad) * radius);

		// get a random anchor position
		VECTOR::vector2f playerPos(Player->m_Transform->Position()[0], Player->m_Transform->Position()[1]);
		VECTOR::vector2f mapPos = MapSpacePosition(playerPos + posOffset);

		int count = ringMinNum + rand() % (ringMaxNum - ringMinNum + 1);
		int width = ringMinWidth + rand() % (ringMaxWidth - ringMinWidth + 1);

		for (int i = 0; i < count; i++)
		{
			for (int j = 0; j < width; j++)
			{
				Item* ring = getRing();
				spawnItemAt(ring, mapPos + VECTOR::vector2f(0, 200) + VECTOR::vector2f(j * ringOffset.x, i * ringOffset.y));
			}
		}
	}
}

void TileMapManager::updateItemPoolStateRock(float timeStep)
{
	float currentDistance = MapSpacePosition(PlayerRod->m_Transform->Position()).y - startPosY;

	float diff = currentDistance - DistanceWalked;
	distanceCounter -= diff;
	DistanceWalked = currentDistance;

	if (distanceCounter < 0.0f)
	{
		spawnCounter++;
		currentHoleDistance = VECTOR::vector2f::lerpF(currentHoleDistance, holeMinSpawnDistance, 0.1f);

		distanceCounter = currentHoleDistance;

		//! random radius hole
		float randomRadiusValue = ((float)rand()) / (float)RAND_MAX;
		float rDiff = maxHoleRadius - RockPlayer->Radius;
		float radius = RockPlayer->Radius + randomRadiusValue * rDiff;

		//! random position hole
		float random = ((float) rand()) / (float)RAND_MAX;
		float diff = (PlayerRod->Radius - radius) * 2;
		float posOffsetX = -(PlayerRod->Radius - radius) + random * diff;

		VECTOR::vector2f posOffset(posOffsetX, holeSpawnOffsetY);

		// get a random anchor position
		VECTOR::vector2f playerPos(PlayerRod->m_Transform->Position()[0], PlayerRod->m_Transform->Position()[1]);
		VECTOR::vector2f mapPos = MapSpacePosition(playerPos + posOffset);

		Item* hole = getHole(radius);
		spawnItemAt(hole, mapPos);
	}
}

void TileMapManager::beginRandomHoleSpawn(float minY, float maxY, float offsetY)
{
	for (float posY = minY; posY < maxY; posY += offsetY)
	{
		//! random radius hole
		float randomRadiusValue = ((float)rand()) / (float)RAND_MAX;
		float rDiff = maxHoleRadius - RockPlayer->Radius;
		float radius = RockPlayer->Radius + randomRadiusValue * rDiff;

		//! random position hole
		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = (PlayerRod->Radius - radius) * 2;
		float posOffsetX = -(PlayerRod->Radius - radius) + random * diff;

		VECTOR::vector2f posOffset(posOffsetX, posY);

		// get a random anchor position
		VECTOR::vector2f playerPos(PlayerRod->m_Transform->Position()[0], PlayerRod->m_Transform->Position()[1]);
		VECTOR::vector2f mapPos = MapSpacePosition(playerPos + posOffset);

		Item* hole = getHole(radius);
		spawnItemAt(hole, mapPos);
	}
}

void TileMapManager::spawnItemAt(Item* item, VECTOR::vector2f pos, float Radius)
{
	// cout << "Spawn At: " << pos.x << ":" << pos.y << endl;
	item->m_Transform->m_IsActive = true;
	item->IsEnabled = true;
	item->m_Transform->SetLocalPosition(pos);
}

Item* TileMapManager::getRing()
{
	// find an available ring, which should be disabled
	for (auto& ring : ringPool)
	{
		if (!ring->m_Transform->m_IsActive)
			return ring;
	}
	// instantiate new item
	Object* obj = m_Transform->ObjManager->CreateStaticObject(cv::Vec2f(0, 0), 0);
	obj->Parent = m_Transform;

	ItemInform& inform = ItemSet["Ring"];
	ImageComponent* img =  obj->AddComponent<ImageComponent>(ImageComponent(inform.ImageName));
	img->RotationEnabled = false;
	img->IsEnabled = true;
	Item* item = obj->AddComponent<Item>(Item(inform));
	item->MapManager = this;
	ringPool.push_back(item);

	// Disable 
	obj->m_IsActive = false;

	return item;
}

Item* TileMapManager::getRock()
{
	// find an available ring, which should be disabled
	for (auto& rock : rockPool)
	{
		if (!rock->m_Transform->m_IsActive)
			return rock;
	}
	// instantiate new item
	Object* obj = m_Transform->ObjManager->CreateStaticObject(cv::Vec2f(0, 0), 0);
	obj->Parent = m_Transform;

	int index = 0 + (rand() % 4);

	ItemInform& inform = ItemSet["Rock"+std::to_string(index)];
	ImageComponent* img = obj->AddComponent<ImageComponent>(ImageComponent(inform.ImageName));
	img->RotationEnabled = false;
	img->IsEnabled = true;
	Item* item = obj->AddComponent<Item>(Item(inform));
	item->MapManager = this;
	rockPool.push_back(item);

	// Disable 
	obj->m_IsActive = false;

	return item;
}

Item* TileMapManager::getHole(float radius)
{
	// find an available ring, which should be disabled
	for (auto& hole : holePool)
	{
		if (!hole->m_Transform->m_IsActive)
		{
			hole->Radius = radius;
			return hole;
		}
	}
	// instantiate new item
	Object* obj = m_Transform->ObjManager->CreateStaticObject(cv::Vec2f(0, 0), 0);
	obj->Parent = m_Transform;

	Item* item = obj->AddComponent<Item>(Item());
	item->MapManager = this;

	item->Score = 0;
	item->Speed = 0;
	item->Radius = radius;
	item->Kill = true;
	item->Eatable = false;
	holePool.push_back(item);

	// Disable 
	obj->m_IsActive = false;

	return item;
}

int TileMapManager::slicedCode(const std::vector<std::vector<char>>& _charMap, VECTOR::vector2i _coord)
{
	// TO DO;
	// return 0;

	int width = _charMap[0].size();
	int height = _charMap.size();
	// false if is the same as coord char
	bool Empty[3][3];
	int EmptyCount = 0;

	// check emptyness around the tile
	for (int x = -1; x < 2; x++)
	{
		// out of map, empty
		if (_coord.x + x >= width || _coord.x + x < 0)
		{
			Empty[x + 1][0] = true;
			Empty[x + 1][1] = true;
			Empty[x + 1][2] = true;
			EmptyCount += 3;
		}
		else
		{
			for (int y = -1; y < 2; y++)
			{
				// Middle, myself
				if (x == 0 && y == 0)
					continue;

				// out of map, empty
				if (_coord.y + y >= height || _coord.y + y < 0)
				{
					Empty[x + 1][y + 1] = true;
					EmptyCount += 1;
				}
				else
				{
					if (_charMap[_coord.y + y][_coord.x + x] != _charMap[_coord.y][_coord.x])
					{
						Empty[x + 1][y + 1] = true;
						EmptyCount += 1;
					}
					else
						Empty[x + 1][y + 1] = false;
				}
			}
		}
	}


	// Up Left, 6
	if (Empty[slicedCode2Coord(6).x][slicedCode2Coord(6).y] &&
		Empty[slicedCode2Coord(7).x][slicedCode2Coord(7).y] &&
		Empty[slicedCode2Coord(4).x][slicedCode2Coord(4).y])
		return 6;
	// Down Left, 1
	else if
		(Empty[slicedCode2Coord(1).x][slicedCode2Coord(1).y] &&
		 Empty[slicedCode2Coord(2).x][slicedCode2Coord(2).y] &&
		 Empty[slicedCode2Coord(4).x][slicedCode2Coord(4).y])
		return 1;
	// Down Right, 3
	else if
		(Empty[slicedCode2Coord(2).x][slicedCode2Coord(2).y] &&
		 Empty[slicedCode2Coord(3).x][slicedCode2Coord(3).y] &&
		 Empty[slicedCode2Coord(5).x][slicedCode2Coord(5).y])
		return 3;
	// Up Right, 8
	else if
		(Empty[slicedCode2Coord(5).x][slicedCode2Coord(5).y] &&
		 Empty[slicedCode2Coord(7).x][slicedCode2Coord(7).y] &&
		 Empty[slicedCode2Coord(8).x][slicedCode2Coord(8).y])
		return 8;
	// Left, 4
	else if
		(Empty[slicedCode2Coord(4).x][slicedCode2Coord(4).y])
		return 4;
	// Down, 2
	else if
		(Empty[slicedCode2Coord(2).x][slicedCode2Coord(2).y])
		return 2;
	// Right, 5
	else if
		(Empty[slicedCode2Coord(5).x][slicedCode2Coord(5).y])
		return 5;
	// Up, 7
	else if
		(Empty[slicedCode2Coord(7).x][slicedCode2Coord(7).y])
		return 7;
	else
		return 0;
}

void TileMapManager::printCharMap(const std::vector<std::vector<char>>& _charMap)
{
	for (int i = _charMap.size() - 1; i >= 0; i--)
	{
		for (int j = 0; j < _charMap[i].size(); j++)
		{
			cout << _charMap[i][j];
		}
		cout << endl;
	}
}

bool TileMapManager::IsGameOver()
{
	if (Mode == Eagle)
	{
		float endY = MapLength * TileOffset;
		VECTOR::vector2f playerMapSpacePos = MapSpacePosition(Player->m_Transform->Position());
		if (playerMapSpacePos.y > endY + startPosY)
			return true;
		else
			return false;
	}
	else
	{
		return false;
	}
}

////////////////////////////////////////////////////////////////////

void MapRegion::Init(TileMapManager* mgr, VECTOR::vector2i id)
{
	MapManager = mgr;
	RegionId = id;
	m_Transform->Parent = mgr->m_Transform;
}

void MapRegion::RestoreItems()
{
	for (int i = 0; i < Items.size(); i++)
	{
		Items[i]->Restore();
	}
}

VECTOR::vector2f MapRegion::CenterWorldPosition()
{
	return VECTOR::vector2f(m_Transform->Position()) + VECTOR::vector2f(Width * MapManager->TileOffset, Height * MapManager->TileOffset) / 2;
}

VECTOR::vector2f MapRegion::RegionSpacePosition(const VECTOR::vector2f& worldPos)
{
	return worldPos - VECTOR::vector2f(m_Transform->Position());
}

////////////////////////////////////////////////////////////////////

Item::Item(ItemInform& inform)
{
	Radius = inform.Radius;
	Score = inform.Score;
	Speed = inform.Speed;
	Kill = inform.Kill;
	Eatable = inform.Eatable;
}

void Item::Update(float timeStep)
{
	float edgeOffset = 300;

	if (AutoDisableIfOutOfScreen)
	{
		if (m_Transform->Position()[1] + Radius < -edgeOffset)
		{
			m_Transform->m_IsActive = false;
			return;
		}
	}
	if (MapManager->Mode == TileMapManager::GameMode::Eagle)
		checkCollisionWithPlayer(MapManager->Player);
	else
		checkCollisionWithPlayer(MapManager->RockPlayer);
}

void Item::LateUpdate(float timeStep)
{
	if (MapManager->Mode == TileMapManager::GameMode::Rock)
	{
		float yOffset = 5;
		cv::Point center
			(m_Transform->Position()[0], m_Transform->ObjManager->m_Scene->GetTargetFrame()->size().height - (m_Transform->Position()[1]));
		cv::Point shadCenter
			(m_Transform->Position()[0], m_Transform->ObjManager->m_Scene->GetTargetFrame()->size().height - (m_Transform->Position()[1] + yOffset));
		// draw shadow circle
		cv::circle(*m_Transform->ObjManager->m_Scene->GetTargetFrame(), shadCenter, Radius, cv::Scalar(130, 130, 130), -1);
		// draw main circle
		cv::circle(*m_Transform->ObjManager->m_Scene->GetTargetFrame(), center, Radius, cv::Scalar(0, 0, 0), -1);
	}
}

void Item::checkCollisionWithPlayer(SlidingMapController* player)
{
	VECTOR::vector2f diff = player->m_Transform->Position() - m_Transform->Position();
	if (diff.length() < Radius + player->Radius)
	{
		CollideWithPlayer(player);
	}

}

void Item::checkCollisionWithPlayer(BalancingCharacter* player)
{
	VECTOR::vector2f diff = player->m_Transform->Position() - m_Transform->Position();
	if (diff.length() < Radius - player->Radius)
	{
		CollideWithPlayer(player);
	}
}


void Item::CollideWithPlayer(SlidingMapController* player)
{
	player->AddScore(Score);
	player->AddSpeed(Speed);
	// player

	if (Kill)
		MapManager->State = TileMapManager::GameState::GameOver;

	if (Eatable)
		m_Transform->m_IsActive = false;
}

void Item::CollideWithPlayer(BalancingCharacter* player)
{
	player->Dead();
	MapManager->State = TileMapManager::GameState::GameOver;

	if (Eatable)
		m_Transform->m_IsActive = false;
}

void Item::Restore()
{
	m_Transform->m_IsActive = true;
	IsEnabled = true;
}

////////////////////////////////////////////////////////////////////


// Right Color Space
cv::Scalar ColorSelectManager::RightMaxColor;
cv::Scalar ColorSelectManager::RightMinColor;
cv::Scalar ColorSelectManager::RightAvgColor;

// Left Color Space
cv::Scalar ColorSelectManager::LeftMaxColor;
cv::Scalar ColorSelectManager::LeftMinColor;
cv::Scalar ColorSelectManager::LeftAvgColor;

ColorSelectManager::ColorSelectManager(const std::vector<VECTOR::vector2f>& _rightPos, const std::vector<VECTOR::vector2f>& _leftPos)
: m_RightTargets(_rightPos), m_LeftTargets(_leftPos), SelectTimes(_rightPos.size() + _leftPos.size())
{
	
}

void ColorSelectManager::loadTitleScene()
{
	
	m_Transform->ObjManager->m_Scene->ScnManager->LoadScene(1);
}

void ColorSelectManager::Update(float timeStep)
{
	if (IsFinished)
	{
		if (Delay > 0.0f)
		{
			CounterText->Text = "LOADING";
			Delay -= timeStep;
		}
		else
		{
			loadTitleScene();
			return;
		}
		return;
	}
	// Wait to start
	if (Delay > 0.0f)
	{
		std::string show = "FOLLOW THE CURSOR";
		CounterText->Text = show;
		Delay -= timeStep;
	}
	// Main start
	else
	{
		// Idle
		if (Selector->State == ColorSelector::SelectorState::Idle)
		{
			// change image color to previous selected color
			if (CurrentIndex != 0)
			{
				// HSV to BGR
				cv::Scalar srcS = Selector->AllColors[Selector->AllColors.size() - 1];
				cout << "COLOR SELECTED HSV: " << srcS[0] << ":" << srcS[1] << ":" << srcS[2] << endl;
				
				// TO DO
				// Selector->m_Transform->GetComponent<ImageComponent>()->Color = srcS;
			}

			//std::cout << "Totla No: " << SelectTimes << " Color" << endl;
			//std::cout << "Selecting No." << CurrentIndex << " Color" << endl;
			// right side finished, record
			if (CurrentIndex == m_RightTargets.size())
			{
				RightMaxColor = Selector->MaxColor;
				RightMinColor = Selector->MinColor;
				RightAvgColor = Selector->AverageColor;

				Selector->Initialize();
			}
			// left side finished, record
			else if (CurrentIndex == SelectTimes)
			{
				LeftMaxColor = Selector->MaxColor;
				LeftMinColor = Selector->MinColor;
				LeftAvgColor = Selector->AverageColor;

				Selector->Initialize();
			}

			// selecting for right space
			if (CurrentIndex < m_RightTargets.size())
			{
				Selector->SelectNewColor(m_RightTargets[CurrentIndex], SelectTime);
				//std::cout << "Pos " << m_RightTargets[CurrentIndex].x << ":" << m_RightTargets[CurrentIndex].y << endl;
			}
			// selecting for left space
			else if (CurrentIndex >= m_RightTargets.size() && (CurrentIndex - m_RightTargets.size()) < m_LeftTargets.size())
			{
				Selector->SelectNewColor(m_LeftTargets[CurrentIndex - m_RightTargets.size()], SelectTime);
				//std::cout << "Pos " << m_LeftTargets[CurrentIndex - m_RightTargets.size()].x << ":" << m_RightTargets[CurrentIndex - m_RightTargets.size()].y << endl;
			}
			// finished, load title scene 
			else
			{
				Selector->SelectNewColor(VECTOR::vector2f(320, 240), 20.0f);
				IsFinished = true;
				Delay = SelectTime;
				return;
			}
			CurrentIndex++;
		}
		// Moving
		else if (Selector->State == ColorSelector::SelectorState::Moving)
		{
		}
		// Counting
		else
		{
			CounterText->Text = std::to_string((int) (Selector->m_TimerToSelectColor + 1));
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////
TitleManager::TitleManager()
{
	m_delayTimer = Delay;
	m_selectTimer = SelectTime;
}

void TitleManager::Update(float timeStep)
{
	VECTOR::vector2f directionVector = RightCursor->m_Transform->Position() - LeftCursor->m_Transform->Position();
	CurrentDegree = -VECTOR::vector2f::angle(VECTOR::vector2f(1, 0), directionVector) * 180 / 3.1415;
	
	///////

	if (State == Preparing)
	{
		m_delayTimer -= timeStep;
		if (m_delayTimer < 0.0f)
		{
			EagleImage->LerpColorTo(hideColor, FadeTime, hideColor[3]);
			ResetImage->LerpColorTo(hideColor, FadeTime, hideColor[3]);
			RockImage->LerpColorTo(hideColor, FadeTime, hideColor[3]);
			m_selectTimer = SelectTime;
			State = Selecting;
		}
	}
	else if (State == Selecting)
	{
		SelectState newState = currentSelectAt(CurrentDegree);
		if (newState != SelectAt)
		{
			SelectAt = newState;
			m_selectTimer = SelectTime;
			// hide previously selected image

			if (selectedImage != nullptr)
				selectedImage->LerpColorTo(hideColor, FadeTime, hideColor[3]);

			if (SelectAt == EagleState)
			{
				InformationText->Text = EagleInform;
				selectedImage = EagleImage;
			}
			else if (SelectAt == ResetState)
			{
				InformationText->Text = ResetInform;
				selectedImage = ResetImage;
			}
			else if (SelectAt == RockState)
			{
				InformationText->Text = RockInform;
				selectedImage = RockImage;
			}
			else
			{
				InformationText->Text = NothingInform;
				selectedImage = nullptr;
			}

			if (selectedImage != nullptr)
				selectedImage->LerpColorTo(showColor, FadeTime, showColor[3]);
		}
		else
		{
			m_selectTimer -= timeStep;
			float sizeR = (SelectTime - m_selectTimer);

			if (sizeR > SelectTime)
				sizeR = SelectTime;

			FillCircle->Radius = EdgeCircle->Radius * (sizeR / SelectTime);
			if (m_selectTimer < 0.0f)
			{
				if (SelectAt == EagleState)
					m_Transform->ObjManager->m_Scene->ScnManager->LoadScene(2);
				else if (SelectAt == ResetState)
					m_Transform->ObjManager->m_Scene->ScnManager->LoadScene(0);
				else if (SelectAt == RockState)
					m_Transform->ObjManager->m_Scene->ScnManager->LoadScene(3);
			}
		}
	}
	//////
	else
	{

	}
}

void TitleManager::LateUpdate(float timeStep)
{
	
}