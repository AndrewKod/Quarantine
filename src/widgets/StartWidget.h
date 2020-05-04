#pragma once


class StartWidget : public GUI::Widget
{
public:
	StartWidget(const std::string& name, rapidxml::xml_node<>* elem);

	void Draw() override;

private:
	void Init();

private:

	Render::Texture* covidMonsterTex;

	Render::Texture* titleTex;	

	Render::Texture* maskTex;
	Render::Texture* virusTex;	
};