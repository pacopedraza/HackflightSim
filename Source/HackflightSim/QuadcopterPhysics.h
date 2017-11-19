/*
   QuadcopterPhysics.h: Physics simulation for quadcopters

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

class QuadcopterPhysics {

    public:

        // Resets all state variables
        void init(void);

        // Updates physics model with Euler angles and motor speeds from boards, time delta from simulator
        void update(float angles[3], float motors[4], float deltaSeconds);

        // Notifies physics that there's been a collision.  
        // XXX This method should get information about the other object, etc.
        void notifyHit(void);

        // Returns true if physics is still handling a collision, false otherwise
        bool handlingCollision(float deltaSeconds);

        // Translational speed in meters per second
        float forwardSpeed;
        float lateralSpeed;
        float verticalSpeed;

		// Vertical acceleration in meters per second per second
		float verticalAcceleration;

        // Vertical position in meters
        float altitude;

        // Rotational speeds, in radians per second
        float angularSpeeds[3];

    private:

        // Starts out false, then true once some thrust is applied, to avoid treating initial floor contact as a collision
        bool flying;

        // Counts down time during which simulation is taken over by collision recovery
        float collidingSeconds;

        float motorsToAngularVelocity(float motors[4], int a, int b, int c, int d);

}; // class QuadcopterPhysics
