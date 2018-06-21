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


module kim_compute_callback_name_f_module
  implicit none
  private

  public &
    from_string, &
    get_string, &
    get_number_of_compute_callback_names, &
    get_compute_callback_name

  interface
    type(kim_compute_callback_name_type) function from_string(string) &
      bind(c, name="KIM_ComputeCallbackNameFromString")
      use, intrinsic :: iso_c_binding
      use kim_compute_callback_name_module, only : &
        kim_compute_callback_name_type
      implicit none
      character(c_char), intent(in) :: string(*)
    end function from_string

    type(c_ptr) function get_string(compute_callback_name) &
      bind(c, name="KIM_ComputeCallbackNameString")
      use, intrinsic :: iso_c_binding
      use kim_compute_callback_name_module, only : &
        kim_compute_callback_name_type
      implicit none
      type(kim_compute_callback_name_type), intent(in), value :: &
        compute_callback_name
    end function get_string

    subroutine get_number_of_compute_callback_names( &
      number_of_compute_callback_names) &
      bind(c, name="KIM_COMPUTE_CALLBACK_NAME_GetNumberOfComputeCallbackNames")
      use, intrinsic :: iso_c_binding
      integer(c_int), intent(out) :: number_of_compute_callback_names
    end subroutine get_number_of_compute_callback_names

    integer(c_int) function get_compute_callback_name(index, &
      compute_callback_name) &
      bind(c, name="KIM_COMPUTE_CALLBACK_NAME_GetComputeCallbackName")
      use, intrinsic :: iso_c_binding
      use kim_compute_callback_name_module, only : &
        kim_compute_callback_name_type
      implicit none
      integer(c_int), intent(in), value :: index
      type(kim_compute_callback_name_type), intent(out) :: compute_callback_name
    end function get_compute_callback_name
  end interface
end module kim_compute_callback_name_f_module

! free functions to implement kim_compute_callback_name_module

subroutine kim_compute_callback_name_from_string(string, compute_callback_name)
  use, intrinsic :: iso_c_binding
  use kim_compute_callback_name_module, only : kim_compute_callback_name_type
  use kim_compute_callback_name_f_module, only : from_string
  implicit none
  character(len=*), intent(in) :: string
  type(kim_compute_callback_name_type), intent(out) :: compute_callback_name

  compute_callback_name = from_string(trim(string)//c_null_char)
end subroutine kim_compute_callback_name_from_string

logical function kim_compute_callback_name_equal(left, right)
  use, intrinsic :: iso_c_binding
  use kim_compute_callback_name_module, only : kim_compute_callback_name_type
  implicit none
  type(kim_compute_callback_name_type), intent(in) :: left
  type(kim_compute_callback_name_type), intent(in) :: right

  kim_compute_callback_name_equal &
    = (left%compute_callback_name_id .eq. right%compute_callback_name_id)
end function kim_compute_callback_name_equal

logical function kim_compute_callback_name_not_equal(left, right)
  use, intrinsic :: iso_c_binding
  use kim_compute_callback_name_module, only : kim_compute_callback_name_type
  use kim_compute_callback_name_module, only : operator(.eq.)
  implicit none
  type(kim_compute_callback_name_type), intent(in) :: left
  type(kim_compute_callback_name_type), intent(in) :: right

  kim_compute_callback_name_not_equal = .not. (left .eq. right)
end function kim_compute_callback_name_not_equal

subroutine kim_compute_callback_name_string(compute_callback_name, string)
  use, intrinsic :: iso_c_binding
  use kim_compute_callback_name_module, only : kim_compute_callback_name_type
  use kim_compute_callback_name_f_module, only : get_string
  implicit none
  type(kim_compute_callback_name_type), intent(in), value :: &
    compute_callback_name
  character(len=*), intent(out) :: string

  type(c_ptr) :: p
  character(len=len(string)+1, kind=c_char), pointer :: fp
  integer(c_int) :: null_index

  p = get_string(compute_callback_name)
  if (c_associated(p)) then
    call c_f_pointer(p, fp)
    null_index = scan(fp, char(0))-1
    string = fp(1:null_index)
  else
    nullify(fp)
    string = ""
  end if
end subroutine kim_compute_callback_name_string

subroutine kim_compute_callback_name_get_number_of_compute_callback_names( &
  number_of_compute_callback_names)
  use, intrinsic :: iso_c_binding
  use kim_compute_callback_name_f_module, only : &
    get_number_of_compute_callback_names
  implicit none
  integer(c_int), intent(out) :: number_of_compute_callback_names

  call get_number_of_compute_callback_names(number_of_compute_callback_names)
end subroutine kim_compute_callback_name_get_number_of_compute_callback_names

subroutine kim_compute_callback_name_get_compute_callback_name(index, &
  compute_callback_name, ierr)
  use, intrinsic :: iso_c_binding
  use kim_compute_callback_name_module, only : kim_compute_callback_name_type
  use kim_compute_callback_name_f_module, only : get_compute_callback_name
  implicit none
  integer(c_int), intent(in), value :: index
  type(kim_compute_callback_name_type), intent(out) :: compute_callback_name
  integer(c_int), intent(out) :: ierr

  ierr = get_compute_callback_name(index-1, compute_callback_name)
end subroutine kim_compute_callback_name_get_compute_callback_name
