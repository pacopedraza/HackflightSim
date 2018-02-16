/*
HackflightSimVehicle.h: main actor class header for HackflightSim

Subclasses UnrealEngine Pawn class

Copyright (C) Simon D. Levy 2017

This file is part of HackflightSim.

HackflightSim is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HackflightSim is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with HackflightSim.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <datatypes.hpp>

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Runtime/Engine/Classes/Sound/SoundCue.h"
#include "Components/AudioComponent.h"

#include "HackflightSimMotor.h"

#include "HackflightSimVehicle.generated.h"

UCLASS(Config=Game)
class AHackflightSimVehicle : public APawn
{
	GENERATED_BODY()

	// StaticMesh component that will be the visuals for our flying pawn
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* VehicleMesh;

	// Chase camera
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* ChaseCamera;

	// Spring arm for chase camera
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* ChaseCameraSpringArm;

	// Follow camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	// Spring arm for follow camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* FollowCameraSpringArm;

	// FPV camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FpvCamera;

	// Spring arm for FPV camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* FpvCameraSpringArm;

	// Audio support: see http://bendemott.blogspot.com/2016/10/unreal-4-playing-sound-from-c-with.html

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	class USoundCue* propellerAudioCue;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	class USoundCue* propellerStartupCue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	class UAudioComponent* propellerAudioComponent;
		

public:

	AHackflightSimVehicle();

	// AActor overrides
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void NotifyHit(
            class UPrimitiveComponent* MyComp, 
            class AActor* Other, 
            class UPrimitiveComponent* OtherComp, 
            bool bSelfMoved, 
            FVector HitLocation, 
            FVector HitNormal, 
            FVector NormalImpulse, 
            const FHitResult& Hit) 
        override;

private:

	typedef enum {

		NORMAL,
		BOUNCING,
		FALLING

	} collision_state_t;

	// Everything we should need to display the vehicle
    float gyroRates[3];
    float translationRates[3];
	HackflightSimMotor * motors[4];

	// Creates a camera and associated spring-arm
    void createCameraWithSpringArm(
            const wchar_t * cameraName, 
            UCameraComponent **camera,
            const wchar_t * springArmName,
            USpringArmComponent **springArm,
            float distance,
            float elevation,
            float pitch,
            bool usePawnControlRotation);

	// Helps us cycle among cameras
	uint8_t activeCameraIndex;
	float keyDownTime;
	void cycleCamera(void);

	collision_state_t collisionState;

	// Counts down time during which simulation is taken over by collision recovery
	float collidingSeconds;

	// We need initial location and orientation to restore vehicle after a collision
	FVector initialLocation;
	FRotator initialRotation;

public:

	FORCEINLINE class UStaticMeshComponent* GetVehicleMesh() const { return VehicleMesh; }
	FORCEINLINE class USpringArmComponent* GetFollowCameraSpringArm() const { return FollowCameraSpringArm; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE class USpringArmComponent* GetChaseCameraSpringArm() const { return ChaseCameraSpringArm; }
	FORCEINLINE class UCameraComponent* GetChaseCamera() const { return ChaseCamera; }
};
