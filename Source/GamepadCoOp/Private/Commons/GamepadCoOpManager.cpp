// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: GamepadCoOp - Plugin Robust Gamepad Management for Seamless Local Multiplayer.
// Planned Release Year: 2025

#include "Commons/GamepadCoOpManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h" 
#include "Engine/GameInstance.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"
#include "Modules/ModuleManager.h"

void UGamepadCoOpManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	IPlatformInputDeviceMapper& Mapper = IPlatformInputDeviceMapper::Get();
	TArray<FInputDeviceId> AllDeviceIds;
	AllDeviceIds.Reset();

	Mapper.GetAllInputDevices(AllDeviceIds);
	for (const FInputDeviceId& DeviceId : AllDeviceIds)
	{
		if (FPlatformUserId UserIsPaired = Mapper.GetUserForInputDevice(DeviceId); UserIsPaired.IsValid())
		{
			Mapper.Internal_MapInputDeviceToUser(DeviceId, UserIsPaired, EInputDeviceConnectionState::Connected);
		}
	}
	
	Mapper.GetOnInputDeviceConnectionChange().AddUObject(this, &UGamepadCoOpManager::HandleDeviceConnected);
}

void UGamepadCoOpManager::Deinitialize()
{
	const IPlatformInputDeviceMapper& Mapper = IPlatformInputDeviceMapper::Get();
	Mapper.GetOnInputDeviceConnectionChange().RemoveAll(this);

	Super::Deinitialize();
}

UGamepadCoOpManager* UGamepadCoOpManager::Get(const UObject* WorldContextObject)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,
	                                                             EGetWorldErrorMode::LogAndReturnNull))
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UGamepadCoOpManager>();
		}
	}
	return nullptr;
}

void UGamepadCoOpManager::HandleDeviceConnected(const EInputDeviceConnectionState ConnectionState, const FPlatformUserId UserId,
                                                const FInputDeviceId DeviceId)
{
	RegisterGamepad(ConnectionState, UserId, DeviceId);
}

FPlatformUserId UGamepadCoOpManager::CoOpPlatformUserId(int32 LocalPlayerControllerId) const
{
	return FPlatformUserId::CreateFromInternalId(LocalPlayerControllerId);
}

void UGamepadCoOpManager::RegisterGamepad(const EInputDeviceConnectionState ConnectionState, const FPlatformUserId UserId,
                                          const FInputDeviceId DeviceId)
{
	FGamepadCoOp NewGamepad;
	NewGamepad.InputDeviceId = DeviceId;
	NewGamepad.PlatformUserId = UserId;
	NewGamepad.ConnectedState = ConnectionState;

	if (ConnectionState == EInputDeviceConnectionState::Connected)
	{
		Gamepads.Add(DeviceId.GetId(), NewGamepad);
		OnGamepadConnected.Broadcast(NewGamepad);
	}

	if (ConnectionState == EInputDeviceConnectionState::Disconnected)
	{
		UnregisterGamepad(DeviceId);
		OnGamepadDisconnected.Broadcast(NewGamepad);
	}
}

void UGamepadCoOpManager::UnregisterGamepad(const FInputDeviceId DeviceId)
{
	OnGamepadDisconnected.Broadcast(Gamepads.FindRef(DeviceId.GetId()));
}

bool UGamepadCoOpManager::GetGamepad(const FInputDeviceId& DeviceId, FGamepadCoOp& OutGamepad) const
{
	if (const FGamepadCoOp* FoundGamepad = Gamepads.Find(DeviceId.GetId()))
	{
		OutGamepad = *FoundGamepad;
		return true;
	}
	return false;
}

void UGamepadCoOpManager::GetAllGamepadsForUser(const FPlatformUserId& UserId, TArray<FGamepadCoOp>& OutGamepads) const
{
	OutGamepads.Empty();
	for (const auto& Pair : Gamepads)
	{
		if (Pair.Value.PlatformUserId == UserId)
		{
			OutGamepads.Add(Pair.Value);
		}
	}
}

void UGamepadCoOpManager::GetAllGamepads(TArray<FGamepadCoOp>& OutGamepads) const
{
	Gamepads.GenerateValueArray(OutGamepads);
}

FInputDeviceId UGamepadCoOpManager::GetPrimaryGamepadForUser(const FPlatformUserId NewUser) const
{
	for (const auto& Pair : Gamepads)
	{
		if (Pair.Value.PlatformUserId == NewUser)
		{
			return FInputDeviceId::CreateFromInternalId(Pair.Key);
		}
	}
	return FInputDeviceId::CreateFromInternalId(INDEX_NONE);
}

FHardwareDeviceIdentifier UGamepadCoOpManager::GetHardwareDeviceIdentifier(const FInputDeviceId Device) const
{
	return UInputDeviceSubsystem::Get()->GetInputDeviceHardwareIdentifier(Device);
}

bool UGamepadCoOpManager::RemapGamepadToUser(const FInputDeviceId& DeviceId, const FPlatformUserId& NewUserId)
{
	const FGamepadCoOp* Gamepad = Gamepads.Find(DeviceId.GetId());
	if (!Gamepad)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemapGamepadToUser: Gamepad not found: %d"), DeviceId.GetId());
		return false;
	}


	const FPlatformUserId OldUserId = Gamepad->PlatformUserId;
	if (OldUserId == NewUserId)
	{
		return true;
	}

	Gamepads[DeviceId.GetId()].PlatformUserId = NewUserId;
	IPlatformInputDeviceMapper::Get().Internal_ChangeInputDeviceUserMapping(DeviceId, NewUserId, OldUserId);
	OnGamepadUserChanged.Broadcast(*Gamepad, NewUserId, OldUserId);
	return true;
}
