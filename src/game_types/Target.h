#pragma once

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

public:

	Target() {}
	//both healthSegments have to have same size
	Target(Render::Texture* targetTex, Render::Texture* healthSegmentTex, Render::Texture* healthSegmentDmgTex,
		int healthSegmentSpace, int maxHealth,
		TimedSpline<FPoint>& trajectorySpline, float splineEndTime, float speedCoef);

	virtual void Draw();

	virtual void Update(float dt);

	virtual void ApplyDamage(int dmgPoints);

	virtual void Destroy();

	IRect GetBitmapRect()		{ return this->targetTex->getBitmapRect(); }
	FPoint GetCurrentPosition() { return this->currentPosition; }

	void SetSpeedCoef(float speedCoef) { this->speedCoef = speedCoef; }

private:


};