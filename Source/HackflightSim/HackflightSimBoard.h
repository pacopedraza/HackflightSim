/*
   HackflightSimGameBoard.h: Hackflight Board class implementation for HackflightSim

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

// Edit this file to adjust physics
#include "HackflightSimParams.h"

#include <board.hpp>

// Need to include windows.h to declare OutputDebugStringA(), but causes compiler warnings,
// so we wrap this one include in a no-warnings pragma.
// See: https://stackoverflow.com/questions/4001736/whats-up-with-the-thousands-of-warnings-in-standard-headers-in-msvc-wall
#pragma warning(push, 0)       
#include <windows.h>
#pragma warning(pop)

#include <varargs.h>

namespace hf {

    class SimBoard : public Board {

        public:

            // These methods are called by Hackflight

            void init(Config& config)
            {
                // Loop timing overrides
                config.loop.imuLoopMicro = 8333;    // approx. simulation period

                angles[0] = 0;
                angles[1] = 0;
                angles[2] = 0;

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

            bool skipArming(void)
            {
                return true;
            }

            void getImu(float eulerAnglesRadians[3], float gyroRadiansPerSecond[3])
            {
                eulerAnglesRadians[0] = angles[0];
                eulerAnglesRadians[1] = angles[1];
                eulerAnglesRadians[2] = angles[2];

                gyroRadiansPerSecond[0] =  gyro[0];
                gyroRadiansPerSecond[1] = -gyro[1];
                gyroRadiansPerSecond[2] = -gyro[2];
            }

            uint64_t getMicros()
            {
                return micros;
            }

            void writeMotor(uint8_t index, float value)
            {
                motors[index] = value;
            }

            void delayMilliseconds(uint32_t msec)
            {
            }

            virtual bool extrasHaveBaro(void)
            {
                return true;
            }

            virtual float extrasGetBaroPressure(void)
            {
                return baroPressurePascals;
            }

            virtual void extrasImuGetAccel(float accelGs[3])
            {
                for (uint8_t k = 0; k<3; ++k) {
                    accelGs[k] = 1.f; // XXX
                }
            }

            void dprintf(const char * fmt, ...)
            {
                va_list ap;
                va_start(ap, fmt);
                char buf[200];
                vsprintf_s(buf, fmt, ap);
                OutputDebugStringA(buf);
                va_end(ap);
            }


            void notifyHit(void)
            {
                // Set movement trajectory to inverse of current trajectory
                forwardSpeed  *= -PARAM_COLLISION_BOUNCEBACK;
                lateralSpeed  *= -PARAM_COLLISION_BOUNCEBACK;
                verticalSpeed *= -PARAM_COLLISION_BOUNCEBACK;;

                // Start collision countdown
                collidingSeconds = PARAM_COLLISION_SECONDS;
            }

            bool handlingCollision(float deltaSeconds)
            {
                bool retval = false;

                if (collidingSeconds > 0) {
                    collidingSeconds -= deltaSeconds;
                    retval = true;
                }

                return retval;
            }


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

            void updatePhysics(float deltaSeconds)
            {
                // Track time
                micros += 1e6 * deltaSeconds;

                // Integrate Euler angle velocities to get Euler angles
                angles[0] += rollSpeed  * deltaSeconds;
                angles[1] -= pitchSpeed * deltaSeconds;
                angles[2] += yawSpeed   * deltaSeconds;

                // Compute pitch, roll, yaw first derivative to simulate gyro
                for (int k=0; k<3; ++k) {
                    gyro[k] = (angles[k] - anglesPrev[k]) / deltaSeconds;
                    anglesPrev[k] = angles[k];
                }

                // Convert vehicle's Z coordinate in meters to barometric pressure in Pascals (millibars)
                // At low altitudes above the sea level, the pressure decreases by about 1200 Pa for every 100 meters
                // (See https://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_variation)
                baroPressurePascals = 1000 * (101.325 - 1.2 * verticalPosition / 100);

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

                // Integrate vertical speed to get verticalPosition
                verticalPosition += verticalSpeed * deltaSeconds;
            }

            // These are shared with HackflightSimVehicle
            float motors[4];
            float angles[3];

        private:

            uint64_t micros;
            float gyro[3];
            float anglesPrev[3];
            float baroPressurePascals;

            // Gravitational constant 
            static constexpr float GRAVITY = 9.80665;

            // Starts out false, then true once some thrust is applied, to avoid treating initial floor contact as a collision
            bool flying;

            // Counts down time during which simulation is taken over by collision recovery
            float collidingSeconds;

            // Vertical acceleration in meters per second per second
            float verticalAcceleration;

            float motorsToAngularVelocity(float motors[4], int a, int b, int c, int d)
            {
                return PARAM_VELOCITY_ROTATE_SCALE * ((motors[a] + motors[b]) - (motors[c] + motors[d]));
            }

    }; // class SimBoard

} // namespace hf
