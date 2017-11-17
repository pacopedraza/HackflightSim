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

void QuadcopterPhysics::notifyHit(void)
{
    // Set movement trajectory to inverse of current trajectory
    forwardSpeed  *= -PARAM_COLLISION_BOUNCEBACK;
    lateralSpeed  *= -PARAM_COLLISION_BOUNCEBACK;
    verticalSpeed *= -PARAM_COLLISION_BOUNCEBACK;;

    // Start collision countdown
    collidingSeconds = PARAM_COLLISION_SECONDS;
}

bool QuadcopterPhysics::handlingCollision(float deltaSeconds)
{
    bool retval = false;

    if (collidingSeconds > 0) {
        collidingSeconds -= deltaSeconds;
        retval = true;
    }

    return retval;
}

float QuadcopterPhysics::motorsToAngularVelocity(float motors[4], int a, int b, int c, int d)
{
    return PARAM_VELOCITY_ROTATE_SCALE * ((motors[a] + motors[b]) - (motors[c] + motors[d]));
}

void QuadcopterPhysics::update(float motors[4], float angles[3], float deltaSeconds)
{
    // Compute body-frame roll, pitch, yaw velocities based on differences between motors
    rollSpeed  = motorsToAngularVelocity(motors, 2, 3, 0, 1);
    pitchSpeed = motorsToAngularVelocity(motors, 1, 3, 0, 2); 
    yawSpeed   = motorsToAngularVelocity(motors, 1, 2, 0, 3); 

    // Overall thrust vector, scaled by arbitrary constant for realism
    float thrust = PARAM_THRUST_SCALE * (motors[0] + motors[1] + motors[2] + motors[3]);

	// Compute right column of of R matrix converting body coordinates to world coordinates.
	// See page 7 of http://repository.upenn.edu/cgi/viewcontent.cgi?article=1705&context=edissertations.
	float phi   = angles[0];
	float theta = angles[1];
	float psi   = angles[2];
	float r02 = cos(psi)*sin(theta) + cos(theta)*sin(phi)*sin(psi);
	float r12 = sin(psi)*sin(theta) - cos(psi)*cos(theta)*sin(phi);
	float r22 = cos(phi)*cos(theta);

	// Overall vertical force = thrust - gravity
	// We first multiply by the sign of the vertical world coordinate direction, because AddActorLocalOffset()
	// will upside-down vehicle rise on negative velocity.
	verticalAcceleration = (r22 < 0 ? -1 : +1) * (r22*thrust - GRAVITY);

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
