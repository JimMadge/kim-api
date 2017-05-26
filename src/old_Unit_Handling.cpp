//
// CDDL HEADER START
//
// The contents of this file are subject to the terms of the Common Development
// and Distribution License Version 1.0 (the "License").
//
// You can obtain a copy of the license at
// http://www.opensource.org/licenses/CDDL-1.0.  See the License for the
// specific language governing permissions and limitations under the License.
//
// When distributing Covered Code, include this CDDL HEADER in each file and
// include the License file in a prominent location with the name LICENSE.CDDL.
// If applicable, add the following below this CDDL HEADER, with the fields
// enclosed by brackets "[]" replaced with your own identifying information:
//
// Portions Copyright (c) [yyyy] [name of copyright owner]. All rights reserved.
//
// CDDL HEADER END
//

//
// Copyright (c) 2013--2017, Regents of the University of Minnesota.
// All rights reserved.
//
// Contributors:
//    Valeriu Smirichinski
//    Ryan S. Elliott
//    Ellad B. Tadmor
//

//
// Release: This file is part of the kim-api.git repository.
//


#include <cstdlib>
#include <iostream>
#include <cstring>
#include <cmath>

#include "old_KIM_API.h"
#include "old_Unit_Handling.h"
#include "old_KIM_API_status.h"

namespace OLD_KIM
{

std::string const Unit_Handling::derived_list[]={
    "dipole", "density", "dynamic_viscosity", "electric_field",
    "force", "mass","pressure", "stress", "torque", "velocity"
};
int Unit_Handling::nderived_list=10;


std::string const Unit_Handling::base_list[]={
    "length",  "energy", "charge", "temperature", "time"
};
int Unit_Handling::nbase_list=5;

 // list of supported base units for Unit_length
std::string const Unit_Handling::length_list[]={
     "A" ,               "Bohr" ,   "cm" ,  "m" , "nm"
};
double Unit_Handling::length_scale[]={
  1.0e-10, 5.291772109217171e-11, 1.0e-2 ,  1.0 , 1.0e-9
};
int Unit_Handling::nlength_list=5;

// list of supported base units for Unit_energy
std::string const Unit_Handling::energy_list[]={
  "amu*A^2/(ps)^2",  "erg",   "eV" ,          "Hartree" ,  "J", "kcal/mol" , "kJ/mol"
};
double Unit_Handling::energy_scale[]={
    1.66053886e-23,  1.0e7, 1.60217646e-19,  4.3597439422e-18, 1.0,   6.9477e-21, 1.66054e-21
};
int Unit_Handling::nenergy_list=7;

// list of supported base units for Unit_charge
std::string const Unit_Handling::charge_list[]={
     "C" ,        "e",   "statC"
};
double Unit_Handling::charge_scale[]={
     1.0,    1.602e-19,     2.99792458e-9 //
};
int Unit_Handling::ncharge_list=3;

// list of supported base units for Unit_temperature
std::string const Unit_Handling::temperature_list[]={
     "K"
};
double Unit_Handling::temperature_scale[]={
     1.0
};
int Unit_Handling::ntemperature_list=1;

// list of supported base units for Unit_time
std::string const Unit_Handling::time_list[]={
   "fs" ,      "ps" ,  "ns",  "s"
};
double Unit_Handling::time_scale[]={
   1.0e-15,    1.0e-12,  1.0e-9,  1.0
};
int Unit_Handling::ntime_list=4;




Unit_Handling::Unit_Handling(){
    char const tmp[]="none";
    Unit_length = tmp;
    Unit_energy = tmp;
    Unit_charge = tmp;
    Unit_temperature = tmp;
    Unit_time = tmp;
    flexible_handling=false;
}

Unit_Handling:: ~Unit_Handling(){

}

void Unit_Handling::check_base_set_flexible(IOline* lines, int nlines, int* error){
    *error =KIM_STATUS_FAIL;

    for (int i=0;i<nlines;i++){
        if(lines[i].name == "Unit_Handling"){
            if(lines[i].value == "flexible") {
                flexible_handling=true;
            }else if(lines[i].value == "fixed") {
                flexible_handling=false;
            }else{
                *error = KIM_STATUS_WRONG_UNIT_HANDLING;
                return;
            }
        }else if(lines[i].name == "Unit_length"){
            Unit_length = lines[i].value;
        }else if(lines[i].name == "Unit_energy"){
            Unit_energy = lines[i].value;
        }else if(lines[i].name == "Unit_charge"){
            Unit_charge = lines[i].value;
        }else if(lines[i].name == "Unit_temperature"){
            Unit_temperature = lines[i].value;
        }else if(lines[i].name == "Unit_time"){
            Unit_time = lines[i].value;
        }
    }
    if(     ! is_Unit_length_supported()) {
        *error = KIM_STATUS_UNSUPPORTED_UNIT_LENGTH;
        return;
    }else if(! is_Unit_energy_supported()){
        *error = KIM_STATUS_UNSUPPORTED_UNIT_ENERGY;
        return;
    }else if(! is_Unit_charge_supported()){
        *error = KIM_STATUS_UNSUPPORTED_UNIT_CHARGE;
        return;
    }else if(! is_Unit_temperature_supported()){
        *error = KIM_STATUS_UNSUPPORTED_UNIT_TEMPERATURE;
        return;
    }else if(! is_Unit_time_supported()){
        *error = KIM_STATUS_UNSUPPORTED_UNIT_TIME;
        return;
    }
    *error =KIM_STATUS_OK;
}
bool Unit_Handling::is_Unit_length_supported(){
    bool result = false;
    for(int i=0;i<nlength_list;i++){
        if (Unit_length == length_list[i]) result = true;
    }
    return result;
}
bool Unit_Handling::is_Unit_energy_supported(){
    bool result = false;
    for(int i=0;i<nenergy_list;i++){
        if (Unit_energy == energy_list[i]) result = true;
    }
    return result;
}
bool Unit_Handling::is_Unit_charge_supported(){
    bool result = false;
    for(int i=0;i<ncharge_list;i++){
        if (Unit_charge == charge_list[i]) result = true;
    }
    return result;
}
bool Unit_Handling::is_Unit_temperature_supported(){
    bool result = false;
    for(int i=0;i<ntemperature_list;i++){
        if (Unit_temperature == temperature_list[i]) result = true;
    }
    return result;
}
bool Unit_Handling::is_Unit_time_supported(){
    bool result = false;
    for(int i=0;i<ntime_list;i++){
        if (Unit_time == time_list[i]) result = true;
    }
    return result;
}

bool Unit_Handling::is_it_Unit_length(std::string const & unit,int *index){
     bool result = false;
    for(int i=0;i<nlength_list;i++){
        if (unit == length_list[i]) {
            result = true;
            *index = i;
            break;
        }
    }
    return result;
}
bool Unit_Handling::is_it_Unit_energy(std::string const & unit,int *index){
    bool result = false;
    for(int i=0;i<nenergy_list;i++){
        if (unit == energy_list[i]) {
            result = true;
            *index = i;
            break;
        }
    }
    return result;
}
bool Unit_Handling::is_it_Unit_charge(std::string const & unit, int *index){
    bool result = false;
    for(int i=0;i<ncharge_list;i++){
        if (unit == charge_list[i]){
            result = true;
            *index = i;
            break;
        }
    }
    return result;
}
bool Unit_Handling::is_it_Unit_temperature(std::string const & unit, int *index){
    bool result = false;
    for(int i=0;i<ntemperature_list;i++){
        if (unit == temperature_list[i]){
            result = true;
            *index = i;
            break;
        }
    }
    return result;
}
bool Unit_Handling::is_it_Unit_time(std::string const & unit,int *index){
    bool result = false;
    for(int i=0;i<ntime_list;i++){
        if (unit == time_list[i]){
            result = true;
            *index = i;
            break;
        }
    }
    return result;
}

void Unit_Handling::init_str(char const* inputstr, int* error){
    *error = KIM_STATUS_FAIL;
    IOline * IOlines=NULL;
    bool readlines_str_success;
    int nlines = IOline::readlines_str((char*) inputstr,&IOlines,readlines_str_success);
    if (!readlines_str_success) return;
    this->check_base_set_flexible(IOlines,nlines,error);
    if (IOlines!= NULL) delete [] IOlines;
    return;
}
bool Unit_Handling::is_it_base(std::string const & unit){
    bool result = false;
    for(int i=0;i<nbase_list;i++){
        if (unit == base_list[i]) result = true;
    }
    return result;
}
bool Unit_Handling::is_it_derived(std::string const & unit){
    bool result = false;
    for(int i=0;i<nderived_list;i++){
        if (unit == derived_list[i]) result = true;
    }
    return result;
}
double Unit_Handling::get_scale_conversion(std::string const & u_from,std::string const &u_to,
                                           int *error){
    *error=KIM_STATUS_FAIL;
     int ind_from =-1, ind_to=-1;
     double convert;

    // compute multiplicative conversion factor for units (i.e., covariant transformation)
    if       (is_it_Unit_length(u_from,&ind_from) && is_it_Unit_length(u_to,&ind_to)){
       *error=KIM_STATUS_OK;
       convert = length_scale[ind_to]/length_scale[ind_from];
    }else if (is_it_Unit_energy(u_from,&ind_from) && is_it_Unit_energy(u_to,&ind_to)){
       *error=KIM_STATUS_OK;
       convert = energy_scale[ind_to]/energy_scale[ind_from];
    }else if (is_it_Unit_charge(u_from,&ind_from) && is_it_Unit_charge(u_to,&ind_to)){
       *error=KIM_STATUS_OK;
       convert = charge_scale[ind_to]/charge_scale[ind_from];
    }else if (is_it_Unit_temperature(u_from,&ind_from) && is_it_Unit_temperature(u_to,&ind_to)){
       *error=KIM_STATUS_OK;
       convert = temperature_scale[ind_to]/temperature_scale[ind_from];
    }else if (is_it_Unit_time(u_from,&ind_from) && is_it_Unit_time(u_to,&ind_to)){
       *error=KIM_STATUS_OK;
       convert = time_scale[ind_to]/time_scale[ind_from];
    }else{
        *error=KIM_STATUS_INCONSISTENT_BASE_UNIT;
        convert = -1.0;
    }

    // return multiplicative conversion factor for components (i.e., contravariant transforamtion)
    return 1.0/convert;
}
bool Unit_Handling::do_unit_match(Unit_Handling &test, Unit_Handling  &model){
    if(model.flexible_handling){
        model = test;
        model.flexible_handling=true;
        return true;
    }
    if(test.Unit_length == model.Unit_length)
        if(test.Unit_energy == model.Unit_energy)
            if(test.Unit_charge == model.Unit_charge)
                if(test.Unit_temperature == model.Unit_temperature)
                    if(test.Unit_time == model.Unit_time) return true;
    return false;
}
void Unit_Handling::print(){
    std::cout<<" Unit_length      : "<<Unit_length<<std::endl;
    std::cout<<" Unit_energy      : "<<Unit_energy<<std::endl;
    std::cout<<" Unit_charge      : "<<Unit_charge<<std::endl;
    std::cout<<" Unit_temperature : "<<Unit_temperature<<std::endl;
    std::cout<<" Unit_time        : "<<Unit_time<<std::endl;
    std::cout<<" flexible_handling: "<< flexible_handling<<std::endl;
}
void Unit_Handling::print(std::ostream& stream){
    stream<<" Unit_length      : "<<Unit_length<<std::endl;
    stream<<" Unit_energy      : "<<Unit_energy<<std::endl;
    stream<<" Unit_charge      : "<<Unit_charge<<std::endl;
    stream<<" Unit_temperature : "<<Unit_temperature<<std::endl;
    stream<<" Unit_time        : "<<Unit_time<<std::endl;
    stream<<" flexible_handling: "<< flexible_handling<<std::endl;
}
int Unit_Handling::get_unit_handling(int *error){
    if(flexible_handling) return 1;
    *error = KIM_STATUS_OK;
    return 0;
}
std::string const & Unit_Handling::get_unit_length(int *error){
    *error = KIM_STATUS_OK;
    return Unit_length;
}
std::string const & Unit_Handling::get_unit_energy(int *error){
    *error = KIM_STATUS_OK;
    return Unit_energy;
}
std::string const & Unit_Handling::get_unit_charge(int *error){
    *error = KIM_STATUS_OK;
    return Unit_charge;
}
std::string const & Unit_Handling::get_unit_temperature(int *error){
    *error = KIM_STATUS_OK;
    return Unit_temperature;
}
std::string const & Unit_Handling::get_unit_time(int *error){
    *error = KIM_STATUS_OK;
    return Unit_time;
}

double Unit_Handling::convert_to_act_unit(void *kim,
      std::string const & length, std::string const & energy, std::string const & charge,
      std::string const & temperature, std::string const & time,
      double length_exponent, double energy_exponent, double charge_exponent,
      double temperature_exponent, double time_exponent, int* kimerror){
      KIM_API_model *kimmdl = (KIM_API_model *)kim;
      std::string const & to_length = kimmdl->get_unit_length(kimerror);
      std::string const & to_energy =kimmdl->get_unit_energy(kimerror);
      std::string const & to_charge =kimmdl->get_unit_charge(kimerror);
      std::string const & to_temperature = kimmdl->get_unit_temperature(kimerror);
      std::string const & to_time = kimmdl->get_unit_time(kimerror);

      int error_length,error_energy,error_charge,error_temperature,error_time;
      double scale_length = kimmdl->get_scale_conversion(length.c_str(),to_length.c_str(),&error_length);
      double scale_energy = kimmdl->get_scale_conversion(energy.c_str(),to_energy.c_str(),&error_energy);
      double scale_charge = kimmdl->get_scale_conversion(charge.c_str(),to_charge.c_str(),&error_charge);
      double scale_temperature = kimmdl->get_scale_conversion(temperature.c_str(),to_temperature.c_str(),&error_temperature);
      double scale_time = kimmdl->get_scale_conversion(time.c_str(),to_time.c_str(),&error_time);

      if      (error_length < KIM_STATUS_OK){
              *kimerror = error_length;
              return -1.0;
      }else if(error_energy < KIM_STATUS_OK){
              *kimerror = error_energy;
              return -1.0;
      }else if(error_charge < KIM_STATUS_OK){
              *kimerror = error_charge;
              return -1.0;
      }else if(error_temperature < KIM_STATUS_OK){
              *kimerror = error_temperature;
              return -1.0;
      }else if(error_time < KIM_STATUS_OK){
              *kimerror = error_time;
              return -1.0;
      }

      return pow(scale_length,length_exponent)*pow(scale_energy,energy_exponent)*
             pow(scale_charge,charge_exponent)*pow(scale_temperature,temperature_exponent)*
             pow(scale_time, time_exponent);

}

} // namespace OLD_KIM

std::ostream &operator<<(std::ostream &stream, OLD_KIM::Unit_Handling &a){
    a.print(stream);
    return stream;
}
