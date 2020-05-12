#include "stdafx.h"
#include "StartWidget.h"

StartWidget::StartWidget(const std::string& name, rapidxml::xml_node<>* elem)
	: BaseWidget(name, elem)
{
	Init();
}

void StartWidget::Init()
{
	BaseWidget::Init();
	this->sloganTex = Core::resourceManager.Get<Render::Texture>("quarantine_slogan");	

	int sampleId = MM::manager.PlayTrack("quarantine_theme", true);

}

void StartWidget::Draw()
{
	BaseWidget::Draw();

	Render::device.PushMatrix();
	Render::device.MatrixTranslate(595.f, 150.f, 0);
	this->sloganTex->Draw();
	Render::device.PopMatrix();
}

