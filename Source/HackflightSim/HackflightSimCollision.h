/*
   HackflightSimCollision.h: collision physics simulation for quadcopters

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

class Collision {

    public:

        // Resets all state variables
        void init(void);

        // Notifies physics that there's been a collision.  
        // XXX This method should get information about the other object, etc.
		void notifyHit(float angularSpeeds[3], float linearSpeeds[3]);

        // Returns true if physics is still handling a collision, false otherwise
        bool handlingCollision(float deltaSeconds);

        // Gets vehicle state
        void getState(float angularSpeeds[3], float linearSpeeds[3]);

    private:

        // Translational speed in meters per second
        float _linearSpeeds[3];

        // Rotational speeds, in radians per second
        float _angularSpeeds[3];

        // Counts down time during which simulation is taken over by collision recovery
        float _collidingSeconds;

}; // class Collision
