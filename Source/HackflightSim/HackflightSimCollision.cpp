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
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with HackflightSim.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "HackflightSimCollision.h"

#include <stdint.h>
#include <math.h>

#include <debug.hpp>


 // Controls duration and extent of bounce-back on collision
static const float COLLISION_SECONDS = 3.0f;
static const float COLLISION_BOUNCEBACK = 0.1f;

void Collision::init(void)
{
        for (uint8_t k=0; k<3; ++k) {
            _linearSpeeds[k] = 0;
            _angularSpeeds[k] = 0;
        }

        _collidingSeconds = 0;
}


void Collision::notifyHit(float angularSpeeds[3], float linearSpeeds[3])
{
    // Set movement trajectory to inverse of current trajectory
    // XXX We need a more realistic result, like tumbling to ground.
    for (uint8_t k=0; k<3; ++k) {
        _linearSpeeds[k] = -COLLISION_BOUNCEBACK * linearSpeeds[k];
        _angularSpeeds[k] = angularSpeeds[k];
    }

    // Start collision countdown
    _collidingSeconds = COLLISION_SECONDS;
}

bool Collision::handlingCollision(float deltaSeconds)
{
    bool retval = false;

    if (_collidingSeconds > 0) {
        _collidingSeconds -= deltaSeconds;
        retval = true;
    }

    return retval;
}

void Collision::getState(float angularSpeeds[3], float linearSpeeds[3])
{
    for (uint8_t k=0; k<3; ++k) {
        angularSpeeds[k] = _angularSpeeds[k];
        linearSpeeds[k] = _linearSpeeds[k];
    }
}
