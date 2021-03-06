#include "stdafx.h"
#include "Globals.h"
#include "Target.h"
#include "Bullet.h"

//both healthSegments have to have same size
Target::Target(Render::Texture * targetTex, Render::Texture * healthSegmentTex, Render::Texture * healthSegmentDmgTex,
	int healthSegmentSpace, int maxHealth,
	TimedSpline<FPoint>& trajectorySpline, float splineEndTime, float speedCoef)
{
	this->timer = 0.f;

	this->targetTex = targetTex;

	this->healthSegmentTex = healthSegmentTex;
	this->healthSegmentDmgTex = healthSegmentDmgTex;
	
	this->healthSegmentSpace = healthSegmentSpace;

	this->maxHealth = maxHealth;
	this->currHealth = maxHealth;	

	this->trajectorySpline = trajectorySpline;
	this->splineEndTime = splineEndTime;	

	this->speedCoef = speedCoef;

	//Calc healthBarOffset

	//targetTex height
	int targetH = this->targetTex->Height();
	//targetTex width
	int targetW = this->targetTex->Width();
	
	//healthSegment width
	int healthSegmW = this->healthSegmentTex->Width();		

	//healthBar width
	float healthBarW = healthSegmW * this->maxHealth + this->healthSegmentSpace * (this->maxHealth - 1);

	this->healthBarOffset.x = targetW / 2 - healthBarW / 2;
	this->healthBarOffset.y = targetH;

	this->currentPosition = FPoint(0.f, 0.f);

	this->bWantsDestroy = false;
	this->bDestroyedByDamage = false;
}

void Target::Draw()
{
	this->CalculateCurrentPosition();

	this->DrawTarget();
}

void Target::Update(float dt)
{
	this->timer += dt;
}

void Target::ApplyDamage(int dmgPoints)
{
	if (this->currHealth > 0)
	{
		this->currHealth -= dmgPoints;
		if (this->currHealth <= 0)
		{
			this->bDestroyedByDamage = true;
			this->Destroy();
		}
		else
			this->Hit();
	}
}

void Target::Destroy()
{
	this->bWantsDestroy = true;
	this->OnDestroy.Invoke(this, this->currentPosition + 
		FPoint(this->targetTex->getBitmapRect().Width()/2, this->targetTex->getBitmapRect().Height() / 2));
}

void Target::Hit()
{
	this->OnHit.Invoke(this, this->currentPosition +
		FPoint(this->targetTex->getBitmapRect().Width() / 2, this->targetTex->getBitmapRect().Height() / 2));
}

bool Target::CheckBulletCollision(Bullet * bullet)
{
	return false;
}

void Target::CalculateCurrentPosition()
{
	float splineTime = this->timer / this->speedCoef;

	if (splineTime >= this->splineEndTime)
	{
		this->timer = 0.f;
	}

	this->currentPosition = this->trajectorySpline.getGlobalFrame(splineTime);
}

void Target::DrawTarget()
{
	//Draw targetTex
	Render::device.PushMatrix();
	Render::device.MatrixTranslate(this->currentPosition.x, this->currentPosition.y, 0);
	this->targetTex->Draw();
	Render::device.PopMatrix();

	//healthSegment width
	int healthSegmW = this->healthSegmentTex->Width();

	float posY = this->currentPosition.y + this->healthBarOffset.y;
	//Draw health bar	
	for (int i = 0; i < this->maxHealth; i++)
	{
		float posX = this->currentPosition.x + this->healthBarOffset.x + healthSegmW * i + this->healthSegmentSpace * i;

		Render::device.PushMatrix();
		Render::device.MatrixTranslate(posX, posY, 0);

		//draw either normal or damaged healthSegments
		i < this->currHealth ? this->healthSegmentTex->Draw() : this->healthSegmentDmgTex->Draw();

		Render::device.PopMatrix();
	}
}
