#include "stdafx.h"
#include "CovidMonster.h"
#include "Bullet.h"

CovidMonster::CovidMonster(Render::Texture * targetTex, Render::Texture * healthSegmentTex, Render::Texture * healthSegmentDmgTex,
	int healthSegmentSpace, int maxHealth,
	TimedSpline<FPoint>& trajectorySpline, float splineEndTime, float speedCoef,
	int maxSporeCount)

	:Target(targetTex, healthSegmentTex, healthSegmentDmgTex, healthSegmentSpace, maxHealth,
		trajectorySpline, splineEndTime, speedCoef)
{
	this->drawAlpha = 1.f;
	this->minAlpha = 0.5f;

	this->sporeCount = 0;
	this->maxSporeCount = maxSporeCount;
}

void CovidMonster::Draw()
{	
	this->drawAlpha = this->minAlpha + (1.f - this->minAlpha)*((float)this->currHealth / (float)this->maxHealth);
 	Render::BeginAlphaMul(this->drawAlpha);
	Target::Draw();
	Render::EndAlphaMul();
}

void CovidMonster::ApplyDamage(int dmgPoints)
{
	//CovidMonster become invulnerable while spores aren't destroyed
	if (this->currHealth > 0 && this->sporeCount == 0)
	{
		this->currHealth -= dmgPoints;

		this->drawAlpha = this->minAlpha + (1 - this->minAlpha)*(this->currHealth / this->maxHealth);

		if (this->currHealth <= 0)
			this->Destroy();
		else
			this->Hit();
	}
}

//void CovidMonster::Destroy()
//{
//	this->bWantsDestroy = true;
//	this->OnDestroy.Invoke(this, this->currentPosition +
//		FPoint(this->targetTex->getBitmapRect().Width() / 2, this->targetTex->getBitmapRect().Height() / 2));
//}
//
//void CovidMonster::Hit()
//{
//	this->OnHit.Invoke(this, this->currentPosition +
//		FPoint(this->targetTex->getBitmapRect().Width() / 2, this->targetTex->getBitmapRect().Height() / 2));
//}


bool CovidMonster::CheckBulletCollision(Bullet * bullet)
{
	FRect collisionRect(this->currentPosition.x, this->currentPosition.x + this->GetBitmapRect().Width(),
				  this->currentPosition.y, this->currentPosition.y + this->GetBitmapRect().Height());

	collisionRect.Inflate(bullet->GetCenterOffset().x);
	collisionRect.xEnd += 1.f;
	collisionRect.yStart -= 1.f;
	
	if (collisionRect.Contains(bullet->GetCurrentPosition()))
	{
		this->ApplyDamage();

		//destroying bullet
		bullet->Destroy();
	}

	return false;
}





void CovidMonster::Heal(int healPoints)
{
	this->currHealth += healPoints;
	if (this->currHealth > this->maxHealth)
		this->currHealth = this->maxHealth;
}

void CovidMonster::AddSpores(int count)
{
	this->sporeCount += count;
	if (this->sporeCount > this->maxSporeCount)
		this->sporeCount = this->maxSporeCount;
}

void CovidMonster::SubtractSpores(int count)
{
	this->sporeCount -= count;
	if (this->sporeCount < 0)
		this->sporeCount = 0;
}
