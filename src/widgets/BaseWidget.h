#pragma once

class BaseWidget : public GUI::Widget
{
public:
	BaseWidget(const std::string& name, rapidxml::xml_node<>* elem);

	void Draw() override;

protected:
	void Init();

private:

	Render::Texture* covidMonsterTex;

	Render::Texture* titleTex;	

	Render::Texture* maskTex;
	Render::Texture* virusTex;
};