#pragma once

///
/// Виджет - основной визуальный элемент на экране.
/// Он отрисовывает себя, а также может содержать другие виджеты.
///
class EndWidget : public GUI::Widget
{

public:
	EndWidget(const std::string& name, rapidxml::xml_node<>* elem);

	~EndWidget();

	void Draw() override;
	void Update(float dt) override;

	void AcceptMessage(const Message& message) override;	

	void SetVictory(bool bVictory) { this->bVictory = bVictory; }

private:
	void Init();

	void DeInit();

private:

	float timer;
	float endTime;	

	float saluteTimer;
	float saluteDelay;
	
	Render::Texture* victoryTex;
	Render::Texture* defeatTex;
	
	bool bVictory;

	EffectsContainer effCont;
	
	int sampleId;

};
