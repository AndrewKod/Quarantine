#pragma once

class Target;
class CovidMonster;
class CovidSpore;
class Bullet;

struct SporeGridCell
{
	//spores located inside grid cell bounds
	//updating every frame
	std::set<size_t> sporeIds;
};

struct SporeTemplate
{
	Render::Texture* covidSporeTexture;

	int maxHealth;

	TimedSpline<FPoint> sporeSpline;

	float splineEndTime;

	float splineStartAngle;

	float speed;

	float atackCommonTime;

	SporeTemplate(Render::Texture* covidSporeTexture, int maxHealth, TimedSpline<FPoint> sporeSpline,
	float splineEndTime, float splineStartAngle, float speed, float atackCommonTime)
		: covidSporeTexture(covidSporeTexture)
		, maxHealth(maxHealth)
		, sporeSpline(sporeSpline)
		, splineEndTime(splineEndTime)
		, splineStartAngle(splineStartAngle)
		, speed(speed)
		, atackCommonTime(atackCommonTime)
	{}	
};

class GameWidget : public GUI::Widget
{
	float gameTime;
	int gameSpeed;
	int speedCoef;
	int maxSporeCount;

	float _timer;
	float gameTimer;

	Render::Texture* wallTex;

	Render::Texture* doorOpenedTex;
	Render::Texture* doorClosedTex;
	bool bDoorOpened;
	FPoint doorPos;

	//texture and spline for covidMonster animation before game start
	Render::Texture* covidMonsterTex;
	TimedSpline<FPoint> covidMonsterSpline;
	bool bMonsterAtTheDoor;
	float covidMonsterEndTime;
	bool bCanWalk;
	
	bool bGameStarted;

	CovidMonster* covidMonster;	

	std::vector<CovidSpore*> covidSpores;
	std::vector<SporeTemplate> covidSporeTemplates;
	float sporeSpawnStartDelay;
	float sporeSpawnGameDelay;
	float currentSporeSpawnDelay;
	float sporeSpawnTimer;


	std::vector<std::vector<SporeGridCell>> sporeGrid;
	int cellSize;
	IPoint gridSize;

	//bounds for spores movement
	FRect sporeScreenBounds;

	////////////////GUN////////////////
	Render::Texture* gunTex;
	Render::Texture* aimTex;
	Render::Texture* bulletTex;	
	Render::Texture* maskTex;

	std::set<Bullet*> bullets;
	std::set<Bullet*> masks;
	
	//updating every frame
	FPoint currentAimPos;
	FPoint aimCenterOffset;
	float aimAngle;

	FPoint gunOriginPos;
	FPoint gunCenterOffset;
	FPoint gunCenterPos;
	float gunAngle;
	//height at left edge of gunTex where bullet spawning
	float gunBulletHeight;
	//distance from gunTex where bullet spawning
	float gunBulletOffset;	

	bool bGunShot;	
	
	///////////////////////////////////////////PARTICLES//////////////////////////////////////////

	EffectsContainer effCont;
	ParticleEffectPtr trailEff;	
	
	Delegate<FPoint> destroySporeDelegate;
	Delegate<FPoint> hitSporeDelegate;

	Delegate<FPoint> destroyMonsterDelegate;
	Delegate<FPoint> hitMonsterDelegate;

	bool bGameOver;
	bool bVictory;

	bool bMasked;
	bool bInfected;
	bool bAtacking;

public:
	GameWidget(const std::string& name, rapidxml::xml_node<>* elem);

	void Draw() override;
	void Update(float dt) override;

	void AcceptMessage(const Message& message) override;

	bool MouseDown(const IPoint& mouse_pos) override;
	void MouseMove(const IPoint& mouse_pos) override;
	void MouseUp(const IPoint& mouse_pos) override;

	void KeyPressed(int keyCode) override;
	void CharPressed(int unicodeChar) override;
	
private:
	void Init();

	void DeInit();

	void CreateCovidMonster();
	void DeleteCovidMonster();


	void TryAddCovidSpore(int templateId, FPoint startPoint = FPoint(0.f,0.f), float startAngle = 0.f);

	void TryAddRandomSpore(FPoint spawnPoint);

	void ConstructSporeGrid();

	///////////////////////CHECK COLLISIONS////////////////////////
	void CheckCollisions();

	void CheckMonsterAndBulletsCollision();

	void CheckSporesAndBulletsCollision(std::set<size_t>& checkedSpores);

	void CheckSporesAndVisitorCollision(FRect collideArea);

	void CheckMasksAndVisitorCollision(FRect collideArea);

	void CheckSporesAndScreenCollision(std::set<size_t>& checkedSpores);

	void CheckSporesWithSporesCollision(std::set<size_t>& checkedSpores);

	void CheckCoupleSporesCollision(size_t originId, IPoint checkingCellId, std::set<size_t>& checkedSpores, bool& bCollisionFound);
	////////////////////////////////////////////////////////////////

	void CreateSporeTemplates();	

	void UpdateSettings();
	
	void DrawSpores();
	void UpdateSpores(float dt);
	void DeleteSpore(size_t sporeId);

	void DrawAim();
	
	void DrawGun();
	
	void DrawBullets();
	void UpdateBullets(float dt);

	void DrawMasks();
	void UpdateMasks(float dt);

	void DrawTimer();

	void ShootGunBullet();

	void ShootGunMask();

	void HitSpore(const void* pSender, FPoint& position);

	void DestroySpore(const void* pSender, FPoint& position);

	void HitCovidMonster(const void * pSender, FPoint& position);

	void DestroyCovidMonster(const void * pSender, FPoint& position);

	void AtackVisitor();

	void StopAtack();

	void GetMaskedAndInfected();

	FPoint CalculateBulletStartPosition();

	void GameOver();
};
