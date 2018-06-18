//
// CDDL HEADER START
//
// The contents of this file are subject to the terms of the Common
// Development and Distribution License Version 1.0 (the "License").
//
// You can obtain a copy of the license at
// http://www.opensource.org/licenses/CDDL-1.0.  See the License for the
// specific language governing permissions and limitations under the License.
//
// When distributing Covered Code, include this CDDL HEADER in each file and
// include the License file in a prominent location with the name
// LICENSE.CDDL.
// If applicable, add the following below this CDDL HEADER, with the fields
// enclosed by brackets "[]" replaced with your own identifying information:
//
// Portions Copyright (c) [yyyy] [name of copyright owner].
// All rights reserved.
//
// CDDL HEADER END
//

//
// Copyright (c) 2016--2018, Regents of the University of Minnesota.
// All rights reserved.
//
// Contributors:
//    Ryan S. Elliott
//

//
// Release: This file is part of the kim-api.git repository.
//

#ifndef KIM_TEMPERATURE_UNIT_HPP_
#include "KIM_TemperatureUnit.hpp"
#endif
extern "C"
{
#ifndef KIM_TEMPERATURE_UNIT_H_
#include "KIM_TemperatureUnit.h"
#endif
}


namespace
{
KIM::TemperatureUnit const makeTemperatureUnitCpp(
    KIM_TemperatureUnit const temperatureUnit)
{
  KIM::TemperatureUnit const * const temperatureUnitCpp
      = reinterpret_cast <KIM::TemperatureUnit const * const>(&temperatureUnit);
  return *temperatureUnitCpp;
}

KIM_TemperatureUnit const makeTemperatureUnitC(
    KIM::TemperatureUnit const temperatureUnit)
{
  KIM_TemperatureUnit const * const temperatureUnitC
      = reinterpret_cast <KIM_TemperatureUnit const * const>(&temperatureUnit);
  return *temperatureUnitC;
}
}  // namespace

extern "C"
{
KIM_TemperatureUnit KIM_TemperatureUnitFromString(char const * const str)
{
  return makeTemperatureUnitC(KIM::TemperatureUnit(std::string(str)));
}

int KIM_TemperatureUnitEqual(KIM_TemperatureUnit const left,
                             KIM_TemperatureUnit const right)
{
  return (left.temperatureUnitID == right.temperatureUnitID);
}

int KIM_TemperatureUnitNotEqual(KIM_TemperatureUnit const left,
                                KIM_TemperatureUnit const right)
{
  return (!KIM_TemperatureUnitEqual(left, right));
}

char const * const KIM_TemperatureUnitString(
    KIM_TemperatureUnit const temperatureUnit)
{
  return makeTemperatureUnitCpp(temperatureUnit).String().c_str();
}

#include "KIM_TemperatureUnit.inc"
KIM_TemperatureUnit const KIM_TEMPERATURE_UNIT_unused = {ID_unused};
KIM_TemperatureUnit const KIM_TEMPERATURE_UNIT_K = {ID_K};

}  // extern "C"
