#include "stdafx.h"
#include "EndWidget.h"

EndWidget::EndWidget(const std::string& name, rapidxml::xml_node<>* elem)
	: Widget(name)
	
{
	Init();
}

void EndWidget::Init()
{	
	victoryTex = Core::resourceManager.Get<Render::Texture>("victory_text");	
	defeatTex = Core::resourceManager.Get<Render::Texture>("defeat_text");
	
	timer = 0.f;
	endTime = 10.f;

	saluteTimer = 0.f;
	saluteDelay = 1.f;

	bVictory = false;
}

void EndWidget::Draw()
{		
	Render::device.SetTexturing(false);
	
	Render::BeginColor(Color(255, 255, 255, 255 * (this->timer / this->endTime)));
	
	Render::DrawRect(0, 0, Render::device.Width(), Render::device.Height());
	
	Render::EndColor();
	
	Render::device.SetTexturing(true);

	Render::device.PushMatrix();
	Render::device.MatrixTranslate(300, 400, 0);

	if (this->bVictory)
		this->victoryTex->Draw();
	else
		this->defeatTex->Draw();

	Render::device.PopMatrix();	

	if(this->bVictory)
		this->effCont.Draw();
}

void EndWidget::Update(float dt)
{	
	if (this->timer < this->endTime)
	{
		this->timer += dt;
		if (this->timer >= this->endTime)
			this->timer = this->endTime;
	}

	if (this->bVictory)
	{
		effCont.Update(dt);
		this->saluteTimer += dt;
		if (this->saluteTimer >= this->saluteDelay)
		{
			this->saluteTimer = 0.f;

			ParticleEffectPtr eff = effCont.AddEffect("BigHit");
			eff->posX = math::random(1, Render::device.Width() - 1);
			eff->posY = math::random(1, Render::device.Height() - 1);;
			eff->Reset();
		}
	}
}



void EndWidget::AcceptMessage(const Message& message)
{
	

	const std::string& publisher = message.getPublisher();
	const std::string& data = message.getData();

	if (publisher == "GameWidget" && data == "bVictory")
	{
		this->bVictory = message.getIntegerParam();
	}
}

