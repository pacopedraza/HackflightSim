/*
HackflightSimVehicle.cpp: class implementation for AHackflightSimVehicle

Simulates vehicle physics and calls UE4 methods to move vehicle

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

#include "HackflightSimVehicle.h"
#include "HackflightSimMotor.h"

#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"

// Edit this file to adjust physics
#include "HackflightSimParams.h"

// Debugging support ---------------------------------------------

// Math support
#define _USE_MATH_DEFINES
#include <math.h>

// Hackflight support ---------------------------------------------

// Main firmware
#include <hackflight.hpp>
hf::Hackflight hackflight;

// Controller input
#include <receivers/sim.hpp>

// PID tuning
#include <models/sim.h>

// Collision simulation -------------------------------------------
#include "HackflightSimCollision.h"
Collision collision;

// Pawn methods ---------------------------------------------------

AHackflightSimVehicle::AHackflightSimVehicle()
{
    // We need this to construct the static mesh for the vehicle
	struct FVehicleConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> VehicleMesh;
		FVehicleConstructorStatics() : VehicleMesh(TEXT("/Game/Hackflight/Meshes/3DFly"))	{ }
	};
	static FVehicleConstructorStatics VehicleConstructorStatics;

	// Create static mesh component for vehicle
	VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
	VehicleMesh->SetStaticMesh(VehicleConstructorStatics.VehicleMesh.Get());	// Set static mesh
	RootComponent = VehicleMesh;
    
	// Create the follow camera
    createCameraWithSpringArm( L"FollowCamera", &FollowCamera, L"FollowCameraSpringArm", 
            &FollowCameraSpringArm, PARAM_CAM_DISTANCE, PARAM_CAM_ELEVATION, 0, true);

	// Create the chase camera
    createCameraWithSpringArm( L"ChaseCamera", &ChaseCamera, L"ChaseCameraSpringArm", 
            &ChaseCameraSpringArm, PARAM_CAM_DISTANCE, PARAM_CAM_ELEVATION, 0, false);

	// Create the FPV camera
    createCameraWithSpringArm( L"FpvCamera", &FpvCamera, L"FpvCameraSpringArm", 
            &FpvCameraSpringArm, 0, 0, 0, false);

	// Start with the follow camera activated
	FollowCamera->Activate();
	ChaseCamera->Deactivate();
	FpvCamera->Deactivate();
	activeCameraIndex = 0;
	keyDownTime = 0;

	// Simulate four motors at specified positions, with specified rotation directions
	motors[0] = new HackflightSimMotor(this, VehicleMesh, PARAM_MOTOR_REAR_X, PARAM_MOTOR_RIGHT_Y,  +1, 0);
	motors[1] = new HackflightSimMotor(this, VehicleMesh, PARAM_MOTOR_FRONT_X, PARAM_MOTOR_RIGHT_Y, -1, 1);
	motors[2] = new HackflightSimMotor(this, VehicleMesh, PARAM_MOTOR_REAR_X, PARAM_MOTOR_LEFT_Y,   -1, 2);
	motors[3] = new HackflightSimMotor(this, VehicleMesh, PARAM_MOTOR_FRONT_X, PARAM_MOTOR_LEFT_Y,  +1, 3);

    // Initialize collision physics
    collision.init();

	// Create new SimBoard object, using 120 Hz as IMU loop speed
	board = new hf::SimBoard();

	// Start Hackflight firmware
	hackflight.init(board, new hf::Controller(), new hf::SimModel());

	// Start motionless
	for (uint8_t k = 0; k < 3; ++k) {
		angularSpeeds[k] = 0;
		linearSpeeds[k] = 0;
	}
	flying = false;
}

void AHackflightSimVehicle::Tick(float deltaSeconds)
{
	// Spacebar cycles through cameras
	if (GetWorld()->GetFirstPlayerController()->GetInputKeyTimeDown(FKey("Spacebar")) > 0) {
		keyDownTime += deltaSeconds;
	}
	else {
		if (keyDownTime > 0) {
			cycleCamera();
		}
		keyDownTime = 0;
	}

    // During collision recovery, vehicle is not controlled by firmware
	if (collision.handlingCollision(deltaSeconds)) {

		collision.getState(angularSpeeds, linearSpeeds);
	}

	else {

        // Update our flight controller
        hackflight.update();

		float motorValues[4];

        // Send current physical state to board
		board->getState(angularSpeeds, linearSpeeds, motorValues, flying);

		// Spin props
		for (int k = 0; k<4; ++k)
			motors[k]->rotate(motorValues[k]);
    }

	// Rotate copter in simulation, after converting radians to degrees
	AddActorLocalRotation(deltaSeconds * FRotator(angularSpeeds[1], angularSpeeds[2], angularSpeeds[0]) * (180 / M_PI));

	// Move copter (UE4 uses cm, so multiply by 100 first)
    AddActorLocalOffset(100*deltaSeconds*FVector(linearSpeeds[0], linearSpeeds[1], linearSpeeds[2]), true);
	

    // Call any parent class Tick implementation
    Super::Tick(deltaSeconds);
}

// Collision handling
void AHackflightSimVehicle::NotifyHit(
        class UPrimitiveComponent* MyComp, 
        class AActor* Other, 
        class UPrimitiveComponent* OtherComp, 
        bool bSelfMoved, 
        FVector HitLocation, 
        FVector HitNormal, 
        FVector NormalImpulse, 
        const FHitResult& Hit)
{
    Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// XXX should pass other stuff, like location, other object, etc.
	if (flying) {
		collision.notifyHit(angularSpeeds, linearSpeeds);
	}
}

// Cycles among our three cameras
void AHackflightSimVehicle::cycleCamera(void)
{
    activeCameraIndex = (activeCameraIndex + 1) % 3;

    switch (activeCameraIndex) {

        case 1:
            FollowCamera->Deactivate();
            ChaseCamera->Activate();
            FpvCamera->Deactivate();
            break;
        case 2:
            FollowCamera->Deactivate();
            ChaseCamera->Deactivate();
            FpvCamera->Activate();
            break;
        default:
            FollowCamera->Activate();
            ChaseCamera->Deactivate();
            FpvCamera->Deactivate();
    }
}

void AHackflightSimVehicle::createCameraWithSpringArm(
        const wchar_t * cameraName, 
        UCameraComponent **camera,
        const wchar_t * springArmName,
        USpringArmComponent **springArm,
        float distance,
        float elevation,
        float pitch,
        bool usePawnControlRotation)
{
    *springArm = CreateDefaultSubobject<USpringArmComponent>(springArmName);
    (*springArm)->SetupAttachment(RootComponent);
    (*springArm)->TargetArmLength = distance;
    (*springArm)->SetRelativeLocation(FVector(0.f, 0.f, elevation));
    (*springArm)->bUsePawnControlRotation = usePawnControlRotation; 
    (*springArm)->SetWorldRotation(FRotator(pitch, 0.f, 0.f));

    *camera = CreateDefaultSubobject<UCameraComponent>(cameraName);
    (*camera)->SetupAttachment(*springArm, USpringArmComponent::SocketName); 
    (*camera)->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}
