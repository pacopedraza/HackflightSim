/*
HackflightSimParams.h: physics simulation parameters for HackflightSim

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

// Arbitrary
static const float PARAM_PROP_SPEED = 40.f;

// Distance, elevation for chase/follow cameras
static const float PARAM_CAM_DISTANCE  = 60.f;
static const float PARAM_CAM_ELEVATION = 17.f;

// Appropriate for 3DFly frame
static const float PARAM_MOTOR_REAR_X  = -3.7f;
static const float PARAM_MOTOR_FRONT_X = +3.7f;
static const float PARAM_MOTOR_RIGHT_Y = +2.8f;
static const float PARAM_MOTOR_LEFT_Y  = -4.7f;

// Scales up thrust (substitutes for mass, torque, etc.)
static const float PARAM_THRUST_SCALE      = 5.f;

// Minimum thrust for liftoff (assumes vehicle oriented for takeoff on start)
static const float PARAM_THRUST_MIN_FLYING = 12.f;

// Controls "snappiness" of response
static const float PARAM_VELOCITY_ROTATE_SCALE    = 100.f;
static const float PARAM_VELOCITY_TRANSLATE_SCALE = 5.f;

// Controls duration and extent of bounce-back on collision
static const float PARAM_COLLISION_SECONDS    = 1.0f;
static const float PARAM_COLLISION_BOUNCEBACK = 0.1f;
