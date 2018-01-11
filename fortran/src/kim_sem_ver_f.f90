!
! CDDL HEADER START
!
! The contents of this file are subject to the terms of the Common Development
! and Distribution License Version 1.0 (the "License").
!
! You can obtain a copy of the license at
! http://www.opensource.org/licenses/CDDL-1.0.  See the License for the
! specific language governing permissions and limitations under the License.
!
! When distributing Covered Code, include this CDDL HEADER in each file and
! include the License file in a prominent location with the name LICENSE.CDDL.
! If applicable, add the following below this CDDL HEADER, with the fields
! enclosed by brackets "[]" replaced with your own identifying information:
!
! Portions Copyright (c) [yyyy] [name of copyright owner]. All rights reserved.
!
! CDDL HEADER END
!

!
! Copyright (c) 2016--2018, Regents of the University of Minnesota.
! All rights reserved.
!
! Contributors:
!    Ryan S. Elliott
!

!
! Release: This file is part of the kim-api.git repository.
!


module kim_sem_ver_f_module
  implicit none
  private

  public &
    get_sem_ver, &
    is_less_than_func, &
    parse_sem_ver

  interface
    subroutine get_sem_ver(version) &
      bind(c, name="KIM_SEM_VER_GetSemVer")
      use, intrinsic :: iso_c_binding
      implicit none
      type(c_ptr), intent(out) :: version
    end subroutine get_sem_ver

    integer(c_int) function is_less_than_func(version_a, version_b, &
      is_less_than) bind(c, name="KIM_SEM_VER_IsLessThan")
      use, intrinsic :: iso_c_binding
      implicit none
      character(c_char), intent(in) :: version_a(*)
      character(c_char), intent(in) :: version_b(*)
      integer(c_int), intent(out) :: is_less_than
    end function is_less_than_func

    integer(c_int) function parse_sem_ver(version, major, minor, patch, &
      prerelease, build_metadata) bind(c, name="KIM_SEM_VER_ParseSemVer")
      use, intrinsic :: iso_c_binding
      implicit none
      character(c_char), intent(in) :: version(*)
      integer(c_int), intent(out) :: major
      integer(c_int), intent(out) :: minor
      integer(c_int), intent(out) :: patch
      type(c_ptr), intent(out) :: prerelease
      type(c_ptr), intent(out) :: build_metadata
    end function parse_sem_ver
  end interface
end module kim_sem_ver_f_module


! free functions to implement kim_sem_ver_module

subroutine kim_sem_ver_get_sem_ver(version)
  use, intrinsic :: iso_c_binding
  use kim_sem_ver_f_module, only : get_sem_ver
  implicit none
  character(len=*), intent(out) :: version

  type(c_ptr) :: p
  character(len=len(version)+1), pointer :: fp
  integer(c_int) :: null_index

  call get_sem_ver(p)
  call c_f_pointer(p, fp)
  null_index = scan(fp, char(0))-1
  version = fp(1:null_index)
end subroutine kim_sem_ver_get_sem_ver

subroutine kim_sem_ver_is_less_than(version_a, version_b, is_less_than, ierr)
  use, intrinsic :: iso_c_binding
  use kim_sem_ver_f_module, only : is_less_than_func
  implicit none
  character(len=*), intent(in) :: version_a
  character(len=*), intent(in) :: version_b
  integer(c_int), intent(out) :: is_less_than
  integer(c_int), intent(out) :: ierr

  ierr = is_less_than_func(trim(version_a)//c_null_char, &
    trim(version_b)//c_null_char, is_less_than)
end subroutine kim_sem_ver_is_less_than

subroutine kim_sem_ver_parse_sem_ver(version, major, minor, patch, &
  prerelease, build_metadata, ierr)
  use, intrinsic :: iso_c_binding
  use kim_sem_ver_f_module, only : parse_sem_ver
  implicit none
  character(len=*), intent(in) :: version
  integer(c_int), intent(out) :: major
  integer(c_int), intent(out) :: minor
  integer(c_int), intent(out) :: patch
  character(len=*), intent(out) :: prerelease
  character(len=*), intent(out) :: build_metadata
  integer(c_int), intent(out) :: ierr

  type(c_ptr) :: p_prerelease
  character(len=len(prerelease)+1), pointer :: fp_prerelease
  type(c_ptr) :: p_build
  character(len=len(build_metadata)+1), pointer :: fp_build
  integer(c_int) :: null_index

  ierr = parse_sem_ver(trim(version)//c_null_char, major, minor, patch, &
    p_prerelease, p_build)
  call c_f_pointer(p_prerelease, fp_prerelease)
  null_index = scan(fp_prerelease, char(0))-1
  prerelease = fp_prerelease(1:null_index)
  call c_f_pointer(p_build, fp_build)
  null_index = scan(fp_build, char(0))-1
  build_metadata = fp_build(1:null_index)
end subroutine kim_sem_ver_parse_sem_ver