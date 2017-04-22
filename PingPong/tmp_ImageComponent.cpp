
	bool imageUpdated = false;
	
	// Resize image if scale has been changed
	VECTOR::vector2f difference = m_PreScale - VECTOR::vector2f(m_Transform->Scale());
	if (difference.length_squared() > 0.01f || m_OutputImage.size().area() == 0)
	{
		m_PreScale = VECTOR::vector2f(m_Transform->Scale());
		calculateResizeImage(m_PreScale);
		imageUpdated = true;
	}

	// Recalculate rotated image if rotation has been changed
	if (m_PreRotAngle != (int) m_Transform->Rotation || m_ResizeRotImage.size().area() == 0)
	{
		m_PreRotAngle = m_Transform->Rotation;
		calculateRotateImage(m_PreRotAngle);
		imageUpdated = true;
	}
	
	// current final image is resize-rotated image
	if (imageUpdated)
	{
		m_OutputImage = m_ResizeRotImage;
		m_OutputMask = m_ResizeRotMask;
	}
	

	m_OutputImage = m_Image;
	m_OutputMask = m_Mask;
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
		cv::Scalar globalMaskColor = (UseGlobalMask)? (GlobalMaskColor / 255.0) : cv::Scalar(1, 1, 1, 1);

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