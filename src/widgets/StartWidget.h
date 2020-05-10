#pragma once
#include "BaseWidget.h"

class StartWidget : public BaseWidget
{
public:
	StartWidget(const std::string& name, rapidxml::xml_node<>* elem);

	void Draw() override;

private:
	void Init();

private:

	Render::Texture* sloganTex;	
};