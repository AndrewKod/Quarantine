#include "stdafx.h"
#include "SettingsWidget.h"
#include "QuarantineAppDelegate.h"

SettingsWidget::SettingsWidget(const std::string& name, rapidxml::xml_node<>* elem)
	: BaseWidget(name, elem)
{	
	Init();
}

void SettingsWidget::Init()
{
	BaseWidget::Init();
}

void SettingsWidget::Draw()
{
	BaseWidget::Draw();

	QuarantineAppDelegate* appDelegate = (QuarantineAppDelegate*)Core::appDelegate;

	if (appDelegate != nullptr)
	{
		if (!Render::isFontLoaded("arial_large"))
			return;

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

