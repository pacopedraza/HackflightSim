/*
HackflightSimMotor.h Motor-simulation class header for HackflightSim

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
along with HackflightSim.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

class HackflightSimMotor {

private:

    class UStaticMeshComponent* PropMesh;
    class USpringArmComponent*  PropSpringArm;
    class UStaticMeshComponent* MotorMesh;
    class USpringArmComponent*  MotorSpringArm;

    int8_t _direction; // +1 clockwise, -1 counterclockwise

public:

	HackflightSimMotor(APawn * vehicle, UStaticMeshComponent* VehicleMesh, float motorX, float motorY, int8_t direction,  uint8_t index);

	void rotate(float speed);
};
