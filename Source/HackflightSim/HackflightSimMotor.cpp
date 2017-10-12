/*
HackflightSimMotor.cpp: Motor-simulation class implementation for HackflightSim

Simulates spinning propellers.

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
along with EM7180.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "HackflightSimMotor.h"

// Edit this file to adjust physics
#include "HackflightSimParams.h"

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"

HackflightSimMotor::HackflightSimMotor(APawn * vehicle, UStaticMeshComponent* VehicleMesh, float motorX, float motorY, int8_t direction, uint8_t index)
{

	// We can reuse the same static mesh for all four motor barrels
	struct FMotorConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> MotorMesh;
		FMotorConstructorStatics() : MotorMesh(TEXT("/Game/Hackflight/Meshes/Motor")) { }
	};
	static FMotorConstructorStatics MotorConstructorStatics;

	// We need a static mesh for each propeller

	struct FProp1ConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PropMesh1;
		FProp1ConstructorStatics() : PropMesh1(TEXT("/Game/Hackflight/Meshes/Prop1")) { }
	};
	static FProp1ConstructorStatics Prop1ConstructorStatics;

	struct FProp2ConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PropMesh2;
		FProp2ConstructorStatics() : PropMesh2(TEXT("/Game/Hackflight/Meshes/Prop2")) { }
	};
	static FProp2ConstructorStatics Prop2ConstructorStatics;

	struct FProp3ConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PropMesh3;
		FProp3ConstructorStatics() : PropMesh3(TEXT("/Game/Hackflight/Meshes/Prop3")) { }
	};
	static FProp3ConstructorStatics Prop3ConstructorStatics;

	struct FProp4ConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PropMesh4;
		FProp4ConstructorStatics() : PropMesh4(TEXT("/Game/Hackflight/Meshes/Prop4")) { }
	};
	static FProp4ConstructorStatics Prop4ConstructorStatics;
    
	this->direction = direction;

	UStaticMesh *staticPropMesh = nullptr;

	// Use the appropriate propeller mesh based on the motor index
	switch (index) {

	case 0:
		staticPropMesh = Prop1ConstructorStatics.PropMesh1.Get();
		break;
	case 1:
		staticPropMesh = Prop2ConstructorStatics.PropMesh2.Get();
		break;
	case 2:
		staticPropMesh = Prop3ConstructorStatics.PropMesh3.Get();
		break;
	case 3:
		staticPropMesh = Prop4ConstructorStatics.PropMesh4.Get();
		break;
	}

	// Associate the propeller mesh with a name and object
	char propMeshName[20];
	sprintf_s(propMeshName, "Prop%dMesh", index + 1);
	PropMesh = vehicle->CreateDefaultSubobject<UStaticMeshComponent>(propMeshName);
	PropMesh->SetStaticMesh(staticPropMesh);

	// Associate the motor mesh with a unique name and object
	char motorMeshName[20];
	sprintf_s(motorMeshName, "Motor%dMesh", index + 1);	
	MotorMesh = vehicle->CreateDefaultSubobject<UStaticMeshComponent>(motorMeshName);
	MotorMesh->SetStaticMesh(MotorConstructorStatics.MotorMesh.Get());

	// Create a spring arm to connect the motor to the vehicle
	char motorSpringArmName[20];
	sprintf_s(motorSpringArmName, "Motor%dSpringArm", index + 1);
	MotorSpringArm = vehicle->CreateDefaultSubobject<USpringArmComponent>(motorSpringArmName);
	MotorSpringArm->SetupAttachment(VehicleMesh);
	MotorSpringArm->TargetArmLength = 0.f;
	MotorSpringArm->SocketOffset = FVector(motorX, motorY, 0);
	MotorMesh->SetupAttachment(MotorSpringArm, USpringArmComponent::SocketName);

	// Create a spring arm to connect the propeller to the motor barrel
	char propSpringArmName[20];
	sprintf_s(propSpringArmName, "Prop%dSpringArm", index + 1);
	PropSpringArm = vehicle->CreateDefaultSubobject<USpringArmComponent>(propSpringArmName);
	PropSpringArm->SetupAttachment(MotorMesh);
	PropSpringArm->TargetArmLength = 0.f;
	PropSpringArm->SocketOffset = FVector(0.f, .95f, 2.f);
	PropMesh->SetupAttachment(PropSpringArm, USpringArmComponent::SocketName);
}

void HackflightSimMotor::rotate(float speed)
{
	FRotator PropRotation(0, speed*PARAM_PROP_SPEED*direction, 0);
	PropMesh->AddLocalRotation(PropRotation);
}
