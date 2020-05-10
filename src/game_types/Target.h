#pragma once

class Bullet;

class Target
{

protected:

	//updating every frame
	float timer;

	Render::Texture* targetTex;

	//both healthSegments have to have same size
	Render::Texture* healthSegmentTex;
	Render::Texture* healthSegmentDmgTex;

	//Space between healthSegments
	int healthSegmentSpace;

	FPoint healthBarOffset;

	int maxHealth;
	int currHealth;	

	TimedSpline<FPoint> trajectorySpline;
	float splineEndTime;
	
	//the larger speedCoef, the slower animation speed
	float speedCoef;

	//updating every frame
	FPoint currentPosition;

	bool bWantsDestroy;
	bool bDestroyedByDamage;

public:

	Event<FPoint> OnHit;
	Event<FPoint> OnDestroy;

public:

	Target() {}
	//both healthSegments have to have same size
	Target(Render::Texture* targetTex, Render::Texture* healthSegmentTex, Render::Texture* healthSegmentDmgTex,
		int healthSegmentSpace, int maxHealth,
		TimedSpline<FPoint>& trajectorySpline, float splineEndTime, float speedCoef);

	virtual void Draw();

	virtual void Update(float dt);

	virtual void ApplyDamage(int dmgPoints = 1);

	virtual void Destroy();

	virtual void Hit();

	virtual bool CheckBulletCollision(Bullet* bullet);

	IRect GetBitmapRect()		{ return this->targetTex->getBitmapRect(); }
	FPoint GetCurrentPosition() { return this->currentPosition; }
	bool WantsDestroy()			{ return this->bWantsDestroy; }
	bool DestroyedByDamage()	{ return this->bDestroyedByDamage; }

	void SetSpeedCoef(float speedCoef) { this->speedCoef = speedCoef; }

private:

	void CalculateCurrentPosition();	

protected:

	void DrawTarget();

};