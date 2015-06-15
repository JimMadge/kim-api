!*******************************************************************************
!**
!**  PROGRAM TEST_NAME_STR
!**
!**  KIM compliant program to compute the energy and forces for an
!**  isolated cluster configuration read from file.
!**
!**  Works with the following NBC methods:
!**        NEIGH_RVEC_H
!**        NEIGH_PURE_H
!**        NEIGH_RVEC_F
!**        NEIGH_PURE_F
!**        MI_OPBC_H
!**        MI_OPBC_F
!**        CLUSTER
!**
!*******************************************************************************

!-------------------------------------------------------------------------------
!
! Main program
!
!-------------------------------------------------------------------------------
program TEST_NAME_STR
  use, intrinsic :: iso_c_binding
  use KIM_API_F03
  use mod_neighborlist
  implicit none
  integer(c_int), parameter :: cd = c_double ! used for literal constants

  integer(c_int), parameter :: nCellsPerSide  = 2
  integer(c_int), parameter :: DIM            = 3
  real(c_double), parameter :: cutpad         = 0.75_cd
  integer(c_int), parameter :: max_species      = 200  ! max species allowed
  integer(c_int), parameter :: max_NBCs       = 20  ! maximum number of NBCs
  real(c_double), parameter :: eps_prec       = epsilon(1.0_cd)
  integer(c_int)            :: in
  integer(c_int)            :: N
  real(c_double)            :: max_force_component
  real(c_double)            :: scaled_eps_prec
  integer(c_int)            :: SizeOne = 1
  integer(c_int)            :: num_species_in_config
  integer(c_int)            :: num_NBCs
  logical                   :: found
  integer(c_int)            :: i,j
  real(c_double)            :: force_err(DIM),ave_force_error
  character(len=KIM_KEY_STRING_LENGTH) :: species_in_config(max_species)
  character(len=KIM_KEY_STRING_LENGTH) :: model_NBCs(max_NBCs)

  !
  ! neighbor list
  !
  type(neighObject_type), target :: neighObject
  real(c_double), allocatable    :: coordsave(:,:)
  logical do_update_list

  !
  ! KIM variables
  !
  character(len=KIM_KEY_STRING_LENGTH) :: testname     = "TEST_NAME_STR"
  character(len=KIM_KEY_STRING_LENGTH) :: modelname
  character(len=KIM_KEY_STRING_LENGTH) :: configfile
  ! configuration species (element symbols)
  character(len=KIM_KEY_STRING_LENGTH), pointer :: conf_species(:)
  real(c_double), pointer :: conf_coors(:,:)  ! configuration coordinates
  real(c_double), pointer :: conf_forces(:,:) ! configuration forces
  real(c_double)          :: conf_energy      ! configuration energy

  character(len=KIM_KEY_STRING_LENGTH) :: NBC_Method
  ! 0- NEIGH_RVEC_H, 1- NEIGH_PURE_H, 2- NEIGH_RVEC_F, 3- NEIGH_PURE_F,
  ! 4- MI_OPBC_H,    5- MI_OPBC_F,    6- CLUSTER
  integer(c_int) nbc
  type(c_ptr)             :: pkim
  integer(c_int)          :: ier, idum, inbc
  character(len=10000)    :: test_descriptor_string
  integer(c_int), pointer :: numberOfParticles;    type(c_ptr) pnParts
  integer(c_int), pointer :: numContrib;           type(c_ptr) pnumContrib
  integer(c_int), pointer :: numberOfSpecies;      type(c_ptr) pnOfSpecies
  integer(c_int), pointer :: particleSpecies(:);   type(c_ptr) pparticleSpecies
  real(c_double), pointer :: cutoff;               type(c_ptr) pcutoff
  real(c_double), pointer :: energy;               type(c_ptr) penergy
  real(c_double), pointer :: coords(:,:);          type(c_ptr) pcoor
  real(c_double), pointer :: forces(:,:);          type(c_ptr) pforces
  real(c_double), pointer :: boxSideLengths(:);    type(c_ptr) pboxSideLengths

  ! Initialize error flag
  ier = KIM_STATUS_OK

  ! Get KIM Model name to use
  print '("Please enter a valid KIM model name: ")'
  read(*,*) modelname

  ! Get filename of file containing configuration
  print '("Please enter the configuration file name: ")'
  read(*,*) configfile

  ! Read in configuration file
  in = 20
  open(unit=in,file=trim(configfile))
  read(in,*,err=10) N
  if (N<1) then
     ier = KIM_STATUS_FAIL
     idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
           "Error: Number of particles is less than 1", ier)
     stop
  endif
  ! dynamically allocate memory
  allocate(conf_species(N),conf_coors(DIM,N),conf_forces(DIM,N))
!@--- IN FUTURE WILL BE CHANGED TO READ IN PERIODIC BOX INFO
!@--- read(in,*,err=20) conf_boxsize(:)
  do i=1,N
     read(in,*,err=30) conf_species(i), conf_coors(:,i)
  enddo
  read(in,*,err=40) conf_energy
  max_force_component = 0.0_cd
  do i=1,N
     read(in,*,err=50) conf_forces(:,i)
     max_force_component = max(max_force_component, &
                               abs(conf_forces(1,i)), &
                               abs(conf_forces(2,i)), &
                               abs(conf_forces(3,i)))
  enddo
  scaled_eps_prec = max_force_component*eps_prec
  goto 100

  ! Error handing on input
  !

10  continue
    ier = KIM_STATUS_FAIL
    idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
           "Error reading in the number of particles N in the configuration", ier)
    stop

!20 continue
   ier = KIM_STATUS_FAIL
   idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
          "Error reading in the periodic box size in the configuration", ier)
   stop

30 continue
   ier = KIM_STATUS_FAIL
   idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
          "Error reading in a data line (species + coors) in configuration", &
          ier)
   stop

40 continue
   ier = KIM_STATUS_FAIL
   idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
          "Error reading in the configuration energy", ier)
   stop

50 continue
   ier = KIM_STATUS_FAIL
   idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
          "Error reading in forces in configuration", ier)
   stop



  ! No errors -- continue from here

100 continue

  ! Generate list of species appearing in config
  !
  num_species_in_config = 1
  species_in_config(1) = conf_species(1)
  do i=2,N
     found = .false.
     do j=1,num_species_in_config
        found = (trim(conf_species(i))==trim(species_in_config(j)))
        if (found) exit
     enddo
     if (.not.found) then
        num_species_in_config = num_species_in_config + 1
        species_in_config(num_species_in_config) = conf_species(i)
     endif
  enddo

  ! Get list of NBCs supported by the model
  !
  call Get_Model_NBC_methods(modelname, max_NBCs, model_NBCs, num_NBCs, ier)
  if (ier.lt.KIM_STATUS_OK) then
     idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                 "Get_Model_NBC_methods", ier)
     stop
  endif

  ! Print output header
  !
  print *
  print *,'VERIFICATION CHECK: COMPUTE ENERGY AND FORCES FOR CONFIGURATION'
  print *
  print '(120(''-''))'
  print '("This is Test          : ",A)', trim(testname)
  print '("Results for KIM Model : ",A)', trim(modelname)
  print *
  print *,'*** Read-in Configuration ***'
  print *
  print '(a,f20.10)', "Energy = ",conf_energy
  print *
  print '(A6,2X,A4,2X,A)',"Part","Spec","Coordinates"
  do i=1,N
     print '(I6,2X,A4,2X,3ES25.15)',i,conf_species(i), conf_coors(:,i)
  enddo
  print *
  print '(A6,2X,A4,2X,A)',"Part","Spec","Force"
  do i=1,N
     print '(I6,2X,A4,2X,3ES25.15)',i,conf_species(i),conf_forces(:,i)
  enddo
  print *

  ! Loop over all NBCs and compute energy and forces for each one
  !
  do inbc = 1, num_NBCs

     ! Write out KIM descriptor string for Test for current NBC
     !
     call Write_KIM_descriptor(model_NBCs(inbc), max_species, species_in_config, &
                               num_species_in_config, test_descriptor_string, ier)
     if (ier.lt.KIM_STATUS_OK) then
        idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                    "Write_KIM_descriptor", ier)
        stop
     endif

     ! Create empty KIM object conforming to fields in the KIM descriptor files
     ! of the Test and Model
     !
     ier = kim_api_string_init(pkim,trim(test_descriptor_string)//char(0), &
                               modelname)
     if (ier.lt.KIM_STATUS_OK) then
        idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                    "kim_api_string_init", ier)
        stop
     endif

     ! Double check that the NBC method being used is what we think it is
     !
     ier = kim_api_get_nbc_method(pkim, NBC_Method)
     if (ier.lt.KIM_STATUS_OK) then
        idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                    "kim_api_get_nbc_method", ier)
        stop
     endif
     if (index(NBC_Method,trim(model_NBCs(inbc))).ne.1) then
        ier = KIM_STATUS_FAIL
        idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
         "Internal Error: Selected NBC method different from requested value", &
         ier)
        stop
     endif

     ! Set NBC code based on selected NBC method
     !
     if (index(NBC_Method,"NEIGH_RVEC_H").eq.1) then
        nbc = 0
     elseif (index(NBC_Method,"NEIGH_PURE_H").eq.1) then
        nbc = 1
     elseif (index(NBC_Method,"NEIGH_RVEC_F").eq.1) then
        nbc = 2
     elseif (index(NBC_Method,"NEIGH_PURE_F").eq.1) then
        nbc = 3
     elseif (index(NBC_Method,"MI_OPBC_H").eq.1) then
        nbc = 4
     elseif (index(NBC_Method,"MI_OPBC_F").eq.1) then
        nbc = 5
     elseif (index(NBC_Method,"CLUSTER").eq.1) then
        nbc = 6
     else
        ier = KIM_STATUS_FAIL
        idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                    "Unknown NBC method", ier)
        stop
     endif

     ! Allocate memory via the KIM system
     !
     call kim_api_allocate(pkim, N, num_species_in_config, ier)
     if (ier.lt.KIM_STATUS_OK) then
        idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                    "kim_api_allocate", ier)
        stop
     endif

     ! Allocate storage for neighbor lists and
     ! store pointers to neighbor list object and access function
     !
     if (nbc.le.5) then
        allocate(neighObject%neighborList(N+1,N))
        if (nbc.eq.0 .or. nbc.eq.2) then
          allocate(neighObject%RijList(DIM,N+1,N))
        endif
        ier = kim_api_set_data(pkim, "neighObject", SizeOne, &
                               c_loc(neighObject))
        if (ier.lt.KIM_STATUS_OK) then
          idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                      "kim_api_set_data", ier)
          stop
        endif
     endif

     ! Set pointer in KIM object to neighbor list routine
     !
     if (nbc.ne.6) then
        ier = kim_api_set_method(pkim, "get_neigh", SizeOne, &
                                 c_funloc(get_neigh))
        if (ier.lt.KIM_STATUS_OK) then
           idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                       "kim_api_set_method", ier)
           stop
        endif
     endif

     ! Initialize Model
     !
     ier = kim_api_model_init(pkim)
     if (ier.lt.KIM_STATUS_OK) then
        idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                    "kim_api_model_init", ier)
        stop
     endif

     ! Unpack data from KIM object
     !
     call kim_api_getm_data(pkim, ier, &
          "numberOfParticles",           pnParts,           1,                               &
          "numberContributingParticles", pnumContrib,       TRUEFALSE(nbc.eq.0.or.nbc.eq.1.or.nbc.eq.4), &
          "numberOfSpecies",             pnOfSpecies,       1,                               &
          "particleSpecies",             pparticleSpecies,  1,                               &
          "coordinates",                 pcoor,             1,                               &
          "cutoff",                      pcutoff,           1,                               &
          "boxSideLengths",              pboxSideLengths,   TRUEFALSE(nbc.eq.4.or.nbc.eq.5), &
          "energy",                      penergy,           1,                               &
          "forces",                      pforces,           1)
     if (ier.lt.KIM_STATUS_OK) then
        idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                    "kim_api_getm_data", ier)
        stop
     endif
     call c_f_pointer(pnParts, numberOfParticles)
     if ((nbc.eq.0).or.(nbc.eq.1).or.(nbc.eq.4)) call c_f_pointer(pnumContrib, &
                                                                  numContrib)
     call c_f_pointer(pnOfSpecies,      numberOfSpecies)
     call c_f_pointer(pparticleSpecies, particleSpecies, [N])
     call c_f_pointer(pcoor, coords, [DIM,N])
     call c_f_pointer(pcutoff, cutoff)
     if ((nbc.eq.4).or.(nbc.eq.5)) call c_f_pointer(pboxSideLengths, &
                                                    boxSideLengths, [DIM])
     call c_f_pointer(penergy, energy)
     call c_f_pointer(pforces, forces, [DIM,N])

     ! Set values in KIM object
     !
     numberOfParticles   = N
     if (nbc.eq.0.or.nbc.eq.1.or.nbc.eq.4) numContrib = N
     numberOfSpecies = num_species_in_config
     do i=1,N
        particleSpecies(i) = kim_api_get_species_code(pkim,&
                                                      trim(conf_species(i)),ier)
     enddo
     if (ier.lt.KIM_STATUS_OK) then
        idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                    "kim_api_get_species_code", ier)
        stop
     endif
     do i=1,N
        coords(:,i) = conf_coors(:,i)
     enddo
     ! set boxSideLengths large enough to make the cluster isolated
     if (nbc.eq.4.or.nbc.eq.5) boxSideLengths(:) = 600.0_cd

     ! Compute neighbor lists
     !
     if (nbc.le.5) then
        do_update_list = .true.
        allocate(coordsave(DIM,N))
        call update_neighborlist(DIM,N,coords,cutoff,cutpad,boxSideLengths, &
                                 NBC_Method,do_update_list,coordsave, &
                                 neighObject,ier)
        if (ier.lt.KIM_STATUS_OK) then
           idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                       "update_neighborlist", ier)
           stop
        endif
     endif

     ! Call model compute to get energy and forces
     !
     ier = kim_api_model_compute(pkim)
     if (ier.lt.KIM_STATUS_OK) then
        idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                    "kim_api_model_compute", ier)
        stop
     endif

     ! print results to screen
     !
     print '(41(''=''))'
     print '("NBC Method = ",A28)', trim(NBC_Method)
     print '(41(''=''))'
     print *
     print '(a,f20.10)', "Energy = ",energy
     print *
     print '(A6,2X,A4,2X,A)',"Part","Spec","Computed Force"
     do i=1,N
        print '(I6,2X,A4,2X,3ES25.15)',i,conf_species(i),forces(:,i)
     enddo
     print *
     print *,'*** Energy and Forces Agreement ***'
     print *
     print '(A6,2X,A4,2X,A)',"Part","Spec","Force Error"
     ave_force_error = 0.0_cd
     do i=1,N
        do j=1,DIM
           force_err(j) = abs(forces(j,i)-conf_forces(j,i))/ &
                          max(abs(conf_forces(j,i)),scaled_eps_prec)
        enddo
        print '(I6,2X,A4,2X,3ES25.15)',i,conf_species(i),force_err(:)
        ave_force_error = ave_force_error + dot_product(force_err,force_err)
     enddo
     ave_force_error = sqrt(ave_force_error)/dble(DIM*N)
     print *
     print '(a,ES25.15)', "Average force error = ",ave_force_error
     print *
     print '(a,ES25.15)', "Energy error        = ",abs(energy-conf_energy)/ &
                                                   max(abs(conf_energy), &
                                                   eps_prec)
     print *

     ! Free temporary storage
     !
     if (nbc.le.5) then ! deallocate neighbor list storage
        deallocate(neighObject%neighborList)
        deallocate(coordsave)
        if (nbc.eq.0.or.nbc.eq.2) then
           deallocate(neighObject%RijList)
        endif
     endif
     ier = kim_api_model_destroy(pkim)
     if (ier.lt.KIM_STATUS_OK) then
        idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                    "kim_api_model_destroy", ier)
        stop
     endif
     call kim_api_free(pkim, ier)
     if (ier.lt.KIM_STATUS_OK) then
        idum = kim_api_report_error(__LINE__, THIS_FILE_NAME, &
                                    "kim_api_free", ier)
        stop
     endif

  enddo ! loop over NBC methods

  ! Print output footer
  !
  print *
  print '(120(''-''))'

  ! Free configuration storage
  !
  deallocate(conf_species,conf_coors,conf_forces)
  stop

end program TEST_NAME_STR
