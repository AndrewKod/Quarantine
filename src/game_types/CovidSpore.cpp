#include "stdafx.h"
#include "Globals.h"
#include "CovidSpore.h"
#include "Utilities.h"
#include "Bullet.h"

CovidSpore::CovidSpore(Render::Texture * targetTex, Render::Texture * healthSegmentTex, Render::Texture * healthSegmentDmgTex,
	int healthSegmentSpace, int maxHealth,
	TimedSpline<FPoint>& trajectorySpline, float splineEndTime, float speedCoef,
	FPoint trajectoryStart, float trajectoryGlobalAngle, float trajectoryStartAngle, float atackCommonTime)

	:Target(targetTex, healthSegmentTex, healthSegmentDmgTex, healthSegmentSpace, maxHealth,
		trajectorySpline, splineEndTime, speedCoef)
{	
	this->commonEndTime = splineEndTime;

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

	this->atackCommonTime = atackCommonTime;

	this->bAtacking = false;
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
	if(!this->bAtacking)
		Target::Draw();
	else
	{
		this->CalculateCurrentPosition();

		Target::DrawTarget();
	}

	this->CalcCenterPosition();
}

void CovidSpore::Update(float dt)
{
	this->deltaTime = dt;
	this->timer += dt;

	//translating trajectorySpline to its end point and restarting spline animation
	if (this->timer / this->speedCoef > this->splineEndTime)
	{		
		if (!this->bAtacking)
		{
			int keysCount = this->trajectorySpline.GetKeysCount();
			this->trajectoryStart = this->trajectorySpline.GetKey(keysCount - 1);
		}
		else
		{
			this->bAtacking = false;
			this->RecalculateTrajectoryFactors();
		}

		RestartSplineAnimation();
	}
}

void CovidSpore::Destroy()
{
	this->bWantsDestroy = true;
	this->OnDestroy.Invoke(this, this->centerPosition);
}

void CovidSpore::Hit()
{
	this->OnHit.Invoke(this, this->centerPosition);
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

	FPoint otherCenterPos = otherSpore->GetCurrentPosition();
	float dist = this->centerPosition.GetDistanceTo(otherCenterPos);

	float dDist = minDist - dist;
	float additionalDist = 5.f;
	float bounceDist = dDist + additionalDist;

	if (dist <= minDist)//bodies are colliding
	{
		FPoint translatedThisPos = this->centerPosition - otherCenterPos;
		//Angle of line between spores centers
		float sporesAngle = translatedThisPos.GetAngle() * 180 / math::PI;

		float thisMirrorAngle = sporesAngle + 90;
		float otherMirrorAngle = sporesAngle - 90;

		Utilities::CorrectingAngle(sporesAngle);
		Utilities::CorrectingAngle(thisMirrorAngle);
		Utilities::CorrectingAngle(otherMirrorAngle);
		
		//transforming this trajectorySpline to calculated point and angle and restarting spline animation
		this->ReflectSporeTrajectory(sporesAngle, thisMirrorAngle, bounceDist);

		//transforming other trajectorySpline to calculated point and angle and restarting spline animation
		otherSpore->ReflectSporeTrajectory(sporesAngle, otherMirrorAngle, bounceDist, true);

		return true;
	}

	return false;
}

void CovidSpore::ReflectSporeTrajectory(float sporesAngle, float mirrorAngle, float bounceDist, bool bIsOpposite)
{
	bool bSuccess = false;
	float SporeDirection = this->GetSporeDirection(bSuccess);

	float ReflectedAngle = Utilities::CalcReflectedAngle(SporeDirection, mirrorAngle);
	Utilities::CorrectingAngle(ReflectedAngle);

	bool bValidReflectedAnge = ValidReflectedAngle(ReflectedAngle, mirrorAngle);

	if (!bSuccess || !bValidReflectedAnge)
	{
		SporeDirection = sporesAngle + 180 * !bIsOpposite;
		ReflectedAngle = sporesAngle + 180 * bIsOpposite;
		if(bIsOpposite)
			Utilities::CorrectingAngle(ReflectedAngle);
	}

	//Calc new trajectory spline angle for transformation
	this->trajectoryGlobalAngle = CalcReflectedSplineAngle(SporeDirection, mirrorAngle);

	FPoint distPoint(bounceDist, 0.f);
	distPoint.Rotate(ReflectedAngle / (180 / math::PI));

	this->trajectoryStart = this->currentPosition + distPoint;

	//transforming this trajectorySpline to calculated point and angle and restarting spline animation
	RestartSplineAnimation();
}

bool CovidSpore::CheckBulletCollision(Bullet* bullet)
{
	float minDist = this->centerOffset.x + bullet->GetCenterOffset().x;

	FPoint otherCenterPos = bullet->GetCurrentPosition();
	float dist = this->centerPosition.GetDistanceTo(otherCenterPos);

	float dDist = minDist - dist;
	float additionalDist = 5.f;
	float bounceDist = dDist + additionalDist;

	if (dist <= minDist)//bodies are colliding
	{
		this->ApplyDamage(1);

		if (this->currHealth > 0 && !this->bAtacking)
		{
			FPoint translatedThisPos = this->centerPosition - otherCenterPos;
			float sporesAngle = translatedThisPos.GetAngle() * 180 / math::PI;

			float thisMirrorAngle = sporesAngle + 90;
			float otherMirrorAngle = sporesAngle - 90;

			Utilities::CorrectingAngle(sporesAngle);
			Utilities::CorrectingAngle(thisMirrorAngle);
			Utilities::CorrectingAngle(otherMirrorAngle);

			//transforming this trajectorySpline to calculated point and angle and restarting spline animation
			this->ReflectSporeTrajectory(sporesAngle, thisMirrorAngle, bounceDist);
		}		
		
		//destroying bullet
		bullet->Destroy();

		return true;
	}

	return false;
}

bool CovidSpore::CheckVisitorCollision(FRect collideArea)
{
	if (collideArea.Contains(this->currentPosition))
	{
		Layer* outdoorLayer = Core::guiManager.getLayer("OutdoorLayer");
		Layer* mainLayer = Core::guiManager.getLayer("MainLayer");
		if (outdoorLayer != nullptr && mainLayer != nullptr)
		{
			Message mess("GameWidget", "infect_visitor");
			outdoorLayer->BroadcastMessage(mess);					
			
			this->bWantsDestroy = true;

			return true;
		}		
		
		
	}

	return false;
}

void CovidSpore::CalculateCurrentPosition()
{
	float splineTime = this->timer / this->speedCoef;

	if (splineTime > this->splineEndTime)
	{
		this->timer = 0.f;		
	}
	if(!this->bAtacking)
		this->currentPosition = this->trajectorySpline.getGlobalFrame(splineTime);
	else
		this->currentPosition = this->atackSpline.getGlobalFrame(splineTime);
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

//prepare to start common trajectorySpline after atacking visitor
void CovidSpore::RecalculateTrajectoryFactors()
{
	this->trajectoryStart = this->currentPosition;
	if (this->trajectoryStart != this->atackSpline.GetKey(0))
	{
		this->trajectoryGlobalAngle = (this->trajectoryStart - this->atackSpline.GetKey(0)).GetAngle() * 180 / math::PI;
		Utilities::CorrectingAngle(this->trajectoryGlobalAngle);
	}
}

void CovidSpore::AtackVisitor(FPoint targetPoint)
{
	if (this->bAtacking)
		return;

	this->bAtacking = true;

	const int divider = 5;

	const float commonDist = Render::device.Width();

	float distStep = commonDist / 5;
	float timeStep = this->atackCommonTime / 5;

	float endDist = this->currentPosition.GetDistanceTo(targetPoint);
	float endTime = this->atackCommonTime * (endDist / commonDist);

	float dX = (targetPoint.x - this->currentPosition.x) / divider;
	float dY = (targetPoint.y - this->currentPosition.y) / divider;
	float dT = endTime / divider;

	float currTime = 0.f;

	this->atackSpline.Clear();
	int count = 0;

	while (count <= divider)
	{
		this->atackSpline.addKey(currTime, FPoint(this->currentPosition.x + dX * count, this->currentPosition.y + dY * count));
		
		currTime += dT;
		count++;
	}

	this->timer = 0;
	this->splineEndTime = endTime;
	//add last point
	//this->atackSpline.addKey(endTime, targetPoint);

	this->atackSpline.CalculateGradient();
}

void CovidSpore::StopAtack()
{
	if (!this->bAtacking)
		return;	

	this->bAtacking = false;

	if (this->bWantsDestroy)
		return;

	this->splineEndTime = this->commonEndTime;

	RecalculateTrajectoryFactors();

	RestartSplineAnimation();
}
