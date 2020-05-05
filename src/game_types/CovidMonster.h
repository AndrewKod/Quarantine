#pragma once

#include "Target.h"

class CovidMonster : public Target
{
	float drawAlpha;
	float minAlpha;

	int sporeCount;
	int maxSporeCount;

public:
	CovidMonster(Render::Texture* targetTex, Render::Texture* healthSegmentTex, Render::Texture* healthSegmentDmgTex,
		int healthSegmentSpace, int maxHealth,
		TimedSpline<FPoint>& trajectorySpline, float splineEndTime, float speedCoef,
		int maxSporeCount);

	void Draw() override;

	void ApplyDamage(int dmgPoints = 1) override;

	/*void Destroy() override;

	void Hit() override;*/

	bool CheckBulletCollision(Bullet* bullet) override;

	//CovidMonster heals itself if visitor infected
	void Heal(int healPoints);

	int GetSporeCount()		{ return this->sporeCount; }
	int GetMaxSporeCount()	{ return this->maxSporeCount; }

	void SetMaxSporeCount(int maxSporeCount) { this->maxSporeCount = maxSporeCount; }

	void AddSpores(int count = 1);
	void SubtractSpores(int count = 1);
};