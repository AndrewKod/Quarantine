#pragma once


class Utilities
{
public:
	static float GetTrajectoryAngle(const TimedSpline<FPoint>& trajectorySpline, FPoint currentPosition,
		float currentSplineTime, float prevSplineTime, bool & bSuccess);

	static void CorrectingAngle(float & angle);

	static float CalcReflectedAngle(float directionAngle, float mirrorAngle);

	static bool CheckScreenCollision(FRect screenBounds, FPoint position, float positonOffset, float bounceOffset,
		float & outMirrorAngle, float& outDefaultDirection, FPoint& outNewTrajectoryStart);

};