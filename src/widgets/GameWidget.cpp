#include "stdafx.h"
#include "Globals.h"
#include "QuarantineAppDelegate.h"
#include "GameWidget.h"
#include "Target.h"
#include "CovidMonster.h"
#include "CovidSpore.h"
#include "Bullet.h"
#include "Utilities.h"
#include "OutdoorWidget.h"


GameWidget::GameWidget(const std::string& name, rapidxml::xml_node<>* elem)
	: Widget(name)
	, _timer(0)
{
	//Init();
}

GameWidget::~GameWidget()
{
	this->DeInit();
}

void GameWidget::Init()
{
	this->wallTex = Core::resourceManager.Get<Render::Texture>("wall");
	
	this->doorOpenedTex = Core::resourceManager.Get<Render::Texture>("opened_door");
	this->doorClosedTex = Core::resourceManager.Get<Render::Texture>("closed_door");
	this->bDoorOpened = false;
	this->doorPos = FPoint(750.f, 142.f);

	this->covidMonsterTex = Core::resourceManager.Get<Render::Texture>("covid_monster");
	this->bMonsterAtTheDoor = false;
	
	this->covidMonsterSpline.addKey(0.0f, FPoint(755.0f, 150.0f));
	this->covidMonsterSpline.addKey(0.25f, FPoint(605.0f, 200.0f));
	this->covidMonsterSpline.addKey(0.5f, FPoint(455.0f, 150.0f));
	this->covidMonsterSpline.addKey(0.75f, FPoint(255.0f, 200.0f));
	this->covidMonsterSpline.addKey(1.0f, FPoint(50.0f, 150.0f));
	this->covidMonsterSpline.CalculateGradient();	

	this->covidMonsterEndTime = 1.0f;

	this->bCanWalk = false;
	
	this->bGameStarted = false;

	this->sporeSpawnStartDelay = 0.1f;
	this->sporeSpawnGameDelay = 3.f;
	this->currentSporeSpawnDelay = this->sporeSpawnStartDelay;
	this->sporeSpawnTimer = 0.f;

	//Get settings defined in input.txt
	this->gameTime = 0.f;
	this->maskedAdditionalTime = 2.f;
	this->sporeAdditionalTime = 0.5f;
	this->gameSpeed = 0;	
	this->speedCoef = 250;
	this->maxSporeCount = 0;

	this->UpdateSettings();	

	//create delegates before covidMonster
	this->destroyMonsterDelegate = Delegate<FPoint>::Create<GameWidget, &GameWidget::DestroyCovidMonster>(this);
	this->hitMonsterDelegate = Delegate<FPoint>::Create<GameWidget, &GameWidget::HitCovidMonster>(this);

	this->CreateCovidMonster();

	this->covidSpores.resize(this->covidMonster->GetMaxSporeCount(), nullptr);

	this->CreateSporeTemplates();

	this->ConstructSporeGrid();

	this->sporeScreenBounds = FRect(0, Render::device.Width(), 100, Render::device.Height());	


	////////////////////GUN////////////////////
	this->gunTex = Core::resourceManager.Get<Render::Texture>("gun");
	this->aimTex = Core::resourceManager.Get<Render::Texture>("aim");
	this->bulletTex = Core::resourceManager.Get<Render::Texture>("bullet");
	this->maskTex = Core::resourceManager.Get<Render::Texture>("medical_mask_small");

	this->currentAimPos = IPoint(0, 0);
	this->aimCenterOffset = FPoint(this->aimTex->getBitmapRect().Width() / 2, this->aimTex->getBitmapRect().Height() / 2);

	this->gunOriginPos = FPoint(Render::device.Width() - gunTex->getBitmapRect().Width(), 0.f);
	this->gunCenterOffset = FPoint(gunTex->getBitmapRect().Width() / 2, gunTex->getBitmapRect().Height() / 2);
	this->gunCenterPos = this->gunOriginPos + this->gunCenterOffset;
	this->gunAngle = 0.f;
	this->bGunShot = false;	

	this->gunBulletHeight = 40.f;
	this->gunBulletOffset = this->bulletTex->getBitmapRect().Width() / 2;
	
	///////////////////////////////////////////PARTICLES//////////////////////////////////////////	

	this->destroySporeDelegate = Delegate<FPoint>::Create<GameWidget, &GameWidget::DestroySpore>(this);
	this->hitSporeDelegate = Delegate<FPoint>::Create<GameWidget, &GameWidget::HitSpore>(this);
	

	this->bGameOver = false;
	this->bVictory = false;

	this->bMasked = false;
	this->bInfected = false;
	this->bAtacking = false;

	//Fade quarantine_sound
	MM::manager.FadeOutTrack(3.f);

	//Play quarantine_song
	int sampleId = MM::manager.PlayTrack("quarantine_song", true);	
	float trackStart = 38.f / 119.f;
	MM::manager.SetPos(sampleId, trackStart);
	
}

void GameWidget::DeInit()
{
	this->DeleteCovidMonster();
	
	for (size_t sporeId = 0; sporeId < this->covidSpores.size(); sporeId++)
	{
		if (this->covidSpores[sporeId] != nullptr)
		{
			delete this->covidSpores[sporeId];
			this->covidSpores[sporeId] = nullptr;
		}
	}
	this->covidSpores.clear();

	for (Bullet* bullet: this->bullets)
	{
		if(bullet!=nullptr)
			delete bullet;		
	}
	this->bullets.clear();

	for (Bullet* mask: this->masks)
	{
		if (mask != nullptr)
			delete mask;
	}	
	this->masks.clear();

	this->effCont.KillAllEffects();
	this->effCont.Finish();
}

void GameWidget::Draw()
{
	this->wallTex->Draw();

	this->effCont.Draw();

	//Draw opened/closed door
	Render::device.PushMatrix();
	Render::device.MatrixTranslate(this->doorPos);
	this->bDoorOpened ? this->doorOpenedTex->Draw() : this->doorClosedTex->Draw();
	Render::device.PopMatrix();	


	if (this->bGameOver)
		return;

	if (this->bCanWalk)
	{
		//the larger coef, the slower animation speed
		float speedCoef = 2.0f;

		float splineTime = this->_timer / speedCoef;

		if (splineTime >= this->covidMonsterEndTime)
		{
			splineTime = math::clamp(0.0f, this->covidMonsterEndTime, splineTime);
			this->_timer = splineTime * speedCoef;
			this->bCanWalk = false;
			this->bGameStarted = true;
		}
		FPoint currentPosition = this->covidMonsterSpline.getGlobalFrame(splineTime);

		Render::device.PushMatrix();
		Render::device.MatrixTranslate(currentPosition.x, currentPosition.y, 0);
		this->covidMonsterTex->Draw();
		Render::device.PopMatrix();
				
		//Spawning Spores
		FPoint newSporeStartPoint(currentPosition.x + math::random(0, this->covidMonsterTex->getBitmapRect().Width()),
			currentPosition.y + math::random(0, this->covidMonsterTex->getBitmapRect().Height()));

		this->TryAddRandomSpore(newSporeStartPoint);
		
		
		this->DrawSpores();
	}

	if (this->bGameStarted)
	{		
		this->covidMonster->Draw();	

		this->DrawSpores();

		this->DrawBullets();

		this->DrawMasks();
		
		this->DrawTimer();
	}

	this->DrawAim();

	this->DrawGun();	

	
}

void GameWidget::Update(float dt)
{
	this->effCont.Update(dt);

	if (this->bGameOver)
		return;

	if(this->bCanWalk)
		this->_timer += dt;
	else if (this->bGameStarted)
	{
		this->covidMonster->Update(dt);		
		this->UpdateBullets(dt);
		this->UpdateMasks(dt);
	}
	if (this->bCanWalk || this->bGameStarted)
	{
		this->sporeSpawnTimer += dt;
		this->UpdateSpores(dt);
	}

	if (this->bGameStarted)
	{
		this->gameTime -= dt;
		if (this->gameTime <= 0.f && !this->bGameOver)
		{
			this->gameTime = 0.f;
			this->bVictory = false;
			this->bGameOver = true;
			this->GameOver();
		}
	}
	
}

bool GameWidget::MouseDown(const IPoint &mouse_pos)
{
	if (this->bGameOver)
		return false;

	if (Core::mainInput.GetMouseLeftButton())
	{
		if(this->sporeScreenBounds.Contains(mouse_pos))
			ShootGunBullet();
	}
	else
	{
		if (this->sporeScreenBounds.Contains(mouse_pos))
			ShootGunMask();
	}
	return false;
}

void GameWidget::MouseMove(const IPoint &mouse_pos)
{
	
}

void GameWidget::MouseUp(const IPoint &mouse_pos)
{
	
}

void GameWidget::AcceptMessage(const Message& message)
{
	const std::string& publisher = message.getPublisher();
	const std::string& data = message.getData();

	if (publisher == "StartLayer" && data == "Init")
	{
		this->Init();
	}
	else if (publisher == "EndLayer" && data == "DeInit")
	{
		this->DeInit();
	}
	else if (publisher == "OutdoorWidget" && data=="bDoorOpened")
	{				
		this->bDoorOpened = message.getIntegerParam();

		this->GetMaskedAndInfected();

		if (this->bGameStarted && this->bDoorOpened && !this->bMasked && !this->bInfected)
		{
			this->bAtacking = true;
			this->AtackVisitor();
		}
		if ((this->bGameStarted && this->bDoorOpened && this->bMasked && !this->bInfected)
			||
			(this->bGameStarted && !this->bDoorOpened && !this->bMasked && !this->bInfected))
		{
			if (this->bAtacking)
			{
				this->bAtacking = false;
				this->StopAtack();
			}
		}
	}
	else if (publisher == "OutdoorWidget" && data == "bMonsterAtTheDoor")
	{
		this->bDoorOpened = false;
		this->_timer = 0;
		this->bCanWalk = true;
	}	
	else if (publisher == "OutdoorWidget" && data == "HealMonster")
	{
		if(this->covidMonster!=nullptr)
			this->covidMonster->Heal();			
	}
	else if (publisher == "OutdoorWidget" && data == "SetMonsterInvincible")
	{
		if (this->covidMonster != nullptr)
			this->covidMonster->SetInvincible();
	}
	else if (publisher == "MainLayer" && data == "UpdateSettings")
	{
		this->UpdateSettings();

		/*if (this->covidMonster != nullptr)
			this->covidMonster->SetMaxSporeCount(this->maxSporeCount);

		float sporeSpeedCoef = this->speedCoef / this->gameSpeed;
		for (size_t sporeId = 0; sporeId < this->covidSpores.size(); sporeId++)
		{
			if (this->covidSpores[sporeId] == nullptr)
				continue;
			this->covidSpores[sporeId]->SetSpeedCoef(sporeSpeedCoef);
		}*/
	}
}

void GameWidget::KeyPressed(int keyCode)
{
	//
	// keyCode - виртуальный код клавиши.
	// В качестве значений для проверки нужно использовать константы VK_.
	//

	if (keyCode == VK_A) {
		// Реакция на нажатие кнопки A
	}
}

void GameWidget::CharPressed(int unicodeChar)
{
	//
	// unicodeChar - Unicode код введённого символа
	//

	if (unicodeChar == L'а') {
		// Реакция на ввод символа 'а'
		TryAddCovidSpore(0);
	}
}


void GameWidget::CreateCovidMonster()
{	
	TimedSpline<FPoint> monsterTrajectory;
	monsterTrajectory.addKey(0.0f, FPoint(50.0f, 150.0f));
	monsterTrajectory.addKey(0.1f, FPoint(75.0f, 250.0f));
	monsterTrajectory.addKey(0.2f, FPoint(100.0f, 150.0f));
	monsterTrajectory.addKey(0.3f, FPoint(125.0f, 250.0f));
	monsterTrajectory.addKey(0.4f, FPoint(150.0f, 150.0f));//
	monsterTrajectory.addKey(0.5f, FPoint(125.0f, 250.0f));
	monsterTrajectory.addKey(0.6f, FPoint(100.0f, 150.0f));
	monsterTrajectory.addKey(0.7f, FPoint(75.0f, 250.0f));
	monsterTrajectory.addKey(0.8f, FPoint(50.0f, 150.0f));
	monsterTrajectory.CalculateGradient();
	float splineEndTime = 0.8f;

	this->covidMonster = new CovidMonster(this->covidMonsterTex,
		Core::resourceManager.Get<Render::Texture>("health_fragment_vertical_blue"),
		Core::resourceManager.Get<Render::Texture>("health_fragment_vertical_red"),
		2, 20, monsterTrajectory, splineEndTime, 5.f, this->maxSporeCount);

	this->covidMonster->OnHit += this->hitMonsterDelegate;
	this->covidMonster->OnDestroy += this->destroyMonsterDelegate;
}

void GameWidget::DeleteCovidMonster()
{
	if (this->covidMonster != nullptr)
	{
		delete this->covidMonster;
		this->covidMonster = nullptr;
	}
}


void GameWidget::TryAddCovidSpore(int templateId, FPoint startPoint, float startAngle)
{
	if (templateId >= 0 && templateId < this->covidSporeTemplates.size())
	{
		SporeTemplate& sporeTemplate = this->covidSporeTemplates[templateId];

		float sporeSpeedCoef = this->speedCoef / this->gameSpeed;

		for (size_t sporeId = 0; sporeId < this->covidSpores.size(); sporeId++)
		{
			if (this->covidSpores[sporeId] == nullptr)
			{
				CovidSpore* newSpore = new CovidSpore(sporeTemplate.covidSporeTexture,
					Core::resourceManager.Get<Render::Texture>("health_fragment_horizontal_blue"),
					Core::resourceManager.Get<Render::Texture>("health_fragment_horizontal_red"),
					2, sporeTemplate.maxHealth, sporeTemplate.sporeSpline, sporeTemplate.splineEndTime, sporeSpeedCoef,
					startPoint, startAngle, sporeTemplate.splineStartAngle, sporeTemplate.atackCommonTime);

				newSpore->OnHit += this->hitSporeDelegate;
				newSpore->OnDestroy += this->destroySporeDelegate;

				this->covidSpores[sporeId] = newSpore;

				this->covidMonster->AddSpores();

				break;
			}
		}
	}
}

void GameWidget::TryAddRandomSpore(FPoint spawnPoint)
{
	if (!this->bGameOver && this->sporeSpawnTimer >= this->currentSporeSpawnDelay)
	{
		if (this->covidMonster != nullptr && this->covidMonster->GetSporeCount() == 0)
			this->currentSporeSpawnDelay = this->sporeSpawnStartDelay;
		else if (this->covidMonster != nullptr && this->covidMonster->GetSporeCount() >= this->covidMonster->GetMaxSporeCount())
			this->currentSporeSpawnDelay = this->sporeSpawnGameDelay;

		this->sporeSpawnTimer = 0.f;

		FRect newSporeBounds = this->sporeScreenBounds.Inflated(-this->cellSize / 2);

		if (!newSporeBounds.Contains(spawnPoint))
		{
			if (spawnPoint.x > newSporeBounds.xEnd)
				spawnPoint.x = newSporeBounds.xEnd;
			else if (spawnPoint.x < newSporeBounds.xStart)
				spawnPoint.x = newSporeBounds.xStart;
			if (spawnPoint.y > newSporeBounds.yEnd)
				spawnPoint.y = newSporeBounds.yEnd;
			else if (spawnPoint.y < newSporeBounds.yStart)
				spawnPoint.y = newSporeBounds.yStart;
		}

		float newSporeAngle = math::random(0.f, 359.9f);

		int newSporeTemplateId = math::random(size_t(0), this->covidSporeTemplates.size() - 1);

		TryAddCovidSpore(newSporeTemplateId, spawnPoint, newSporeAngle);
	}
}

void GameWidget::ConstructSporeGrid()
{
	this->cellSize = 0;
	this->gridSize = IPoint(0, 0);
	for (size_t i = 0; i < this->covidSporeTemplates.size(); i++)
	{
		IRect rect = this->covidSporeTemplates[i].covidSporeTexture->getBitmapRect();
		int texSize = rect.width;
		if (texSize > this->cellSize)
		{
			this->cellSize = texSize;
		}
	}

	if (this->cellSize > 0)
	{
		int gridWidth = Render::device.Width() / this->cellSize;
		if (Render::device.Width() % this->cellSize > 0)
			gridWidth++;

		int gridHeight = Render::device.Height() / this->cellSize;
		if (Render::device.Height() % this->cellSize > 0)
			gridHeight++;
		
		this->gridSize.x = gridWidth;
		this->gridSize.y = gridHeight;

		std::vector<SporeGridCell> newRow; 
		newRow.resize(gridWidth, SporeGridCell());
			
		this->sporeGrid.resize(gridHeight, newRow);				
	}
}

//Checking spore collision with screen, other spores and bullets
//At first, collision tests for bullets performing 
//Then screen collision tests performing
//And after - collision with other spores
//If spore collides with something - the other collision tests will not performing at current frame
void GameWidget::CheckCollisions()
{	
	//Check bullets collision with covidMonster
	if (this->covidMonster!=nullptr && this->covidMonster->GetSporeCount() == 0 && !this->bInfected)
	{
		this->CheckMonsterAndBulletsCollision();
		return;
	}	
	
	this->GetMaskedAndInfected();	

	std::set<size_t> checkedSpores;
	//Check Bullet collision
	this->CheckSporesAndBulletsCollision(checkedSpores);

	//Check spores collision with visitor if door is opened
	if (this->bDoorOpened && !this->bMasked && !this->bInfected)
	{
		IRect doorRect = this->doorClosedTex->getBitmapRect();
		FRect collideArea(this->doorPos.x, this->doorPos.x + doorRect.Width(),
			this->doorPos.y, this->doorPos.y + doorRect.Height());
		Utilities::SqueezeRectangle(collideArea, 30.f);

		this->CheckSporesAndVisitorCollision(collideArea);

		this->CheckMasksAndVisitorCollision(collideArea);

		//skip all spore collision tests if visitor atacked
		return;
	}	

	//Check Screen collision
	this->CheckSporesAndScreenCollision(checkedSpores);

	//Check collision with other spores by searching in neighbouring grid cells
	this->CheckSporesWithSporesCollision(checkedSpores);
}

void GameWidget::CheckMonsterAndBulletsCollision()
{
	for (Bullet* bullet : this->bullets)
	{
		if (this->covidMonster != nullptr && this->covidMonster->CheckBulletCollision(bullet))
		{
			//covidMonster can get only one damage point after all spores destroyed
			this->covidMonster->SetInvincible();
			//and spawns new swarm of spores
			this->currentSporeSpawnDelay = this->sporeSpawnStartDelay;
		}
	}

	if (this->covidMonster->WantsDestroy())
	{
		this->DeleteCovidMonster();
	}
}

void GameWidget::CheckSporesAndBulletsCollision(std::set<size_t>& checkedSpores)
{
	std::set<size_t>::iterator foundCheckedId;
	//Check Bullet collision
	for (Bullet* bullet : this->bullets)
	{
		IPoint cellId(bullet->GetCurrentPosition().x / this->cellSize, bullet->GetCurrentPosition().y / this->cellSize);

		for (int k = cellId.y - 1; k <= cellId.y + 1; k++)
		{
			if (k < 0 || k >= this->gridSize.y)
				continue;
			for (int l = cellId.x - 1; l <= cellId.x + 1; l++)
			{
				if (l < 0 || l >= this->gridSize.x)
					continue;
				//check spores for collision with bullet

				std::set<size_t>& sporeIds = this->sporeGrid[k][l].sporeIds;
				for (std::set<size_t>::iterator IdsIt = sporeIds.begin(); IdsIt != sporeIds.end(); IdsIt++)
				{
					size_t sporeId = *IdsIt;
					if (this->covidSpores[sporeId] != nullptr && this->covidSpores[sporeId]->CheckBulletCollision(bullet))
					{
						checkedSpores.insert(sporeId);
					}
				}
			}
		}
	}
}

void GameWidget::CheckSporesAndVisitorCollision(FRect collideArea)
{
	bool bStopAtack = false;
	for (size_t sporeId = 0; sporeId < this->covidSpores.size(); sporeId++)
	{
		if (this->covidSpores[sporeId] != nullptr && this->covidSpores[sporeId]->CheckVisitorCollision(collideArea))
		{
			bStopAtack = true;
			this->bInfected = true;
			break;
		}
	}

	if (this->bAtacking && bStopAtack)
	{
		this->bAtacking = false;
		this->StopAtack();
	}
}

void GameWidget::CheckMasksAndVisitorCollision(FRect collideArea)
{
	bool bStopAtack = false;
	for (Bullet* mask:this->masks)
	{
		if (mask->CheckAreaCollision(collideArea))
		{
			if (!this->bMasked)
			{
				this->bMasked = true;
				//Add time if visitor get mask
				this->gameTime += this->maskedAdditionalTime;
			}
			bStopAtack = true;
			break;
		}
	}

	if (this->bAtacking && bStopAtack)
	{
		this->bAtacking = false;
		this->StopAtack();
	}
}

void GameWidget::CheckSporesAndScreenCollision(std::set<size_t>& checkedSpores)
{
	std::set<size_t>::iterator foundCheckedId;
	for (size_t sporeId = 0; sporeId < this->covidSpores.size(); sporeId++)
	{
		if (this->covidSpores[sporeId] == nullptr)
			continue;

		foundCheckedId = checkedSpores.find(sporeId);
		if (foundCheckedId == checkedSpores.end())//checkedSpores do not contains current spore
		{
			if (this->covidSpores[sporeId]->CheckScreenCollision(this->sporeScreenBounds))
			{
				checkedSpores.insert(sporeId);
			}
		}
	}
}

//Check collision with other spores by searching in neighbouring grid cells
void GameWidget::CheckSporesWithSporesCollision(std::set<size_t>& checkedSpores)
{
	std::set<size_t>::iterator foundCheckedId;
	bool bCollisionFound = false;
	//Check collision with other spores by searching in neighbouring grid cells
	for (size_t sporeId = 0; sporeId < this->covidSpores.size(); sporeId++)
	{
		if (this->covidSpores[sporeId] == nullptr)
			continue;

		foundCheckedId = checkedSpores.find(sporeId);
		if (foundCheckedId == checkedSpores.end())//checkedSpores do not contains current spore yet
		{
			IPoint cellId = this->covidSpores[sporeId]->GetCellId();
			//At first, check current grid cell for other spores and perform collision tests
			CheckCoupleSporesCollision(sporeId, cellId, checkedSpores, bCollisionFound);

			//checking other neighbouring cells
			if (!bCollisionFound)//checkedSpores do not contains current spore yet
			{
				for (int k = cellId.y - 1; k <= cellId.y + 1; k++)
				{
					if (k < 0 || k >= this->gridSize.y)
						continue;
					for (int l = cellId.x - 1; l <= cellId.x + 1; l++)
					{
						if (l < 0 || l >= this->gridSize.x || (l == cellId.x && k == cellId.y))
							continue;
						//check spores for collision with other spores
						IPoint checkingCellId(l, k);
						CheckCoupleSporesCollision(sporeId, checkingCellId, checkedSpores, bCollisionFound);

						if (bCollisionFound)
							break;
					}
					if (bCollisionFound)
						break;
				}
			}
		}
	}
}

void GameWidget::CheckCoupleSporesCollision(size_t originSporeId, IPoint checkingCellId, 
	std::set<size_t>& checkedSpores, bool & bCollisionFound)
{
	std::set<size_t>::iterator foundCheckedId;
	std::set<size_t>& sporeIds = this->sporeGrid[checkingCellId.y][checkingCellId.x].sporeIds;
	for (std::set<size_t>::iterator IdsIt = sporeIds.begin(); IdsIt != sporeIds.end(); IdsIt++)
	{
		size_t otherSporeId = *IdsIt;

		if (this->covidSpores[otherSporeId] == nullptr)
			continue;

		if (this->covidSpores[originSporeId]->GetCellId() == checkingCellId && otherSporeId == originSporeId)
			continue;

		foundCheckedId = checkedSpores.find(otherSporeId);

		if (foundCheckedId == checkedSpores.end() &&
			this->covidSpores[originSporeId]->CheckSporeCollision(this->covidSpores[otherSporeId]))
		{
			checkedSpores.insert(originSporeId);
			checkedSpores.insert(otherSporeId);
			bCollisionFound = true;
			break;
		}
	}

}

void GameWidget::CreateSporeTemplates()
{	
	TimedSpline<FPoint> sporeTrajectory0;
	sporeTrajectory0.addKey(0.0f, FPoint(0.0f, 0.0f));//
	sporeTrajectory0.addKey(0.1f, FPoint(50.0f, 50.0f));
	sporeTrajectory0.addKey(0.2f, FPoint(100.0f, 0.0f));
	sporeTrajectory0.CalculateGradient();
	float splineEndTime0 = 0.2f;

	float splineStartAngle0 = sporeTrajectory0.getGlobalFrame(0.01).GetAngle() * 180 / math::PI;

	SporeTemplate template0(Core::resourceManager.Get<Render::Texture>("virus_red_small"), 3,
		sporeTrajectory0, splineEndTime0, splineStartAngle0, 50, 1.f);
	
	

	TimedSpline<FPoint> sporeTrajectory1;
	sporeTrajectory1.addKey(0.0f, FPoint(0.0f, 0.0f));//
	sporeTrajectory1.addKey(0.1f, FPoint(50.0f, 100.0f));
	sporeTrajectory1.addKey(0.2f, FPoint(100.0f, 0.0f));
	sporeTrajectory1.addKey(0.3f, FPoint(150.0f, -100.0f));
	sporeTrajectory1.addKey(0.4f, FPoint(200.0f, 0.0f));//	
	sporeTrajectory1.CalculateGradient();
	float splineEndTime1 = 0.4f;

	float splineStartAngle1 = sporeTrajectory1.getGlobalFrame(0.01).GetAngle() * 180 / math::PI;	

	SporeTemplate template1(Core::resourceManager.Get<Render::Texture>("virus_blue_small"), 2,
		sporeTrajectory1, splineEndTime1, splineStartAngle1, 100, 0.5f);

	this->covidSporeTemplates.push_back(template0);
	this->covidSporeTemplates.push_back(template1);
}

void GameWidget::UpdateSettings()
{
	QuarantineAppDelegate* appDelegate = (QuarantineAppDelegate*)Core::appDelegate;
	if (appDelegate != nullptr)
	{
		this->gameTime = appDelegate->GetTime();
		this->gameSpeed = appDelegate->GetSpeed();
		this->maxSporeCount = appDelegate->GetCountTarget();
	}
}

void GameWidget::DrawSpores()
{
	//Globals.h
#ifdef WITH_DEBUG
	if (!Render::isFontLoaded("arial_large"))
	{
		Render::BindFont("arial_large");
	}
#endif

	for (size_t i = 0; i < this->covidSpores.size(); i++)
	{
		if (this->covidSpores[i] == nullptr)
			continue;

		if (this->covidSpores[i]->WantsDestroy())
		{
			//Adding time if spore destroyed by bullet
			if(this->covidSpores[i]->DestroyedByDamage())
				this->gameTime += this->sporeAdditionalTime;

			this->DeleteSpore(i);			
			continue;
		}

		this->covidSpores[i]->Draw();

		//stdafx.h	
#ifdef WITH_DEBUG	
		if (!Render::isFontLoaded("arial_large"))
		{
			FPoint center = this->covidSpores[i]->GetCenterPosition();
			Render::PrintString(center.x, center.y, utils::lexical_cast(i), 1.0f, CenterAlign, TopAlign);
		}
#endif

		//covidSpores and sporeGrid processing 
		IPoint oldCell = this->covidSpores[i]->GetCellId();

		FPoint centerPos = this->covidSpores[i]->GetCenterPosition();
		IPoint newCell(centerPos.x / this->cellSize, centerPos.y / this->cellSize);

		if (newCell.x >= 0 && newCell.y >= 0 && newCell.x < this->gridSize.x&&newCell.y < this->gridSize.y)
		{
			if (newCell != oldCell)
			{
				if (oldCell.x != -1)
				{
					this->sporeGrid[oldCell.y][oldCell.x].sporeIds.erase(i);
				}
				this->sporeGrid[newCell.y][newCell.x].sporeIds.insert(i);
				this->covidSpores[i]->SetCellId(newCell);
			}
		}
	}

	this->CheckCollisions();	

	if (this->covidMonster != nullptr)
	{
		FPoint newSporeStartPoint(this->covidMonster->GetCurrentPosition().x + math::random(0, this->covidMonster->GetBitmapRect().Width()),
			this->covidMonster->GetCurrentPosition().y + math::random(0, this->covidMonster->GetBitmapRect().Height()));

		this->TryAddRandomSpore(newSporeStartPoint);
	}
}

void GameWidget::UpdateSpores(float dt)
{
	for (size_t sporeId = 0; sporeId < this->covidSpores.size(); sporeId++)
	{
		if (this->covidSpores[sporeId] == nullptr)
			continue;

		this->covidSpores[sporeId]->Update(dt);
	}
}

void GameWidget::DeleteSpore(size_t sporeId)
{
	IPoint cellId = this->covidSpores[sporeId]->GetCellId();
	this->sporeGrid[cellId.y][cellId.x].sporeIds.erase(sporeId);

	delete this->covidSpores[sporeId];
	this->covidSpores[sporeId] = nullptr;

	this->covidMonster->SubtractSpores();	

	//reset sporeSpawnTimer for delay before new spore swarm spawned
	if (this->covidMonster->GetSporeCount() == 0)
		this->sporeSpawnTimer = 0.f;
}

void GameWidget::DrawAim()
{
	FPoint mouse_pos = Core::mainInput.GetMousePos();
	
	if (this->sporeScreenBounds.Contains(mouse_pos))
	{
		this->currentAimPos = mouse_pos - this->aimCenterOffset;
	}

	Render::BeginAlphaMul(0.7f);
	Render::device.PushMatrix();
	Render::device.MatrixTranslate((float)this->currentAimPos.x, (float)this->currentAimPos.y, 0);
	this->aimTex->Draw();
	Render::device.PopMatrix();
	Render::EndAlphaMul();
}

void GameWidget::DrawGun()
{
	//calc gun angle 
	FPoint translatedAimPos = this->currentAimPos - this->gunCenterPos;
	this->aimAngle = translatedAimPos.GetAngle() * 180 / math::PI;

	this->gunAngle = aimAngle - 180.f;

	FPoint translatedGunPos = this->gunOriginPos - this->gunCenterPos;
	translatedGunPos.Rotate(this->gunAngle / (180 / math::PI));
	if (this->bGunShot)
	{
		float recoilDist = 10.f;
		float translatedDist = translatedGunPos.GetDistanceToOrigin();		
		FPoint recoilOffset((recoilDist < translatedDist ? recoilDist : translatedDist * 0.2f), 0.f);
		recoilOffset.Rotate(aimAngle / (180 / math::PI));
		translatedGunPos -= recoilOffset;
		this->bGunShot = false;
	}
	
	translatedGunPos += this->gunCenterPos;

	Render::device.PushMatrix();
	Render::device.MatrixTranslate(translatedGunPos);
	Render::device.MatrixRotate(math::Vector3(0, 0, 1), this->gunAngle /*/ (180 / math::PI)*/);
	//Render::device.MatrixTranslate(this->gunCenterOffset);
	this->gunTex->Draw();
	Render::device.PopMatrix();
}

void GameWidget::DrawBullets()
{
	std::vector<Bullet*> bulletsToDestroy;
	for (Bullet* bullet: this->bullets)
	{
		bullet->Draw();
		if (bullet->WantsDestroy())
			bulletsToDestroy.push_back(bullet);
	}		

	for (size_t i = 0; i < bulletsToDestroy.size(); i++)
	{
		this->bullets.erase(bulletsToDestroy[i]);
		delete bulletsToDestroy[i];
		bulletsToDestroy[i] = nullptr;
	}
}

void GameWidget::UpdateBullets(float dt)
{
	for (Bullet* bullet : this->bullets)
	{
		bullet->Update(dt);
	}
}

void GameWidget::DrawMasks()
{
	std::vector<Bullet*> masksToDestroy;
	for (Bullet* mask : this->masks)
	{
		mask->Draw();
		if (mask->WantsDestroy())
			masksToDestroy.push_back(mask);
	}

	for (size_t i = 0; i < masksToDestroy.size(); i++)
	{
		this->masks.erase(masksToDestroy[i]);
		delete masksToDestroy[i];
		masksToDestroy[i] = nullptr;
	}
}

void GameWidget::UpdateMasks(float dt)
{
	for (Bullet* mask : this->masks)
	{
		mask->Update(dt);
	}
}

void GameWidget::DrawTimer()
{
	if (!Render::isFontLoaded("arial_giant"))
		return;

	Render::BindFont("arial_giant");

	std::string timeString = utils::lexical_cast((int)this->gameTime);

	float stringwidth = Render::getStringWidth(timeString, "arial_giant");

	int x = (Render::device.Width() - stringwidth) / 2;

	Render::PrintString(x, 0, timeString, 1.0f, LeftAlign, BottomAlign);

}

void GameWidget::ShootGunBullet()
{
	if (this->bGameStarted)
	{
		this->bGunShot = true;
		FRect bulletScreenBounds = this->sporeScreenBounds.Inflated(-this->bulletTex->getBitmapRect().Width()/2);
		//expand bulletScreenBounds for capturing points in correct coordinates
		bulletScreenBounds.xEnd += 1.f;
		bulletScreenBounds.yStart -= 1.f;		

		Bullet* bullet = new Bullet(this->bullets.size(), this->bulletTex, 135,
			bulletScreenBounds, this->aimAngle, this->CalculateBulletStartPosition(), 3);
		
		this->bullets.insert(bullet);
	}
}

void GameWidget::ShootGunMask()
{
	if (this->bGameStarted)
	{
		this->bGunShot = true;
		FRect bulletScreenBounds = this->sporeScreenBounds.Inflated(-this->maskTex->getBitmapRect().Width() / 2);
		//expand bulletScreenBounds for capturing points in correct coordinates
		bulletScreenBounds.xEnd += 1.f;
		bulletScreenBounds.yStart -= 1.f;

		Bullet* mask = new Bullet(this->bullets.size(), this->maskTex, 135,
			bulletScreenBounds, this->aimAngle, this->CalculateBulletStartPosition(), 1);

		this->masks.insert(mask);
	}
}

void GameWidget::HitSpore(const void * pSender, FPoint & position)
{
	ParticleEffectPtr eff = effCont.AddEffect("Hit");
	eff->posX = position.x;
	eff->posY = position.y;
	eff->Reset();
}

void GameWidget::DestroySpore(const void * pSender, FPoint & position)
{
	ParticleEffectPtr eff = effCont.AddEffect("Explosion");
	eff->posX = position.x;
	eff->posY = position.y;
	eff->Reset();
}

void GameWidget::HitCovidMonster(const void * pSender, FPoint & position)
{	
	ParticleEffectPtr eff = effCont.AddEffect("BigHit");
	eff->posX = position.x;
	eff->posY = position.y;
	eff->Reset();
}

void GameWidget::DestroyCovidMonster(const void * pSender, FPoint & position)
{
	ParticleEffectPtr eff = effCont.AddEffect("BigExplosion");
	eff->posX = position.x;
	eff->posY = position.y;
	eff->Reset();

	this->bGameOver = true;
	this->bVictory = true;		

	this->GameOver();	
}

void GameWidget::AtackVisitor()
{
	FPoint atackPoint(this->doorPos.x + this->doorClosedTex->getBitmapRect().Width() / 2,
					  this->doorPos.y + this->doorClosedTex->getBitmapRect().Height() / 2);

	for (size_t sporeId = 0; sporeId < this->covidSpores.size(); sporeId++)
	{
		if (this->covidSpores[sporeId] == nullptr)
			continue;

		this->covidSpores[sporeId]->AtackVisitor(atackPoint);
	}
}

void GameWidget::StopAtack()
{
	for (size_t sporeId = 0; sporeId < this->covidSpores.size(); sporeId++)
	{
		if (this->covidSpores[sporeId] == nullptr)
			continue;

		this->covidSpores[sporeId]->StopAtack();
	}
}

void GameWidget::GetMaskedAndInfected()
{
	Layer* outdoorLayer = Core::guiManager.getLayer("OutdoorLayer");
	if (outdoorLayer != nullptr)
	{
		GUI::Widget* widget = outdoorLayer->getWidget("OutdoorWidget");
		OutdoorWidget* outdoorWidget = (OutdoorWidget*)widget;
		if (outdoorWidget != nullptr)
		{
			this->bMasked = outdoorWidget->GetMasked();
			this->bInfected = outdoorWidget->GetInfected();
		}
	}
}

FPoint GameWidget::CalculateBulletStartPosition()
{
	FPoint startPoint = this->gunOriginPos;
	startPoint.y += this->gunBulletHeight;

	//translate startPoint to origin
	FPoint transStartPoint = startPoint - this->gunCenterPos;
	float transPointAngle = transStartPoint.GetAngle() * 180 / math::PI;
	float startPointDist = transStartPoint.GetDistanceToOrigin();
	startPointDist += this->gunBulletOffset;
	transStartPoint = FPoint(startPointDist, 0.f);
	transStartPoint.Rotate((this->aimAngle - (180.f - transPointAngle)) / (180 / math::PI));

	//translate start point back to it's space
	startPoint = transStartPoint + this->gunCenterPos;
	
	return startPoint;
}

void GameWidget::GameOver()
{
	Layer* endLayer = Core::guiManager.getLayer("EndLayer");
	if (endLayer != nullptr)
	{
		Message mess("GameWidget", "bVictory", this->bVictory);
		endLayer->BroadcastMessage(mess);
		Core::mainScreen.pushLayer("EndLayer");
	}
}


