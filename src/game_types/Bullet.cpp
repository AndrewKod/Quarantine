#include "stdafx.h"
#include "Globals.h"
#include "Bullet.h"
#include "Utilities.h"

Bullet::Bullet()
{
	CalculateTrajectory(5.f, 50.f, FPoint(0.f, 0.f));	
}

Bullet::Bullet(int bulletId, Render::Texture * bulletTex, float speed,
	FRect screenBounds,	float startAngle, FPoint startPoint)
{
	this->bulletId = bulletId;
	this->bulletTex = bulletTex;	
	this->speed = speed;	
	this->splineSpeedCoef = 2.f;
	this->timeMultiplier = 1.f / 25.f;
	this->screenBounds = screenBounds;
	this->centerOffset = FPoint(bulletTex->getBitmapRect().Width()/2, bulletTex->getBitmapRect().Width()/2);

	this->maxRicochets = 3;
	this->currRicochets = 0;

	this->timer = 0.f;
	this->deltaTime = 0.f;

	this->bStartsAbroad = true;

	this->bWantsDestroy = false;

	this->trailEff = this->effCont.AddEffect("Iskra");
	this->trailEff->Reset();
	
	CalculateTrajectory(startAngle, this->speed, startPoint);
	//CalculateTrajectory(90.f, this->speed, startPoint);	
}

Bullet::~Bullet()
{
	if (this->trailEff != nullptr)
	{
		this->trailEff->Finish();
		this->trailEff = nullptr;
	}
}

void Bullet::Draw()
{
	this->CalculateCurrentPosition();
	if (this->bWantsDestroy)
		return;

	if (!this->screenBounds.Contains(this->currentPosition))
	{
		if (!this->bStartsAbroad)
		{
			RecalculateTrajectory();

			if (this->bWantsDestroy)
				return;

			this->timer = 0.f;
			this->deltaTime = 0.f;
			this->CalculateCurrentPosition();

			if (this->bWantsDestroy)
				return;
		}			
	}
	else
		this->bStartsAbroad = false;


	//Draw bulletTex
	Render::device.PushMatrix();
	Render::device.MatrixTranslate(this->drawPosition);
	this->bulletTex->Draw();
	Render::device.PopMatrix();

	//stdafx.h	
#ifdef WITH_DEBUG
	Render::BindFont("arial");

	Render::PrintString(this->currentPosition, utils::lexical_cast(this->bulletId), 2.0f, CenterAlign, TopAlign);
#endif

	//Updating trail position
	this->trailEff->posX = this->currentPosition.x;
	this->trailEff->posY = this->currentPosition.y;

	this->effCont.Draw();

	if (this->bWantsDestroy)
		this->Destroy();
}

void Bullet::Update(float dt)
{
	this->timer += dt;
	this->deltaTime = dt;

	this->effCont.Update(dt);
}

float Bullet::GetBulletDirection(bool & bSuccess)
{
	float splineTime = this->timer / this->splineSpeedCoef;
	float prevSplineTime = (this->timer - this->deltaTime) / this->splineSpeedCoef;
	float bulletDirection = Utilities::GetTrajectoryAngle(this->trajectorySpline, this->currentPosition,
		splineTime, prevSplineTime, bSuccess);

	return bulletDirection;
}

void Bullet::Destroy()
{		
	this->bWantsDestroy = true;
	//this->OnDestroy.Invoke(this, this->bulletId);	
}

void Bullet::CalculateTrajectory(float startAngle, float startSpeed, FPoint startPoint)
{
	bool bPointAbroad = !this->screenBounds.Contains(startPoint);
	if(this->bStartsAbroad)
		this->bStartsAbroad = bPointAbroad;
	const int maxAbroadPoints = 10;
	int abroadPointsCount = 0;

	this->trajectorySpline.Clear();	
	this->speedSpline.Clear();
	//Gravity Acceleration
	const int g = 10;

	if (startAngle != 90.f && startAngle != 270.f)
	{
		this->CalculateParabolicTrajectory(startAngle, startSpeed, startPoint, bPointAbroad, maxAbroadPoints, abroadPointsCount);
	}
	else//gun shooting either verticaly up or down
	{
		this->CalculateVerticalTrajectory(startAngle, startSpeed, startPoint, bPointAbroad, maxAbroadPoints, abroadPointsCount);
	}
	if (this->trajectorySpline.GetKeysCount() > 1)
		this->trajectorySpline.CalculateGradient();
	else
		this->bWantsDestroy = true;
}

float Bullet::CalculateY(float x, float startSpeed, float startAngleRad)
{
	//Gravity Acceleration
	int g = 10;
	return x * math::tan(startAngleRad) - (g*math::sqr(x) / (2 * math::sqr(startSpeed)*math::sqr(math::cos(startAngleRad))));
}

void Bullet::CalculateCurrentPosition()
{
	float splineTime = this->timer / this->splineSpeedCoef;

	this->currentPosition = this->trajectorySpline.getGlobalFrame(splineTime);
	
	this->drawPosition = this->currentPosition - this->centerOffset;

	if (splineTime >= this->trajectoryEndTime && this->bStartsAbroad)
		this->bWantsDestroy = true;
}

void Bullet::RecalculateTrajectory()
{
	this->currRicochets++;
	//destroying bullet if maxRicochets reached
	if (this->currRicochets >= this->maxRicochets)
	{
		this->bWantsDestroy = true;
	}
	else
	{
		float mirrorAngle = 0.f;
		float dafaultTrajectoryAngle = 0.f;
		FPoint newTrajectoryStart(0.f, 0.f);

		bool bCollidesWithScreen = Utilities::CheckScreenCollision(this->screenBounds, this->currentPosition, 0.f,
			0.f, mirrorAngle, dafaultTrajectoryAngle, newTrajectoryStart);

		if (bCollidesWithScreen)
		{
			float splineTime = this->timer / this->splineSpeedCoef;
			float prevSplineTime = (this->timer - this->deltaTime) / this->splineSpeedCoef;
			float currSpeed = this->speedSpline.getGlobalFrame(splineTime);
			currSpeed -= 5.f;
			if (currSpeed < 1.f)
				currSpeed = 1.f;

			bool bSuccess = false;
			float currTrajectoryAngle = Utilities::GetTrajectoryAngle(this->trajectorySpline, this->currentPosition,
				splineTime, prevSplineTime, bSuccess);
			if (!bSuccess)
			{
				currTrajectoryAngle = dafaultTrajectoryAngle;
			}
			Utilities::CorrectingAngle(currTrajectoryAngle);

			float reflectedTrajectoryAngle = Utilities::CalcReflectedAngle(currTrajectoryAngle, mirrorAngle);
			Utilities::CorrectingAngle(reflectedTrajectoryAngle);

			this->CalculateTrajectory(reflectedTrajectoryAngle, currSpeed, newTrajectoryStart);
		}
	}
}

float Bullet::CalculateFlyLength(float startAngle, float startSpeed)
{	
	const int g = 10;
	float angleForCalcs = 0.f;
	if (startAngle >= 0.f && startAngle < 90.f)
	{
		angleForCalcs = startAngle;
	}
	else if (startAngle > 90.f && startAngle <= 180.f)
	{
		angleForCalcs = 180.f - startAngle;
	}
	else if (startAngle > 180.f && startAngle < 270.f)
	{
		angleForCalcs = startAngle - 180.f;
	}
	else
	{
		angleForCalcs = 90.f - (startAngle - 270.f);
	}


	if (angleForCalcs >= 0 && angleForCalcs < 5)
		return startSpeed;
	else
		return math::sqr(startSpeed)*math::sin(angleForCalcs / (180 / math::PI) * 2) / g;
}

void Bullet::ProcessTrajectoryVariables(bool & bStop, bool bContainsPoint, bool& bPointAbroad,
	const int maxAbroadPoints, int& abroadPointsCount)
{
	if (!bContainsPoint && bPointAbroad)
	{
		abroadPointsCount++;
		if (abroadPointsCount >= maxAbroadPoints)
			bStop = true;
	}
	else if (bContainsPoint && bPointAbroad)
	{
		bPointAbroad = false;
	}
	else if (!bContainsPoint && !bPointAbroad)
	{
		bStop = true;
	}
}

void Bullet::CalculateParabolicTrajectory(float startAngle, float startSpeed, FPoint startPoint,
	bool& bPointAbroad, const int maxAbroadPoints, int& abroadPointsCount)
{
	const int g = 10;

	float startAngleRad = startAngle / (180 / math::PI);

	float flyLength = this->CalculateFlyLength(startAngle, startSpeed);

	float time = 0.f;
	float splineTime = 0.f;
	float x = 0.f;
	float y = 0.f;

	float step = flyLength / 10;

	bool bPointsToRight = startAngle < 90.f || startAngle > 270.f;

	FPoint splinePoint = startPoint;
	float currSpeedX = startSpeed * math::cos(startAngleRad);
	float currSpeedY = 0.f;

	bool bStop = false;

	//Add keys to trajectorySpline until first point outside screenBounds 
	do
	{
		time = x / startSpeed / math::cos(startAngleRad);
		currSpeedY = startSpeed * math::sin(startAngleRad) - g * time;
		FPoint speedPoint(currSpeedX, currSpeedY);
		float currSpeed = speedPoint.GetDistanceToOrigin();

		splineTime = x / startSpeed / math::cos(startAngleRad) * this->timeMultiplier;
		y = CalculateY(x, startSpeed, startAngleRad);

		splinePoint = FPoint(x, y);
		splinePoint += startPoint;

		this->trajectoryEndTime = splineTime;
		this->trajectorySpline.addKey(splineTime, splinePoint);
		this->speedSpline.addKey(splineTime, currSpeed);

		bPointsToRight ? x += step : x -= step;

		bool bContainsPoint = this->screenBounds.Contains(splinePoint);
		ProcessTrajectoryVariables(bStop, bContainsPoint, bPointAbroad,
			maxAbroadPoints, abroadPointsCount);

	} while (!bStop/*this->screenBounds.Contains(splinePoint)*//*overCount < 20*/);
}

void Bullet::CalculateVerticalTrajectory(float startAngle, float startSpeed, FPoint startPoint,
	bool& bPointAbroad, const int maxAbroadPoints, int& abroadPointsCount)
{
	const int g = 10;

	float time = 0.f;
	float splineTime = 0.f;
	float currSpeed = startSpeed;
	FPoint splinePoint = startPoint;
	float dist = 0.f;
	bool bUpRight = startAngle == 90.f;

	bool bStop = false;

	do
	{
		splinePoint.y = startPoint.y + dist * (bUpRight ? 1.f : -1.f);

		this->trajectoryEndTime = splineTime;
		this->speedSpline.addKey(splineTime, math::abs(currSpeed));
		this->trajectorySpline.addKey(splineTime, splinePoint);

		splineTime += (1.f * this->timeMultiplier);
		time += 1.f;
		currSpeed = startSpeed + g * time * (bUpRight ? -1.f : 1.f);
		dist = startSpeed * time + g * math::sqr(time) / 2 * (bUpRight ? -1.f : 1.f);

		bool bContainsPoint = this->screenBounds.Contains(splinePoint);
		this->ProcessTrajectoryVariables(bStop, bContainsPoint, bPointAbroad,
			maxAbroadPoints, abroadPointsCount);

	} while (!bStop/*this->screenBounds.Contains(splinePoint)*/);
}
