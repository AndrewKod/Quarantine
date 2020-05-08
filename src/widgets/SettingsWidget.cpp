#include "stdafx.h"
#include "SettingsWidget.h"
#include "QuarantineAppDelegate.h"

SettingsWidget::SettingsWidget(const std::string& name, rapidxml::xml_node<>* elem)
	: Widget(name)
{	
	Init();
}

void SettingsWidget::Init()
{
	this->covidMonsterTex = Core::resourceManager.Get<Render::Texture>("covid_monster_scr");

	this->titleTex = Core::resourceManager.Get<Render::Texture>("quarantine_title");
	
	this->maskTex = Core::resourceManager.Get<Render::Texture>("medical_mask");
	this->virusTex = Core::resourceManager.Get<Render::Texture>("virus_red_big");

}

void SettingsWidget::Draw()
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

	QuarantineAppDelegate* appDelegate = (QuarantineAppDelegate*)Core::appDelegate;

	if (appDelegate != nullptr)
	{
		Render::BindFont("arial_large");
		float fontScale = 1.f;

		Render::PrintString(734, 335, "Count Target", fontScale, CenterAlign);
		Render::PrintString(734, 285, "Speed", fontScale, CenterAlign);
		Render::PrintString(734, 235, "Time", fontScale, CenterAlign);

		Render::PrintString(934, 335, utils::lexical_cast(appDelegate->GetCountTarget()), fontScale, CenterAlign);
		Render::PrintString(934, 285, utils::lexical_cast(appDelegate->GetSpeed()), fontScale, CenterAlign);
		Render::PrintString(934, 235, utils::lexical_cast(appDelegate->GetTime()), fontScale, CenterAlign);
	}
}

