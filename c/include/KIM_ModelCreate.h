/*                                                                            */
/* CDDL HEADER START                                                          */
/*                                                                            */
/* The contents of this file are subject to the terms of the Common           */
/* Development and Distribution License Version 1.0 (the "License").          */
/*                                                                            */
/* You can obtain a copy of the license at                                    */
/* http://www.opensource.org/licenses/CDDL-1.0.  See the License for the      */
/* specific language governing permissions and limitations under the License. */
/*                                                                            */
/* When distributing Covered Code, include this CDDL HEADER in each file and  */
/* include the License file in a prominent location with the name             */
/* LICENSE.CDDL.                                                              */
/* If applicable, add the following below this CDDL HEADER, with the fields   */
/* enclosed by brackets "[]" replaced with your own identifying information:  */
/*                                                                            */
/* Portions Copyright (c) [yyyy] [name of copyright owner].                   */
/* All rights reserved.                                                       */
/*                                                                            */
/* CDDL HEADER END                                                            */
/*                                                                            */

/*                                                                            */
/* Copyright (c) 2016--2017, Regents of the University of Minnesota.          */
/* All rights reserved.                                                       */
/*                                                                            */
/* Contributors:                                                              */
/*    Ryan S. Elliott                                                         */
/*                                                                            */

/*                                                                            */
/* Release: This file is part of the kim-api.git repository.                  */
/*                                                                            */


#ifndef KIM_MODEL_CREATE_H_
#define KIM_MODEL_CREATE_H_

#ifndef KIM_FUNC_H_
#include "KIM_func.h"
#endif

/* Forward declarations */
#ifndef KIM_LOG_VERBOSITY_DEFINED_
#define KIM_LOG_VERBOSITY_DEFINED_
typedef struct KIM_LogVerbosity KIM_LogVerbosity;
#endif

#ifndef KIM_SPECIES_NAME_DEFINED_
#define KIM_SPECIES_NAME_DEFINED_
typedef struct KIM_SpeciesName KIM_SpeciesName;
#endif

#ifndef KIM_LANGUAGE_NAME_DEFINED_
#define KIM_LANGUAGE_NAME_DEFINED_
typedef struct KIM_LanguageName KIM_LanguageName;
#endif

#ifndef KIM_NUMBERING_DEFINED_
#define KIM_NUMBERING_DEFINED_
typedef struct KIM_Numbering KIM_Numbering;
#endif

#ifndef KIM_LENGTH_UNIT_DEFINED_
#define KIM_LENGTH_UNIT_DEFINED_
typedef struct KIM_LengthUnit KIM_LengthUnit;
#endif

#ifndef KIM_ENERGY_UNIT_DEFINED_
#define KIM_ENERGY_UNIT_DEFINED_
typedef struct KIM_EnergyUnit KIM_EnergyUnit;
#endif

#ifndef KIM_CHARGE_UNIT_DEFINED_
#define KIM_CHARGE_UNIT_DEFINED_
typedef struct KIM_ChargeUnit KIM_ChargeUnit;
#endif

#ifndef KIM_TEMPERATURE_UNIT_DEFINED_
#define KIM_TEMPERATURE_UNIT_DEFINED_
typedef struct KIM_TemperatureUnit KIM_TemperatureUnit;
#endif

#ifndef KIM_TIME_UNIT_DEFINED_
#define KIM_TIME_UNIT_DEFINED_
typedef struct KIM_TimeUnit KIM_TimeUnit;
#endif

#ifndef KIM_SUPPORT_STATUS_DEFINED_
#define KIM_SUPPORT_STATUS_DEFINED_
typedef struct KIM_SupportStatus KIM_SupportStatus;
#endif

#ifndef KIM_ARGUMENT_NAME_DEFINED_
#define KIM_ARGUMENT_NAME_DEFINED_
typedef struct KIM_ArgumentName KIM_ArgumentName;
#endif

#ifndef KIM_CALLBACK_NAME_DEFINED_
#define KIM_CALLBACK_NAME_DEFINED_
typedef struct KIM_CallbackName KIM_CallbackName;
#endif


struct KIM_ModelCreate;

#ifndef KIM_MODEL_CREATE_DEFINED_
#define KIM_MODEL_CREATE_DEFINED_
typedef struct KIM_ModelCreate KIM_ModelCreate;
#endif


int KIM_ModelCreate_SetModelNumbering(
    KIM_ModelCreate * const modelCreate,
    KIM_Numbering const numbering);

void KIM_ModelCreate_SetInfluenceDistancePointer(
    KIM_ModelCreate * const modelCreate,
    double * const influenceDistance);

void KIM_ModelCreate_SetNeighborListCutoffsPointer(
    KIM_ModelCreate * const modelCreate,
    int const numberOfCutoffs, double const * const cutoffs);

int KIM_ModelCreate_SetRefreshPointer(
    KIM_ModelCreate * const modelCreate,
    KIM_LanguageName const languageName, func * const fptr);
int KIM_ModelCreate_SetDestroyPointer(
    KIM_ModelCreate * const modelCreate,
    KIM_LanguageName const languageName, func * const fptr);
int KIM_ModelCreate_SetComputePointer(
    KIM_ModelCreate * const modelCreate,
    KIM_LanguageName const languageName, func * const fptr);

int KIM_ModelCreate_SetSpeciesCode(
    KIM_ModelCreate * const modelCreate,
    KIM_SpeciesName const speciesName, int const code);

int KIM_ModelCreate_SetArgumentSupportStatus(
    KIM_ModelCreate * const modelCreate,
    KIM_ArgumentName const argumentName, KIM_SupportStatus const supportStatus);

int KIM_ModelCreate_SetCallbackSupportStatus(
    KIM_ModelCreate * const modelCreate,
    KIM_CallbackName const callbackName, KIM_SupportStatus const supportStatus);

int KIM_ModelCreate_SetParameterPointerInteger(
    KIM_ModelCreate * const modelCreate,
    int const extent, int * const ptr, char const * const description);

int KIM_ModelCreate_SetParameterPointerDouble(
    KIM_ModelCreate * const modelCreate,
    int const extent, double * const ptr, char const * const description);

void KIM_ModelCreate_SetModelBufferPointer(
    KIM_ModelCreate * const modelCreate, void * const ptr);

int KIM_ModelCreate_SetUnits(
    KIM_ModelCreate * const modelCreate,
    KIM_LengthUnit const lengthUnit,
    KIM_EnergyUnit const energyUnit,
    KIM_ChargeUnit const chargeUnit,
    KIM_TemperatureUnit const temperatureUnit,
    KIM_TimeUnit const timeUnit);

int KIM_ModelCreate_ConvertUnit(
    KIM_ModelCreate const * const modelCreate,
    KIM_LengthUnit const fromLengthUnit,
    KIM_EnergyUnit const fromEnergyUnit,
    KIM_ChargeUnit const fromChargeUnit,
    KIM_TemperatureUnit const fromTemperatureUnit,
    KIM_TimeUnit const fromTimeUnit,
    KIM_LengthUnit const toLengthUnit,
    KIM_EnergyUnit const toEnergyUnit,
    KIM_ChargeUnit const toChargeUnit,
    KIM_TemperatureUnit const toTemperatureUnit,
    KIM_TimeUnit const toTimeUnit,
    double const lengthExponent,
    double const energyExponent,
    double const chargeExponent,
    double const temperatureExponent,
    double const timeExponent,
    double * const conversionFactor);

void KIM_ModelCreate_LogEntry(
    KIM_ModelCreate const * const modelCreate,
    KIM_LogVerbosity const logVerbosity, char const * const message,
    int const lineNumber, char const * const fileName);

char const * const KIM_ModelCreate_String(
    KIM_ModelCreate const * const modelCreate);

#endif  /* KIM_MODEL_CREATE_H_ */
