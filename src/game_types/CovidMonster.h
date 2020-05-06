#pragma once

#include "Target.h"

class CovidMonster : public Target
{
	float drawAlpha;
	float minAlpha;

	int sporeCount;
	int maxSporeCount;

	bool bInvincible;

	bool bCanInviteVisitor;
	float inviteDelay;
	float inviteTimer;

	int healPoints;

public:
	CovidMonster(Render::Texture* targetTex, Render::Texture* healthSegmentTex, Render::Texture* healthSegmentDmgTex,
		int healthSegmentSpace, int maxHealth,
		TimedSpline<FPoint>& trajectorySpline, float splineEndTime, float speedCoef,
		int maxSporeCount);

	void Draw() override;

	void Update(float dt) override;

	void ApplyDamage(int dmgPoints = 1) override;

	/*void Destroy() override;

	void Hit() override;*/

	bool CheckBulletCollision(Bullet* bullet) override;

	//CovidMonster heals itself if visitor infected
	void Heal();

	int GetSporeCount()		{ return this->sporeCount; }
	int GetMaxSporeCount()	{ return this->maxSporeCount; }
	bool CanInviteVisitor() { return this->bCanInviteVisitor; }
	float GetInviteDelay()	{ return this->inviteDelay; }


	void SetMaxSporeCount(int maxSporeCount) { this->maxSporeCount = maxSporeCount; }

	void SetInvincible() { this->bInvincible = true; }

	void AddSpores(int count = 1);
	void SubtractSpores(int count = 1);

	void CheckHealth();
};