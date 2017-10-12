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
along with EM7180.  If not, see <http://www.gnu.org/licenses/>.
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

		void init(Config& config)
		{
			// Loop timing overrides
			config.loop.imuLoopMicro = 8333;    // approx. simulation period

			angles[0] = 0;
			angles[1] = 0;
			angles[2] = 0;
		}

		bool skipArming(void)
		{
			return true;
		}

		void getImu(float eulerAnglesRadians[3], int16_t gyroRaw[3])
		{
			eulerAnglesRadians[0] = angles[0];
			eulerAnglesRadians[1] = angles[1];
			eulerAnglesRadians[2] = angles[2];

            // Scale gyro to +/-4096
            static const float GYROSCALE = 60;
            gyroRaw[0] =  (int16_t)(GYROSCALE * gyro[0]);
            gyroRaw[1] = -(int16_t)(GYROSCALE * gyro[1]);
            gyroRaw[2] = -(int16_t)(GYROSCALE * gyro[2]);
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

	private:

		uint64_t micros;
        float gyro[3];
        float anglesPrev[3];

	public:

		void update(float CurrentRollSpeed, float CurrentPitchSpeed, float CurrentYawSpeed, float DeltaSeconds)
		{
			// Track time
			micros += 1e6 * DeltaSeconds;

			// Integrate Euler angle velocities to get Euler angles
			angles[0] += CurrentRollSpeed  * DeltaSeconds;
			angles[1] -= CurrentPitchSpeed * DeltaSeconds;
			angles[2] += CurrentYawSpeed   * DeltaSeconds;

            // Compute pitch, roll, yaw first derivative to simulate gyro
            for (int k=0; k<3; ++k) {
                gyro[k] = (angles[k] - anglesPrev[k]) / DeltaSeconds;
                anglesPrev[k] = angles[k];
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

		// These are shared with HackflightSimVehicle
        float motors[4];
		float angles[3];

	}; // class SimBoard

} // namespace hf
