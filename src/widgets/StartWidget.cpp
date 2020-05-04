#include "stdafx.h"
#include "StartWidget.h"

StartWidget::StartWidget(const std::string& name, rapidxml::xml_node<>* elem)
	: Widget(name)	
{
	Init();
}

void StartWidget::Init()
{
	this->covidMonsterTex = Core::resourceManager.Get<Render::Texture>("covid_monster_scr");

	this->titleTex = Core::resourceManager.Get<Render::Texture>("quarantine_title");

	this->maskTex = Core::resourceManager.Get<Render::Texture>("medical_mask");
	this->virusTex = Core::resourceManager.Get<Render::Texture>("virus_red_big");	
	
}

void StartWidget::Draw()
{
	this->covidMonsterTex->Draw();

	Render::device.PushMatrix();
	Render::device.MatrixTranslate(595.f, 600.f, 0);
	this->titleTex->Draw();
	Render::device.PopMatrix();

	Render::device.PushMatrix();
	Render::device.MatrixTranslate(720.f, 350.f, 0);
	this->maskTex->Draw();
	Render::device.PopMatrix();

	Render::device.PushMatrix();
	Render::device.MatrixTranslate(715.f, 510.f, 0);
	this->virusTex->Draw();
	Render::device.PopMatrix();

	Render::device.PushMatrix();
	Render::device.MatrixTranslate(860.f, 510.f, 0);
	this->virusTex->Draw();
	Render::device.PopMatrix();
}

