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

	SporeTemplate(Render::Texture* covidSporeTexture, int maxHealth, TimedSpline<FPoint> sporeSpline,
	float splineEndTime, float splineStartAngle, float speed)
		: covidSporeTexture(covidSporeTexture)
		, maxHealth(maxHealth)
		, sporeSpline(sporeSpline)
		, splineEndTime(splineEndTime)
		, splineStartAngle(splineStartAngle)
		, speed(speed)
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

	//texture and spline for covidMonster animation before game start
	Render::Texture* covidMonsterTex;
	TimedSpline<FPoint> covidMonsterSpline;
	bool bMonsterAtTheDoor;
	float covidMonsterEndTime;
	bool bCanTick;
	bool bCanGetDmg;

	bool bGameStarted;

	CovidMonster* covidMonster;
	

	std::vector<CovidSpore*> covidSpores;
	std::vector<SporeTemplate> covidSporeTemplates;
	float sporeSpawnStartDelay;
	float sporeSpawnGameDelay;
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

	std::set<Bullet*> bullets;
	
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
	float gunTimer;

	bool bGunShot;
	float bulletStartSpeed;
	float bulletCurrentSpeed;

	
	
	///////////////////////////////////////////PARTICLES//////////////////////////////////////////

	EffectsContainer effCont;
	ParticleEffectPtr trailEff;
	//ParticleEffectPtr hitEff;
	//ParticleEffectPtr destroyEff;
	
	Delegate<FPoint> destroySporeDelegate;
	Delegate<FPoint> hitSporeDelegate;

	Delegate<FPoint> destroyMonsterDelegate;
	Delegate<FPoint> hitMonsterDelegate;

	bool bGameOver;
	bool bVictory;

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

	void CreateCovidMonster();

	void TryAddCovidSpore(int templateId, FPoint startPoint = FPoint(0.f,0.f), float startAngle = 0.f);

	void TryAddRandomSpore(FPoint spawnPoint);

	void ConstructSporeGrid();

	void CheckCollisions();

	void CheckCoupleSporesCollision(size_t originId, IPoint checkingCellId, std::set<size_t>& checkedSpores, bool& bCollisionFound);

	void CreateSporeTemplates();	

	void UpdateSettings();
	
	void DrawSpores();
	void UpdateSpores(float dt);
	void DeleteSpore(size_t sporeId);

	void DrawAim();
	
	void DrawGun();
	
	void DrawBullets();
	void UpdateBullets(float dt);

	void ShootGunBullet();

	void ShootGunMask();

	void HitSpore(const void* pSender, FPoint& position);

	void DestroySpore(const void* pSender, FPoint& position);

	void HitCovidMonster(const void * pSender, FPoint& position);

	void DestroyCovidMonster(const void * pSender, FPoint& position);
};
