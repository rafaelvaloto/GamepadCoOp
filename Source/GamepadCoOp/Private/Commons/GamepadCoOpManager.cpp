// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: GamepadCoOp - Plugin Robust Gamepad Management for Seamless Local Multiplayer.
// Planned Release Year: 2025

#include "Commons/GamepadCoOpManager.h"
#include "Engine/GameInstance.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"
#include "Modules/ModuleManager.h"

void UGamepadCoOpManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IPlatformInputDeviceMapper& Mapper = IPlatformInputDeviceMapper::Get();
	Mapper.GetOnInputDeviceConnectionChange().AddUObject(this, &UGamepadCoOpManager::HandleDeviceConnected);

	TArray<FInputDeviceId> AllDeviceIds;
	Mapper.GetAllInputDevices(AllDeviceIds);
	for (const FInputDeviceId& DeviceId : AllDeviceIds)
	{
		const FPlatformUserId UserId = Mapper.GetUserForInputDevice(DeviceId);
		HandleDeviceConnected(EInputDeviceConnectionState::Connected, UserId, DeviceId);
	}
}

void UGamepadCoOpManager::Deinitialize()
{
	IPlatformInputDeviceMapper& Mapper = IPlatformInputDeviceMapper::Get();
	Mapper.GetOnInputDeviceConnectionChange().RemoveAll(this);
	
	Super::Deinitialize();
}

UGamepadCoOpManager* UGamepadCoOpManager::Get(const UObject* WorldContextObject)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UGamepadCoOpManager>();
		}
	}
	return nullptr;
}

FPlatformUserId UGamepadCoOpManager::CoOpPlatformUserId(int32 LocalPlayerControllerId) const
{
	return FPlatformUserId::CreateFromInternalId(LocalPlayerControllerId);
}

void UGamepadCoOpManager::HandleDeviceConnected(EInputDeviceConnectionState ConnectionState, FPlatformUserId UserId, FInputDeviceId DeviceId)
{
	RegisterGamepad(ConnectionState, UserId, DeviceId);
}

void UGamepadCoOpManager::HandleDeviceDisconnected(const FInputDeviceId DeviceId)
{
	UnregisterGamepad(DeviceId);
}

void UGamepadCoOpManager::HandleDeviceUserChanged(const FInputDeviceId DeviceId, const FPlatformUserId NewUser, const FPlatformUserId OldUser)
{
    if (FGamepadCoOp* Gamepad = Gamepads.Find(DeviceId))
    {
        Gamepad->PlatformUserId = NewUser;
        OnGamepadUserChanged.Broadcast(*Gamepad, NewUser, OldUser);
    }
}


void UGamepadCoOpManager::RegisterGamepad(EInputDeviceConnectionState ConnectionState, FPlatformUserId UserId, FInputDeviceId DeviceId)
{
	if (Gamepads.Contains(DeviceId))
	{
		return;
	}
	
	IPlatformInputDeviceMapper& Mapper = IPlatformInputDeviceMapper::Get();
	if (!UserId.IsValid())
	{
		return;
	}
	
	FGamepadCoOp NewGamepad;
	NewGamepad.InputDeviceId = DeviceId;
	NewGamepad.PlatformUserId = UserId;
	NewGamepad.ConnectedState = EInputDeviceConnectionState::Connected;

	Gamepads.Add(DeviceId, NewGamepad);
	OnGamepadConnected.Broadcast(NewGamepad);
}

void UGamepadCoOpManager::UnregisterGamepad(const FInputDeviceId DeviceId)
{
	if (FGamepadCoOp RemovedGamepad; Gamepads.RemoveAndCopyValue(DeviceId, RemovedGamepad))
	{
		OnGamepadDisconnected.Broadcast(RemovedGamepad);
	}
}

bool UGamepadCoOpManager::GetGamepad(const FInputDeviceId& DeviceId, FGamepadCoOp& OutGamepad) const
{
	if (const FGamepadCoOp* FoundGamepad = Gamepads.Find(DeviceId))
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
			return Pair.Key;
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
	const FGamepadCoOp* Gamepad = Gamepads.Find(DeviceId);
	if (!Gamepad)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemapGamepadToUser: Tentativa de remapear um gamepad desconhecido com DeviceId: %d"), DeviceId.GetId());
		return false;
	}


	const FPlatformUserId OldUserId = Gamepad->PlatformUserId;
	
	if (OldUserId == NewUserId)
	{
		return true;
	}

	IPlatformInputDeviceMapper::Get().Internal_ChangeInputDeviceUserMapping(DeviceId, NewUserId, OldUserId);
	OnGamepadUserChanged.Broadcast(*Gamepad, NewUserId, OldUserId);
	Gamepads[DeviceId].PlatformUserId = NewUserId;
	return true;
}
