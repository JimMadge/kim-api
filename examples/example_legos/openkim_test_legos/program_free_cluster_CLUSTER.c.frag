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
* Copyright (c) 2013--2015, Regents of the University of Minnesota.
* All rights reserved.
*
* Contributors:
*    Ryan S. Elliott
*    Ellad B. Tadmor
*    Valeriu Smirichinski
*
*/


/*                                                                      */
/* KIM compliant program to compute the energy of and forces and virial */
/* on an isolated cluster of SPECIES_NAME_STR particles                              */
/*                                                                      */
/* Release: This file is part of the kim-api.git repository.            */
/*                                                                      */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "KIM_API_C.h"
#include "KIM_API_status.h"

#define FCCSPACING    5.260
#define NCELLSPERSIDE 2
#define DIM           3
#define ASPECIES        1
#define NCLUSTERPARTS (4*(NCELLSPERSIDE*NCELLSPERSIDE*NCELLSPERSIDE) + 6*(NCELLSPERSIDE*NCELLSPERSIDE) + 3*(NCELLSPERSIDE) + 1)

/* Define prototypes */
static void create_FCC_configuration(double FCCspacing, int nCellsPerSide, int periodic,
                                     double *coords, int *MiddlePartId);


/* Main program */
int main(int argc, char* argv[])
{
   /* Local variable declarations */
   int i;


   /* KIM variable declarations */
   char testname[] = "TEST_NAME_STR";
   char modelname[KIM_KEY_STRING_LENGTH];
   void* pkim;
   int status;
   int species_code;

   /* model inputs */
   int* numberOfParticles;
   int* numberOfSpecies;
   int* particleSpecies;
   double* coords;
   /* model outputs */
   double* cutoff;
   double* energy;
   double* forces;
   double* virial;
   int middleDum;

   /* Get KIM Model name to use */
   printf("Please enter a valid KIM model name: \n");
   status = scanf("%s",modelname);
   if (1 != status)
   {
     KIM_API_report_error(__LINE__, __FILE__, "Unable to read modelname",
                          status);
     exit(1);
   }

   /* Initialize the KIM Model */
   status = KIM_API_file_init(&pkim, "descriptor.kim", modelname);
   if (KIM_STATUS_OK > status)
   {
      KIM_API_report_error(__LINE__, __FILE__, "KIM_API_file_init", status);
      exit(1);
   }

   /* Allocate memory via the KIM system */
   KIM_API_allocate(pkim, NCLUSTERPARTS, ASPECIES, &status);
   if (KIM_STATUS_OK > status)
   {
      KIM_API_report_error(__LINE__, __FILE__, "KIM_API_allocate", status);
      exit(1);
   }

   /* call Model's init routine */
   status = KIM_API_model_init(pkim);
   if (KIM_STATUS_OK > status)
   {
      KIM_API_report_error(__LINE__, __FILE__, "KIM_API_model_init", status);
      exit(1);
   }

   /* Unpack data from KIM object */
   KIM_API_getm_data(pkim, &status, 8*3,
                     "numberOfParticles",   &numberOfParticles,   1,
                     "numberOfSpecies",     &numberOfSpecies,     1,
                     "particleSpecies",     &particleSpecies,     1,
                     "coordinates",         &coords,              1,
                     "cutoff",              &cutoff,              1,
                     "energy",              &energy,              1,
                     "virial",              &virial,              1,
                     "forces",              &forces,              1);
   if (KIM_STATUS_OK > status)
   {
      KIM_API_report_error(__LINE__, __FILE__, "KIM_API_getm_data", status);
      exit(1);
   }

   /* Set values */
   *numberOfParticles = NCLUSTERPARTS;
   *numberOfSpecies = ASPECIES;
   species_code = KIM_API_get_species_code(pkim, "SPECIES_NAME_STR", &status);
   if (KIM_STATUS_OK > status)
   {
      KIM_API_report_error(__LINE__, __FILE__, "KIM_API_get_species_code", status);
      exit(1);
   }
   for (i = 0; i < *numberOfParticles; ++i)
   {
      particleSpecies[i] = species_code;
   }

   /* set up the cluster particles positions */
   create_FCC_configuration(FCCSPACING, NCELLSPERSIDE, 0, coords, &middleDum);

   /* Call model compute */
   status = KIM_API_model_compute(pkim);
   if (KIM_STATUS_OK > status)
   {
      KIM_API_report_error(__LINE__, __FILE__, "KIM_API_model_compute", status);
      exit(1);
   }

   /* print results to screen */
   printf("--------------------------------------------------------------------------------\n");
   printf("This is Test          : %s\n",testname);
   printf("Results for KIM Model : %s\n",modelname);
   printf("Forces:\n");
   printf("Part     "
          "X                        "
          "Y                        "
          "Z                        "
          "\n");
   for (i = 0; i < *numberOfParticles; ++i)
   {
      printf("%2i   %25.15e%25.15e%25.15e\n", i,
             forces[i*DIM + 0],
             forces[i*DIM + 1],
             forces[i*DIM + 2]
            );
   }
   printf("\n");
   printf("Energy        = %25.15e\n"
          "Global Virial = %25.15e%25.15e%25.15e\n"
          "                %25.15e%25.15e%25.15e\n",
          *energy,
          virial[0],
          virial[1],
          virial[2],
          virial[3],
          virial[4],
          virial[5]
         );


   /* don't forget to destroy and deallocate */
   status = KIM_API_model_destroy(pkim);
   if (KIM_STATUS_OK > status)
   {
      KIM_API_report_error(__LINE__, __FILE__, "KIM_API_model_destory", status);
      exit(1);
   }
   KIM_API_free(&pkim, &status);
   if (KIM_STATUS_OK > status)
   {
      KIM_API_report_error(__LINE__, __FILE__, "KIM_API_free", status);
      exit(1);
   }

   /* everything is great */
   return 0;
}
