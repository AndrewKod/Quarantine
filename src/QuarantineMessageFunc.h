#pragma once

class QuarantineMessageFunc : public MessageFunc {
public:
	void AcceptMessage(const Message& message) override {

		if (message.getPublisher() == "Start_Start_btn" && message.getData() == "down")
		{
			Core::mainScreen.popLayer();

			Message mess("StartLayer", "Init");

			//Push game layers
			Layer* outdoorLayer = Core::guiManager.getLayer("OutdoorLayer");
			if (outdoorLayer != nullptr)
			{				
				outdoorLayer->BroadcastMessage(mess);

				Core::mainScreen.pushLayer("OutdoorLayer");
			}

			Layer* mainLayer = Core::guiManager.getLayer("MainLayer");
			if (mainLayer != nullptr)
			{
				mainLayer->BroadcastMessage(mess);

				Core::mainScreen.pushLayer("MainLayer");
			}

			//Init EndLayer but not pushing it yet
			Layer* endLayer = Core::guiManager.getLayer("EndLayer");
			if (endLayer != nullptr)
			{
				endLayer->BroadcastMessage(mess);
			}
		}

		else if (message.getPublisher() == "Start_Settings_btn" && message.getData() == "down")
		{
			QuarantineAppDelegate* appDelegate = (QuarantineAppDelegate*)Core::appDelegate;
			if (appDelegate != nullptr)
			{
				appDelegate->SaveOldSettings();
			}

			Core::mainScreen.popLayer();
			
			//Push settings layer
			Layer* settingsLayer = Core::guiManager.getLayer("SettingsLayer");
			if (settingsLayer != nullptr)
			{
				Core::mainScreen.pushLayer("SettingsLayer");
			}
		}
		else if (message.getPublisher() == "Settings_OK_btn" && message.getData() == "down")
		{
			//Save Changes in input.txt
			QuarantineAppDelegate* appDelegate = (QuarantineAppDelegate*)Core::appDelegate;
			if (appDelegate != nullptr)
			{
				appDelegate->SaveSettingsToFile();
			}

			Core::mainScreen.popLayer();

			Layer* mainLayer = Core::guiManager.getLayer("MainLayer");
			if (mainLayer != nullptr)
			{
				Message mess("MainLayer", "UpdateSettings");
				mainLayer->BroadcastMessage(mess);
			}

			//Push start layer
			Layer* startLayer = Core::guiManager.getLayer("StartLayer");
			if (startLayer != nullptr)
			{
				Core::mainScreen.pushLayer("StartLayer");
			}
		}
		else if (message.getPublisher() == "Settings_Cancel_btn" && message.getData() == "down")
		{
			QuarantineAppDelegate* appDelegate = (QuarantineAppDelegate*)Core::appDelegate;
			if (appDelegate != nullptr)
			{
				appDelegate->ResetSettings();
			}

			Core::mainScreen.popLayer();

			//Push start layer
			Layer* startLayer = Core::guiManager.getLayer("StartLayer");
			if (startLayer != nullptr)
			{
				Core::mainScreen.pushLayer("StartLayer");
			}
		}

		else if (message.getPublisher() == "Settings_SubtractCount_btn" && message.getData() == "down")
		{
			QuarantineAppDelegate* appDelegate = (QuarantineAppDelegate*)Core::appDelegate;
			if (appDelegate != nullptr)
			{
				appDelegate->SubtractCountTarget(5);
			}			
		}
		else if (message.getPublisher() == "Settings_SubtractSpeed_btn" && message.getData() == "down")
		{
			QuarantineAppDelegate* appDelegate = (QuarantineAppDelegate*)Core::appDelegate;
			if (appDelegate != nullptr)
			{
				appDelegate->SubtractSpeed(5);
			}
		}
		else if (message.getPublisher() == "Settings_SubtractTime_btn" && message.getData() == "down")
		{
			QuarantineAppDelegate* appDelegate = (QuarantineAppDelegate*)Core::appDelegate;
			if (appDelegate != nullptr)
			{
				appDelegate->SubtractTime(5);
			}
		}
		else if (message.getPublisher() == "Settings_AddCount_btn" && message.getData() == "down")
		{
			QuarantineAppDelegate* appDelegate = (QuarantineAppDelegate*)Core::appDelegate;
			if (appDelegate != nullptr)
			{
				appDelegate->AddCountTarget(5);
			}
		}
		else if (message.getPublisher() == "Settings_AddSpeed_btn" && message.getData() == "down")
		{
			QuarantineAppDelegate* appDelegate = (QuarantineAppDelegate*)Core::appDelegate;
			if (appDelegate != nullptr)
			{
				appDelegate->AddSpeed(5);
			}
		}
		else if (message.getPublisher() == "Settings_AddTime_btn" && message.getData() == "down")
		{
			QuarantineAppDelegate* appDelegate = (QuarantineAppDelegate*)Core::appDelegate;
			if (appDelegate != nullptr)
			{
				appDelegate->AddTime(5);
			}
		}
		else if (message.getPublisher() == "End_OK_btn" && message.getData() == "down")
		{
			//pop MainLayer, OutdoorLayer, EndLayer
			Core::mainScreen.popLayer();
			Core::mainScreen.popLayer();
			Core::mainScreen.popLayer();			
			
			Message mess("EndLayer", "DeInit");				

			Layer* endLayer = Core::guiManager.getLayer("EndLayer");
			if (endLayer != nullptr)
			{
				endLayer->BroadcastMessage(mess);
			}

			Layer* outdoorLayer = Core::guiManager.getLayer("OutdoorLayer");
			if (outdoorLayer != nullptr)
			{
				outdoorLayer->BroadcastMessage(mess);
			}

			Layer* mainLayer = Core::guiManager.getLayer("MainLayer");
			if (mainLayer != nullptr)
			{
				mainLayer->BroadcastMessage(mess);
			}			

			Core::mainScreen.pushLayer("StartLayer");
		}
	}
};
