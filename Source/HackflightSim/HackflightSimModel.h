/*
HackflightSimModel.h: PID and other model values for HackflightSim

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

#include <model.hpp>

namespace hf {

	class SimModel : public Model {

	public:

		SimModel(void) {

			// Level (accelerometer): set to zero for rate mode
			levelP = 0.10f;

			// Rate (gyro): P must be positive
			ratePitchrollP = .00001f;// 0.125f;
			ratePitchrollI = 0;// 0.05f;
			ratePitchrollD = 0;// 0.01f;

			// Yaw: P must be positive
			yawP = 0.0001f;// 0.1f;
			yawI = 0;// 0.05f;
		}
	};

} // namespace hf
