#include "stdafx.h"
#include "Globals.h"
#include "QuarantineAppDelegate.h"
#include "StartWidget.h"
#include "SettingsWidget.h"
#include "OutdoorWidget.h"
#include "GameWidget.h"
#include "EndWidget.h"
#include "QuarantineMessageFunc.h"

#define FILE_NAME		"input.txt"
#define COUNT_TARGET	"CountTarget"
#define SPEED			"Speed"
#define TIME			"Time"

enum {
	WINDOW_WIDTH = 1024,
	WINDOW_HEIGHT = 768
};

QuarantineAppDelegate::QuarantineAppDelegate()
{
	this->messageFunc = new QuarantineMessageFunc();

	this->countTarget = 0;
	this->speed = 0;
	this->time = 0;

	this->countTargetOld = 0;
	this->speedOld = 0;
	this->timeOld = 0;


	bool bValidFile = true;	
	
	//Read input.txt or fill it by default values if file isn't created yet or invalid
	IO::StreamPtr stream = Core::fileSystem.OpenUpdate(FILE_NAME);
	IO::TextReader* reader = new IO::TextReader(stream.get());

	std::string buffer;

	buffer = reader->ReadAsciiLine();	
	bValidFile = GetSettingValueFromString(buffer, COUNT_TARGET, this->countTarget);
	if (bValidFile)
	{
		buffer = reader->ReadAsciiLine();
		bValidFile = GetSettingValueFromString(buffer, SPEED, this->speed);
	}
	if (bValidFile)
	{
		buffer = reader->ReadAsciiLine();
		bValidFile = GetSettingValueFromString(buffer, TIME, this->time);
	}

	stream.reset();
	delete reader;
	

	if (!bValidFile)
	{
		//Set default settings
		this->countTarget = 20;
		this->speed = 50;
		this->time = 50;

		SaveSettingsToFile();
	}	
}

void QuarantineAppDelegate::GameContentSize(int deviceWidth, int deviceHeight, int &width, int &height)
{
	//
	// Здесь задаётся размер игрового экрана в логических единицах.
	//
	// Это значит, что и в оконном, и в полноэкранном режиме размер экрана
	// всегда будет равен WINDOW_WIDTH x WINDOW_HEIGHT "пикселей".
	//
	width = WINDOW_WIDTH;
	height = WINDOW_HEIGHT;
}

void QuarantineAppDelegate::ScreenMode(DeviceMode &mode)
{
	mode = DeviceMode::Windowed;
}

void QuarantineAppDelegate::RegisterTypes()
{
	//
	// Чтобы можно было декларативно использовать виджет в xml по имени,
	// его необходимо зарегистрировать таким образом.
	//
	REGISTER_WIDGET_XML(StartWidget, "StartWidget");
	REGISTER_WIDGET_XML(SettingsWidget, "SettingsWidget");
	REGISTER_WIDGET_XML(OutdoorWidget, "OutdoorWidget");
	REGISTER_WIDGET_XML(GameWidget, "GameWidget");
	REGISTER_WIDGET_XML(EndWidget, "EndWidget");	
}

void QuarantineAppDelegate::LoadResources()
{
	//
	// Обычно в этом методе выполняется скрипт, в котором определяется,
	// какие ресурсы нужно загрузить и какой игровой слой положить на экран.
	//
	Core::LuaExecuteStartupScript("start.lua");


	//Button Messages Processing
	Layer* startLayer = Core::guiManager.getLayer("StartLayer");	
	if (startLayer != nullptr)
		startLayer->messageFunc = this->messageFunc;

	Layer* settingsLayer = Core::guiManager.getLayer("SettingsLayer");
	if (settingsLayer != nullptr)
		settingsLayer->messageFunc = this->messageFunc;

	//Not Necessary
	Layer* outdoorLayer = Core::guiManager.getLayer("OutdoorLayer");
	if (outdoorLayer != nullptr)
		outdoorLayer->messageFunc = this->messageFunc;

	Layer* mainLayer = Core::guiManager.getLayer("MainLayer");
	if (mainLayer != nullptr)
		mainLayer->messageFunc = this->messageFunc;

	Layer* endLayer = Core::guiManager.getLayer("EndLayer");
	if (endLayer != nullptr)
		endLayer->messageFunc = this->messageFunc;
}

void QuarantineAppDelegate::OnResourceLoaded() {
	if (Core::appInstance->AllResourcesLoaded()) {
		Core::appInstance->SetLoaded();
	}
}

void QuarantineAppDelegate::OnPostDraw() {
	//stdafx.h	
#ifdef WITH_DEBUG	
	
	if (!Render::isFontLoaded("arial"))
		return;

	//
	// Перед вызовом метода Render::PrintString() нужно привязать нужный шрифт
	//
	Render::BindFont("arial");

	int dy = Render::getFontHeight();
	int x = Render::device.Width() - 5;
	int y = Render::device.Height() - 20;

	Render::PrintString(x, y, std::string("FPS: ") + utils::lexical_cast(Core::appInstance->GetCurrentFps()), 1.0f, RightAlign, BottomAlign);
	Render::PrintString(x, y -= dy, std::string("Video: ") + utils::lexical_cast(Render::device.GetVideoMemUsage() / 1024) + std::string("K"), 1.0f, RightAlign, BottomAlign);
	Render::PrintString(x, y -= dy, std::string("Audio: ") + utils::lexical_cast(Core::resourceManager.GetMemoryInUse<MM::AudioResource>() / 1024) + std::string("K"), 1.0f, RightAlign, BottomAlign);
	Render::PrintString(x, y -= dy, std::string("Animations: ") + utils::lexical_cast((Core::resourceManager.GetMemoryInUse<Render::StreamingAnimation>() + Core::resourceManager.GetMemoryInUse<Render::Animation>()) / 1024) + std::string("K"), 1.0f, RightAlign, BottomAlign);
	Render::PrintString(x, y -= dy, std::string("Textures: ") + utils::lexical_cast(Core::resourceManager.GetMemoryInUse<Render::Texture>() / 1024) + std::string("K"), 1.0f, RightAlign, BottomAlign);
	Render::PrintString(x, y -= dy, std::string("Particles: ") + utils::lexical_cast(Core::resourceManager.GetMemoryInUse<ParticleEffect>() / 1024) + std::string("K"), 1.0f, RightAlign, BottomAlign);
	Render::PrintString(x, y -= dy, std::string("Models: ") + utils::lexical_cast(Core::resourceManager.GetMemoryInUse<Render::ModelAnimation>() / 1024) + std::string("K"), 1.0f, RightAlign, BottomAlign);

#endif
}

void QuarantineAppDelegate::SaveSettingsToFile()
{
	std::string buffer = "";

	AddBufferLine(buffer, COUNT_TARGET, std::to_string(countTarget));
	AddBufferLine(buffer, SPEED, std::to_string(speed));
	AddBufferLine(buffer, TIME, std::to_string(time));

	IO::OutputStreamPtr oStream = Core::fileSystem.OpenWrite(FILE_NAME);

	oStream->Write(buffer.data(), buffer.length());
	oStream.reset();
}

void QuarantineAppDelegate::AddBufferLine(std::string& buffer, const std::string& propertyName, const std::string& propertyValue)
{
	buffer.append(propertyName);
	buffer.append("=");
	buffer.append(propertyValue);
	buffer.append("\r\n");	
}

bool QuarantineAppDelegate::GetSettingValueFromString(const std::string& buffer, const std::string& settingName, int & outValue)
{
	int pos = buffer.find(settingName);
	if (buffer.find(settingName) != -1)
	{
		int valuePos = buffer.find("=") + 1;
		std::string valueStr = buffer.substr(valuePos);
		outValue = std::stoi(valueStr);

		if (outValue == 0)
			return false;

		return true;
	}

	return false;
}

void QuarantineAppDelegate::SaveOldSettings()
{
	this->countTargetOld = this->countTarget;
	this->speedOld = this->speed;
	this->timeOld = this->time;
}

void QuarantineAppDelegate::ResetSettings()
{
	this->countTarget = this->countTargetOld;
	this->speed = this->speedOld;
	this->time = this->timeOld;
}

void QuarantineAppDelegate::AddCountTarget(unsigned int amount)
{
	this->countTarget += amount;
	if (this->countTarget > 995)
		this->countTarget = 995;
}

void QuarantineAppDelegate::AddSpeed(unsigned int amount)
{
	this->speed += amount;
	if (this->speed > 995)
		this->speed = 995;
}

void QuarantineAppDelegate::AddTime(unsigned int amount)
{
	this->time += amount;
	if (this->time > 995)
		this->time = 995;
}

void QuarantineAppDelegate::SubtractCountTarget(unsigned int amount)
{
	this->countTarget -= amount;
	if (this->countTarget < 5)
		this->countTarget = 5;
}

void QuarantineAppDelegate::SubtractSpeed(unsigned int amount)
{
	this->speed -= amount;
	if (this->speed < 5)
		this->speed = 5;
}

void QuarantineAppDelegate::SubtractTime(unsigned int amount)
{
	this->time -= amount;
	if (this->time < 5)
		this->time = 5;
}


