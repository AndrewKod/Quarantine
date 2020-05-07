#include "stdafx.h"
#include "OutdoorWidget.h"

OutdoorWidget::OutdoorWidget(const std::string& name, rapidxml::xml_node<>* elem)
	: Widget(name)
	, _timer(0)	
{
	Init();
}

void OutdoorWidget::Init()
{	
	this->_timer = 0.0f;
	this->bCanTick = true;

	this->backgroundTex = Core::resourceManager.Get<Render::Texture>("landscape");	

	this->currVisitorTex = Core::resourceManager.Get<Render::Texture>("covid_monster");
	this->bMonsterInvaded = false;
	this->bMonsterMsgSend = false;
	
	this->visitors.push_back(Core::resourceManager.Get<Render::Texture>("man"));
	this->visitors.push_back(Core::resourceManager.Get<Render::Texture>("woman"));
	this->visitors.push_back(Core::resourceManager.Get<Render::Texture>("doctor"));
	this->visitors.push_back(Core::resourceManager.Get<Render::Texture>("young_man"));
	this->visitors.push_back(Core::resourceManager.Get<Render::Texture>("young_woman"));

	this->visitorsMasked.push_back(Core::resourceManager.Get<Render::Texture>("man_masked"));
	this->visitorsMasked.push_back(Core::resourceManager.Get<Render::Texture>("woman_masked"));
	this->visitorsMasked.push_back(Core::resourceManager.Get<Render::Texture>("doctor_masked"));
	this->visitorsMasked.push_back(Core::resourceManager.Get<Render::Texture>("young_man_masked"));
	this->visitorsMasked.push_back(Core::resourceManager.Get<Render::Texture>("young_woman_masked"));

	this->visitorsInfected.push_back(Core::resourceManager.Get<Render::Texture>("man_infected"));
	this->visitorsInfected.push_back(Core::resourceManager.Get<Render::Texture>("woman_infected"));
	this->visitorsInfected.push_back(Core::resourceManager.Get<Render::Texture>("doctor_infected"));
	this->visitorsInfected.push_back(Core::resourceManager.Get<Render::Texture>("young_man_infected"));
	this->visitorsInfected.push_back(Core::resourceManager.Get<Render::Texture>("young_woman_infected"));
	
	int i = 0;
	float time = 0.0f;
	float x = -145.0f;
	float y = 150.0f;


	//Creating splines
	int segmentsCount = 24;
	int atTheDoorStart = 10;
	int leaveStart = 20;
	while (i < segmentsCount)
	{
		if (i <= atTheDoorStart)
		{
			if (this->visitorComingSpline.GetKeysCount() == 0)
				this->comingStartTime = time;
			this->visitorComingSpline.addKey(time, FPoint(x, y));
		}
		if (i >= leaveStart)
		{
			if (this->visitorLeaveSpline.GetKeysCount() == 0)
				this->leaveStartTime = time;
			this->visitorLeaveSpline.addKey(time, FPoint(x, y));
		}
		if (i >= atTheDoorStart && i <= leaveStart)
		{
			if (this->visitorAtTheDoorSpline.GetKeysCount() == 0)
				this->atTheDoorStartTime = time;
			this->visitorAtTheDoorSpline.addKey(time, FPoint(x, y));
		}
		
		i++;
		time += 0.05f;
		if (i < atTheDoorStart || i > leaveStart)
			x += 100.0f;
		if (i % 2 == 0)
		{
			y = 150;
		}
		else
			y = 200;		
	}
	this->visitorComingSpline.CalculateGradient();
	this->visitorAtTheDoorSpline.CalculateGradient();
	this->visitorLeaveSpline.CalculateGradient();

	time -= 0.05f;
	this->endTime = time;

	this->bComingFinished = false;
	this->bAtTheDoorFinished = false;
	this->bLeaveFinished = false;

	this->bInfected = false;
	this->bMasked = false;

	this->bVisitorExploded = false;
}

void OutdoorWidget::HealMonster()
{
	Layer* mainLayer = Core::guiManager.getLayer("MainLayer");
	if (mainLayer != nullptr)
	{		
		Message mess("OutdoorWidget", "HealMonster");
		mainLayer->BroadcastMessage(mess);
	}
}

void OutdoorWidget::SetMonsterInvincible()
{
	Layer* mainLayer = Core::guiManager.getLayer("MainLayer");
	if (mainLayer != nullptr)
	{		
		Message mess("OutdoorWidget", "SetMonsterInvincible");
		mainLayer->BroadcastMessage(mess);
	}
}

void OutdoorWidget::Draw()
{
	backgroundTex->Draw();

	FPoint currentPosition;

	if (this->bCanTick)
	{
		bool bWantsExplodeVisitor = false;

		//the larger coef, the slower animation speed
		float speedCoef = 10.0f;

		float splineTime = this->_timer / speedCoef;		
		
		//comingSpline using
		if (!this->bComingFinished)
		{
			if (splineTime >= this->atTheDoorStartTime)
			{				
				this->bComingFinished = true;

				//stop spline animation for covid monster
				if (!this->bMonsterInvaded)
				{
					this->bMonsterInvaded = true;
					this->bCanTick = false;
				}
			}
			currentPosition = this->visitorComingSpline.getGlobalFrame(splineTime);
		}
		//atTheDoorSpline using after comingSpline passed
		else if (!this->bAtTheDoorFinished)
		{
			if (splineTime >= this->leaveStartTime)
			{				
				this->bAtTheDoorFinished = true;
			}
			currentPosition = this->visitorAtTheDoorSpline.getGlobalFrame(splineTime);
		}
		//leaveSpline using after atTheDoorSpline passed
		else if (!this->bLeaveFinished)
		{
			if (this->bInfected)
				bWantsExplodeVisitor = true;

			if (splineTime >= this->endTime)
			{				
				this->bLeaveFinished = true;
				this->bCanTick = false;
				this->bInfected = false;
				this->bMasked = false;
			}
			currentPosition = this->visitorLeaveSpline.getGlobalFrame(splineTime);
		}

		

		Layer* mainLayer = Core::guiManager.getLayer("MainLayer");
		if (mainLayer != nullptr)
		{
			if (this->bMonsterInvaded && !this->bMonsterMsgSend)
			{
				this->bMonsterMsgSend = true;
				Message mess("OutdoorWidget", "bMonsterAtTheDoor");
				mainLayer->BroadcastMessage(mess);
			}
			else
			{
				FRect doorRect(740.f, 850.f, 130.f, 445.f);// door rectangle

				//open/close door on MainLayer if visitor at the door
				Message mess("OutdoorWidget", "bDoorOpened",
					doorRect.Contains(currentPosition) ? 1 : 0);
				mainLayer->BroadcastMessage(mess);
			}
		}


		if (!bWantsExplodeVisitor && !this->bVisitorExploded)
		{
			Render::device.PushMatrix();
			Render::device.MatrixTranslate(currentPosition.x, currentPosition.y, 0);
			this->currVisitorTex->Draw();
			Render::device.PopMatrix();
		}
		else
		{
			if (!this->bVisitorExploded)
			{
				this->bVisitorExploded = true;

				this->HealMonster();

				ParticleEffectPtr eff = effCont.AddEffect("BigHit");
				eff->posX = currentPosition.x + this->currVisitorTex->getBitmapRect().Width() / 2;
				eff->posY = currentPosition.y + this->currVisitorTex->getBitmapRect().Height() * 0.75f;
				eff->Reset();

				//this->bInfected = false;
			}
		}
	}

	effCont.Draw();
}

void OutdoorWidget::Update(float dt)
{
	if(this->bCanTick)
		this->_timer += dt;

	effCont.Update(dt);
}

bool OutdoorWidget::MouseDown(const IPoint &mouse_pos)
{
	if (Core::mainInput.GetMouseRightButton())
	{
		
	}
	else
	{
		
	}
	return false;
}

void OutdoorWidget::MouseMove(const IPoint &mouse_pos)
{
	
}

void OutdoorWidget::MouseUp(const IPoint &mouse_pos)
{
	
}

void OutdoorWidget::AcceptMessage(const Message& message)
{
	//
	// Виджету могут посылаться сообщения с параметрами.
	//

	const std::string& publisher = message.getPublisher();
	const std::string& data = message.getData();

	if (publisher == "GameWidget" && data == "invite_visitor")
	{
		this->_timer = 0.f;
		this->bCanTick = true;
		this->bComingFinished = false;
		this->bAtTheDoorFinished = false;
		this->bLeaveFinished = false;
		this->bVisitorExploded = false;

		this->currVisitorId = math::random(this->visitors.size() - 1);
		this->currVisitorTex = this->visitors[this->currVisitorId];
	}
	else if (publisher == "GameWidget" && data == "infect_visitor")
	{
		if (!this->bMasked)
		{
			//set bInfected and infected texture once
			if (!this->bInfected)
			{
				this->bInfected = true;
				this->currVisitorTex = this->visitorsInfected[this->currVisitorId];
				//prevent damaging monster while visitor alive
				this->SetMonsterInvincible();
			}
		}
	}
	else if (publisher == "GameWidget" && data == "mask_visitor")
	{
		if (!this->bInfected)
		{
			//set bMasked and masked texture once
			if (!this->bMasked)
			{
				this->bMasked = true;
				this->currVisitorTex = this->visitorsMasked[this->currVisitorId];
			}
		}
	}
}

void OutdoorWidget::KeyPressed(int keyCode)
{
	//
	// keyCode - виртуальный код клавиши.
	// В качестве значений для проверки нужно использовать константы VK_.
	//

	if (keyCode == VK_A) {
		// Реакция на нажатие кнопки A
	}
}

void OutdoorWidget::CharPressed(int unicodeChar)
{
	//
	// unicodeChar - Unicode код введённого символа
	//

	if (unicodeChar == L'а') {
		// Реакция на ввод символа 'а'
	}
}
