/*
QuadcopterPhysics.h: simple physics modeling for quadcopters

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

#pragma once

class QuadcopterPhysics {

    private:

        // Gravitational constant 
        static constexpr float GRAVITY = 9.80665;

        // Starts out false, then true once some thrust is applied, to avoid treating initial floor contact as a collision
        bool flying;

        // Counts down time during which simulation is taken over by collision recovery
        float collidingSeconds;

        // Vertical acceleration in meters per second per second
        float verticalAcceleration;

    public:

        // Translational speed in meters per second
        float forwardSpeed;
        float lateralSpeed;
        float verticalSpeed;

        // Vertical position in meters
        float verticalPosition;

        // Rotational speeds, in radians per second
        float rollSpeed;
        float pitchSpeed;
        float yawSpeed;

        bool handlingCollision(float deltaSeconds);

        float motorsToAngularVelocity(float motors[4], int a, int b, int c, int d);

        void init(void);

        void update(float motors[4], float angles[3], float deltaSeconds);

        void notifyHit(void);
};
