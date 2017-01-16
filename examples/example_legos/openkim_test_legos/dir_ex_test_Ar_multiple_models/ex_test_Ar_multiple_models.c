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
 * Copyright (c) 2013--2017, Regents of the University of Minnesota.
 * All rights reserved.
 *
 * Contributors:
 *    Ryan S. Elliott
 *    Stephen M. Whalen
 *
 */

/*                                                                      */
/* Release: This file is part of the kim-api.git repository.            */
/*                                                                      */


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "KIM_API_C.h"
#include "KIM_API_status.h"

#define NAMESTRLEN    128

#define FCCSPACING    5.260
#define DIM           3
#define NCELLSPERSIDE 2
#define NCLUSTERPARTS (4*(NCELLSPERSIDE*NCELLSPERSIDE*NCELLSPERSIDE) + \
                       6*(NCELLSPERSIDE*NCELLSPERSIDE)                 \
                       + 3*(NCELLSPERSIDE) + 1)

#define REPORT_ERROR(LN, FL, MSG, STAT) {       \
    KIM_API_report_error(LN, FL, MSG, STAT);    \
    exit(STAT);                                 \
  }

/* Define neighborlist structure */
typedef struct
{
  int iteratorId;
  int* NNeighbors;
  int* neighborList;
  double* RijList;
} NeighList;

/* Define prototypes */
void fcc_periodic_neighborlist(int half, int CellsPerHalfSide, double cutoff,
                               double FCCspacing, NeighList* nl);

void fcc_cluster_neighborlist(int half, int numberOfParticles, double* coords,
                              double cutoff, NeighList* nl);

int get_periodic_neigh(void* kimmdl, int *mode, int *request, int* part,
                       int* numnei, int** nei1part, double** Rij);

int get_cluster_neigh(void* kimmdl, int *mode, int *request, int* part,
                      int* numnei, int** nei1part, double** Rij);

void create_FCC_cluster(double FCCspacing, int nCellsPerSide, double *coords);


/* Main program */
int main()
{
  /* Local variable declarations */
  double const MinSpacing = 0.8*FCCSPACING;
  double const MaxSpacing = 1.2*FCCSPACING;
  double const SpacingIncr = 0.025*FCCSPACING;
  double CurrentSpacing;
  double CellsPerCutoff[2];
  double cutpad = 0.75; /* Angstroms */
  int NNeighbors[2];
  int i;
  int status;
  int halfflag_periodic_model_0;
  int halfflag_periodic_model_1;
  int halfflag_cluster_model_0;
  int halfflag_cluster_model_1;


  /* KIM variable declarations */
  void* pkim_periodic_model_0;
  void* pkim_periodic_model_1;
  void* pkim_cluster_model_0;
  void* pkim_cluster_model_1;
  /* model inputs */
  int numberOfParticles_periodic = 1;
  int numberOfParticles_cluster = NCLUSTERPARTS;
  int numberOfSpecies = 1;
  int particleSpeciesShapePeriodic[1];
  int particleSpeciesShapeCluster[1];
  int particleSpecies_periodic_model_0;
  int particleSpecies_periodic_model_1;
  int particleSpecies_cluster_model_0[NCLUSTERPARTS];
  int particleSpecies_cluster_model_1[NCLUSTERPARTS];
  int numContrib_periodic = 1;
  int numContrib_cluster  = NCLUSTERPARTS;
  int coordinatesShapePeriodic[2];
  int coordinatesShapeCluster[2];
  double coords_periodic[DIM] = {0.0, 0.0, 0.0};
  double coords_cluster[NCLUSTERPARTS][DIM];
  NeighList nl_periodic_model_0;
  NeighList nl_periodic_model_1;
  NeighList nl_cluster_model_0;
  NeighList nl_cluster_model_1;
  /* model outputs */
  double cutoff_periodic_model_0;
  double cutoff_periodic_model_1;
  double cutoff_cluster_model_0;
  double cutoff_cluster_model_1;
  double energy_periodic_model_0;
  double energy_periodic_model_1;
  double energy_cluster_model_0;
  double energy_cluster_model_1;

  char testname[] = "ex_test_Ar_multiple_models";
  char testkimfile[] = "descriptor.kim";
  char modelname0[NAMESTRLEN];
  char modelname1[NAMESTRLEN];

  /* Get KIM Model names */
  printf("Please enter two valid KIM Model names: \n");
  status = scanf("%s %s", modelname0, modelname1);
  if (2 != status)
  {
    REPORT_ERROR(__LINE__, __FILE__, "Unable to read two model names",
                 status);
  }

  /* initialize the two models */
  status = KIM_API_file_init(&pkim_periodic_model_0, testkimfile, modelname0);
  if (KIM_STATUS_OK > status)
    REPORT_ERROR(__LINE__, __FILE__,
                 "KIM_API_file_init() for MODEL_ZERO for periodic",
                 status);
  status = KIM_API_file_init(&pkim_cluster_model_0, testkimfile, modelname0);
  if (KIM_STATUS_OK > status)
    REPORT_ERROR(__LINE__, __FILE__,
                 "KIM_API_file_init() for MODEL_ZERO for cluster",
                 status);
  status = KIM_API_file_init(&pkim_periodic_model_1, testkimfile, modelname1);
  if (KIM_STATUS_OK > status)
    REPORT_ERROR(__LINE__, __FILE__,
                 "KIM_API_file_init() for MODEL_ONE for periodic",
                 status);
  status = KIM_API_file_init(&pkim_cluster_model_1, testkimfile, modelname1);
  if (KIM_STATUS_OK > status)
    REPORT_ERROR(__LINE__, __FILE__,
                 "KIM_API_file_init() for MODEL_ONE cluster",
                 status);

  /* Assign shapes & register memory */
  particleSpeciesShapePeriodic[0] = numberOfParticles_periodic;
  particleSpeciesShapeCluster[0] = numberOfParticles_cluster;
  coordinatesShapePeriodic[0] = numberOfParticles_periodic;
  coordinatesShapePeriodic[1] = 3;
  coordinatesShapeCluster[0] = numberOfParticles_cluster;
  coordinatesShapeCluster[1] = 3;

  /* Model 0, periodic */
  KIM_API_set_shape(pkim_periodic_model_0, "particleSpecies", particleSpeciesShapePeriodic, 1, &status);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_set_shape",status);
  KIM_API_set_shape(pkim_periodic_model_0, "coordinates", coordinatesShapePeriodic, 2, &status);
  if (KIM_STATUS_OK > status)
    REPORT_ERROR(__LINE__, __FILE__,"KIM_API_set_shape",status);
  KIM_API_setm_data(pkim_periodic_model_0, &status, 7*4,
                    "numberOfParticles", 1,                              &numberOfParticles_periodic,       1,
                    "numberOfSpecies",   1,                              &numberOfSpecies,                  1,
                    "particleSpecies",   numberOfParticles_periodic,     &particleSpecies_periodic_model_0, 1,
                    "coordinates",       DIM*numberOfParticles_periodic, coords_periodic,                   1,
                    "neighObject",       1,                              &nl_periodic_model_0,              1,
                    "cutoff",            1,                              &cutoff_periodic_model_0,          1,
                    "energy",            1,                              &energy_periodic_model_0,          1);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_setm_data",status);
  status = KIM_API_set_method(pkim_periodic_model_0, "get_neigh", 1, (func_ptr) &get_periodic_neigh);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_set_method",status);

  /* Model 1, periodic */
  KIM_API_set_shape(pkim_periodic_model_1, "particleSpecies", particleSpeciesShapePeriodic, 1, &status);
  if (KIM_STATUS_OK > status)
    REPORT_ERROR(__LINE__, __FILE__,"KIM_API_set_shape",status);
  KIM_API_set_shape(pkim_periodic_model_1, "coordinates", coordinatesShapePeriodic, 2, &status);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_set_shape",status);
  KIM_API_setm_data(pkim_periodic_model_1, &status, 7*4,
                    "numberOfParticles", 1,                              &numberOfParticles_periodic,       1,
                    "numberOfSpecies",   1,                              &numberOfSpecies,                  1,
                    "particleSpecies",   numberOfParticles_periodic,     &particleSpecies_periodic_model_1, 1,
                    "coordinates",       DIM*numberOfParticles_periodic, coords_periodic,                   1,
                    "neighObject",       1,                              &nl_periodic_model_1,              1,
                    "cutoff",            1,                              &cutoff_periodic_model_1,          1,
                    "energy",            1,                              &energy_periodic_model_1,          1);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_setm_data",status);
  status = KIM_API_set_method(pkim_periodic_model_1, "get_neigh", 1, (func_ptr) &get_periodic_neigh);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_set_method",status);

  /* Model 0, cluster */
  KIM_API_set_shape(pkim_cluster_model_0, "particleSpecies", particleSpeciesShapeCluster, 1, &status);
  if (KIM_STATUS_OK > status)
    REPORT_ERROR(__LINE__, __FILE__,"KIM_API_set_shape",status);
  KIM_API_set_shape(pkim_cluster_model_0, "coordinates", coordinatesShapeCluster, 2, &status);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_set_shape",status);
  KIM_API_setm_data(pkim_cluster_model_0, &status, 7*4,
                    "numberOfParticles", 1,                             &numberOfParticles_cluster,       1,
                    "numberOfSpecies",   1,                             &numberOfSpecies,                 1,
                    "particleSpecies",   numberOfParticles_cluster,     &particleSpecies_cluster_model_0, 1,
                    "coordinates",       DIM*numberOfParticles_cluster, coords_cluster,                   1,
                    "neighObject",       1,                             &nl_cluster_model_0,              1,
                    "cutoff",            1,                             &cutoff_cluster_model_0,          1,
                    "energy",            1,                             &energy_cluster_model_0,          1);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_setm_data",status);
  status = KIM_API_set_method(pkim_cluster_model_0, "get_neigh", 1, (func_ptr) &get_cluster_neigh);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_set_method",status);

  /* Model 1, cluster */
  KIM_API_set_shape(pkim_cluster_model_1, "particleSpecies", particleSpeciesShapeCluster, 1, &status);
  if (KIM_STATUS_OK > status)
    REPORT_ERROR(__LINE__, __FILE__,"KIM_API_set_shape",status);
  KIM_API_set_shape(pkim_cluster_model_1, "coordinates", coordinatesShapeCluster, 2, &status);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_set_shape",status);
  KIM_API_setm_data(pkim_cluster_model_1, &status, 7*4,
                    "numberOfParticles", 1,                             &numberOfParticles_cluster,       1,
                    "numberOfSpecies",   1,                             &numberOfSpecies,                 1,
                    "particleSpecies",   numberOfParticles_cluster,     &particleSpecies_cluster_model_1, 1,
                    "coordinates",       DIM*numberOfParticles_cluster, coords_cluster,                   1,
                    "neighObject",       1,                             &nl_cluster_model_1,              1,
                    "cutoff",            1,                             &cutoff_cluster_model_1,          1,
                    "energy",            1,                             &energy_cluster_model_1,          1);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_setm_data",status);
  status = KIM_API_set_method(pkim_cluster_model_1, "get_neigh", 1, (func_ptr) &get_cluster_neigh);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_set_method",status);

  /* call model init routines */
  status = KIM_API_model_init(pkim_periodic_model_0);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_model_init", status);
  status = KIM_API_model_init(pkim_cluster_model_0);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_model_init", status);
  status = KIM_API_model_init(pkim_periodic_model_1);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_model_init", status);
  status = KIM_API_model_init(pkim_cluster_model_1);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_model_init", status);

  /* setup particleSpecies */
  particleSpecies_periodic_model_0 = KIM_API_get_species_code(pkim_periodic_model_0, "Ar", &status);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"get_species_code", status);

  particleSpecies_cluster_model_0[0] = KIM_API_get_species_code(pkim_cluster_model_0, "Ar", &status);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"get_species_code", status);
  for (i = 1; i < NCLUSTERPARTS; ++i)
    particleSpecies_cluster_model_0[i] = particleSpecies_cluster_model_0[0];
  particleSpecies_periodic_model_1 = KIM_API_get_species_code(pkim_periodic_model_1, "Ar", &status);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"get_species_code", status);

  particleSpecies_cluster_model_1[0] = KIM_API_get_species_code(pkim_cluster_model_1, "Ar", &status);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"get_species_code", status);
  for (i = 1; i < NCLUSTERPARTS; ++i)
    particleSpecies_cluster_model_1[i] = particleSpecies_cluster_model_1[0];


  /* Determine which neighbor list type to use */
  halfflag_periodic_model_0 = (KIM_API_is_half_neighbors(pkim_periodic_model_0, &status));
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"is_half_neighbors", status);
  KIM_API_setm_data(pkim_periodic_model_0, &status, 1*4,
                    "numberContributingParticles", 1, &numContrib_periodic, (1==halfflag_periodic_model_0));
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_setm_data", status);

  halfflag_periodic_model_1 = (KIM_API_is_half_neighbors(pkim_periodic_model_1, &status));
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"is_half_neighbors", status);
  KIM_API_setm_data(pkim_periodic_model_1, &status, 1*4,
                    "numberContributingParticles", 1, &numContrib_periodic, (1==halfflag_periodic_model_1));
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_setm_data", status);

  halfflag_cluster_model_0 = (KIM_API_is_half_neighbors(pkim_cluster_model_0, &status));
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"is_half_neighbors", status);
  KIM_API_setm_data(pkim_cluster_model_0, &status, 1*4,
                    "numberContributingParticles", 1, &numContrib_cluster, (1==halfflag_cluster_model_0));
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_setm_data", status);

  halfflag_cluster_model_1 = (KIM_API_is_half_neighbors(pkim_cluster_model_1, &status));
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"is_half_neighbors", status);
  KIM_API_setm_data(pkim_cluster_model_1, &status, 1*4,
                    "numberContributingParticles", 1, &numContrib_cluster, (1==halfflag_cluster_model_1));
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"KIM_API_setm_data", status);


  /* setup neighbor lists */
  CellsPerCutoff[0] = ceil(cutoff_periodic_model_0/MinSpacing + 0.05); /* 0.05 is safety factor */
  CellsPerCutoff[1] = ceil(cutoff_periodic_model_1/MinSpacing + 0.05); /* 0.05 is safety factor */
  NNeighbors[0] = 4*pow((2*CellsPerCutoff[0] + 1),DIM);
  NNeighbors[1] = 4*pow((2*CellsPerCutoff[1] + 1),DIM);
  /* allocate memory for list */
  nl_periodic_model_0.NNeighbors = (int*) malloc(sizeof(int));
  if (NULL==nl_periodic_model_0.NNeighbors) REPORT_ERROR(__LINE__, __FILE__,"malloc unsuccessful", -1);

  nl_cluster_model_0.NNeighbors = (int*) malloc(NCLUSTERPARTS*sizeof(int));
  if (NULL==nl_cluster_model_0.NNeighbors) REPORT_ERROR(__LINE__, __FILE__,"malloc unsuccessful", -1);

  nl_periodic_model_1.NNeighbors = (int*) malloc(sizeof(int));
  if (NULL==nl_periodic_model_1.NNeighbors) REPORT_ERROR(__LINE__, __FILE__,"malloc unsuccessful", -1);

  nl_cluster_model_1.NNeighbors = (int*) malloc(NCLUSTERPARTS*sizeof(int));
  if (NULL==nl_cluster_model_1.NNeighbors) REPORT_ERROR(__LINE__, __FILE__,"malloc unsuccessful", -1);

  nl_periodic_model_0.neighborList = (int*) malloc(NNeighbors[0]*sizeof(int));
  if (NULL==nl_periodic_model_0.neighborList) REPORT_ERROR(__LINE__, __FILE__,"malloc unsuccessful", -1);

  nl_cluster_model_0.neighborList = (int*) malloc(NCLUSTERPARTS*NCLUSTERPARTS*sizeof(int));
  if (NULL==nl_cluster_model_0.neighborList) REPORT_ERROR(__LINE__, __FILE__,"malloc unsuccessful", -1);

  nl_periodic_model_1.neighborList = (int*) malloc(NNeighbors[1]*sizeof(int));
  if (NULL==nl_periodic_model_1.neighborList) REPORT_ERROR(__LINE__, __FILE__,"malloc unsuccessful", -1);

  nl_cluster_model_1.neighborList = (int*) malloc(NCLUSTERPARTS*NCLUSTERPARTS*sizeof(int));
  if (NULL==nl_cluster_model_1.neighborList) REPORT_ERROR(__LINE__, __FILE__,"malloc unsuccessful", -1);

  nl_periodic_model_0.RijList = (double*) malloc(DIM*NNeighbors[0]*sizeof(double));
  if (NULL==nl_periodic_model_0.RijList) REPORT_ERROR(__LINE__, __FILE__,"malloc unsuccessful", -1);

  nl_cluster_model_0.RijList = (double*) malloc(DIM*NCLUSTERPARTS*NCLUSTERPARTS*sizeof(double));
  if (NULL==nl_cluster_model_0.RijList) REPORT_ERROR(__LINE__, __FILE__,"malloc unsuccessful", -1);

  nl_periodic_model_1.RijList = (double*) malloc(DIM*NNeighbors[1]*sizeof(double));
  if (NULL==nl_periodic_model_1.RijList) REPORT_ERROR(__LINE__, __FILE__,"malloc unsuccessful", -1);

  nl_cluster_model_1.RijList = (double*) malloc(DIM*NCLUSTERPARTS*NCLUSTERPARTS*sizeof(double));
  if (NULL==nl_cluster_model_1.RijList) REPORT_ERROR(__LINE__, __FILE__,"malloc unsuccessful", -1);

  /* ready to compute */
  printf("--------------------------------------------------------------------------------\n");
  printf("This is Test  : %s\n",     testname);
  printf("MODEL_ZERO is : %s\n",   modelname0);
  printf("MODEL_ONE  is : %s\n\n", modelname1);
  printf("Energy                    MODEL_ZERO_periodic   MODEL_ZERO_cluster    MODEL_ONE_periodic    MODEL_ONE_cluster     Spacing\n");

  for (CurrentSpacing = MinSpacing; CurrentSpacing < MaxSpacing; CurrentSpacing += SpacingIncr)
  {
    /* update coordinates for cluster */
    create_FCC_cluster(CurrentSpacing, NCELLSPERSIDE, &(coords_cluster[0][0]));

    /* compute neighbor lists */
    fcc_periodic_neighborlist(halfflag_periodic_model_0,
                              CellsPerCutoff[0], (cutoff_periodic_model_0 + cutpad),
                              CurrentSpacing, &nl_periodic_model_0);
    fcc_cluster_neighborlist(halfflag_cluster_model_0,
                             NCLUSTERPARTS, &(coords_cluster[0][0]),
                             (cutoff_cluster_model_0 + cutpad), &nl_cluster_model_0);
    fcc_periodic_neighborlist(halfflag_periodic_model_1,
                              CellsPerCutoff[1], (cutoff_periodic_model_1 + cutpad),
                              CurrentSpacing, &nl_periodic_model_1);
    fcc_cluster_neighborlist(halfflag_cluster_model_1,
                             NCLUSTERPARTS, &(coords_cluster[0][0]),
                             (cutoff_cluster_model_1 + cutpad), &nl_cluster_model_1);

    /* call compute functions */
    status = KIM_API_model_compute(pkim_periodic_model_0);
    if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"compute", status);

    status = KIM_API_model_compute(pkim_cluster_model_0);
    if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"compute", status);

    status = KIM_API_model_compute(pkim_periodic_model_1);
    if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"compute", status);

    status = KIM_API_model_compute(pkim_cluster_model_1);
    if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"compute", status);

    /* print the results */
    printf("Energy for %i parts = %20.10e, %20.10e, %20.10e, %20.10e, %20.10e\n",
           NCLUSTERPARTS,
           energy_periodic_model_0*NCLUSTERPARTS,
           energy_cluster_model_0,
           energy_periodic_model_1*NCLUSTERPARTS,
           energy_cluster_model_1,
           CurrentSpacing);
  }


  /* call model destroy */
  status = KIM_API_model_destroy(pkim_periodic_model_0);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"destroy", status);

  status = KIM_API_model_destroy(pkim_cluster_model_0);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"destroy", status);

  status = KIM_API_model_destroy(pkim_periodic_model_1);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"destroy", status);

  status = KIM_API_model_destroy(pkim_cluster_model_1);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"destroy", status);

  /* free memory of neighbor lists */
  free(nl_periodic_model_0.NNeighbors);
  free(nl_cluster_model_0.NNeighbors);
  free(nl_periodic_model_1.NNeighbors);
  free(nl_cluster_model_1.NNeighbors);
  free(nl_periodic_model_0.neighborList);
  free(nl_cluster_model_0.neighborList);
  free(nl_periodic_model_1.neighborList);
  free(nl_cluster_model_1.neighborList);
  free(nl_periodic_model_0.RijList);
  free(nl_cluster_model_0.RijList);
  free(nl_periodic_model_1.RijList);
  free(nl_cluster_model_1.RijList);

  /* free pkim objects */
  KIM_API_free(&pkim_periodic_model_0, &status);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"free", status);
  KIM_API_free(&pkim_periodic_model_1, &status);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"free", status);
  KIM_API_free(&pkim_cluster_model_0, &status);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"free", status);
  KIM_API_free(&pkim_cluster_model_1, &status);
  if (KIM_STATUS_OK > status) REPORT_ERROR(__LINE__, __FILE__,"free", status);

  /* everything is great */
  return 0;
}


void fcc_periodic_neighborlist(int half, int CellsPerHalfSide, double cutoff,
                               double FCCspacing, NeighList* nl)
{
  /* local variables */
  double dx[DIM];
  double r2;
  double cutoff2;
  double FCCshifts[4][DIM];
  double latVec[DIM];
  int a;
  int i;
  int j;
  int k;
  int m;
  int n;
  double tol = 0.0001 * FCCspacing;

  cutoff2 = cutoff*cutoff;

  /* cubic FCC cell positions */
  FCCshifts[0][0] = 0.0;            FCCshifts[0][1] = 0.0;            FCCshifts[0][2] = 0.0;
  FCCshifts[1][0] = 0.5*FCCspacing; FCCshifts[1][1] = 0.5*FCCspacing; FCCshifts[1][2] = 0.0;
  FCCshifts[2][0] = 0.5*FCCspacing; FCCshifts[2][1] = 0.0;            FCCshifts[2][2] = 0.5*FCCspacing;
  FCCshifts[3][0] = 0.0;            FCCshifts[3][1] = 0.5*FCCspacing; FCCshifts[3][2] = 0.5*FCCspacing;

  if (half)
  {
    a = 0;
    for (i=0; i <= CellsPerHalfSide; ++i)
    {
      latVec[0] = i*FCCspacing;
      for (j=-CellsPerHalfSide; j <= CellsPerHalfSide; ++j)
      {
        latVec[1] = j*FCCspacing;
        for (k=-CellsPerHalfSide; k <= CellsPerHalfSide; ++k)
        {
          latVec[2] = k*FCCspacing;
          for (m=0; m < 4; ++m)
          {
            /* compute dx and dot product at same time */
            r2 = 0.0;
            for (n=0;n<DIM;++n)
            {
              dx[n] = latVec[n] + FCCshifts[m][n];
              r2 += dx[n]*dx[n];
            }

            if (r2 < cutoff2)
            {
              if (! ((0 == i) &&
                     (0 == j) &&
                     (0 == k) &&
                     (0 == m))
                  && (dx[0] > tol ||
                      (abs(dx[0]) < tol && (dx[1] > tol ||
                                            (abs(dx[1]) < tol && dx[2] > tol)))) )
              {
                /* We have a neighbor */
                (*nl).neighborList[a] = 0;
                for (n = 0; n<DIM; ++n)
                  (*nl).RijList[DIM*a + n] = dx[n];
                a++;
              }
            }
          }
        }
      }
    }
    /* there are `a' neighbors */
    *((*nl).NNeighbors) = a;
  }
  else
  {
    a = 0;
    for (i=-CellsPerHalfSide; i <= CellsPerHalfSide; ++i)
    {
      latVec[0] = i*FCCspacing;
      for (j=-CellsPerHalfSide; j <= CellsPerHalfSide; ++j)
      {
        latVec[1] = j*FCCspacing;
        for (k=-CellsPerHalfSide; k <= CellsPerHalfSide; ++k)
        {
          latVec[2] = k*FCCspacing;
          for (m=0; m < 4; ++m)
          {
            /* compute dx and dot product at same time */
            r2 = 0.0;
            for (n=0;n<DIM;++n)
            {
              dx[n] = latVec[n] + FCCshifts[m][n];
              r2 += dx[n]*dx[n];
            }

            if (r2 < cutoff2)
            {
              if (! ((0 == i) &&
                     (0 == j) &&
                     (0 == k) &&
                     (0 == m)))
              {
                /* We have a neighbor */
                (*nl).neighborList[a] = 0;
                for (n = 0; n<DIM; ++n)
                  (*nl).RijList[DIM*a + n] = dx[n];
                a++;
              }
            }
          }
        }
      }
    }
    /* there are `a' neighbors */
    *((*nl).NNeighbors) = a;
  }

  return;
}

int get_periodic_neigh(void* kimmdl, int *mode, int *request, int* part,
                       int* numnei, int** nei1part, double** Rij)
{
  /* local variables */
  intptr_t* pkim = *((intptr_t**) kimmdl);
  int partToReturn;
  int status;
  int* numberOfParticles;
  NeighList* nl;

  /* initialize numnei */
  *numnei = 0;

  /* unpack neighbor list object */
  numberOfParticles = (int*) KIM_API_get_data(pkim, "numberOfParticles", &status);
  if (KIM_STATUS_OK > status)
  {
    KIM_API_report_error(__LINE__, __FILE__,"get_data", status);
    return status;
  }

  nl = (NeighList*) KIM_API_get_data(pkim, "neighObject", &status);
  if (KIM_STATUS_OK > status)
  {
    KIM_API_report_error(__LINE__, __FILE__,"get_data", status);
    return status;
  }

  /* check mode and request */
  if (0 == *mode) /* iterator mode */
  {
    if (0 == *request) /* reset iterator */
    {
      (*nl).iteratorId = -1;
      return KIM_STATUS_NEIGH_ITER_INIT_OK;
    }
    else if (1 == *request) /* increment iterator */
    {
      (*nl).iteratorId++;
      if ((*nl).iteratorId >= *numberOfParticles)
      {
        return KIM_STATUS_NEIGH_ITER_PAST_END;
      }
      else
      {
        partToReturn = (*nl).iteratorId;
      }
    }
    else /* invalid request value */
    {
      KIM_API_report_error(__LINE__, __FILE__,"Invalid request in get_periodic_neigh", KIM_STATUS_NEIGH_INVALID_REQUEST);
      return KIM_STATUS_NEIGH_INVALID_REQUEST;
    }
  }
  else if (1 == *mode) /* locator mode */
  {
    if ((*request >= *numberOfParticles) || (*request < 0)) /* invalid id */
    {
      KIM_API_report_error(__LINE__, __FILE__,"Invalid part ID in get_periodic_neigh", KIM_STATUS_PARTICLE_INVALID_ID);
      return KIM_STATUS_PARTICLE_INVALID_ID;
    }
    else
    {
      partToReturn = *request;
    }
  }
  else /* invalid mode */
  {
    KIM_API_report_error(__LINE__, __FILE__,"Invalid mode in get_periodic_neigh", KIM_STATUS_NEIGH_INVALID_MODE);
    return KIM_STATUS_NEIGH_INVALID_MODE;
  }

  /* set the returned part */
  *part = partToReturn;

  /* set the returned number of neighbors for the returned part */
  *numnei = *((*nl).NNeighbors);

  /* set the location for the returned neighbor list */
  *nei1part = (*nl).neighborList;

  /* set the pointer to Rij to appropriate value */
  *Rij = (*nl).RijList;

  return KIM_STATUS_OK;
}

void create_FCC_cluster(double FCCspacing, int nCellsPerSide, double *coords)
{
  /* local variables */
  double FCCshifts[4][DIM];
  double latVec[DIM];
  int a;
  int i;
  int j;
  int k;
  int m;
  int n;

  /* create a cubic FCC cluster of parts */
  FCCshifts[0][0] = 0.0;            FCCshifts[0][1] = 0.0;            FCCshifts[0][2] = 0.0;
  FCCshifts[1][0] = 0.5*FCCspacing; FCCshifts[1][1] = 0.5*FCCspacing; FCCshifts[1][2] = 0.0;
  FCCshifts[2][0] = 0.5*FCCspacing; FCCshifts[2][1] = 0.0;            FCCshifts[2][2] = 0.5*FCCspacing;
  FCCshifts[3][0] = 0.0;            FCCshifts[3][1] = 0.5*FCCspacing; FCCshifts[3][2] = 0.5*FCCspacing;

  a = 0;
  for (i = 0; i < nCellsPerSide; ++i)
  {
    latVec[0] = ((double) i)*FCCspacing;
    for (j = 0; j < nCellsPerSide; ++j)
    {
      latVec[1] = ((double) j)*FCCspacing;
      for (k = 0; k < nCellsPerSide; ++k)
      {
        latVec[2] = ((double) k)*FCCspacing;
        for (m = 0; m < 4; ++m)
        {
          for (n = 0; n < DIM; ++n)
          {
            coords[a*DIM + n] = latVec[n] + FCCshifts[m][n];
          }
          a++;
        }
      }
      /* add in the remaining three faces */
      /* pos-x face */
      latVec[0] = NCELLSPERSIDE*FCCspacing;
      latVec[1] = ((double) i)*FCCspacing;
      latVec[2] = ((double) j)*FCCspacing;
      for (n = 0; n < DIM; ++n)
      {
        coords[a*DIM + n] = latVec[n];
      }
      a++;
      for (n = 0; n < DIM; ++n)
      {
        coords[a*DIM + n] = latVec[n] + FCCshifts[3][n];
      }
      a++;
      /* pos-y face */
      latVec[0] = ((double) i)*FCCspacing;
      latVec[1] = NCELLSPERSIDE*FCCspacing;
      latVec[2] = ((double) j)*FCCspacing;
      for (n = 0; n < DIM; ++n)
      {
        coords[a*DIM + n] = latVec[n];
      }
      a++;
      for (n = 0; n < DIM; ++n)
      {
        coords[a*DIM + n] = latVec[n] + FCCshifts[2][n];
      }
      a++;
      /* pos-z face */
      latVec[0] = ((double) i)*FCCspacing;
      latVec[1] = ((double) j)*FCCspacing;
      latVec[2] = NCELLSPERSIDE*FCCspacing;
      for (n = 0; n < DIM; ++n)
      {
        coords[a*DIM + n] = latVec[n];
      }
      a++;
      for (n = 0; n < DIM; ++n)
      {
        coords[a*DIM + n] = latVec[n] + FCCshifts[1][n];
      }
      a++;
    }
    /* add in the remaining three edges */
    latVec[0] = ((double) i)*FCCspacing;
    latVec[1] = NCELLSPERSIDE*FCCspacing;
    latVec[2] = NCELLSPERSIDE*FCCspacing;
    for (n = 0; n < DIM; ++n)
    {
      coords[a*DIM + n] = latVec[n];
    }
    a++;
    latVec[0] = NCELLSPERSIDE*FCCspacing;
    latVec[1] = ((double) i)*FCCspacing;
    latVec[2] = NCELLSPERSIDE*FCCspacing;
    for (n = 0; n < DIM; ++n)
    {
      coords[a*DIM + n] = latVec[n];
    }
    a++;
    latVec[0] = NCELLSPERSIDE*FCCspacing;
    latVec[1] = NCELLSPERSIDE*FCCspacing;
    latVec[2] = ((double) i)*FCCspacing;
    for (n = 0; n < DIM; ++n)
    {
      coords[a*DIM + n] = latVec[n];
    }
    a++;
  }
  /* add in the remaining corner */
  for (n = 0; n < DIM; ++n)
  {
    coords[a*DIM + n] = NCELLSPERSIDE*FCCspacing;
  }
  a++;

  return;
}


void fcc_cluster_neighborlist(int half, int numberOfParticles, double* coords,
                              double cutoff, NeighList* nl)
{
  /* local variables */
  int i;
  int j;
  int k;
  int a;

  double dx[DIM];
  double r2;
  double cutoff2;

  cutoff2 = cutoff*cutoff;

  for (i = 0; i < numberOfParticles; ++i)
  {
    a = 0;
    for (j = 0; j < numberOfParticles; ++j)
    {
      r2 = 0.0;
      for (k = 0; k < DIM; ++k)
      {
        dx[k] = coords[j*DIM + k] - coords[i*DIM + k];
        r2 += dx[k]*dx[k];
      }

      if (r2 < cutoff2)
      {
        if ((half && i < j) || (!half && i != j))
        {
          /* part j is a neighbor of part i */
          (*nl).neighborList[i*NCLUSTERPARTS + a] = j;
          for (k = 0; k < DIM; ++k)
          {
            (*nl).RijList[i*DIM*NCLUSTERPARTS + a*DIM + k] = dx[k];
          }
          a++;
        }
      }
    }
    /* part i has `a' neighbors */
    (*nl).NNeighbors[i] = a;
  }

  return;
}

int get_cluster_neigh(void* kimmdl, int *mode, int *request, int* part,
                      int* numnei, int** nei1part, double** Rij)
{
  /* local variables */
  intptr_t* pkim = *((intptr_t**) kimmdl);
  int partToReturn;
  int status;
  int* numberOfParticles;
  NeighList* nl;

  /* initialize numnei */
  *numnei = 0;

  /* unpack neighbor list object */
  numberOfParticles = (int*) KIM_API_get_data(pkim, "numberOfParticles", &status);
  if (KIM_STATUS_OK > status)
  {
    KIM_API_report_error(__LINE__, __FILE__,"get_data", status);
    return status;
  }

  nl = (NeighList*) KIM_API_get_data(pkim, "neighObject", &status);
  if (KIM_STATUS_OK > status)
  {
    KIM_API_report_error(__LINE__, __FILE__,"get_data", status);
    return status;
  }

  /* check mode and request */
  if (0 == *mode) /* iterator mode */
  {
    if (0 == *request) /* reset iterator */
    {
      (*nl).iteratorId = -1;
      return KIM_STATUS_NEIGH_ITER_INIT_OK;
    }
    else if (1 == *request) /* increment iterator */
    {
      (*nl).iteratorId++;
      if ((*nl).iteratorId >= *numberOfParticles)
      {
        return KIM_STATUS_NEIGH_ITER_PAST_END;
      }
      else
      {
        partToReturn = (*nl).iteratorId;
      }
    }
    else /* invalid request value */
    {
      KIM_API_report_error(__LINE__, __FILE__,"Invalid request in get_cluster_neigh", KIM_STATUS_NEIGH_INVALID_REQUEST);
      return KIM_STATUS_NEIGH_INVALID_REQUEST;
    }
  }
  else if (1 == *mode) /* locator mode */
  {
    if ((*request >= *numberOfParticles) || (*request < 0)) /* invalid id */
    {
      KIM_API_report_error(__LINE__, __FILE__,"Invalid part ID in get_cluster_neigh", KIM_STATUS_PARTICLE_INVALID_ID);
      return KIM_STATUS_PARTICLE_INVALID_ID;
    }
    else
    {
      partToReturn = *request;
    }
  }
  else /* invalid mode */
  {
    KIM_API_report_error(__LINE__, __FILE__,"Invalid mode in get_cluster_neigh", KIM_STATUS_NEIGH_INVALID_MODE);
    return KIM_STATUS_NEIGH_INVALID_MODE;
  }

  /* set the returned part */
  *part = partToReturn;

  /* set the returned number of neighbors for the returned part */
  *numnei = (*nl).NNeighbors[*part];

  /* set the location for the returned neighbor list */
  *nei1part = &((*nl).neighborList[(*part)*NCLUSTERPARTS]);

  /* set the pointer to Rij to appropriate value */
  *Rij = &((*nl).RijList[(*part)*DIM*NCLUSTERPARTS]);

  return KIM_STATUS_OK;
}
