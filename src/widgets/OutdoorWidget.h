#pragma once

///
/// Виджет - основной визуальный элемент на экране.
/// Он отрисовывает себя, а также может содержать другие виджеты.
///
class OutdoorWidget : public GUI::Widget
{
public:
	OutdoorWidget(const std::string& name, rapidxml::xml_node<>* elem);	

	void Draw() override;
	void Update(float dt) override;

	void AcceptMessage(const Message& message) override;

	bool MouseDown(const IPoint& mouse_pos) override;
	void MouseMove(const IPoint& mouse_pos) override;
	void MouseUp(const IPoint& mouse_pos) override;

	void KeyPressed(int keyCode) override;
	void CharPressed(int unicodeChar) override;

private:
	void Init();

private:
	float _timer;
	bool bCanTick;

	Render::Texture* backgroundTex;

	Render::Texture* doorOpenedTex;
	Render::Texture* doorClosedTex;

	Render::Texture* currVisitorTex;
	size_t currVisitorId;

	std::vector<Render::Texture*> visitors;
	std::vector<Render::Texture*> visitorsMasked;
	std::vector<Render::Texture*> visitorsInfected;
	
	//Visitors spline animation divided on 3 parts
	//For covid monster only coming spline using
	TimedSpline<FPoint> visitorComingSpline;
	TimedSpline<FPoint> visitorAtTheDoorSpline;
	TimedSpline<FPoint> visitorLeaveSpline;

	float comingStartTime;
	float atTheDoorStartTime;
	float leaveStartTime;
	float endTime;

	bool bComingFinished;
	bool bAtTheDoorFinished;
	bool bLeaveFinished;

	bool bMonsterInvaded;
	bool bMonsterMsgSend;

	float visitDelay;
	float delayTimer;

	bool bInfected;
	bool bMasked;
};
