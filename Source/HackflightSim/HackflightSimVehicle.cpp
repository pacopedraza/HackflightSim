/*
HackflightSimVehicle.cpp: class implementation for AHackflightSimVehicle

Simulates vehicle physics

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
#include "HackflightSimModel.h"

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

    // We're not flying until there's positive thrust
    flying = false;

    // No collisions yet
    collidingSeconds = 0;
    
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

	// Create four motors at specified positions, with specified rotation directions
	motors[0] = new HackflightSimMotor(this, VehicleMesh, PARAM_MOTOR_REAR_X, PARAM_MOTOR_RIGHT_Y,  +1, 0);
	motors[1] = new HackflightSimMotor(this, VehicleMesh, PARAM_MOTOR_FRONT_X, PARAM_MOTOR_RIGHT_Y, -1, 1);
	motors[2] = new HackflightSimMotor(this, VehicleMesh, PARAM_MOTOR_REAR_X, PARAM_MOTOR_LEFT_Y,   -1, 2);
	motors[3] = new HackflightSimMotor(this, VehicleMesh, PARAM_MOTOR_FRONT_X, PARAM_MOTOR_LEFT_Y,  +1, 3);

	// Set initial velocities
	forwardSpeed = 0;
	lateralSpeed = 0;
	verticalSpeed = 0;

	// Start at "seal level"
	verticalPosition = 0;

	// No vertical accleration yet
	verticalAcceleration = 0;

	// Create new SimBoard object
	board = new hf::SimBoard();

	// Start Hackflight firmware
	hackflight.init(board, new hf::Controller(), new hf::SimModel());
}

// Interacts with Hackflight firmware to control vehicle
void AHackflightSimVehicle::update(float deltaSeconds)
{
    // Update our flight controller
    hackflight.update();

    // Compute body-frame roll, pitch, yaw velocities based on differences between motors
    float rollSpeed  = motorsToAngularVelocity(2, 3, 0, 1);
    float pitchSpeed = motorsToAngularVelocity(1, 3, 0, 2); 
    float yawSpeed   = motorsToAngularVelocity(1, 2, 0, 3); 

    // Calculate change in rotation this frame
    FRotator deltaRotation(0, 0, 0);
    deltaRotation.Pitch = pitchSpeed * deltaSeconds;
    deltaRotation.Yaw   = yawSpeed   * deltaSeconds;
    deltaRotation.Roll  = rollSpeed  * deltaSeconds;

    // Rotate copter in simulation, after converting radians to degrees
    AddActorLocalRotation(deltaRotation*(180/M_PI));

    // Send current rotational values and vertical position in meters to board, so firmware can compute PIDs
    board->update(rollSpeed, pitchSpeed, yawSpeed, verticalPosition, deltaSeconds);

    // Overall thrust vector, scaled by arbitrary constant for realism
    float thrust = PARAM_THRUST_SCALE * (board->motors[0] + board->motors[1] + board->motors[2] + board->motors[3]);

	// Compute right column of of R matrix converting body coordinates to world coordinates.
	// See page 7 of http://repository.upenn.edu/cgi/viewcontent.cgi?article=1705&context=edissertations.
	float phi = board->angles[0];
	float theta = board->angles[1];
	float psi = board->angles[2];
	float r02 = cos(psi)*sin(theta) + cos(theta)*sin(phi)*sin(psi);
	float r12 = sin(psi)*sin(theta) - cos(psi)*cos(theta)*sin(phi);
	float r22 = cos(phi)*cos(theta);

	// Overall vertical force = thrust - gravity
	// We first multiply by the sign of the vertical world coordinate direction, because AddActorLocalOffset()
	// will upside-down vehicle rise on negative velocity.
	verticalAcceleration = (r22 < 0 ? -1 : +1) * (r22*thrust - GRAVITY);

	board->dprintf("%+2.2f\n", verticalAcceleration);

    // Once there's enough thrust, we're flying
    if (verticalAcceleration > 0) {
        flying = true;
    }

	if (flying) {

		// Integrate vertical force to get vertical speed
		verticalSpeed += (verticalAcceleration * deltaSeconds);

		// To get forward and lateral speeds, integrate thrust along world coordinates
		lateralSpeed -= thrust * PARAM_VELOCITY_TRANSLATE_SCALE * r12;
		forwardSpeed += thrust * PARAM_VELOCITY_TRANSLATE_SCALE * r02;
	}
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
    if (collidingSeconds > 0) {
        collidingSeconds -= deltaSeconds;
    }

    // Normal operation, vehicle controlled by firmware
    else {
        update(deltaSeconds);
    }

    // Compute current translation movement
    const FVector LocalMove = FVector(forwardSpeed*deltaSeconds, lateralSpeed*deltaSeconds, verticalSpeed*deltaSeconds); 

	// Integrate vertical speed to get verticalPosition
	verticalPosition += verticalSpeed * deltaSeconds;

    // Move copter (UE4 uses cm, so multiply by 100 first)
    AddActorLocalOffset(100*LocalMove, true);

    // Rotate props
    for (int k=0; k<4; ++k)
        motors[k]->rotate(board->motors[k]);

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

    // Set movement trajectory to inverse of current trajectory
    forwardSpeed  *= -PARAM_COLLISION_BOUNCEBACK;
    lateralSpeed  *= -PARAM_COLLISION_BOUNCEBACK;
    verticalSpeed *= -PARAM_COLLISION_BOUNCEBACK;;

    // Start collision countdown
    collidingSeconds = PARAM_COLLISION_SECONDS;
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


float AHackflightSimVehicle::motorsToAngularVelocity(int a, int b, int c, int d)
{
    return PARAM_VELOCITY_ROTATE_SCALE * 
        ((board->motors[a] + board->motors[b]) - (board->motors[c] + board->motors[d]));
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
