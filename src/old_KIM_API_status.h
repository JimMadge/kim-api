/*
*
* CDDL HEADER START
*
* The contents of this file are subject to the terms of the Common Development
* and Distribution License Version 1.0 (the "License").
*
* You can obtain a copy of the license at
* http://www.opensource.org/licenses/CDDL-1.0.  See the License for the
* specific language governing permissions and limitations under the License.
*
* When distributing Covered Code, include this CDDL HEADER in each file and
* include the License file in a prominent location with the name LICENSE.CDDL.
* If applicable, add the following below this CDDL HEADER, with the fields
* enclosed by brackets "[]" replaced with your own identifying information:
*
* Portions Copyright (c) [yyyy] [name of copyright owner]. All rights reserved.
*
* CDDL HEADER END
*
*
*
* Copyright (c) 2013--2017, Regents of the University of Minnesota.
* All rights reserved.
*
* Contributors:
*    Valeriu Smirichinski
*    Ryan S. Elliott
*    Ellad B. Tadmor
*
*/

/*******************************************************************************
*
*  Release: This file is part of the kim-api.git repository.
*
*******************************************************************************/


/* Note: All STATUS codes associated with an error must be less than KIM_STATUS_OK */

#ifndef KIMHDR_OLD_KIM_API_STATUS_H
#define KIMHDR_OLD_KIM_API_STATUS_H

#define KIM_STATUS_MODEL_UNSUPPORTED_CONFIGURATION -19
#define KIM_STATUS_INCONSISTENT_BASE_UNIT          -18
#define KIM_STATUS_UNSUPPORTED_UNIT_TIME           -17
#define KIM_STATUS_UNSUPPORTED_UNIT_TEMPERATURE    -16
#define KIM_STATUS_UNSUPPORTED_UNIT_CHARGE         -15
#define KIM_STATUS_UNSUPPORTED_UNIT_ENERGY         -14
#define KIM_STATUS_UNSUPPORTED_UNIT_LENGTH         -13
#define KIM_STATUS_WRONG_UNIT_HANDLING             -12

#define KIM_STATUS_WRONG_GROUP_ARGUMENT_KEY        -11
#define KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_4      -10
#define KIM_STATUS_WRONG_MULTIPLE_ARGS              -9
#define KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_2       -8
#define KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_3       -7
#define KIM_STATUS_NEIGH_METHOD_NOT_PROVIDED        -6
#define KIM_STATUS_NEIGH_TOO_MANY_NEIGHBORS         -5
#define KIM_STATUS_API_OBJECT_INVALID               -4
#define KIM_STATUS_PARTICLE_INVALID_ID              -3
#define KIM_STATUS_PARTICLE_INVALID_SPECIES         -2
#define KIM_STATUS_ARG_UNKNOWN                      -1
#define KIM_STATUS_FAIL                              0
#define KIM_STATUS_OK                                1


#define KIM_COMPUTE_TRUE                             1
#define KIM_COMPUTE_FALSE                            0


#define KIM_ARCH32BIT                                0
#define KIM_ARCH64BIT                                1

#define KIM_LINK_STATIC                              0
#define KIM_LINK_DYNAMIC_LOAD                        1

#endif /* KIMHDR_OLD_KIM_API_STATUS_H */