#pragma once

#include "BaseWidget.h"

class SettingsWidget : public BaseWidget
{
public:
	SettingsWidget(const std::string& name, rapidxml::xml_node<>* elem);

	void Draw() override;	

private:
	void Init();

private:
	
};
