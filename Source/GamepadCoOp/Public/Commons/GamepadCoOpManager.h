// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: GamepadCoOp - Plugin Robust Gamepad Management for Seamless Local Multiplayer.
// Planned Release Year: 2025

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/InputSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Structs/FGamepadCoOp.h"
#include "GamepadCoOpManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamepadConnectionChanged, const FGamepadCoOp&, Gamepad);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGamepadUserChanged, const FGamepadCoOp&, Gamepad, FPlatformUserId, NewUser, FPlatformUserId, OldUser);

UCLASS(Blueprintable, BlueprintType)
class GAMEPADCOOP_API UGamepadCoOpManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "GamepadCoOp", meta = (WorldContext = "WorldContextObject"))
	static UGamepadCoOpManager* Get(const UObject* WorldContextObject);
	
	UPROPERTY(BlueprintAssignable, Category = "GamepadCoOp|Events")
	FOnGamepadConnectionChanged OnGamepadConnected;
	
	UPROPERTY(BlueprintAssignable, Category = "GamepadCoOp|Events")
	FOnGamepadConnectionChanged OnGamepadDisconnected;
	
	UPROPERTY(BlueprintAssignable, Category = "GamepadCoOp|Events")
	FOnGamepadUserChanged OnGamepadUserChanged;

	UFUNCTION(BlueprintPure, Category = "GamepadCoOp|Mapping")
	bool GetGamepad(const FInputDeviceId& DeviceId, FGamepadCoOp& OutGamepad) const;

	UFUNCTION(BlueprintPure, Category = "GamepadCoOp|MakeUser")
	FPlatformUserId CoOpPlatformUserId(int32 LocalPlayerControllerId) const;

	UFUNCTION(BlueprintPure, Category = "GamepadCoOp|Mapping")
	void GetAllGamepadsForUser(const FPlatformUserId& UserId, TArray<FGamepadCoOp>& OutGamepads) const;

	UFUNCTION(BlueprintPure, Category = "GamepadCoOp|Mapping")
	void GetAllGamepads(TArray<FGamepadCoOp>& OutGamepads) const;

	UFUNCTION(BlueprintPure, Category = "GamepadCoOp|Mapping")
	FInputDeviceId GetPrimaryGamepadForUser(const FPlatformUserId NewUser) const;
	
	UFUNCTION(BlueprintPure, Category = "GamepadCoOp|Mapping")
	FHardwareDeviceIdentifier GetHardwareDeviceIdentifier(const FInputDeviceId Device) const;

	UFUNCTION(BlueprintCallable, Category = "GamepadCoOp|Mapping")
	bool RemapGamepadToUser(const FInputDeviceId& DeviceId, const FPlatformUserId& NewUserId);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void HandleDeviceConnected(EInputDeviceConnectionState ConnectionState, FPlatformUserId UserId, FInputDeviceId DeviceId);

	void HandleDeviceDisconnected(const FInputDeviceId DeviceId);

    void HandleDeviceUserChanged(const FInputDeviceId DeviceId, const FPlatformUserId NewUser, const FPlatformUserId OldUser);

	void RegisterGamepad(EInputDeviceConnectionState ConnectionState, FPlatformUserId UserId, FInputDeviceId DeviceId);

	void UnregisterGamepad(const FInputDeviceId DeviceId);
	
	TMap<FInputDeviceId, FGamepadCoOp> Gamepads;
};