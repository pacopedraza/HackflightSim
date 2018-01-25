/*
   HackflightSimCollision.cpp: collision physics simulation for quadcopters

   Copyright (C) Simon D. Levy 2017

   This file is part of HackflightSim.

   HackflightSim is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   HackflightSim is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.kkkkkkkkkkkkjjjjjjjjjjjjj
   You should have received a copy of the GNU General Public License
   along with HackflightSim.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "HackflightSimCollision.h"

#include <cstring>

#include <stdint.h>
#include <math.h>

#include <debug.hpp>

 // Controls duration and extent of bounce-back on collision
static const float BOUNCEBACK_SECONDS = 2.0f;
static const float BOUNCEBACK_FORCE   = 0.5f;

void Collision::init(void)
{
	memset(&vehicleState, 0, sizeof(vehicle_state_t));

    collidingSeconds = 0;
}


void Collision::notifyHit(vehicle_state_t * state)
{
	// Set movement trajectory to inverse of current trajectory
	// XXX We need a more realistic result, like tumbling to ground.
	for (uint8_t k = 0; k < 3; ++k) {
		vehicleState.pose.position[k].deriv = -BOUNCEBACK_FORCE * state->pose.position[k].deriv;
		vehicleState.pose.orientation[k].deriv = state->pose.orientation[k].deriv;
	}

	// Start collision countdown
	collidingSeconds = BOUNCEBACK_SECONDS;
}

collision_state_t Collision::getCollisionState(float deltaSeconds)
{
	collision_state_t retval = FLYING;

    if (collidingSeconds > 1) {
        collidingSeconds -= deltaSeconds;
        retval = BOUNCING;
    }

	else if (collidingSeconds > 0) {
		retval = FALLING;
	}

    return retval;
}

void Collision::getState(vehicle_state_t * state)
{
	memcpy(state, &vehicleState, sizeof(vehicle_state_t));
}
