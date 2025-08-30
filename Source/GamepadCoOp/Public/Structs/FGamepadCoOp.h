// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: GamepadCoOp - Plugin Robust Gamepad Management for Seamless Local Multiplayer.
// Planned Release Year: 2025

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "FGamepadCoOp.generated.h"

USTRUCT(BlueprintType)
struct FGamepadCoOp
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category="Device")
	FInputDeviceId InputDeviceId;
	
	UPROPERTY(BlueprintReadOnly, Category="Device")
	FPlatformUserId PlatformUserId;

	UPROPERTY(BlueprintReadOnly, Category="Device")
	EInputDeviceConnectionState ConnectedState;

	FGamepadCoOp(): ConnectedState()
	{
	}
};
