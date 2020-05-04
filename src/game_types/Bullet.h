#pragma once

class Bullet
{
	//updating every frame
	float timer;

	//using to calc bullet's collision angle
	//updating every frame
	float deltaTime;

	int bulletId;

	Render::Texture* bulletTex;		

	TimedSpline<FPoint> trajectorySpline;
	float trajectoryEndTime;
	FRect screenBounds;

	bool bStartsAbroad;

	TimedSpline<float> speedSpline;

	float speed;
	//the larger speedCoef, the slower animation speed
	float splineSpeedCoef;

	//using to control time between trajectorySpline keys in trajectorySpline generation
	float timeMultiplyer;


	//updating every frame
	FPoint currentPosition;	
	FPoint drawPosition;
	FPoint centerOffset;

	//grid cell coords
	//updating every frame
	IPoint cellId;		

	int maxRicochets;
	int currRicochets;

	bool bWantsDestroy;

public:

	//Event<int> OnDestroy; 

public:

	Bullet();

	//bulletTex have to have equal width and height
	//Start point of trajectorySpline have to be (0;0) for easy spline transformations
	//Also trajectorySpline have to be periodic
	Bullet(int bulletId,
		Render::Texture* bulletTex, 		
		float speed,
		FRect screenBounds,
		float startAngle,
		FPoint startPoint	
	);	

	void Draw();
	void Update(float dt);

	IRect GetBitmapRect()		{ return this->bulletTex->getBitmapRect(); }
	FPoint GetCurrentPosition() { return this->currentPosition; }
	FPoint GetCenterOffset()	{ return this->centerOffset; }
	IPoint GetCellId()			{ return this->cellId; }
	bool WantsDestroy()			{ return this->bWantsDestroy; }

	void SetCellId(IPoint cellId) { this->cellId = cellId; }	

	float GetBulletDirection(bool& bSuccess);

	void Destroy();



private:

	void CalculateTrajectory(float startAngle, float startSpeed, FPoint startPoint);

	float CalculateY(float x, float startSpeed, float startAngleRad);

	void CalculateCurrentPosition();

	void RecalculateTrajectory();

	float CalculateFlyLength(float startAngle, float startSpeed);

	void ProcessTrajectoryVariables(bool & bStop, bool bContainsPoint, bool& bPointAbroad,
		const int maxAbroadPoints, int& abroadPointsCount);

	void CalculateParabolicTrajectory(float startAngle, float startSpeed, FPoint startPoint,
		 bool& bPointAbroad, const int maxAbroadPoints, int& abroadPointsCount);

	void CalculateVerticalTrajectory(float startAngle, float startSpeed, FPoint startPoint,
		bool& bPointAbroad, const int maxAbroadPoints, int& abroadPointsCount);
};