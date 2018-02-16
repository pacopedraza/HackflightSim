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
#include "Engine/Engine.h"

// Edit this file to adjust
#include "HackflightSimParams.h"

// Math support
#define _USE_MATH_DEFINES
#include <math.h>

// Hackflight support ---------------------------------------------

// Main firmware
#include <hackflight.hpp>
hf::Hackflight hackflight;

// Controller input
#ifdef _WIN32
#include <receivers/sim/windows.hpp>
#else
#include <receivers/sim/linux.hpp>
#endif

// Additional PID controllers
#include <pid_controllers/altitude_hold.hpp>
hf::AltitudeHold altitudeHold = hf::AltitudeHold(0.04f, 0.50f, 6.00f);

#include <pid_controllers/position_hold.hpp>
hf::PositionHold positionHold = hf::PositionHold(0.f, 0.f, 0.0f);

// Board simulation
#include "HackflightSimBoard.hpp"

// PID tuning
hf::Stabilizer stabilizer = hf::Stabilizer(
	0.10f,      // Level P
	.00001f,     // Gyro cyclic P
	0,			// Gyro cyclic I
	0,			// Gyro cyclic D
	0,			// Gyro yaw P
	0);			// Gyro yaw I


// Board simulation
#include <boards/sim/sim.hpp>
hf::SimBoard board;

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

	// Add altithude-hold and position-hold PID controllers to Hackflight firmware
	hackflight.addPidController(&altitudeHold);
	hackflight.addPidController(&positionHold);

	// Start Hackflight firmware
	hackflight.init(&board, new hf::Controller(), &stabilizer);
	
	// Store initial position, orientation for recovery after collision
	initialLocation = GetActorLocation();
	initialRotation = GetActorRotation();

	// No collision yet
	collisionState = NORMAL;


	// http://bendemott.blogspot.com/2016/10/unreal-4-playing-sound-from-c-with.html 

	// Load our Sound Cue for the propeller sound we created in the editor... 
	// note your path may be different depending
	// on where you store the asset on disk.
	static ConstructorHelpers::FObjectFinder<USoundCue> propellerCue(TEXT("'/Game/Hackflight/MotorSoundCue'"));
	
	// Store a reference to the Cue asset - we'll need it later.
	propellerAudioCue = propellerCue.Object;

	// Create an audio component, the audio component wraps the Cue, 
	// and allows us to ineract with
	// it, and its parameters from code.
	propellerAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PropellerAudioComp"));

	// I don't want the sound playing the moment it's created.
	propellerAudioComponent->bAutoActivate = false;

	// I want the sound to follow the pawn around, so I attach it to the Pawns root.
	propellerAudioComponent->SetupAttachment(GetRootComponent());

	// I want the sound to come from slighty in front of the pawn.
	propellerAudioComponent->SetRelativeLocation(FVector(100.0f, 0.0f, 0.0f));
}

void AHackflightSimVehicle::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (propellerAudioCue->IsValidLowLevelFast()) {
		propellerAudioComponent->SetSound(propellerAudioCue);
	}
}

// Called when the game starts or when spawned
void AHackflightSimVehicle::BeginPlay()
{
	Super::BeginPlay();

	// Note because the Cue Asset is set to loop the sound,
	// once we start playing the sound, it will play 
	// continiously...

	// You can fade the sound in... 
	float startTime = 9.f;
	float volume = 1.0f;
	float fadeTime = 1.0f;
	propellerAudioComponent->FadeIn(fadeTime, volume, startTime);

	// Or you can start playing the sound immediately.
	propellerAudioComponent->Play();
	
}
void AHackflightSimVehicle::Tick(float deltaSeconds)
{
	// Call any parent class Tick implementation
	Super::Tick(deltaSeconds);

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

	collision_state_t retval = NORMAL;

	if (collidingSeconds > 1) {
		collidingSeconds -= deltaSeconds;
		retval = BOUNCING;
	}

	else if (collidingSeconds > 0) {
		retval = FALLING;
	}

	collisionState = retval;

	float motorValues[4] = { 0.5f, 0.5f, 0.5f, 0.5f };

	switch (collisionState) {

	case BOUNCING:
		break;
		
	case FALLING:
		VehicleMesh->SetSimulatePhysics(true);
		break;

	default:

		// Update our flight controller
		hackflight.update();

		// Get current vehicle state from board
		board.simGetVehicleState(gyroRates, motorValues);
	}

	// Spin props, accumulating average motor value
	float motorSum = 0;
	for (int k = 0; k < 4; ++k) {
		motors[k]->rotate(motorValues[k]);
		motorSum += motorValues[k];
	}

	// Modulate the pitch and voume of the propeller sound
	propellerAudioComponent->SetFloatParameter(FName("pitch"), motorSum / 4);
	propellerAudioComponent->SetFloatParameter(FName("volume"), motorSum / 4);

	// Rotate copter in simulation, after converting radians to degrees
	AddActorLocalRotation(deltaSeconds * FRotator(gyroRates[1], gyroRates[2], gyroRates[0]) * (180 / M_PI));

	// Move copter (UE4 uses cm, so multiply by 100 first)
	AddActorLocalOffset(100 * deltaSeconds*FVector(translationRates[0], translationRates[1], translationRates[2]), true);
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

	switch (collisionState) {

	case NORMAL:

		// Set movement trajectory to inverse of current trajectory
		for (uint8_t k = 0; k < 3; ++k) {
			translationRates[k] *= -PARAM_BOUNCEBACK_FORCE;
		}

		// Start collision countdown
		collidingSeconds = PARAM_BOUNCEBACK_SECONDS;	

		break;

	case FALLING:

		// Return control of physics to firmware
		VehicleMesh->SetSimulatePhysics(false);

		// Start Hackflight firmware
		hackflight.init(&board, new hf::Controller(), &stabilizer);

		// No collision
		collisionState = NORMAL;
		collidingSeconds = 0;

		// Return vehicle to its starting position and orientation
		SetActorLocation(initialLocation);
		SetActorRotation(initialRotation);

		break;
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
