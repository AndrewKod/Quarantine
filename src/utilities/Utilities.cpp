#include "stdafx.h"
#include "Utilities.h"


float Utilities::GetTrajectoryAngle(const TimedSpline<FPoint>& trajectorySpline, FPoint currentPosition, 
	float currentSplineTime, float prevSplineTime, bool & bSuccess)
{
	if (currentSplineTime != 0.0f)
	{		
		FPoint prevFramePos = trajectorySpline.getGlobalFrame(prevSplineTime);

		//Getting angle of line builded on prevFramePos and currentPosition
		FPoint transCurrPos = currentPosition - prevFramePos;

		if (transCurrPos != FPoint(0.f, 0.f))
		{
			bSuccess = true;
			return transCurrPos.GetAngle() * 180 / math::PI;
		}
	}
	bSuccess = false;
	return 0.0f;
}


void Utilities::CorrectingAngle(float & angle)
{
	if (angle < 0)
		angle = 360 + angle;

	while (angle >= 360)
	{
		angle -= 360;
	}
}

float Utilities::CalcReflectedAngle(float directionAngle, float mirrorAngle)
{
	Utilities::CorrectingAngle(directionAngle);
	Utilities::CorrectingAngle(mirrorAngle);

	return 360 - directionAngle + mirrorAngle * 2;;
}

bool Utilities::CheckScreenCollision(FRect screenBounds, FPoint position, float positonOffset, float bounceOffset,
	float & outMirrorAngle, float& outDefaultDirection, FPoint& outNewTrajectoryStart)
{
	float dLeft = position.x - screenBounds.xStart;
	float dRight = screenBounds.xEnd - position.x;
	float dTop = screenBounds.yEnd - position.y;
	float dBot = position.y - screenBounds.yStart;

	bool bAtLeft = dLeft <= positonOffset;
	bool bAtRigth = dRight <= positonOffset;
	bool bAtTop = dTop <= positonOffset;
	bool bAtBot = dBot <= positonOffset;

	int intersectedEdges = 0;

	//angle of edge from which object reflects 
	//theoreticaly, in case when object at the screen corner - it can intersect two edges
	//and mirrorAngle is average between angles of edges
	outMirrorAngle = 0.f;
	//for case when transCurrPos point calculated further has zero coords
	outDefaultDirection = 0.f;

	outNewTrajectoryStart = position - FPoint(positonOffset, positonOffset);

	if (bAtRigth)
	{
		outMirrorAngle += 270.f;
		intersectedEdges++;
		outNewTrajectoryStart.x -= (positonOffset - dRight + bounceOffset);//place object on bounceOffset pixels further from the edge
		outDefaultDirection += 0.f;
	}
	if (bAtTop)
	{
		//if spore is at right top corner, the top edge's angle is 360
		//otherwise - it's 0
		outMirrorAngle += 360.f*(outMirrorAngle > 0);
		intersectedEdges++;
		outNewTrajectoryStart.y -= (positonOffset - dTop + bounceOffset);//place object on bounceOffset pixels further from the edge
		outDefaultDirection += 90.f;
	}
	if (bAtLeft)
	{
		outMirrorAngle += 90.f;
		intersectedEdges++;
		outNewTrajectoryStart.x += (positonOffset - dLeft + bounceOffset);//place object on bounceOffset pixels further from the edge
		outDefaultDirection += 180.f;
	}
	if (bAtBot)
	{
		outMirrorAngle += 180.f;
		intersectedEdges++;
		outNewTrajectoryStart.y += (positonOffset - dBot + bounceOffset);//place object on bounceOffset pixels further from the edge
		if (bAtRigth)
			outDefaultDirection += 360.f;
		outDefaultDirection += 270.f;
	}

	if (bAtRigth || bAtLeft || bAtTop || bAtBot)
	{
		outMirrorAngle /= intersectedEdges;
		outDefaultDirection /= intersectedEdges;		

		return true;
	}

	return false;
}

bool Utilities::SqueezeRectangle(FRect & rect, float value)
{
	value = math::abs(value);
	if (value > rect.Width() / 2 || value > rect.Height() / 2)
		return false;

	rect.xStart += value;
	rect.yStart += value;
	rect.xEnd -= value;
	rect.yEnd -= value;

	return true;
}
