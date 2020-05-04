#include "stdafx.h"
#include "Globals.h"
#include "CovidSpore.h"
#include "Utilities.h"

CovidSpore::CovidSpore(Render::Texture * targetTex, Render::Texture * healthSegmentTex, Render::Texture * healthSegmentDmgTex,
	int healthSegmentSpace, int maxHealth,
	TimedSpline<FPoint>& trajectorySpline, float splineEndTime, float speedCoef,
	FPoint trajectoryStart, float trajectoryGlobalAngle, float trajectoryStartAngle)

	:Target(targetTex, healthSegmentTex, healthSegmentDmgTex, healthSegmentSpace, maxHealth,
		trajectorySpline, splineEndTime, speedCoef)
{	
	this->trajectoryStart = trajectoryStart;	
	this->trajectoryGlobalAngle = trajectoryGlobalAngle;
	this->trajectoryStartAngle = trajectoryStartAngle;	

	for (int i = 0; i < this->trajectorySpline.GetKeysCount(); i++)
	{
		this->trajectoryKeys.push_back(this->trajectorySpline.GetKey(i));
	}	
	
	this->TransformTrajectory();

	this->currentPosition = trajectoryStart;
	this->centerOffset = FPoint(this->targetTex->Width() / 2, this->targetTex->Height() / 2);
	this->CalcCenterPosition();

	this->cellId = IPoint(-1, -1);

	this->deltaTime = 0.f;
}

float CovidSpore::GetSporeDirection(bool & bSuccess)
{	
	float currSplineTime = this->timer / this->speedCoef;
	float prevSplineTime = (this->timer - this->deltaTime) / this->speedCoef;
	float sporeDirection = Utilities::GetTrajectoryAngle(this->trajectorySpline, this->currentPosition,
		currSplineTime, prevSplineTime, bSuccess);

	return sporeDirection;
}

void CovidSpore::RestartSplineAnimation()
{
	this->timer = 0.f;
	this->TransformTrajectory();
}

void CovidSpore::Draw()
{
	Target::Draw();

	this->CalcCenterPosition();
}

void CovidSpore::Update(float dt)
{
	this->deltaTime = dt;
	this->timer += dt;

	//translating trajectorySpline to its end point and restarting spline animation
	if (this->timer / this->speedCoef > this->splineEndTime)
	{		
		int keysCount = this->trajectorySpline.GetKeysCount();
		this->trajectoryStart = this->trajectorySpline.GetKey(keysCount - 1);

		RestartSplineAnimation();
	}
}

bool CovidSpore::CheckScreenCollision(FRect screenBounds)
{
	float mirrorAngle = 0.f;
	float dafaultSporeDirection = 0.f;
	FPoint newTrajectoryStart(0.f, 0.f);

	bool bCollidesWithScreen = Utilities::CheckScreenCollision(screenBounds, this->centerPosition, this->centerOffset.x,
		10.f, mirrorAngle, dafaultSporeDirection, newTrajectoryStart);	

	if (bCollidesWithScreen)
	{
		bool bSuccess = false;		
		float sporeDirection = this->GetSporeDirection(bSuccess);		
		if (!bSuccess)
		{
			sporeDirection = dafaultSporeDirection;
		}		

		//Calc new trajectory spline angle for transformation
		this->trajectoryGlobalAngle = CalcReflectedSplineAngle(sporeDirection, mirrorAngle);

		this->trajectoryStart = newTrajectoryStart;

		//transforming trajectorySpline to calculated point and angle and restarting spline animation
		RestartSplineAnimation();

		return true;
	}

	return false;
}

bool CovidSpore::CheckSporeCollision(CovidSpore* otherSpore)
{
	float minDist = this->centerOffset.x + otherSpore->GetCenterOffset().x;

	FPoint otherCenterPos = otherSpore->GetCenterPosition();
	float dist = this->centerPosition.GetDistanceTo(otherCenterPos);

	if (dist <= minDist)//spores are colliding
	{
		FPoint translatedThisPos = this->centerPosition - otherCenterPos;
		float lineAngle = translatedThisPos.GetAngle() * 180 / math::PI;

		float thisMirrorAngle = lineAngle + 90;
		float otherMirrorAngle = lineAngle - 90;

		Utilities::CorrectingAngle(lineAngle);
		Utilities::CorrectingAngle(thisMirrorAngle);
		Utilities::CorrectingAngle(otherMirrorAngle);

		//////////////////////////////////CHANGES/////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		bool bSuccess = false;
		float thisSporeDirection = this->GetSporeDirection(bSuccess);

		float thisReflectedAngle = Utilities::CalcReflectedAngle(thisSporeDirection, thisMirrorAngle);
		Utilities::CorrectingAngle(thisReflectedAngle);

		bool bValidReflectedAnge = ValidReflectedAngle(thisReflectedAngle, thisMirrorAngle);

		if (!bSuccess || !bValidReflectedAnge)
		{
			thisSporeDirection = lineAngle + 180;
			thisReflectedAngle = lineAngle;
		}

		bSuccess = false;
		float otherSporeDirection = otherSpore->GetSporeDirection(bSuccess);

		float otherReflectedAngle = Utilities::CalcReflectedAngle(otherSporeDirection, otherMirrorAngle);
		Utilities::CorrectingAngle(otherReflectedAngle);

		bValidReflectedAnge = ValidReflectedAngle(otherReflectedAngle, otherMirrorAngle);
		if (!bSuccess || !bValidReflectedAnge)
		{
			otherSporeDirection = lineAngle /*+ 180*/;
			otherReflectedAngle = lineAngle + 180;
			Utilities::CorrectingAngle(otherReflectedAngle);
		}
		
		//Calc new trajectory spline angle for transformation
		this->trajectoryGlobalAngle = CalcReflectedSplineAngle(thisSporeDirection, thisMirrorAngle);		

		float dDist = minDist - dist;
		float additionalDist = 5.f;
		float bounceDist = dDist + additionalDist;

		FPoint distPoint(bounceDist, 0.f);
		distPoint.Rotate(thisReflectedAngle / (180 / math::PI));

		this->trajectoryStart = this->currentPosition + distPoint;

		//transforming this trajectorySpline to calculated point and angle and restarting spline animation
		RestartSplineAnimation();

		//transforming other trajectorySpline to calculated point and angle and restarting spline animation
		otherSpore->SetTrajectoryGlobalAngle(CalcReflectedSplineAngle(otherSporeDirection, otherMirrorAngle));		

		distPoint = FPoint(bounceDist, 0.f);
		distPoint.Rotate(otherReflectedAngle / (180 / math::PI));

		otherSpore->SetTrajectoryStart(otherSpore->GetCurrentPosition() + distPoint);

		otherSpore->RestartSplineAnimation();

		return true;
	}

	return false;
}

float CovidSpore::CalcReflectedSplineAngle(float sporeDirectionAngle, float mirrorAngle)
{	
	//Reflected trajectory angle
	float reflectedAngle = Utilities::CalcReflectedAngle(sporeDirectionAngle, mirrorAngle);

	//Reflected spline angle
	float splineAngle = reflectedAngle - this->trajectoryStartAngle;

	Utilities::CorrectingAngle(splineAngle);

	return splineAngle;
}





//Don't forget to change trajectoryStart and reset timer
//before this function call
void CovidSpore::TransformTrajectory()
{	
	for (size_t i = 0; i < this->trajectoryKeys.size(); i++)
	{
		FPoint transformedPoint = this->trajectoryKeys[i];

		transformedPoint.Rotate(this->trajectoryGlobalAngle / (180 / math::PI));
		transformedPoint += this->trajectoryStart;		

		this->trajectorySpline.SetKey(i, transformedPoint);
	}
	
	this->trajectorySpline.CalculateGradient();
}


void CovidSpore::CalcCenterPosition()
{
	this->centerPosition = this->currentPosition + this->centerOffset;
}

//Check if spore's reflected angle points to opposite spore
bool CovidSpore::ValidReflectedAngle(float reflectedAngle, float mirrorAngle)
{
	float dMirrorAngle = 180.f - mirrorAngle;

	float dReflectedAngle = reflectedAngle + dMirrorAngle;

	Utilities::CorrectingAngle(dReflectedAngle);

	if (dReflectedAngle > 180.f)
		return false;

	return true;
}
