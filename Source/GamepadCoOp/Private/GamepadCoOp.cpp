// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: GamepadCoOp - Plugin Robust Gamepad Management for Seamless Local Multiplayer.
// Planned Release Year: 2025

#include "GamepadCoOp.h"

#include "Commons/GamepadCoOpManager.h"

#define LOCTEXT_NAMESPACE "FGamepadCoOpModule"

void FGamepadCoOpModule::StartupModule()
{
	IModularFeatures::Get().RegisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);
}

void FGamepadCoOpModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGamepadCoOpModule, GamepadCoOp)