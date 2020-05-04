#pragma once

#include "Target.h"

class CovidSpore :public Target
{
	FPoint	trajectoryStart; 

	//global spline angle in degrees using for spline transformations
	float	trajectoryGlobalAngle;

	//angle of direction of trajectory at spline start in degrees
	float	trajectoryStartAngle;

	std::vector<FPoint> trajectoryKeys;	

	//grid cell coords
	//updating every frame
	IPoint cellId;

	//coords of center of texture
	//updating every frame
	FPoint centerPosition;
	FPoint centerOffset;	

	//using to calc spore's collision angle
	//updating every frame
	float deltaTime;

public:

	//targetTex have to have equal width and height
	//Start point of trajectorySpline have to be (0;0) for easy spline transformations
	//Also trajectorySpline have to be periodic
	CovidSpore(Render::Texture* targetTex, Render::Texture* healthSegmentTex, Render::Texture* healthSegmentDmgTex,
		int healthSegmentSpace, int maxHealth,
		TimedSpline<FPoint>& trajectorySpline, float splineEndTime, float speedCoef,
		FPoint	trajectoryStart, float	trajectoryGlobalAngle, float trajectoryStartAngle);


	FPoint GetTrajectoryStart()			{ return this->trajectoryStart; }
	float GetTrajectoryGlobalAngle()	{ return this->trajectoryGlobalAngle; }
	float GetTrajectoryStartAngle()		{ return this->trajectoryStartAngle; }
	IPoint GetCellId()					{ return this->cellId; }
	//FPoint GetCurrentPosition()			{ return this->currentPosition; }
	FPoint GetCenterPosition()			{ return this->centerPosition; }
	FPoint GetCenterOffset()			{ return this->centerOffset; }

	void SetTrajectoryStart(FPoint trajectoryStart)				{ this->trajectoryStart = trajectoryStart; }
	void SetTrajectoryGlobalAngle(float trajectoryGlobalAngle)	{ this->trajectoryGlobalAngle = trajectoryGlobalAngle; }
	void SetCellId(IPoint cellId)								{ this->cellId = cellId; }

	float GetSporeDirection(bool& bSuccess);

	void RestartSplineAnimation();
	
	void Draw() override;

	void Update(float dt) override;
	
	//returns true if spore collides screenBounds
	bool CheckScreenCollision(FRect screenBounds);

	//returns true if spores colliding
	bool CheckSporeCollision(CovidSpore* otherSpore);

private:

	//Don't forget to change trajectoryStart and reset timer
	//before this function call
	void TransformTrajectory();

	float CalcReflectedSplineAngle(float sporeDirectionAngle, float mirrorAngle);
	
	void CalcCenterPosition();
	
	bool ValidReflectedAngle(float reflectedAngle, float mirrorAngle);

};