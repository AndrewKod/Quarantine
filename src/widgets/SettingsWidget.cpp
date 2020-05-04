#include "stdafx.h"
#include "SettingsWidget.h"
#include "QuarantineAppDelegate.h"

SettingsWidget::SettingsWidget(const std::string& name, rapidxml::xml_node<>* elem)
	: Widget(name)
{	
}

void SettingsWidget::Draw()
{
	QuarantineAppDelegate* appDelegate = (QuarantineAppDelegate*)Core::appDelegate;

	if (appDelegate != nullptr)
	{
		Render::BindFont("arial_large");
		float fontScale = 1.f;

		Render::PrintString(120, 335, "Count Target", fontScale, CenterAlign);
		Render::PrintString(120, 285, "Speed", fontScale, CenterAlign);
		Render::PrintString(120, 235, "Time", fontScale, CenterAlign);

		Render::PrintString(320, 335, utils::lexical_cast(appDelegate->GetCountTarget()), fontScale, CenterAlign);
		Render::PrintString(320, 285, utils::lexical_cast(appDelegate->GetSpeed()), fontScale, CenterAlign);
		Render::PrintString(320, 235, utils::lexical_cast(appDelegate->GetTime()), fontScale, CenterAlign);
	}

}

