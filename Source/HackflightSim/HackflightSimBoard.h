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

#include <board.hpp>

// Need to include windows.h to declar OutputDebugStringA(), but causes compiler warnings,
// so we wrap this one include in a no-warnings pragma.
// See: https://stackoverflow.com/questions/4001736/whats-up-with-the-thousands-of-warnings-in-standard-headers-in-msvc-wall
#pragma warning(push, 0)       
#include <windows.h>
#pragma warning(pop)

#include <varargs.h>

namespace hf {

	class SimBoard : public Board {

		// These methods are called by Hackflight

		void init(Config& config)
		{
			// Loop timing overrides
			config.loop.imuLoopMicro = 8333;    // approx. simulation period

			angles[0] = 0;
			angles[1] = 0;
			angles[2] = 0;

			// Start at "sea level"
			verticalPositionMeters = 0;
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
			// Convert vehicle's Z coordinate in meters to barometric pressure in Pascals (millibars)
			// At low altitudes above the sea level, the pressure decreases by about 1200 Pa for every 100 meters
			// (See https://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_variation)
			return 1000 * (101.325 - 1.2 * verticalPositionMeters / 100);
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


	private:

		uint64_t micros;
        float gyro[3];
        float anglesPrev[3];
		float verticalPositionMeters;

	public:

		// This method and variables are shared with HackflightSimVehicle
 
		void update(float CurrentRollSpeed, float CurrentPitchSpeed, float CurrentYawSpeed, float CurrentVerticalSpeedCmPerSec, float DeltaSeconds)
		{
			// Track time
			micros += 1e6 * DeltaSeconds;

			// Integrate Euler angle velocities to get Euler angles
			angles[0] += CurrentRollSpeed  * DeltaSeconds;
			angles[1] -= CurrentPitchSpeed * DeltaSeconds;
			angles[2] += CurrentYawSpeed   * DeltaSeconds;

			// Integrate vertical speed to get verticalPosition
			verticalPositionMeters += CurrentVerticalSpeedCmPerSec/100 * DeltaSeconds;

			dprintf("%+2.2f\n", verticalPositionMeters);

            // Compute pitch, roll, yaw first derivative to simulate gyro
            for (int k=0; k<3; ++k) {
                gyro[k] = (angles[k] - anglesPrev[k]) / DeltaSeconds;
                anglesPrev[k] = angles[k];
            }
        }

		// These are shared with HackflightSimVehicle
        float motors[4];
		float angles[3];

	}; // class SimBoard

} // namespace hf
