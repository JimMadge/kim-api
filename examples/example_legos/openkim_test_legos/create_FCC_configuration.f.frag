!-------------------------------------------------------------------------------
!
! create_FCC_configuration subroutine
!
!  creates a cubic configuration of FCC particles with lattice spacing
!  `FCCspacing' and `nCellsPerSide' cells along each direction.
!
!  With periodic==.true. this will result in a total number of particles equal
!  to 4*(nCellsPerSide)**3 + 6*(nCellsPerSide)**2 + 3*(nCellsPerSide) + 1
!
!  With periodic==.false. this will result in a total number of particles equal
!  to 4*(nCellsPerSide)**3
!
!  Returns the Id of the particle situated in the middle of the configuration
!  (this particle will have the most neighbors.)
!
!-------------------------------------------------------------------------------
subroutine create_FCC_configuration(FCCspacing, nCellsPerSide, periodic, &
                                    coords, MiddlePartId)
  use, intrinsic :: iso_c_binding
  use KIM_API_F03
  implicit none
  integer(c_int), parameter :: cd = c_double ! used for literal constants

  !-- Transferred variables
  real(c_double), intent(in)  :: FCCspacing
  integer(c_int), intent(in)  :: nCellsPerSide
  logical,        intent(in)  :: periodic
  real(c_double), intent(out) :: coords(3,*)
  integer(c_int), intent(out) :: MiddlePartId
  !
  ! cluster setup variables
  !
  real(c_double) FCCshifts(3,4)
  real(c_double) latVec(3)
  integer(c_int) a, i, j, k, m

  ! Create a cubic FCC cluster
  !
  FCCshifts(1,1) = 0.0_cd
  FCCshifts(2,1) = 0.0_cd
  FCCshifts(3,1) = 0.0_cd
  FCCshifts(1,2) = 0.5_cd*FCCspacing
  FCCshifts(2,2) = 0.5_cd*FCCspacing
  FCCshifts(3,2) = 0.0_cd
  FCCshifts(1,3) = 0.5_cd*FCCspacing
  FCCshifts(2,3) = 0.0_cd
  FCCshifts(3,3) = 0.5_cd*FCCspacing
  FCCshifts(1,4) = 0.0_cd
  FCCshifts(2,4) = 0.5_cd*FCCspacing
 FCCshifts(3,4) = 0.5_cd*FCCspacing

  MiddlePartID = 1 ! Always put middle particle as #1
  a = 1            ! leave space for middle particle as particle #1
  do i=1,nCellsPerSide
     latVec(1) = (i-1)*FCCspacing
     do j=1,nCellsPerSide
        latVec(2) = (j-1)*FCCspacing
        do k=1,nCellsPerSide
           latVec(3) = (k-1)*FCCspacing
           do m=1,4
              a = a+1
              coords(:,a) = latVec + FCCshifts(:,m)
              if ((i.eq.nCellsPerside/2+1).and.(j.eq.nCellsPerSide/2+1) .and. &
                   (k.eq.nCellsPerSide/2+1) .and. (m.eq.1)) then
                 coords(:,1) = latVec + FCCshifts(:,m) ! put middle particle as #1
                 a = a - 1
              endif
           enddo
        enddo
        if (.not. periodic) then
            ! Add in the remaining three faces
            ! pos-x face
            latVec(1) = nCellsPerSide*FCCspacing
            latVec(2) = (i-1)*FCCspacing
            latVec(3) = (j-1)*FCCspacing
            a = a+1; coords(:,a) = latVec
            a = a+1; coords(:,a) = latVec + FCCshifts(:,4)
            ! pos-y face
            latVec(1) = (i-1)*FCCspacing
            latVec(2) = nCellsPerSide*FCCspacing
            latVec(3) = (j-1)*FCCspacing
            a = a+1; coords(:,a) = latVec
            a = a+1; coords(:,a) = latVec + FCCshifts(:,3)
            ! pos-z face
            latVec(1) = (i-1)*FCCspacing
            latVec(2) = (j-1)*FCCspacing
            latVec(3) = nCellsPerSide*FCCspacing
            a = a+1; coords(:,a) = latVec
            a = a+1; coords(:,a) = latVec + FCCshifts(:,2)
         endif
     enddo
     if (.not. periodic) then
         ! Add in the remaining three edges
         latVec(1) = (i-1)*FCCspacing
         latVec(2) = nCellsPerSide*FCCspacing
         latVec(3) = nCellsPerSide*FCCspacing
         a = a+1; coords(:,a) = latVec
         latVec(1) = nCellsPerSide*FCCspacing
         latVec(2) = (i-1)*FCCspacing
         latVec(3) = nCellsPerSide*FCCspacing
         a = a+1; coords(:,a) = latVec
         latVec(1) = nCellsPerSide*FCCspacing
         latVec(2) = nCellsPerSide*FCCspacing
         latVec(3) = (i-1)*FCCspacing
         a = a+1; coords(:,a) = latVec
      endif
  enddo
  if (.not. periodic) then
      ! Add in the remaining corner
      a = a+1; coords(:,a) = nCellsPerSide*FCCspacing
  endif

  return

end subroutine create_FCC_configuration
