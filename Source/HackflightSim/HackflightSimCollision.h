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

#include <datatypes.hpp>

class Collision {

    public:

        // Resets all state variables
        void init(void);

        // Notifies physics that there's been a collision.  
        // XXX This method should get information about the other object, etc.
		void notifyHit(vehicle_state_t * state);

        // Returns true if physics is still handling a collision, false otherwise
        bool handlingCollision(float deltaSeconds);

        // Gets vehicle state
        void getState(vehicle_state_t * state);

    private:

		// Vehicle state (position, angles and their first derivatives)
		vehicle_state_t vehicleState;

        // Counts down time during which simulation is taken over by collision recovery
        float collidingSeconds;

}; // class Collision
