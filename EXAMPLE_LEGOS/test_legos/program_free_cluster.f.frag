!*******************************************************************************
!**
!**  PROGRAM TEST_NAME_STR
!**
!**  KIM compliant program to compute the energy of and forces on an isolated 
!**  cluster of SPECIES_NAME_STR atoms
!**
!**  Works with the following NBC methods:
!**        MI-OPBC-H
!**        MI-OPBC-F
!**        NEIGH-PURE-H
!**        NEIGH-PURE-F
!**        NEIGH-RVEC-F
!**
!**  Authors: Valeriu Smirichinski, Ryan S. Elliott, Ellad B. Tadmor
!**
!**  Copyright 2011 Ellad B. Tadmor, Ryan S. Elliott, and James P. Sethna
!**  All rights reserved.
!**
!*******************************************************************************


!-------------------------------------------------------------------------------
!
! Main program
!
!-------------------------------------------------------------------------------
program TEST_NAME_STR
  use KIMservice
  implicit none

  integer,                  external  :: get_neigh_no_Rij
  integer,                  external  :: get_neigh_Rij
  double precision,         parameter :: FCCspacing     = FCC_SPACING_STR
  integer,                  parameter :: nCellsPerSide  = 2
  integer,                  parameter :: DIM            = 3
  integer,                  parameter :: ATypes         = 1
  integer(kind=kim_intptr), parameter :: &
       N = 4*(nCellsPerSide)**3 + 6*(nCellsPerSide)**2 + 3*(nCellsPerSide) + 1
  integer(kind=kim_intptr), parameter :: SizeOne        = 1

  ! neighbor list
  integer, allocatable          :: neighborList(:,:)
  integer, allocatable          :: NLRvecLocs(:)
  double precision, allocatable :: RijList(:,:,:)

  !
  ! KIM variables
  !
  character*80              :: testname     = "TEST_NAME_STR"
  character*80              :: modelname
  character*64 :: NBC_Method; pointer(pNBC_Method,NBC_Method)
  integer :: nbc  ! 0- MI-OPBC-H, 1- MI-OPBC-F, 2- NEIGH-PURE-H, 3- NEIGH-PURE-F, 4- NEIGH-RVCE-F
  integer(kind=kim_intptr)  :: pkim
  integer                   :: ier
  integer(kind=8) numberOfAtoms; pointer(pnAtoms,numberOfAtoms)
  integer numberAtomTypes;       pointer(pnAtomTypes,numberAtomTypes)
  integer atomTypesdum(1);       pointer(patomTypesdum,atomTypesdum)

  real*8 cutoff;           pointer(pcutoff,cutoff)
  real*8 energy;           pointer(penergy,energy)
  real*8 coordum(DIM,1);   pointer(pcoor,coordum)
  real*8 forcesdum(DIM,1); pointer(pforces,forcesdum)
  real*8 boxlength(DIM);   pointer(pboxlength,boxlength)
  integer N4
  real*8, pointer  :: coords(:,:), forces(:,:)
  integer, pointer :: atomTypes(:)
  integer :: middleDum
  N4 = N

  
  ! Get KIM Model name to use
  print *, "Please enter a valid KIM model name: "
  read(*,*) modelname

  ! Initialize the KIM object
  ier = kim_api_init_f(pkim, testname, modelname)
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_init_f", ier)
     stop
  endif

  ! Allocate memory via the KIM system
  call kim_api_allocate_f(pkim, N, ATypes, ier)
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_allocate_f", ier)
     stop
  endif

  ! call model's init routine
  ier = kim_api_model_init_f(pkim)
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_model_init", ier)
     stop
  endif

  ! determine which NBC scenerio to use
  pNBC_Method = kim_api_get_nbc_method_f(pkim, ier) ! don't forget to free
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_get_nbc_method", ier)
     stop
  endif
  if (index(NBC_Method,"MI-OPBC-H").eq.1) then
     nbc = 0
  elseif (index(NBC_Method,"MI-OPBC-F").eq.1) then
     nbc = 1
  elseif (index(NBC_Method,"NEIGH-PURE-H").eq.1) then
     nbc = 2
  elseif (index(NBC_Method,"NEIGH-PURE-F").eq.1) then
     nbc = 3
  elseif (index(NBC_Method,"NEIGH-RVEC-F").eq.1) then
     nbc = 4
  else
     ier = 0
     call report_error(__LINE__, "Unknown NBC method", ier)
     return
  endif

  ! Unpack data from KIM object
  !
  pnAtoms = kim_api_get_data_f(pkim, "numberOfAtoms", ier);
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_get_data_f", ier)
     stop
  endif

  pnAtomTypes = kim_api_get_data_f(pkim, "numberAtomTypes", ier)
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_get_data_f", ier)
     stop
  endif

  patomTypesdum = kim_api_get_data_f(pkim, "atomTypes", ier)
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_get_data_f", ier)
     stop
  endif
  call toIntegerArrayWithDescriptor1d(atomTypesdum, atomTypes, N4)

  pcoor = kim_api_get_data_f(pkim, "coordinates", ier)
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_get_data_f", ier)
     stop
  endif
  call toRealArrayWithDescriptor2d(coordum, coords, DIM, N4)

  pcutoff = kim_api_get_data_f(pkim, "cutoff", ier)
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_get_data_f", ier)
     stop
  endif

  if (nbc.le.1) then
     pboxlength = kim_api_get_data_f(pkim, "boxlength", ier)
     if (ier.le.0) then
        call report_error(__LINE__, "kim_api_get_data_f", ier)
        stop
     endif
  endif

  penergy = kim_api_get_data_f(pkim, "energy", ier)
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_get_data_f", ier)
     stop
  endif

  pforces = kim_api_get_data_f(pkim, "forces", ier)
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_get_data_f", ier)
     stop
  endif
  call toRealArrayWithDescriptor2d(forcesdum, forces, DIM, N4)

  ! Set values
  numberOfAtoms   = N
  numberAtomTypes = ATypes
  atomTypes(:)    = kim_api_get_atypecode_f(pkim, "SPECIES_NAME_STR", ier)
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_get_atypecode_f", ier)
     stop
  endif

  ! set up the cluster atom positions
  call create_FCC_configuration(FCCspacing, nCellsPerSide, .false., coords, middleDum)
  if (nbc.le.1) boxlength(:)  = 600.d0 ! large enough to make the cluster isolated

  ! compute neighbor lists
  allocate(neighborList(N+1, N))
  if (nbc.eq.4) then
     allocate(RijList(3,N,N))
  endif
  !
  if (nbc.eq.0) then
     call MI_OPBC_neighborlist(.true., N, coords, (cutoff+0.75), boxlength, neighborList)
  elseif (nbc.eq.1) then
     call MI_OPBC_neighborlist(.false., N, coords, (cutoff+0.75), boxlength, neighborList)
  elseif (nbc.eq.2) then
     call NEIGH_PURE_cluster_neighborlist(.true., N, coords, (cutoff+0.75), neighborList)
  elseif (nbc.eq.3) then
     call NEIGH_PURE_cluster_neighborlist(.false., N, coords, (cutoff+0.75), neighborList)
  elseif (nbc.eq.4) then
     call NEIGH_RVEC_F_cluster_neighborlist(N, coords, (cutoff+0.75), N, neighborList, RijList)
  endif

  ! store pointers to neighbor list object and access function
  if (nbc.le.3) then
     ier = kim_api_set_data_f(pkim, "neighObject", SizeOne, loc(neighborList))
     if (ier.le.0) then
        call report_error(__LINE__, "kim_api_set_data_f", ier)
        stop
     endif
  else
     allocate(NLRvecLocs(3))
     NLRvecLocs(1) = loc(neighborList)
     NLRvecLocs(2) = loc(RijList)
     NLRvecLocs(3) = N+1
     ier = kim_api_set_data_f(pkim, "neighObject", SizeOne, loc(NLRvecLocs))
     if (ier.le.0) then
        call report_error(__LINE__, "kim_api_set_data_f", ier)
        stop
     endif
  endif

  if (nbc.eq.0) then
     ier = kim_api_set_data_f(pkim, "get_half_neigh", SizeOne, loc(get_neigh_no_Rij))
     if (ier.le.0) then
        call report_error(__LINE__, "kim_api_set_data_f", ier)
        stop
     endif
  elseif (nbc.eq.1) then
     ier = kim_api_set_data_f(pkim, "get_full_neigh", SizeOne, loc(get_neigh_no_Rij))
     if (ier.le.0) then
        call report_error(__LINE__, "kim_api_set_data_f", ier)
        stop
     endif
  elseif (nbc.eq.2) then
     ier = kim_api_set_data_f(pkim, "get_half_neigh", SizeOne, loc(get_neigh_no_Rij))
     if (ier.le.0) then
        call report_error(__LINE__, "kim_api_set_data_f", ier)
        stop
     endif
  elseif (nbc.eq.3) then
     ier = kim_api_set_data_f(pkim, "get_full_neigh", SizeOne, loc(get_neigh_no_Rij))
     if (ier.le.0) then
        call report_error(__LINE__, "kim_api_set_data_f", ier)
        stop
     endif
  elseif (nbc.eq.4) then
     ier = kim_api_set_data_f(pkim, "get_full_neigh", SizeOne, loc(get_neigh_Rij))
     if (ier.le.0) then
        call report_error(__LINE__, "kim_api_set_data_f", ier)
        stop
     endif
  endif

  ! Call model compute
  call kim_api_model_compute_f(pkim, ier)
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_model_compute", ier)
     stop
  endif

  ! print results to screen
  print *, "***********************************************************************************************"
  print *, "Results for KIM Model: ", modelname
  print *, "Using NBC: ", NBC_Method
  print *, "Forces:"
  print *, "  X                   Y                   Z"
  print 10, forces
10 format(f20.15, f20.15, f20.15)
  print *
  print *, "Energy = ", energy


  ! Don't forget to free and/or deallocate
  call free(pNBC_Method) 
  deallocate(neighborList)
  if (nbc.eq.4) then
     deallocate(NLRvecLocs)
     deallocate(RijList)
  endif

  call kim_api_model_destroy_f(pkim, ier)
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_model_destroy", ier)
     stop
  endif
  call kim_api_free(pkim, ier)
  if (ier.le.0) then
     call report_error(__LINE__, "kim_api_free", ier)
     stop
  endif

  stop
end program TEST_NAME_STR