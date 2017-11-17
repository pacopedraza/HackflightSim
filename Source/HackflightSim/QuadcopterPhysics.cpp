/*
QuadcopterPhysics.cpp: simple physics modeling for quadcopters

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
along with HackfilghtSim.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "QuadcopterPhysics.h"

// Edit this file to adjust physics
#include "HackflightSimParams.h"

void QuadcopterPhysics::init(void)
{
	// We're not flying until there's positive thrust
	flying = false;

	// No collisions yet
	collidingSeconds = 0;

	// Set initial velocities
	forwardSpeed = 0;
	lateralSpeed = 0;
	verticalSpeed = 0;

	// Start at "seal level"
	verticalPosition = 0;

	// No vertical accleration yet
	verticalAcceleration = 0;
}
float QuadcopterPhysics::motorsToAngularVelocity(float motors[4], int a, int b, int c, int d)
{
    return PARAM_VELOCITY_ROTATE_SCALE * ((motors[a] + motors[b]) - (motors[c] + motors[d]));
}
 
