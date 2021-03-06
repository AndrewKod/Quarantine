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

	//Temp For Tests
	this->maxSporeCount = maxSporeCount;
	//Temp For Tests

	this->bCanInviteVisitor = false;
	this->inviteDelay = 15.f;
	this->inviteTimer = 0.f;
	this->healPoints = 3;

	this->bInvincible = true;
}

void CovidMonster::Draw()
{	
	this->drawAlpha = this->minAlpha + (1.f - this->minAlpha)*((float)this->currHealth / (float)this->maxHealth);
 	Render::BeginAlphaMul(this->drawAlpha);
	Target::Draw();
	Render::EndAlphaMul();
}

void CovidMonster::Update(float dt)
{
	Target::Update(dt);
	this->inviteTimer += dt;
	if ( this->inviteTimer >= this->inviteDelay)
	{
		this->inviteTimer = 0.f;
		if (this->bCanInviteVisitor)
		{
			Layer* outdoorLayer = Core::guiManager.getLayer("OutdoorLayer");
			if (outdoorLayer != nullptr)
			{
				Message mess("GameWidget", "invite_visitor");
				outdoorLayer->BroadcastMessage(mess);
			}
		}
	}
}

void CovidMonster::ApplyDamage(int dmgPoints)
{
	//CovidMonster become invulnerable while spores aren't destroyed
	if (!this->bInvincible && this->currHealth > 0 && this->sporeCount == 0)
	{
		this->currHealth -= dmgPoints;
		this->CheckHealth();

		this->drawAlpha = this->minAlpha + (1 - this->minAlpha)*(this->currHealth / this->maxHealth);

		if (this->currHealth <= 0)
			this->Destroy();
		else
			this->Hit();
	}
}




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

		return true;
	}

	return false;
}





void CovidMonster::Heal()
{
	this->currHealth += this->healPoints;
	this->CheckHealth();
	if (this->currHealth > this->maxHealth)
		this->currHealth = this->maxHealth;
}

void CovidMonster::AddSpores(int count)
{
	this->sporeCount += count;
	this->bInvincible = true;
	if (this->sporeCount > this->maxSporeCount)
		this->sporeCount = this->maxSporeCount;
}

void CovidMonster::SubtractSpores(int count)
{
	this->sporeCount -= count;
	if (this->sporeCount <= 0)
	{
		this->bInvincible = false;
		this->sporeCount = 0;
	}
}

void CovidMonster::CheckHealth()
{
	if (this->maxHealth - this->currHealth >= this->healPoints)
		this->bCanInviteVisitor = true;
	else
		this->bCanInviteVisitor = false;
}
