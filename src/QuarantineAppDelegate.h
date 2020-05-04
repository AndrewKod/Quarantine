#ifndef __QUARANTINETAPPDELEGATE_H__
#define __QUARANTINETAPPDELEGATE_H__

#pragma once

class QuarantineMessageFunc;

class QuarantineAppDelegate : public Core::EngineAppDelegate {

	//One messages processing for all layers
	QuarantineMessageFunc* messageFunc;		

	int countTarget;
	int speed;
	int time;

	int countTargetOld;
	int speedOld;
	int timeOld;

public:
	QuarantineAppDelegate();

	virtual void GameContentSize(int deviceWidth, int deviceHeight, int &width, int &height) override;
	virtual void ScreenMode(DeviceMode &mode) override;

	virtual void RegisterTypes() override;

	virtual void LoadResources() override;
	virtual void OnResourceLoaded() override;

	virtual void OnPostDraw() override;	

	int GetCountTarget()	{ return this->countTarget; }
	int GetSpeed()			{ return this->speed; }
	int GetTime()			{ return this->time; }

	void SaveSettingsToFile();
	void SaveOldSettings();
	void ResetSettings();

	void AddCountTarget(unsigned int amount);
	void AddSpeed(unsigned int amount);
	void AddTime(unsigned int amount);

	void SubtractCountTarget(unsigned int amount);
	void SubtractSpeed(unsigned int amount);
	void SubtractTime(unsigned int amount);

private:
	void AddBufferLine(std::string& buffer, const std::string& propertyName, const std::string& propertyValue);
	bool GetSettingValueFromString(const std::string& buffer, const std::string& settingName, int& outValue);
	
};

#endif // __QUARANTINETAPPDELEGATE_H__