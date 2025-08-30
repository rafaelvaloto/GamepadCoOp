// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: GamepadCoOp - Plugin Robust Gamepad Management for Seamless Local Multiplayer.
// Planned Release Year: 2025

#pragma once

#include "CoreMinimal.h"
#include "IInputDeviceModule.h"
#include "Modules/ModuleManager.h"

class FGamepadCoOpModule : public IInputDeviceModule
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	virtual TSharedPtr<class IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override
	{
		return nullptr;
	}
};
