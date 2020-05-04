#include "stdafx.h"
#include "Globals.h"
#include "Bullet.h"
#include "Utilities.h"

Bullet::Bullet()
{
	CalculateTrajectory(5.f, 50.f, FPoint(0.f, 0.f));	
}

Bullet::Bullet(int bulletId, Render::Texture * bulletTex, ParticleEffectPtr trailEff, float speed,
	FRect screenBounds,	float startAngle, FPoint startPoint)
{
	this->bulletId = bulletId;
	this->bulletTex = bulletTex;
	this->trailEff = trailEff;
	this->speed = speed;	
	this->splineSpeedCoef = 2.f;
	this->timeMultiplyer = 1.f / 25.f;
	this->screenBounds = screenBounds;
	this->centerOffset = FPoint(bulletTex->getBitmapRect().Width()/2, bulletTex->getBitmapRect().Width()/2);

	this->maxRicochets = 3;
	this->currRicochets = 0;

	this->timer = 0.f;
	this->deltaTime = 0.f;

	this->bWantsDestroy = false;
	
	CalculateTrajectory(startAngle, this->speed, startPoint);
	//CalculateTrajectory(90.f, this->speed, startPoint);	
}

void Bullet::Draw()
{
	this->CalculateCurrentPosition();
	if (this->bWantsDestroy)
		return;

	if (!this->screenBounds.Contains(this->currentPosition))
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

	if (this->bWantsDestroy)
		this->Destroy();
}

void Bullet::Update(float dt)
{
	this->timer += dt;
	this->deltaTime = dt;
}

void Bullet::Destroy()
{		
	//this->OnDestroy.Invoke(this, this->bulletId);	
}

void Bullet::CalculateTrajectory(float startAngle, float startSpeed, FPoint startPoint)
{
	bool bPointAbroad = this->screenBounds.Contains(startPoint);
	const int maxAbroadPoints = 10;

	this->trajectorySpline.Clear();	
	this->speedSpline.Clear();
	//Gravity Acceleration
	const int g = 10;

	if (startAngle != 90.f&&startAngle != 270.f)
	{
		float startAngleRad = startAngle / (180 / math::PI);

		float flyLength = 0.f;
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
			flyLength = startSpeed;
		else
			flyLength = math::sqr(startSpeed)*math::sin(angleForCalcs / (180 / math::PI) * 2) / g;		

		float time = 0.f;
		float splineTime = 0.f;
		float x = 0.f;
		float y = 0.f;

		float step = flyLength / 10;		

		bool bPointsToRight = startAngle < 90.f || startAngle > 270.f;			
			
		FPoint splinePoint = startPoint;
		float currSpeedX = startSpeed * math::cos(startAngleRad);
		float currSpeedY = 0.f;

		//int overCount = 0;

		//Add keys to trajectorySpline until first point outside screenBounds 
		do
		{
			time = x / startSpeed / math::cos(startAngleRad);
			currSpeedY = startSpeed * math::sin(startAngleRad) - g * time;
			FPoint vPoint(currSpeedX, currSpeedY);
			float currSpeed = vPoint.GetDistanceToOrigin();			

			splineTime = x / startSpeed / math::cos(startAngleRad) * this->timeMultiplyer;
			y = CalculateY(x, startSpeed, startAngleRad);

			splinePoint = FPoint(x, y);
			splinePoint += startPoint/* + this->centerOffset*/;			

			this->trajectoryEndTime = splineTime;
			this->trajectorySpline.addKey(splineTime, splinePoint);
			this->speedSpline.addKey(splineTime, currSpeed);
			
			bPointsToRight ? x += step : x -= step;

			///////////////////////TESTS///////////////////////
			/*if (!this->screenBounds.Contains(splinePoint))
				overCount++;*/

		} while (this->screenBounds.Contains(splinePoint)/*overCount < 20*/);
	}
	else//gun shooting either verticaly up or down
	{
		float time = 0.f;
		float splineTime = 0.f;
		float currSpeed = startSpeed;
		FPoint splinePoint = startPoint;
		float dist = 0.f;
		bool bUpRight = startAngle == 90.f;
				
		do 
		{
			splinePoint.y = startPoint.y + dist * (bUpRight ? 1.f : -1.f);
			
			this->trajectoryEndTime = splineTime;
			this->speedSpline.addKey(splineTime, math::abs(currSpeed));
			this->trajectorySpline.addKey(splineTime, splinePoint);
			
			splineTime += (1.f * this->timeMultiplyer);
			time += 1.f;
			currSpeed = startSpeed + g * time * (bUpRight ? -1.f : 1.f);
			dist = startSpeed * time + g * math::sqr(time) / 2 * (bUpRight ? -1.f : 1.f);
			
		} while (this->screenBounds.Contains(splinePoint));
	}
	this->trajectorySpline.CalculateGradient();
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

	if (splineTime >= this->trajectoryEndTime)
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
