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
// Copyright (c) 2013--2014, Regents of the University of Minnesota.
// All rights reserved.
//
// Contributors:
//    Valeriu Smirichinski
//    Ryan S. Elliott
//    Ellad B. Tadmor
//    Tobias Brink
//

//
// Release: This file is part of the kim-api.git repository.
//


#include <cstdlib>
#include <cctype>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>


#include "KIM_API.h"
#include "KIM_API_status.h"
#include "KIM_API_DIRS.h"

#if KIM_LINK_VALUE == KIM_LINK_DYNAMIC_LOAD
#include <unistd.h>
#include <dlfcn.h>
#endif

#define KIM_LINE_LENGTH 512
#define CPP_MAX_STRING_LITERAL_LENGTH 65536

static void strip_char_string(char* nm)
{
   //strip spaces and tabs from back
   for(int i=(int)strlen(nm); i>0; i--){
      if((nm[i-1]!=' ') && (nm[i-1]!='\t')){nm[i]='\0'; break;}
   }
   //strip spaces and tabs from front
   int c=0,key=0;
   for(int i=0;i<=(int)strlen(nm);i++){
      if((nm[i]!=' ') && (nm[i]!='\t')){key=1;}
      if(key==1){nm[c]=nm[i]; c++;}
   }
}

static bool read_file_to_stringstream(const char* infile, std::stringstream& ss)
{
   std::ifstream myfile;
   myfile.open(infile);
   if(!myfile){
      std::cout<<"* Error (read_file_to_stringstream): can not open file :"<<infile<<":"<<std::endl;
      return false;
   }

   ss << myfile.rdbuf();
   myfile.close();

   return true;
}

//#define intptr_t int  // for 32 bit machines
KIMBaseElementFlag:: KIMBaseElementFlag(){
    peratom=0;
    freeable=1;
    calculate=0;
 }

 KIMBaseElementUnit:: KIMBaseElementUnit(){
    init();
}
void KIMBaseElementUnit::init(){
    strncpy(dim,"none",KIM_KEY_STRING_LENGTH);
}
int Atom_Map::comparator(const void* a1, const void* a2){
    Atom_Map *am1 =(Atom_Map *)a1;
    Atom_Map *am2 =(Atom_Map *)a2;
    return strcmp(am1->symbol,am2->symbol);
}

KIM_IOline::KIM_IOline(){
    goodformat=false;init2empty();
}

bool KIM_IOline:: getFields(char *inString){
            char *tmp;

            init2empty();
            //check for comments part and get it removed
            tmp = strpbrk(inString,"#");
            if(tmp !=NULL) {
                strncpy(comments,tmp,strlen(tmp)+1);
                tmp[0]='\0';

            }
             strip(inString);

            if(inString[0] == '\0') return false;
            //check if it is input or output section
            strip(inString);

            if(isitinput(inString)){
                input = true;
                output = false;

                return false;
            }else if(isitoutput(inString)){

                input = false;
                output = true;
                return false;
            }

            //parse field
            tmp = strtok(inString," \t");if(tmp == NULL) return false;
            strncpy(name,tmp,strlen(tmp)+1);


            tmp = strtok(NULL," \t");if(tmp == NULL) return false;
            strncpy(type,tmp,strlen(tmp)+1);

            if(strcmp(type,"flag")==0) {
                strncpy(dim,"none",5);
                strncpy(shape,"[]",3);
                return true;
            }

            if(strcmp(type,"spec")==0) {
                strncpy(dim,"none",5);
                tmp = strtok(NULL," \t");if(tmp == NULL) return false;
                strcat(shape,"[");
                strcat(shape,tmp);
                strcat(shape,"]");
                return true;
            }

            tmp = strtok(NULL," \t");if(tmp == NULL) return false;
            strncpy(dim,tmp,strlen(tmp)+1);

            tmp = strtok(NULL," \t");if(tmp == NULL) return false;
            strncpy(shape,tmp,strlen(tmp)+1);

            tmp = strtok(NULL," \t");if(tmp == NULL) return true;
            strncpy(requirements,tmp,strlen(tmp)+1);

            return true;
}

int KIM_IOline::get_rank(){
            char *tmp;
            char* shapetmp = new char[strlen(shape)+1];
            strncpy(shapetmp,shape,strlen(shape)+1);
            if(shapetmp[0]=='[' && shapetmp[strlen(shape)-1]==']'){
                tmp = strtok(shapetmp,"[,]");
                if (tmp==NULL) {
                  delete [] shapetmp;
                  return 0;
                }
                int c=0;
                while (tmp!=NULL){
                    tmp = strtok(NULL,"[,]");
                    c++;
                }
                delete [] shapetmp;
                return c;
            }
            std::cout<<"* Error (KIM_IOline::get_rank): bad shape format"<<std::endl;
            delete [] shapetmp;
            return 0;
 }
int *  KIM_IOline::get_shape(){

            char* shapetmp = new char[strlen(shape)+1];
            strncpy(shapetmp,shape,strlen(shape)+1);
            int rnk = get_rank();
            if (rnk < 1) {
              delete [] shapetmp;
              return NULL;
            }
            int *shp = new int[rnk];
            int i=0;
            char *tmp =strtok(shapetmp,"[,]");
            while(tmp!=NULL){
                double dd = strtod(tmp,&tmp);
                shp[i]=(int)dd;
                tmp = strtok(NULL,"[,]");
                i++;
            }
            delete [] shapetmp;
            return shp;
 }
int * KIM_IOline::get_shape(int nparts, int nspecies){
            char* shapetmp = new char[strlen(shape)+1];
            char tmpstring[128];
            strncpy(shapetmp,shape,strlen(shape)+1);
            int rnk = get_rank();
            if (rnk < 1) return NULL;
            int *shp = new int[rnk];
            int i=0;
            char *tmp =strtok(shapetmp,"[,]");
            while(tmp!=NULL){
                double dd = strtod(tmp,&tmp);
                shp[i]=(int)dd;
                if(shp[i]==0){
                    strcpy(tmpstring,tmp);
                    strip(tmpstring);
                    if(strcmp(tmpstring,"numberSpecies")==0) shp[i]=nspecies;
                    if(strcmp(tmpstring,"numberOfParticles")==0) shp[i]=(int)nparts;
                }
                tmp = strtok(NULL,"[,]");
                i++;
            }
            delete [] shapetmp;
            return shp;
}

bool KIM_IOline::isitsizedefined(){

             int rnk =this->get_rank();
             if (rnk < 1) return false;
             int * shp = this->get_shape();
             if (shp==NULL) return false;

             int c=1;
             for (int i=0; i<rnk;i++) c=c*shp[i];

             delete [] shp;

             if (c > 0) return true;
             return false;
}
bool KIM_IOline:: isitperatom(){
             char* shapetmp = new char[strlen(shape)+1];
             char tmpstring[128];
             strncpy(shapetmp,shape,strlen(shape)+1);
             char *tmp =strtok(shapetmp,"[,]");
             if(tmp==NULL)
             {
                delete [] shapetmp;
                return false;
             }
             while(tmp!=NULL){
                strcpy(tmpstring,tmp);
                strip(tmpstring);
                if(strcmp(tmpstring,"numberOfParticles")==0)
                {
                   delete [] shapetmp;
                   return true;
                }
                tmp = strtok(NULL,"[,]");
             }
             delete [] shapetmp;
             return false;
}
bool KIM_IOline::isitoptional(){
            if(strcmp(this->requirements,"optional")==0) return true;
            return false;
}

void KIM_IOline:: strip(char * strv){strip_char_string(strv);}
void KIM_IOline::strip(){
            strip(name);
            strip(type);
            strip(dim);
            strip(shape);
            strip(requirements);
 }
void KIM_IOline:: init2empty(){
            name[0]='\0';
            type[0]='\0';
            dim[0]='\0';
            shape[0]='\0';
            requirements[0]='\0';
            comments[0]='\0';
}
bool KIM_IOline:: isitinput(const char*str){
            char tocmp [] ="MODEL_INPUT:";
            if(strlen(str)<strlen(tocmp)) return false;
            if(strncmp(str,tocmp,strlen(tocmp))==0) return true;
            return false;
}
bool KIM_IOline:: isitoutput(const char*str){
            char tocmp [] ="MODEL_OUTPUT:";
            if(strlen(str)<strlen(tocmp)) return false;
            if(strncmp(str,tocmp,strlen(tocmp))==0) return true;
            return false;
}

std::ostream &operator<<(std::ostream &stream, KIM_IOline a){
        stream<<a.name<<" "<<a.type<<" "<<a.dim<<" ";
        stream<<a.shape<<" "<<a.requirements;
        stream << std::endl;
        return stream;
}
std::istream &operator>>(std::istream &stream, KIM_IOline &a){
        char inputline[KIM_LINE_LENGTH];
        stream.getline(inputline,KIM_LINE_LENGTH-1);
        if(stream.fail() && !stream.eof()){
           std::cout << "* Error (operator>> KIM_IOline): Input line in .kim file longer than KIM_LINE_LENGTH (default 512) characters.\n"
                << "         The line is: `"
                << inputline << "'";
           a.goodformat=false;
        }else{
           if(a.getFields(inputline)){
              a.goodformat=true;
           }else{
              a.goodformat=false;
           }
        }
        return stream;
}

void IOline:: strip(){
            strip(name);
            strip(value);
            strip(comment);
}
void IOline::strip(char *nm){strip_char_string(nm);}

IOline::IOline(){
    goodformat=false;
    for(int i=0; i<121;i++) comment[i]='\0';
    for(int i=0; i<101;i++) name[i]='\0';
    for(int i=0; i<101;i++) value[i]='\0';
}
bool IOline:: getFields(char *inputString){
                int i;
                for(i=0; i<=(int)strlen(inputString); i++){
                    if(*(inputString+i)=='#'){return false;};
                    if(*(inputString+i)==':' && *(inputString+i+1)=='='){name[i]='\0';i+=2;break;};
                        name[i]=*(inputString+i);
                }
                if(i>=(int)strlen(inputString)){return false;};
                int j=0;

                for(;i<=(int)strlen(inputString); i++){
                        if(*(inputString+i)=='#'){value[j]='\0';i+=2;break;};
                        value[j]=*(inputString+i);
                        j++;
                        value[j]='\0';
                }

                j=0;
                if(i>=(int)strlen(inputString)){comment[0]='\0';strip();return true;};
                for(;i<=(int)strlen(inputString); i++){
                        comment[j]=*(inputString + i);
                        comment[j+1]='\0';
                        j++;
                }

                strip();

                return true;
}

int IOline::readlines_str(const char* instrn, IOline** inlines, bool& success){
   success = true;
    int counter=0;
        IOline inlne;
        *inlines=NULL;
        std::string in_instrn=instrn;
        std::stringstream myfile (in_instrn,std::stringstream::in|std::stringstream::out) ;
        std::stringstream myfile1 (in_instrn,std::stringstream::in|std::stringstream::out) ;
         if(!myfile){
            std::cout<<"* Error (IOline::readlines_str): can not parse instrn."<< std::endl
                     <<"        Offending line is: `" << in_instrn << "'" << std::endl;
             success = false;
             return -1;
         }
         myfile.seekp(std::stringstream::beg);
          while(!myfile.eof()){
                myfile>> inlne;
                if(inlne.goodformat) counter++;
        }


        if (counter < 1) return counter;


        (*inlines) =  new IOline [counter] ;


        myfile1.seekp(std::stringstream::beg);

        counter=0;
        while(!myfile1.eof()){
                myfile1 >> inlne;

                if(inlne.goodformat) {

                    (*inlines)[counter]=inlne;

                    counter++;
                }
        }
        return counter;
}

std::ostream &operator<<(std::ostream &stream, IOline a){
        stream<<a.name<<" := "<<a.value<<" # "<<a.comment;
        return stream;
}
std::istream &operator>>(std::istream &stream, IOline &a){
        char inputline[KIM_LINE_LENGTH];
        stream.getline(inputline,KIM_LINE_LENGTH-1);
        if(a.getFields(inputline)){
                a.goodformat=true;
        }else{
                a.goodformat=false;
        }
        return stream;
}

KIMBaseElement:: KIMBaseElement(){
            nullify();
}
KIMBaseElement::~KIMBaseElement(){

}
bool KIMBaseElement:: init(const char *nm,const char * tp,intptr_t sz, intptr_t rnk, int *shp,void * pdata){
            flag = new KIMBaseElementFlag;

            unit = new KIMBaseElementUnit;
            name = new char[strlen(nm)+1];

            type = new char[KIM_KEY_STRING_LENGTH];
            strcpy(name,nm);
            strncpy(type,tp,KIM_KEY_STRING_LENGTH);


            if(rnk < 0) {
               std::cout << "* Error (KIMBaseElement::init): KIMBaseElement_init:rnk < 0"<<std::endl;
                return false;
            }
            size = sz;
            rank = rnk;
            if(rank==1){
                shape = new int[rank];

                if(shp!=NULL){
                   shape[0]=shp[0];
                }else{
                    shape[0]=sz;
                }
            }
            if(rank > 1){
                shape = new int[rank];
                if (shp == NULL) {
                  std::cout << "* Error (KIMBaseElement::init): KIMBaseElement_init:shp==NULL"<<std::endl;
                  return false;
                }
                for (int i=0;i<rank;i++) shape[i]=shp[i];
            }
            if (rank !=0) ptrptr = pdata;
            data.p = (void *)  (*(( intptr_t **)pdata));


            flag->freeable = 1;
            if(sz==0) flag->freeable = 0;
            flag->calculate = 1;

            return true;
}
bool KIMBaseElement::init(const char *nm,const char * tp,intptr_t sz, intptr_t rnk, int *shp){
            bool getelemsize_success;
            int szelement=getelemsize(tp, getelemsize_success);
            if (! getelemsize_success) return false;
            char *data = NULL;

            if(sz>0) data=new char[szelement*sz];

            if (!init(nm, tp,sz, rnk, shp,&data)) return false;

            if (sz>0) flag->freeable=0;

            ptrptr=NULL;
            return true;
}
void KIMBaseElement::free(){
             if(flag!=NULL) if (flag->freeable == 0){
                delete [] (char *)data.p;
            }
           if(name!= NULL) delete [] name;
           if(type!=NULL) delete [] type;
           if(shape!=NULL) delete [] shape;
           if(flag!=NULL) delete  flag;
           if(unit!=NULL) delete unit;
           //if(ptrptr!=NULL) delete [] ptrptr;
           //if(reserved!=NULL) delete [] reserved;
           nullify();

}
void KIMBaseElement::nullify(){
            data.p=NULL;
            ptrptr = NULL;
            shape = NULL;
            reserved=NULL;
            size=0;
            rank=0;
            name=NULL;
            type =NULL;
            flag = NULL;
            unit = NULL;

}
bool KIMBaseElement::equiv(KIM_IOline& kimioline, bool skip_specials){
   //switch off check for virial and process_dnEdr related things
   if(skip_specials){
      if(strcmp(kimioline.name,"virial") == 0  ||
         strcmp(kimioline.name,"particleVirial")==0 ||
         strcmp(kimioline.name,"hessian")    ==0 ||
         strcmp(kimioline.name,"process_dEdr")    ==0 ||
         strcmp(kimioline.name,"process_d2Edr2")    ==0  ) return true;
   }
   if(strcmp(kimioline.name,name)==0)
      if(strcmp(kimioline.type,type)==0){
         if(strcmp(kimioline.type,"flag")==0) return true;
         if(strcmp(kimioline.dim,unit->dim)==0)
            if(kimioline.get_rank()==(int)(rank)) {
               int *shp = kimioline.get_shape();
               for(int i=0; i< (int)(rank);i++){
                  if(shp[i]!=shape[i]){
                     delete [] shp;
                     return false;
                  }
               }
               delete [] shp;
               return true;
            }
      }
   return false;
}

int KIMBaseElement::getelemsize(const char *tp, bool& success){
            success = true;
            char realkey[KIM_KEY_STRING_LENGTH]="real";      //key defenitions
            char doublekey[KIM_KEY_STRING_LENGTH]="double";
            char integerkey[KIM_KEY_STRING_LENGTH]="integer";
            char ptrkey[KIM_KEY_STRING_LENGTH]="pointer";
            char methodkey[KIM_KEY_STRING_LENGTH]="method";
            char flagkey[KIM_KEY_STRING_LENGTH]="flag";// add here to expand...

            if(strcmp(realkey,tp)==0){
                return(sizeof(float));
            }else if (strcmp(integerkey,tp)==0){
                return(sizeof(int));
            }else if (strcmp(ptrkey,tp)==0){
                return(sizeof(int *));
            }else if (strcmp(doublekey,tp)==0){
                return (sizeof(double));
            }else if (strcmp(methodkey,tp)==0){
                return (sizeof(int *));
             }else if (strcmp(flagkey,tp)==0){
                return 0;
            }else{// add here more in else if block...
               std::cout << "* Error (KIMBaseElement::getelemsize): Unknown Type in KIM descriptor file line." << std::endl
                    << "         `" << tp <<"' is not one of: " << realkey << ", "
                    << doublekey << ", " << integerkey << ", " << ptrkey << ", "
                    << flagkey << std::endl;
               success = false;
               return -1;
            }
}

std::ostream &operator<<(std::ostream &stream, KIMBaseElement a){
    if (a.data.p==NULL && a.name == NULL && a.type==NULL) {
        stream <<" KIMBaseElement is nullified "<<std::endl;
        return stream;
    }
    stream<<"name  : "<<a.name<< std::endl
          <<" type          : "<<a.type << std::endl
          <<" rank          : "<<a.rank<<std::endl;
    if (a.rank>0 && a.shape!=NULL){
       stream<<" shape         : [";
       for(int i=0;i<a.rank-1;i++) stream<< a.shape[i] << ",";
       stream << a.shape[a.rank-1] << "]" << std::endl;
    }
    stream<<" size          : "<<a.size<<std::endl;
    stream<<" flag calculate: "<<a.flag->calculate<<"   // 0 -- do not calculate, 1 -- calculate"<<std::endl;
    stream<<" flag freeable : "<<a.flag->freeable<< "   // 0 -- freeable,         1 -- not freeable"<<std::endl;
    stream<<" flag peratom  : "<<a.flag->peratom<<  "   // 0 -- peratom,          1 -- per something else"<<std::endl;
    stream<<" dimension     : "<<a.unit->dim<<std::endl;
    // printin gata itself


    stream<<" data: ";


    if(a.data.p == NULL) {
        stream <<"NULL"<<std::endl;
        return stream;
    }else if(strcmp(a.type,"double")==0){

        for(int i=0;i<a.size;i++) stream<< ((double*)(a.data.p))[i]<<" ";

    }else if(strcmp(a.type,"real")==0){
        for(int i=0;i<a.size;i++) stream<< ((float*)(a.data.p))[i]<<" ";
    }else if(strcmp(a.type,"integer")==0){
        for(int i=0;i<a.size;i++) stream<< ((int*)(a.data.p))[i]<<" ";
    }else{
        stream<<"address:"<<(intptr_t)a.data.p;
    }
    stream<<std::endl;

    return stream;
}

KIM_API_model:: KIM_API_model(){
       inlines=NULL;
       //method_A init
       strcpy(NBC_method_A,"CLUSTER");
       strcpy(&arg_NBC_method_A[0][0],"coordinates");
       narg_NBC_method_A=1;

       //method_B init
       strcpy(NBC_method_B,"MI_OPBC_H");
       strcpy(&arg_NBC_method_B[0][0],"coordinates");
       strcpy(&arg_NBC_method_B[1][0],"boxSideLengths");
       strcpy(&arg_NBC_method_B[2][0],"numberContributingParticles");
       strcpy(&arg_NBC_method_B[3][0],"neighObject");
       strcpy(&arg_NBC_method_B[4][0],"get_neigh");
       narg_NBC_method_B=5;

       //method_C init
       strcpy(NBC_method_C,"MI_OPBC_F");
       strcpy(&arg_NBC_method_C[0][0],"coordinates");
       strcpy(&arg_NBC_method_C[1][0],"boxSideLengths");
       strcpy(&arg_NBC_method_C[2][0],"neighObject");
       strcpy(&arg_NBC_method_C[3][0],"get_neigh");
       narg_NBC_method_C=4;

       //method_D init
       strcpy(NBC_method_D,"NEIGH_RVEC_F");
       strcpy(&arg_NBC_method_D[0][0],"coordinates");
       strcpy(&arg_NBC_method_D[1][0],"neighObject");
       strcpy(&arg_NBC_method_D[2][0],"get_neigh");
       narg_NBC_method_D=3;

       //method_E init
       strcpy(NBC_method_E,"NEIGH_RVEC_H");
       strcpy(&arg_NBC_method_E[0][0],"coordinates");
       strcpy(&arg_NBC_method_E[1][0],"numberContributingParticles");
       strcpy(&arg_NBC_method_E[2][0],"neighObject");
       strcpy(&arg_NBC_method_E[3][0],"get_neigh");
       narg_NBC_method_E=4;

       //method_F init
       strcpy(NBC_method_F,"NEIGH_PURE_F");
       strcpy(&arg_NBC_method_F[0][0],"coordinates");
       strcpy(&arg_NBC_method_F[1][0],"neighObject");
       strcpy(&arg_NBC_method_F[2][0],"get_neigh");
       narg_NBC_method_F=3;

       //method_G init
       strcpy(NBC_method_G,"NEIGH_PURE_H");
       strcpy(&arg_NBC_method_G[0][0],"coordinates");
       strcpy(&arg_NBC_method_G[1][0],"numberContributingParticles");
       strcpy(&arg_NBC_method_G[2][0],"neighObject");
       strcpy(&arg_NBC_method_G[3][0],"get_neigh");
       narg_NBC_method_G=4;

       n_NBC_methods = number_NBC_methods;
       nnarg_NBC = new int[n_NBC_methods];
       NBC_methods = new char* [n_NBC_methods];
       arg_NBC_methods = new char**[n_NBC_methods];
       nnarg_NBC[0] =  narg_NBC_method_A;
       nnarg_NBC[1] =  narg_NBC_method_B;
       nnarg_NBC[2] =  narg_NBC_method_C;
       nnarg_NBC[3] =  narg_NBC_method_D;
       nnarg_NBC[4] =  narg_NBC_method_E;
       nnarg_NBC[5] =  narg_NBC_method_F;
       nnarg_NBC[6] =  narg_NBC_method_G;

       for(int i=0;i<n_NBC_methods;i++){
           arg_NBC_methods[i] = new char * [nnarg_NBC[i]];
       }

       NBC_methods[0] = &NBC_method_A[0];
       NBC_methods[1] = &NBC_method_B[0];
       NBC_methods[2] = &NBC_method_C[0];
       NBC_methods[3] = &NBC_method_D[0];
       NBC_methods[4] = &NBC_method_E[0];
       NBC_methods[5] = &NBC_method_F[0];
       NBC_methods[6] = &NBC_method_G[0];

       for (int i=0; i<nnarg_NBC[0];i++) arg_NBC_methods[0][i] = & arg_NBC_method_A[i][0];
       for (int i=0; i<nnarg_NBC[1];i++) arg_NBC_methods[1][i] = & arg_NBC_method_B[i][0];
       for (int i=0; i<nnarg_NBC[2];i++) arg_NBC_methods[2][i] = & arg_NBC_method_C[i][0];
       for (int i=0; i<nnarg_NBC[3];i++) arg_NBC_methods[3][i] = & arg_NBC_method_D[i][0];
       for (int i=0; i<nnarg_NBC[4];i++) arg_NBC_methods[4][i] = & arg_NBC_method_E[i][0];
       for (int i=0; i<nnarg_NBC[5];i++) arg_NBC_methods[5][i] = & arg_NBC_method_F[i][0];
       for (int i=0; i<nnarg_NBC[6];i++) arg_NBC_methods[6][i] = & arg_NBC_method_G[i][0];

       strcpy(NBC_method_current,"none");


       model_index_shift=0;
       neiOfAnAtom = NULL;
       neiOfAnAtomSize = 0;
       AUX_index_shift =0;
       ErrorCode=1;
       AtomsTypes = NULL;
       nAtomsTypes = 0;
       locator_neigh_mode=false;
       iterator_neigh_mode=false;
       both_neigh_mode=false;
       model_buffer=NULL;

       virial_ind=-1;
       particleVirial_ind=-1;
       hessian_ind=-1;
       process_dEdr_ind=-1;
       process_d2Edr2_ind=-1;

       test_doing_process_dEdr = false;
       test_doing_process_d2Edr2 = false;
       virial_need2add=false;
       particleVirial_need2add=false;
       hessian_need2add=false;

       model_lib_handle = NULL;
}
KIM_API_model:: ~KIM_API_model(){
      // free();

      for(int i=0;i<n_NBC_methods;i++){
         delete [] arg_NBC_methods[i];
      }

      delete [] arg_NBC_methods;
      delete [] NBC_methods;
      delete []  nnarg_NBC;

      if (neiOfAnAtomSize > 0) {
         delete [] neiOfAnAtom;
      }
}
int KIM_API_model:: preinit(const char * initfile,const char *modelname){
   std::stringstream buffer;
   if (!read_file_to_stringstream(initfile, buffer)) return KIM_STATUS_FAIL;

   return prestring_init((char*) buffer.str().c_str());
 }

int KIM_API_model::prestring_init(const char *instrn){
        if (!read_file_str(instrn,&inlines,&numlines))
        {
          ErrorCode = KIM_STATUS_FAIL;
          return ErrorCode;
        }

        int *shape=NULL;
        char pointer_str [] = "pointer";
        //get Atoms Types and nAtomsTypes
        if (! init_AtomsTypes()) {
            ErrorCode=KIM_STATUS_FAIL;
            return ErrorCode;
        }
        model.init(name_temp,pointer_str,(intptr_t)(numlines-nAtomsTypes+3),1,shape);
        model.size =(intptr_t)(numlines-nAtomsTypes);

        int ii=0;
        for (int i=0; i< numlines;i++){

            //check for spec type
            if (!(strcmp(inlines[i].type,"spec")==0)) {

                KIMBaseElement *el = new KIMBaseElement ;

                int rank=inlines[i].get_rank();
                shape =inlines[i].get_shape();
                char * name =& (inlines[i].name[0]);
                char * type =& (inlines[i].type[0]);

                if (el->init(name,type,0,rank,shape)) //preinit element with zero size
                {
                   //here to add checking is it derived or is it base units
                   strncpy(el->unit->dim,inlines[i].dim,strlen(inlines[i].dim)+1);

                   el->flag->calculate = 1;
                   el->flag->peratom = 1;//per something else
                   if(inlines[i].isitperatom()) el->flag->peratom = 0; //per atom
                }
                else
                {
                   ErrorCode = KIM_STATUS_FAIL;
                }
                KIMBaseElement **pel =(KIMBaseElement**) model.data.p;
                pel[ii] =  el;
                ii++;
                delete [] shape;
            }
        }
        if (ErrorCode == KIM_STATUS_FAIL) return ErrorCode;

        //resize inlines (remove spec type variables)
        KIM_IOline * inlinesnew = new KIM_IOline[numlines - nAtomsTypes+3];
        ii=0;
        for (int i=0; i< numlines;i++){
            //check for spec type
            if (!(strcmp(inlines[i].type,"spec")==0)) {
                inlinesnew[ii]=inlines[i];
                ii++;
            }
        }
        delete [] inlines;
        inlines = inlinesnew;
        inlinesnew = NULL;
        numlines=numlines - nAtomsTypes;
        //end resize inlines


        //extra input like unitFixed flag and,later may be, authors
        IOline *extrainput;
        bool readlines_str_success;
        IOline::readlines_str(instrn,&extrainput, readlines_str_success);
        if (!readlines_str_success)
        {
          ErrorCode = KIM_STATUS_FAIL;
          return ErrorCode;
        }
        // do nothing for now
        delete [] extrainput;
        unit_h.init_str(instrn,&ErrorCode);
        if(ErrorCode < KIM_STATUS_OK) return ErrorCode;

        // check for version number reported by Model/Simulator
        if(!does_it_have_a_version_number(instrn) && 0!=strcmp("standard", name_temp))
        {
          std::cout << "* Error (KIM_API_model::prestring_init()): "<<name_temp << " '.kim' file contains invalid KIM_API_Version setting." << std::endl;
          ErrorCode = KIM_STATUS_FAIL;
          return ErrorCode;
        }

        ErrorCode = KIM_STATUS_OK;
        return ErrorCode;
}

bool KIM_API_model::does_it_have_a_version_number(
    const char* const instrn)
{
  IOline * IOlines=NULL;
  bool readlines_str_success;
  int nlines = IOline::readlines_str((char*) instrn,&IOlines,readlines_str_success);
  if (!readlines_str_success) return false;

  for (int i=0; i<nlines;++i)
  {
    if (!strcmp(IOlines[i].name, "KIM_API_Version"))
    {
      char temp_str[KIM_KEY_STRING_LENGTH];
      strncpy(temp_str, IOlines[i].value, KIM_KEY_STRING_LENGTH - 1);

      // Get Major version
      char* tmp = strtok(temp_str, ".");
      if (tmp == NULL)
      {
        delete [] IOlines;
        return false;
      }
      char* end;
      tempVersionMajor = strtol(tmp, &end, 10);
      if ('\0' != *end)
      {
        delete [] IOlines;
        return false;
      }

      // Get Minor version
      tmp = strtok(NULL, ".");
      if (tmp == NULL)
      {
        delete [] IOlines;
        return false;
      }
      tempVersionMinor = strtol(tmp, &end, 10);
      if ('\0' != *end)
      {
        delete [] IOlines;
        return false;
      }

      // Check that remaining string is a valid PATCH value
      tmp = strtok(NULL, ".");
      if (NULL != tmp)
      {
        strtol(tmp, &end, 10);
        if ('\0' != *end)
        {
          delete [] IOlines;
          return false;
        }
      }

      delete [] IOlines;
      return true;
    }
  }

  if (IOlines != NULL) delete [] IOlines;
  return false;
}

int KIM_API_model::get_version(const char** const version)
{
  *version = KIM_API_VERSION;
  return KIM_STATUS_OK;
}

int KIM_API_model::get_version_major(int* const major)
{
  *major = KIM_API_VERSION_MAJOR;
  return KIM_STATUS_OK;
}

int KIM_API_model::get_version_minor(int* const minor)
{
  *minor = KIM_API_VERSION_MINOR;
  return KIM_STATUS_OK;
}

int KIM_API_model::get_version_patch(int* const patch)
{
  *patch = KIM_API_VERSION_PATCH;
  return KIM_STATUS_OK;
}

int KIM_API_model::get_version_prerelease(const char** const prerelease)
{
  *prerelease = KIM_API_VERSION_PRERELEASE;
  return KIM_STATUS_OK;
}

int KIM_API_model::get_version_build_metadata(const char** const build_metadata)
{
  *build_metadata = KIM_API_VERSION_BUILD_METADATA;
  return KIM_STATUS_OK;
}

int KIM_API_model::version_newer(const char* const versionA,
                                 const char* const versionB,
                                 int* const result)
{
  unsigned len = strlen(versionA);
  if (len < strlen(versionB)) len = strlen(versionB);
  len = len + 1;

  int A_Major;
  int A_Minor;
  int A_Patch;
  char* const A_Prerelease = new char[len];
  char* const A_Build = new char[len];

  int B_Major;
  int B_Minor;
  int B_Patch;
  char* const B_Prerelease = new char[len];
  char* const B_Build = new char[len];

  std::stringstream A_PR;
  std::string A_tok;
  std::stringstream B_PR;
  std::string B_tok;

  int retval = KIM_STATUS_FAIL;
  *result = -1;

  if (!parse_semver(versionA, &A_Major, &A_Minor, &A_Patch, A_Prerelease,
                    A_Build))
  {
    retval = KIM_STATUS_FAIL;
    goto exit;
  }

  if (!parse_semver(versionB, &B_Major, &B_Minor, &B_Patch, B_Prerelease,
                    B_Build))
  {
    retval = KIM_STATUS_FAIL;
    goto exit;
  }

  retval = KIM_STATUS_OK;

  if (A_Major > B_Major)
  {
    *result = 1;
    goto exit;
  }
  else if (A_Major < B_Major)
  {
    *result = 0;
    goto exit;
  }
  // Major values are equal

  if (A_Minor > B_Minor)
  {
    *result = 1;
    goto exit;
  }
  else if (A_Minor < B_Minor)
  {
    *result = 0;
    goto exit;
  }
  // Minor values are equal

  if (A_Patch > B_Patch)
  {
    *result = 1;
    goto exit;
  }
  else if (A_Patch < B_Patch)
  {
    *result = 0;
    goto exit;
  }
  // Patch values are equal

  if ('\0' == A_Prerelease[0] && '\0' != B_Prerelease[0])
  {
    *result = 1;
    goto exit;
  }
  else if ('\0' != A_Prerelease[0] && '\0' == B_Prerelease[0])
  {
    *result = 0;
    goto exit;
  }
  else if ('\0' == A_Prerelease[0] && '\0' == B_Prerelease[0])
  {
    *result = 0; // A and B are equal
    goto exit;
  }
  // Both are prereleases of the same version

  A_PR << A_Prerelease;
  B_PR << B_Prerelease;

  std::getline(A_PR, A_tok, '.');
  std::getline(B_PR, B_tok, '.');

  while (!A_PR.eof() && !B_PR.eof())
  {
    int Aint;
    char* A_End;
    Aint = strtol(A_tok.c_str(), &A_End, 10);
    int Bint;
    char* B_End;
    Bint = strtol(B_tok.c_str(), &B_End, 10);
    if ('\0' != *A_End && '\0' == *B_End) // A numeric & B alpha
    {
      *result = 1;
      goto exit;
    }
    else if ('\0' == *A_End && '\0' != *B_End) // A alpha & B numeric
    {
      *result = 0;
      goto exit;
    }
    else if ('\0' != *A_End && '\0' != *B_End) // Both alpha
    {
      if (A_tok > B_tok)
      {
        *result = 1;
        goto exit;
      }
      else if (A_tok < B_tok)
      {
        *result = 0;
        goto exit;
      }
      // equal strings
    }
    else // Both numeric
    {
      if (Aint > Bint)
      {
        *result = 1;
        goto exit;
      }
      else if (Aint < Bint)
      {
        *result = 0;
        goto exit;
      }
      // equal numbers
    }
    // equal identifiers

    std::getline(A_PR, A_tok, '.');
    std::getline(B_PR, B_tok, '.');
  }

  if (A_PR.eof() && !B_PR.eof())
  {
    *result = 1;
    goto exit;
  }
  else if (!A_PR.eof() &&  B_PR.eof())
  {
    *result = 0;
    goto exit;
  }
  else
  {
    // versions are equal (thus, A is not newer than B)
    *result = 0;
    goto exit;
  }

 exit:
  delete [] A_Prerelease;
  delete [] A_Build;
  delete [] B_Prerelease;
  delete [] B_Build;

  return retval;
}

bool KIM_API_model::parse_semver(const char* const version, int* const major,
                                 int* const minor, int* const patch,
                                 char* const prerelease,
                                 char* const build_metadata)
{
  unsigned len = strlen(version) + 1;
  char* const ver = new char[len];
  strcpy(ver, version);

  bool hasPrerelease = false;
  bool hasBuild = false;
  for (unsigned i=0; i<len; ++i)
  {
    if ('-' == ver[i])
    {
      hasPrerelease = true;

      for (; i<len; ++i)
      {
        if ('+' == ver[i])
        {
          hasBuild = true;
          break;
        }
      }
      break;
    }
  }

  char* tok;
  tok = strtok(ver, ".");

  char* end;
  int val;

  val = strtol(tok, &end, 10);
  if ('\0' != *end)
  {
    delete [] ver;
    return false;
  }
  *major = val;

  tok = strtok(NULL, ".");
  if (NULL == tok)
  {
    delete [] ver;
    return false;
  }
  val = strtol(tok, &end, 10);
  if ('\0' != *end)
  {
    delete [] ver;
    return false;
  }
  *minor = val;

  tok = strtok(NULL, "-");
  if (NULL == tok)
  {
    delete [] ver;
    return false;
  }
  val = strtol(tok, &end, 10);
  if ('\0' != *end)
  {
    delete [] ver;
    return false;
  }
  *patch = val;

  if (hasPrerelease)
  {
    tok = strtok(NULL, "+");
    if (NULL == tok) // empty prerelease
    {
      delete [] ver;
      return false;
    }
    // validate prerelease
    for (unsigned i=0; i < strlen(tok); ++i)
    {
      if (!isalnum(tok[i]) && !('-' == tok[i]) && !('.' == tok[i]))
      {
        delete [] ver;
        return false;
      }
    }
    strcpy(prerelease, tok);

    if (hasBuild)
    {
      tok = strtok(NULL, "+");
      if (NULL == tok) // empty build
      {
        delete [] ver;
        return false;
      }
      // validate build
      for (unsigned i=0; i < strlen(tok); ++i)
      {
        if (!isalnum(tok[i]) && !('-' == tok[i]) && !('.' == tok[i]))
        {
          delete [] ver;
          return false;
        }
      }
      strcpy(build_metadata, tok);
    }
  }

  delete [] ver;
  return true;
}

int KIM_API_model::get_version_model_major(int* const major) const
{
  *major = modelVersionMajor;
  return KIM_STATUS_OK;
}

int KIM_API_model::get_version_model_minor(int* const minor) const
{
  *minor = modelVersionMinor;
  return KIM_STATUS_OK;
}

int KIM_API_model::get_version_simulator_major(int* const major) const
{
  *major = simulatorVersionMajor;
  return KIM_STATUS_OK;
}

int KIM_API_model::get_version_simulator_minor(int* const minor) const
{
  *minor = simulatorVersionMinor;
  return KIM_STATUS_OK;
}

void KIM_API_model::free(int *error){
   free();
   *error = KIM_STATUS_OK; // no failures detectable
}

 void KIM_API_model::free(){
         KIMBaseElement **pel =  (KIMBaseElement **)  model.data.p;
        if(model.data.p != NULL)  for (int i =0;i<model.size;i++) {
            pel[i]->free();
            delete pel[i];
            pel[i]=NULL;
        }
        model.free();

        if(inlines != NULL) {
            delete [] inlines;
            inlines=NULL;
        }


        numlines=0;

        if(AtomsTypes !=NULL) {
            delete [] AtomsTypes;
            AtomsTypes=NULL;
            nAtomsTypes=0;
        }
 }

int KIM_API_model::set_data(const char * nm, intptr_t size, void *dt){
        // set data into preinit element correctly calculates all
        int error;
        int ind=get_index((char*) nm, &error);
        if (ind<0) {
            return error;
        } //no data in KIM_API_model
        return set_data_by_index(ind, size, dt);
}
int KIM_API_model::set_method(const char * nm, intptr_t size, func_ptr dt){
        // set data into preinit element correctly calculates all
        int error;
        int ind=get_index((char*) nm, &error);
        if (ind<0) {
            return error;
        } //no data in KIM_API_model
        return set_method_by_index(ind, size, dt);
}
int KIM_API_model::set_data_by_index(int ind, intptr_t size, void* dt){
    if (ind<0) {
            return KIM_STATUS_FAIL;
        } //no data in KIM_API_model
        int c=1;
       if((*this)[ind].flag->freeable == 0) {
           if((*this)[ind].data.p!=NULL) delete [] (char *)((*this)[ind].data.p);
       }
       (*this)[ind].data.p = dt;

        (*this)[ind].size = size;

        if ((*this)[ind].rank > 1) {
            for (int i=1;i<(*this)[ind].rank;i++) {
                c=c * (*this)[ind].shape[i];
            }
            if(c!=0) (*this)[ind].shape[0] = size/c;
        }
        if ((*this)[ind].rank==1){
            (*this)[ind].shape[0] = size;
        }
        (*this)[ind].flag->freeable = 1;
        return KIM_STATUS_OK;
}
int KIM_API_model::set_method_by_index(int ind, intptr_t size, func_ptr dt){
    if (ind<0) {
            return KIM_STATUS_FAIL;
        } //no data in KIM_API_model
        int c=1;
       if((*this)[ind].flag->freeable == 0) {
           if((*this)[ind].data.p!=NULL) delete [] (char *)((*this)[ind].data.p);
       }
       (*this)[ind].data.fp = dt;

        (*this)[ind].size = size;

        if ((*this)[ind].rank > 1) {
            for (int i=1;i<(*this)[ind].rank;i++) {
                c=c * (*this)[ind].shape[i];
            }
            if(c!=0) (*this)[ind].shape[0] = size/c;
        }
        if ((*this)[ind].rank==1){
            (*this)[ind].shape[0] = size;
        }
        (*this)[ind].flag->freeable = 1;
        return KIM_STATUS_OK;
}

void * KIM_API_model::get_data(const char *nm,int *error){
   int i=get_index(nm, error);
   return get_data_by_index(i, error);
}

func_ptr KIM_API_model::get_method(const char *nm,int *error){
   int i=get_index(nm, error);
   return get_method_by_index(i, error);
}

void * KIM_API_model::get_data_by_index(int ind, int* error){
        *error = KIM_STATUS_FAIL;
        if (ind<0) return NULL;
        *error =KIM_STATUS_OK;
        return (*this)[ind].data.p;
}

func_ptr KIM_API_model::get_method_by_index(int ind, int* error){
        *error = KIM_STATUS_FAIL;
        if (ind<0) return NULL;
        *error =KIM_STATUS_OK;
        return (*this)[ind].data.fp;
}

int KIM_API_model::get_index(const char *nm,int *error){
        for(int i=0; i< model.size;i++){
            if(strcmp((*this)[i].name,nm)==0) {
                *error =KIM_STATUS_OK;
                return i;
            }
        }
        *error = KIM_STATUS_FAIL;
        return -1;
}

intptr_t KIM_API_model::get_size(const char *nm,int *error){
        int ind=get_index(nm,error);
        return get_size_by_index(ind, error);
}
intptr_t KIM_API_model::get_rank(const char *nm,int *error){
        int ind=get_index(nm,error);
        return get_rank_by_index(ind, error);
}
intptr_t KIM_API_model::get_shape(const char *nm,int * shape, int *error){
        int ind=get_index(nm,error);
        return get_shape_by_index(ind, shape, error);
}

void KIM_API_model::set_shape(const char * nm, int* shape, int rank, int* error){
    //size will be calculated and set too
        int ind=get_index((char*) nm,error);
        *error =KIM_STATUS_ARG_UNKNOWN;
        if (ind < 0) return;
        if((intptr_t)(rank) != (*this)[ind].rank) {
            *error= KIM_STATUS_ARG_INVALID_RANK; //rank do not match
            return;
        }

        if((*this)[ind].rank == 0){
            (*this)[ind].size=1;
            *error = KIM_STATUS_OK; //success
            return;
        }else if((*this)[ind].rank ==1){
            (*this)[ind].shape[0]=shape[0];
            (*this)[ind].size=(intptr_t)shape[0];
            if (shape[0] < 0) {
                *error=KIM_STATUS_ARG_INVALID_SHAPE; //negative index
            }else{
                *error = KIM_STATUS_OK; //success
            }
            return;
        }else if((*this)[ind].rank>1){
            int sz=1;
            for (int i=0;i<rank;i++) {
                if (shape[i]<0){
                    *error = KIM_STATUS_ARG_INVALID_SHAPE; //negative index
                    return;
                }
                sz=sz*shape[i];
            }
            (*this)[ind].size=(intptr_t)sz;
            for (int i=0; i< rank; i++) (*this)[ind].shape[i]=shape[i];
            *error=KIM_STATUS_OK;//success
            return;
        }else{
            *error=KIM_STATUS_ARG_UNKNOWN;
            return;
        }
}
void KIM_API_model::set_compute(const char *nm, int flag, int *error){
   *error = KIM_STATUS_FAIL;
   int ind = get_index(nm, error);
   set_compute_by_index(ind, flag, error);
   return;
}
void KIM_API_model::set_compute_by_index(int ind, int flag, int *error){
   *error = KIM_STATUS_FAIL;
   if ((flag == 1) || (flag == 0)){
      (*this)[ind].flag->calculate = flag;
      *error = KIM_STATUS_OK;
   }
   else
      *error = KIM_STATUS_FAIL;

   return;
}
int KIM_API_model::get_compute(const char *nm, int* error){
   int ind = get_index(nm, error);
   if (*error != KIM_STATUS_OK) return KIM_STATUS_ARG_UNKNOWN;
   return get_compute_by_index(ind, error);
}
KIMBaseElement & KIM_API_model::operator[](int i){
        if ((i > (*this).model.size) || (i < 0)){
          std::cout<<"* Error (KIM_API_model::operator[](int i): invalid index: " << i <<std::endl;
           KIM_API_model::fatal_error_print();
           exit(326);
        }
        KIMBaseElement **pel =(KIMBaseElement**) model.data.p;
        return *pel[i];
}
KIMBaseElement & KIM_API_model::operator[](const char *nm){
        int error;
        int ind=get_index(nm,&error);
        if (error == KIM_STATUS_FAIL){
          std::cout<<"* Error (KIM_API_model::operator[](char *nm): name not found: " << nm <<std::endl;
           KIM_API_model::fatal_error_print();
           exit(325);
        }
        KIMBaseElement **pel =(KIMBaseElement**) model.data.p;
        return *pel[ind];
}

bool KIM_API_model::read_file_str(const char* strstream, KIM_IOline** lns, int* numlns){
        int counter=0;
        KIM_IOline inln;

        //open string as stream from char *
        std::string in_strstream=strstream ;
        std::stringstream myfile (in_strstream, std::stringstream::in|std::stringstream::out);
        std::stringstream myfile1 (in_strstream, std::stringstream::in|std::stringstream::out);
        if(!myfile){
            std::cout<<"* Error (KIM_API_model::read_file_str): can not access KIM descriptor file as input string."<<std::endl;
            return false;
        }


        myfile.seekp(std::stringstream::beg);//set to the begining
        while(!myfile.eof()){
                myfile >> inln;
                if(inln.goodformat) counter++;
        }

        myfile1.seekp(std::stringstream::beg);//set to the begining

        *numlns = counter;
        *lns = new KIM_IOline[counter+3];

        //myfile.open(initfile);
        counter=0;


        while(!myfile1.eof()){
                myfile1 >> inln;

                if(inln.goodformat) {
                    (*lns)[counter]=inln;
                    counter++;
                }
        }

        return true;
}

bool KIM_API_model::is_it_match(KIM_API_model & mdtst,KIM_IOline * IOlines,int nlns, bool ignore_optional, bool match_regular){
    bool match;
    //check if lines are match with Model api variable
    match =true;
    for (int i=0; i<nlns;i++){
        match=false;

        if(!ignore_optional && IOlines[i].isitoptional()){
            match=true;
        }

        if(strcmp(IOlines[i].type,"spec")==0){
            match=true;
        }
        if ( is_it_par(IOlines[i].name) ) match=true;

        for(int j=0;j<mdtst.model.size;j++){
            if(mdtst[j].equiv(IOlines[i],match_regular)) {
                match = true;
                break;
            }
        }
        if(!match) {
           std::cout << "* Info (KIM_API_model::is_it_match): The following descriptor file line may not match with " << mdtst.model.name << "'s descriptor file."<<std::endl;
            std::cout<<IOlines[i]<<std::endl;
            return match;
        }
    }
    return match;
}//will be private

bool KIM_API_model::is_it_match_noFlagCount(KIM_API_model & mdtst,KIM_IOline * IOlines,int nlns, bool ignore_optional){
    bool match;
    //check if lines are match with Model api variable
    match =true;
    for (int i=0; i<nlns;i++){
        match=false;

        if(!ignore_optional && IOlines[i].isitoptional()){
            match=true;
        }

        if(strcmp(IOlines[i].type,"spec")==0){
            match=true;
        }

        if ( is_it_par(IOlines[i].name) ) match=true;

        if(strcmp(IOlines[i].type,"flag")==0){
            match=true;
        }
        for(int j=0;j<mdtst.model.size;j++){
            if(mdtst[j].equiv(IOlines[i],true)) {
                match = true;
            }else if(strcmp(IOlines[i].type,"flag")==0){
                match = true;
            }else if(is_it_par(IOlines[i].name)){
                match = true;
            }

            if (!match){
               for (int k=0; k<5;++k){
                  for (int m=0; m<mdtst.nnarg_NBC[k]; ++m){
                     if (!strcmp(mdtst.arg_NBC_methods[k][m],IOlines[i].name)){
                        match=true;
                        break;
                     }
                  }
                  if (match) break;
               }
            }
        }
        if(!match) {
            std::cout << "* Warning (KIM_API_model::is_it_match_noFlagCount): The following line in the Model descriptor file does not match."<<std::endl;
            std::cout<<IOlines[i]<<std::endl;
            return match;
        }
    }
    return match;
}//will be private

bool KIM_API_model::is_it_match(KIM_API_model &test,KIM_API_model & mdl){
    //preinit model from standard template kim file
   KIM_API_model stdmdl;

   extern const int STANDARD_KIM_STR_CHUNKS_NAME;
   extern const char* const STANDARD_KIM_STR_NAME[];
   char* const standard_kim = new char[STANDARD_KIM_STR_CHUNKS_NAME * CPP_MAX_STRING_LITERAL_LENGTH];
   standard_kim[0] = '\0';
   for (int i=0;i<STANDARD_KIM_STR_CHUNKS_NAME;++i)
   {
      strcat(standard_kim, STANDARD_KIM_STR_NAME[i]);
   }

   stdmdl.name_temp = (char*) "standard";
   if(!stdmdl.prestring_init(standard_kim)){
      std::cout<<" preinit of :"<<"standard.kim"<<" failed"<<std::endl;
      stdmdl.free();
      delete [] standard_kim;
      return false;
   }
   delete [] standard_kim;

    // test and mdl must be preinit.
    bool test2modelmatch= is_it_match(test,mdl.inlines,mdl.numlines,false,true);
    bool model2testmatch= is_it_match(mdl,test.inlines,test.numlines,true,true);

    bool test2modelmatch_noDC= is_it_match_noFlagCount(test,mdl.inlines,mdl.numlines,false);
    bool model2testmatch_noDC= is_it_match_noFlagCount(mdl,test.inlines,test.numlines,true);

    bool test2standardmatch = is_it_match(stdmdl,test.inlines,test.numlines,true,false);

    bool model2standardmatch = is_it_match(stdmdl,mdl.inlines,mdl.numlines,true,false);

    bool test2standardAtomsTypesMatch = do_AtomsTypes_match(test,stdmdl);
    bool model2standardAtomsTypesMatch = do_AtomsTypes_match(mdl,stdmdl);
    bool test2modelAtomsTypesMatch = do_AtomsTypes_match(test,mdl);
    bool AtomsTypesMatch=test2standardAtomsTypesMatch&&model2standardAtomsTypesMatch&&test2modelAtomsTypesMatch;

    stdmdl.free();

    bool NBC_methodsmatch = this->NBC_methods_match(test,mdl);
    NBC_methodsmatch=NBC_methodsmatch&&test.check_consistance_NBC_method();
    NBC_methodsmatch=NBC_methodsmatch&&mdl.check_consistance_NBC_method();
    bool process_fij_related = this->fij_related_things_match(test,mdl);
    bool units_match = Unit_Handling::do_unit_match(test.unit_h,mdl.unit_h);

    if(!test2standardmatch) std::cout<<"* Error (KIM_API_model::is_it_match): There are non-standard variables in Simulator descriptor file:"<<std::endl;
    if(!model2standardmatch) std::cout<<"* Error (KIM_API_model::is_it_match): There are non-standard variables in Model descriptor file:"<<std::endl;
    if(!test2standardAtomsTypesMatch) std::cout<<"* Error (KIM_API_model::is_it_match): There are non-standard Species in Simulator descriptor file:"<<std::endl;
    if(!model2standardAtomsTypesMatch) std::cout<<"* Error (KIM_API_model::is_it_match):there are non-standard Species in Model descriptor file:"<<std::endl;
    if(!test2modelAtomsTypesMatch) std::cout<<"* Error (KIM_API_model::is_it_match): Simulator-Model Species do not match:"<<std::endl;
    if(!NBC_methodsmatch) std::cout<<"* Error (KIM_API_model::is_it_match): NBC methods do not match:"<<std::endl;
    if(!process_fij_related) std::cout<<
       "* Error (KIM_API_model::is_it_match): (virial,particleVirial,hessian,process_d1/2Edr) do not match:"<<std::endl;
    if(!units_match){
       std::cout<<"* Error (KIM_API_model::is_it_match): units do not match:"<<std::endl;
    }else{
        this->unit_h = mdl.unit_h;
    }

    bool flag_match = do_flag_match(test,mdl);

    if (test2modelmatch && model2testmatch && test2standardmatch && process_fij_related &&
            model2standardmatch && AtomsTypesMatch && NBC_methodsmatch && units_match) return true;
    if (test2modelmatch_noDC && model2testmatch_noDC && test2standardmatch && process_fij_related &&
             model2standardmatch && AtomsTypesMatch && NBC_methodsmatch && units_match){
       return flag_match;
    }
    return false;
}

bool KIM_API_model::is_it_in_and_is_it_flag(KIM_API_model& mdl,const char * name){
   int error;
   int i = mdl.get_index(name,&error);
   if (i<0) return false;
   if (strcmp(mdl[i].type,"flag")!=0) return false;
   return true;
}
bool KIM_API_model::is_it_in(KIM_API_model& mdl, const char* name){
   int error;
   int i = mdl.get_index(name,&error);
   if (i<0) return false;
   return true;
}
bool KIM_API_model::do_flag_match(KIM_API_model& tst, KIM_API_model& mdl){
   int error;
    // here the assumption : besides flag type , everything is a match

    // check flag for tst
   bool ZeroBasedLists_tst =is_it_in_and_is_it_flag(tst, (char*) "ZeroBasedLists");
   bool OneBasedLists_tst =is_it_in_and_is_it_flag(tst, (char*) "OneBasedLists");



   bool Neigh_IterAccess_tst=is_it_in_and_is_it_flag(tst, (char*) "Neigh_IterAccess");
   bool Neigh_LocaAccess_tst=is_it_in_and_is_it_flag(tst, (char*) "Neigh_LocaAccess");
   bool Neigh_BothAccess_tst=is_it_in_and_is_it_flag(tst, (char*) "Neigh_BothAccess");

    // check flag for mdl
   bool ZeroBasedLists_mdl =is_it_in_and_is_it_flag(mdl, (char*) "ZeroBasedLists");
   bool OneBasedLists_mdl =is_it_in_and_is_it_flag(mdl, (char*) "OneBasedLists");

   bool Neigh_IterAccess_mdl=is_it_in_and_is_it_flag(mdl, (char*) "Neigh_IterAccess");

   bool Neigh_LocaAccess_mdl=is_it_in_and_is_it_flag(mdl, (char*) "Neigh_LocaAccess");

   bool Neigh_BothAccess_mdl=is_it_in_and_is_it_flag(mdl, (char*) "Neigh_BothAccess");



    //logic for Zero or One base list handling
    if ((!ZeroBasedLists_tst && !OneBasedLists_tst)||(ZeroBasedLists_tst && OneBasedLists_tst) ) {
        std::cout<< "* Error (KIM_API_model::do_flag_match): Simulator descriptor file must have ONE of ZeroBasedLists or OneBasedLists."<<std::endl;
        return false;
    }
     if ((!ZeroBasedLists_mdl && !OneBasedLists_mdl)||(ZeroBasedLists_mdl && OneBasedLists_mdl)) {
        std::cout<< "* Error (KIM_API_model::do_flag_match): Model descriptor file must have ONE of ZeroBasedLists or OneBasedLists."<<std::endl;
        return false;
    }
    model_index_shift = 0;
    if (ZeroBasedLists_tst && OneBasedLists_mdl) model_index_shift = 1;
    if (OneBasedLists_tst && ZeroBasedLists_mdl) model_index_shift = -1;
    if (OneBasedLists_mdl) AUX_index_shift =1;
    int ind_LocaAccess_mdl = mdl.get_index((char*) "Neigh_LocaAccess", &error);
    int ind_IterAcces_mdl = mdl.get_index((char*) "Neigh_IterAccess", &error);


    //logic for checking Both/Loca/Iter
    // checking if test o.k. when model requires both

    if (Neigh_BothAccess_mdl){

        if(!(Neigh_BothAccess_tst || (Neigh_LocaAccess_tst && Neigh_IterAccess_tst))){
            std::cout<< "* Error (KIM_API_model::do_flag_match): Model descriptor file requres Neigh_BothAccess."<<std::endl;
            return false;
        }
        mdl.both_neigh_mode=true;
     //checking if test o.k. when model may work with loca or iter
     }else if (Neigh_LocaAccess_mdl && Neigh_IterAccess_mdl){

        if(!(Neigh_LocaAccess_tst || (Neigh_IterAccess_tst || Neigh_BothAccess_tst))){
            std::cout<< "* Error (KIM_API_model::do_flag_match): Model descriptor file requres IterAccess or LocaAccess."<<std::endl;
            return false;
        }
        if ((Neigh_LocaAccess_tst && Neigh_IterAccess_tst) || Neigh_BothAccess_tst) {
            if (ind_LocaAccess_mdl < ind_IterAcces_mdl) {
                mdl.locator_neigh_mode =true;
            }else {
                mdl.iterator_neigh_mode = true;
            }
        }else if (Neigh_LocaAccess_tst) {
            mdl.locator_neigh_mode=true;
        } else if(Neigh_IterAccess_tst){
            mdl.iterator_neigh_mode=true;
        }


     //checking if test o.k. with loca
     }else if(Neigh_LocaAccess_mdl){
         if(!(Neigh_LocaAccess_tst || Neigh_BothAccess_tst)) {
             std::cout<< "* Error (KIM_API_model::do_flag_match): Model descriptor file requres Neigh_LocaAccess."<<std::endl;
             return false;
         }

         mdl.locator_neigh_mode = true;
     //checking if test o.k. with iter
     }else if(Neigh_IterAccess_mdl){
         if(!(Neigh_IterAccess_tst || Neigh_BothAccess_tst)) {
             std::cout<< "* Error (KIM_API_model::do_flag_match): Model descriptor file requres Neigh_IterAccess."<<std::endl;
             return false;
         }

         mdl.iterator_neigh_mode = true;
     }

    // it is ok to not have any of *neigh_mode if only CLUSTER
    bool cluster_only = true;
    for (int i=1;i<5;++i) { // Assuming NBC_methods[0] is CLUSTER
       if (is_it_in_and_is_it_flag(mdl, mdl.NBC_methods[i])){
          cluster_only = false;
          break;
       }
    }
    if(!(mdl.locator_neigh_mode||mdl.iterator_neigh_mode||mdl.both_neigh_mode) && !cluster_only) return false;

    return true;

}
bool KIM_API_model::do_AtomsTypes_match(KIM_API_model& test, KIM_API_model& mdl){
    bool match;

    if (test.nAtomsTypes == 0 && strcmp(mdl.model.name,"standard")==0) return true;
    if (test.nAtomsTypes == 0 && mdl.nAtomsTypes == 0) return true;
    if (test.nAtomsTypes == 0 && mdl.nAtomsTypes > 0) return false;
    if (test.nAtomsTypes > 0 && mdl.nAtomsTypes == 0) return false;
    for (int i=0;i < test.nAtomsTypes; i++){
        match = false;
        for (int j=0;j<mdl.nAtomsTypes;j++){
            if(strcmp(test.AtomsTypes[i].symbol, mdl.AtomsTypes[j].symbol)==0){
                mdl.AtomsTypes[j].requestedByTest = true;
                match = true;
                break;
            }
        }
        if (!match) {
            std::cout <<"* Error (KIM_API_model::do_AtomsTypes_match): The following symbol: "<<test.AtomsTypes[i].symbol<<" in ";
            std::cout<< test.model.name << " is not found in "<<mdl.model.name<<std::endl;
            return false;
        }
    }
    return true;
}

bool KIM_API_model::is_it_fixed_par(const char* name){
     char tmpname[KIM_KEY_STRING_LENGTH]="";
     strncpy(&tmpname[0],name,strlen(name)+1);
     char * tmp = strtok(tmpname,"_");if(tmp == NULL) return false;
     if(strcmp(tmp,"PARAM")==0) {
         tmp = strtok(NULL,"_");if(tmp == NULL) return false;
         if(strcmp(tmp,"FIXED")==0) return true;
     }
     return false;
}
bool KIM_API_model::is_it_free_par(const char* name){
     char tmpname[KIM_KEY_STRING_LENGTH]="";
     strncpy(&tmpname[0],name,strlen(name)+1);
     char * tmp = strtok(tmpname,"_");if(tmp == NULL) return false;
     if(strcmp(tmp,"PARAM")==0) {
         tmp = strtok(NULL,"_");if(tmp == NULL) return false;
         if(strcmp(tmp,"FREE")==0) return true;
     }
     return false;
}
bool KIM_API_model::is_it_par(const char* name){
    return is_it_free_par(name) || is_it_fixed_par(name);
}

#if KIM_LINK_VALUE != KIM_LINK_DYNAMIC_LOAD
extern "C"{
  #include "model_kim_str_include.h"
}

int KIM_API_model::get_model_kim_str_len(const char* const modelname,
                                         int* const kimStringLen)
{
     //redirecting std::cout > kimlog
    char kimlog[2048] = "./kim.log";
    std::streambuf * psbuf, * backup; std::ofstream filekimlog;
    filekimlog.open(kimlog);
    backup = std::cout.rdbuf();psbuf = filekimlog.rdbuf();std::cout.rdbuf(psbuf);

    unsigned int in_mdlstr_len = 0;
    const unsigned char* in_mdlstr = NULL;

    #include "model_kim_str_include.cpp"

    if (in_mdlstr == NULL){
       std::cout<<"* Error (KIM_API_model::get_model_kim_str): Unknown KIM Model name " << modelname << "." << std::endl;
       *kimStringLen = 0;
       return KIM_STATUS_FAIL;
    }

    //redirecting back to > std::cout
    std::cout.rdbuf(backup); filekimlog.close();
    *kimStringLen = (int) in_mdlstr_len;
    return KIM_STATUS_OK;
}

int KIM_API_model::get_model_kim_str(const char* const modelname,
                                     char** const kimString)
{
     //redirecting std::cout > kimlog
    char kimlog[2048] = "./kim.log";
    std::streambuf * psbuf, * backup; std::ofstream filekimlog;
    filekimlog.open(kimlog);
    backup = std::cout.rdbuf();psbuf = filekimlog.rdbuf();std::cout.rdbuf(psbuf);

    int in_mdlstr_chunks = 0;
    const char** in_mdlstr = NULL;

    #include "model_kim_str_include.cpp"

    if (in_mdlstr == NULL){
       std::cout<<"* Error (KIM_API_model::get_model_kim_str): Unknown KIM Model name " << modelname << "." << std::endl;
       *kimString = NULL;
       return KIM_STATUS_FAIL;
    }

    char* const str_cpy = (char* const) malloc(in_mdlstr_chunks * CPP_MAX_STRING_LITERAL_LENGTH);
    str_cpy[0] = '\0';
    for (int i=0; i<in_mdlstr_chunks; ++i)
    {
       strcat(str_cpy, in_mdlstr[i]);
    }

    //redirecting back to > std::cout
    std::cout.rdbuf(backup); filekimlog.close();
    *kimString = str_cpy;
    return KIM_STATUS_OK;
}

#else

int KIM_API_model::get_model_kim_str_len(const char* const modelname,
                                         int* const kimStringLen)
{
    void * tmp_model_lib_handle = NULL;
    char model_kim_str_name[KIM_LINE_LENGTH];
    sprintf(model_kim_str_name,"%s_" KIM_STR_NAME,modelname);
    char model_kim_str_len_name[KIM_LINE_LENGTH];
    sprintf(model_kim_str_len_name,"%s_" KIM_STR_NAME "_len",modelname);

    //redirecting std::cout > kimlog
    char kimlog[2048] = "./kim.log";
    std::streambuf * psbuf, * backup; std::ofstream filekimlog;
    filekimlog.open(kimlog);
    backup = std::cout.rdbuf();psbuf = filekimlog.rdbuf();std::cout.rdbuf(psbuf);

    std::list<std::string> lst;
    directoryPath(KIM_MODELS_DIR, &lst);
    std::list<std::string>::iterator itr;
    for (itr = lst.begin(); itr != lst.end(); ++itr)
    {
       itr->append("/");
       itr->append(modelname); itr->append("/");
       itr->append(MODELLIBFILE); itr->append(".so");
       //std::cout<< "* Info (KIM_API_model::get_model_kim_str): Looking for Model shared library file " << itr->c_str() <<std::endl;
       if (0 == access(itr->c_str(), F_OK))
       {
         tmp_model_lib_handle = dlopen(itr->c_str(), RTLD_NOW);
       }
       if (tmp_model_lib_handle != NULL) break;
    }
    if(tmp_model_lib_handle == NULL) {
       //redirecting back to > std::cout
       std::cout<< "* Error (KIM_API_model::get_model_kim_str): A problem occurred with the Model shared library file for Model name: ";
       std::cout<<modelname<<std::endl<<dlerror()<<std::endl;
       std::cout.rdbuf(backup); filekimlog.close();
       fprintf(stderr,"A problem occurred with the Model shared library file for Model name: %s.\n",modelname);
       *kimStringLen = 0;
       return KIM_STATUS_FAIL;
    }
    else
    {
      std::cout<< "* Info (KIM_API_model::get_model_kim_str): Found Model shared library file for Model name: " << modelname << std::endl;
    }


    const unsigned int* const model_str_len = (const unsigned int* const) dlsym(tmp_model_lib_handle, model_kim_str_len_name);
    char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cout << "* Error (KIM_API_model::get_model_kim_str): Cannot load symbol: " << dlsym_error <<std::endl;
        dlclose(tmp_model_lib_handle);

        //redirecting back to > std::cout
        std::cout.rdbuf(backup); filekimlog.close();

        *kimStringLen = 0;
        return KIM_STATUS_FAIL;
    }

    *kimStringLen = (int) *model_str_len;

    dlclose(tmp_model_lib_handle);
   //redirecting back to > std::cout
    std::cout.rdbuf(backup); filekimlog.close();
    return KIM_STATUS_OK;
}

int KIM_API_model::get_model_kim_str(const char* const modelname,
                                     char** const kimString)
{
    void * tmp_model_lib_handle = NULL;
    char model_kim_str_name[KIM_LINE_LENGTH];
    sprintf(model_kim_str_name,"%s_" KIM_STR_NAME,modelname);
    char model_kim_str_chunks_name[KIM_LINE_LENGTH];
    sprintf(model_kim_str_chunks_name,"%s_" KIM_STR_NAME "_chunks",modelname);

    //redirecting std::cout > kimlog
    char kimlog[2048] = "./kim.log";
    std::streambuf * psbuf, * backup; std::ofstream filekimlog;
    filekimlog.open(kimlog);
    backup = std::cout.rdbuf();psbuf = filekimlog.rdbuf();std::cout.rdbuf(psbuf);

    std::list<std::string> lst;
    directoryPath(KIM_MODELS_DIR, &lst);
    std::list<std::string>::iterator itr;
    for (itr = lst.begin(); itr != lst.end(); ++itr)
    {
       itr->append("/");
       itr->append(modelname); itr->append("/");
       itr->append(MODELLIBFILE); itr->append(".so");
       //std::cout<< "* Info (KIM_API_model::get_model_kim_str): Looking for Model shared library file " << itr->c_str() <<std::endl;
       if (0 == access(itr->c_str(), F_OK))
       {
         tmp_model_lib_handle = dlopen(itr->c_str(), RTLD_NOW);
       }
       if (tmp_model_lib_handle != NULL) break;
    }
    if(tmp_model_lib_handle == NULL) {
       //redirecting back to > std::cout
       std::cout<< "* Error (KIM_API_model::get_model_kim_str): A problem occurred with the Model shared library file for Model name: ";
       std::cout<<modelname<<std::endl<<dlerror()<<std::endl;
       std::cout.rdbuf(backup); filekimlog.close();
       fprintf(stderr,"A problem occurred with the Model shared library file for Model name: %s.\n",modelname);
       *kimString = NULL;
       return KIM_STATUS_FAIL;
    }
    else
    {
      std::cout<< "* Info (KIM_API_model::get_model_kim_str): Found Model shared library file for Model name: " << modelname << std::endl;
    }


    const int* const model_str_chunks = (const int* const) dlsym(tmp_model_lib_handle, model_kim_str_chunks_name);
    char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cout << "* Error (KIM_API_model::get_model_kim_str): Cannot load symbol: " << dlsym_error <<std::endl;
        dlclose(tmp_model_lib_handle);

        //redirecting back to > std::cout
        std::cout.rdbuf(backup); filekimlog.close();

        *kimString = NULL;
        return KIM_STATUS_FAIL;
    }
    const char** const model_str_ptr = (const char** const) dlsym(tmp_model_lib_handle, model_kim_str_name);
    dlsym_error = dlerror();
    if (dlsym_error) {
        std::cout << "* Error (KIM_API_model::get_model_kim_str): Cannot load symbol: " << dlsym_error <<std::endl;
        dlclose(tmp_model_lib_handle);

        //redirecting back to > std::cout
        std::cout.rdbuf(backup); filekimlog.close();

        *kimString = NULL;
        return KIM_STATUS_FAIL;
    }

    char* const str_cpy = (char* const) malloc(*model_str_chunks * CPP_MAX_STRING_LITERAL_LENGTH);
    str_cpy[0] = '\0';
    for (int i=0; i<*model_str_chunks; ++i)
    {
       strcat(str_cpy, model_str_ptr[i]);
    }

    dlclose(tmp_model_lib_handle);
   //redirecting back to > std::cout
    std::cout.rdbuf(backup); filekimlog.close();
    *kimString = str_cpy;
    return KIM_STATUS_OK;
}
#endif

int KIM_API_model::file_init(const char* testkimfile, const char* modelname){

    //redirecting std::cout > kimlog
   char kimlog[2048] = "./kim.log";
    std::streambuf * psbuf, * backup; std::ofstream filekimlog;
    filekimlog.open(kimlog);
    backup = std::cout.rdbuf();psbuf = filekimlog.rdbuf();std::cout.rdbuf(psbuf);

    int error;
    char* in_mdlstr;
    error = get_model_kim_str(modelname, &in_mdlstr);
    if (error == KIM_STATUS_OK) {
       name_temp = modelname;
       error = init_str_modelname(testkimfile,in_mdlstr);
    }

    std::free(in_mdlstr);
   //redirecting back to > std::cout
    std::cout.rdbuf(backup); filekimlog.close();

    return error;
}

void KIM_API_model::fatal_error_print(){
    printf("* KIM FATAL ERROR: See kim.log file for details\n");
}


int KIM_API_model::init_str_modelname(const char* testinputfile, const char* inmdlstr){
   int error;

    //check test-model match and preinit test-model-API
    KIM_API_model test,mdl;
    //preinit test and model API object
    mdl.name_temp = name_temp;
    if(!mdl.prestring_init(inmdlstr)){
      const char* msg;
      get_status_msg(mdl.ErrorCode, &msg);
        std::cout<<"prestring_init  failed with error status: "<<msg<<std::endl;
        return KIM_STATUS_FAIL;
    }

    test.name_temp = "Simulator";
    if(!test.preinit(testinputfile,"Simulator")){
      const char* msg;
      get_status_msg(test.ErrorCode, &msg);
        std::cout<<"preinit  failed with error status: "<<msg<<std::endl;
        return KIM_STATUS_FAIL;
    }

    //check if they match
    if (is_it_match(test,mdl)){
       this->name_temp = mdl.model.name;
        this->prestring_init(inmdlstr);
        this->unit_h=test.unit_h;
        if (!(this->irrelevantVars2donotcompute(test,*this))) return KIM_STATUS_FAIL;
        for (int i=0;i<this->nAtomsTypes;++i) {
           this->AtomsTypes[i].requestedByTest = mdl.AtomsTypes[i].requestedByTest;
        }

        strcpy(this->NBC_method_current, mdl.NBC_method_current);
        locator_neigh_mode=mdl.locator_neigh_mode;
        iterator_neigh_mode=mdl.iterator_neigh_mode;
        both_neigh_mode=mdl.both_neigh_mode;

        this->modelVersionMajor = mdl.tempVersionMajor;
        this->modelVersionMinor = mdl.tempVersionMinor;
        this->simulatorVersionMajor = test.tempVersionMajor;
        this->simulatorVersionMinor = test.tempVersionMinor;

        test.free(); mdl.free();
        char computestr [] = "compute";
        compute_index = get_index(computestr, &error);
        get_neigh_index = get_index((char*) "get_neigh", &error);
        if (!(this->fij_related_things_add_set_index())) return KIM_STATUS_FAIL;

        return KIM_STATUS_OK;
    }else{

       std::cout<<"Do not match  " << mdl.model.name  << " and "<< "Simulator" <<std::endl;
       mdl.free();
       test.free();

       return KIM_STATUS_FAIL;
    }
}

int KIM_API_model::preinit(const char* modelname){
    //redirecting std::cout > kimlog
   char kimlog[2048] = "./kim.log";
    std::streambuf * psbuf, * backup; std::ofstream filekimlog;
    filekimlog.open(kimlog);
    backup = std::cout.rdbuf();psbuf = filekimlog.rdbuf();std::cout.rdbuf(psbuf);
    //preinit model

    int error;
    int result = KIM_STATUS_FAIL;
    char* in_mdlstr;
    error = get_model_kim_str(modelname, &in_mdlstr);
    if (error == KIM_STATUS_OK)
    {
       this->name_temp = modelname;
       result = this->prestring_init(in_mdlstr);
    }
    std::free(in_mdlstr);
    //redirecting back to > std::cout
    std::cout.rdbuf(backup); filekimlog.close();
    return result;
}

int KIM_API_model::string_init(const char* in_tststr, const char* modelname){
   int error;
    //redirecting std::cout > kimlog
   char kimlog[2048] = "./kim.log";
    std::streambuf * psbuf, * backup; std::ofstream filekimlog;
    filekimlog.open(kimlog);
    backup = std::cout.rdbuf();psbuf = filekimlog.rdbuf();std::cout.rdbuf(psbuf);

    //check test-model match and preinit test-model-API
    KIM_API_model test,mdl;
    char* in_mdlstr;
    error = get_model_kim_str(modelname, &in_mdlstr);
    if (error != KIM_STATUS_OK) {
       //redirecting back to > std::cout
       std::cout.rdbuf(backup); filekimlog.close();
       return error;
    }

    mdl.name_temp = modelname;
    error = mdl.prestring_init(in_mdlstr);
    if(error != KIM_STATUS_OK)
    {
      const char* msg;
      get_status_msg(mdl.ErrorCode, &msg);
       std::cout<<"mdl.prestring_init failed with error status:"<<msg<<std::endl;
       std::free(in_mdlstr);
       //redirecting back to > std::cout
       std::cout.rdbuf(backup); filekimlog.close();
       return error;
    }

    //preinit test and model API object
    test.name_temp = (char*) "simulator_name";
    error = test.prestring_init(in_tststr);
    if(error != KIM_STATUS_OK)
    {
      const char* msg;
      get_status_msg(test.ErrorCode, &msg);
       std::cout<<"test.prestring_init failed with error status:"<<msg<<std::endl;
       std::free(in_mdlstr);
       mdl.free();
       //redirecting back to > std::cout
       std::cout.rdbuf(backup); filekimlog.close();
       return error;
    }

    //check if they match
    if (is_it_match(test,mdl)){
       this->name_temp = mdl.model.name;
        this->prestring_init(in_mdlstr);
        std::free(in_mdlstr);
        this->unit_h=test.unit_h;
        if (!(this->irrelevantVars2donotcompute(test,*this))) return KIM_STATUS_FAIL;
        for (int i=0;i<this->nAtomsTypes;++i) {
           this->AtomsTypes[i].requestedByTest = mdl.AtomsTypes[i].requestedByTest;
        }

        strcpy(this->NBC_method_current, mdl.NBC_method_current);
        locator_neigh_mode=mdl.locator_neigh_mode;
        iterator_neigh_mode=mdl.iterator_neigh_mode;
        both_neigh_mode=mdl.both_neigh_mode;

        this->modelVersionMajor = mdl.tempVersionMajor;
        this->modelVersionMinor = mdl.tempVersionMinor;
        this->simulatorVersionMajor = test.tempVersionMajor;
        this->simulatorVersionMinor = test.tempVersionMinor;

        test.free(); mdl.free();
        char computestr [] = "compute";
        compute_index = get_index(computestr, &error);
        get_neigh_index = get_index((char*) "get_neigh", &error);
        if (!(this->fij_related_things_add_set_index()))
        {
           //redirecting back to > std::cout
           std::cout.rdbuf(backup); filekimlog.close();
           return KIM_STATUS_FAIL;
        }
        else
        {
           //redirecting back to > std::cout
           std::cout.rdbuf(backup); filekimlog.close();
           return KIM_STATUS_OK;
        }
    }else{
       std::free(in_mdlstr);
       mdl.free();
       std::cout<<"Do not match  " << modelname << " and "<< test.model.name <<std::endl;
       test.free();
       //redirecting back to > std::cout
       std::cout.rdbuf(backup); filekimlog.close();
       return KIM_STATUS_FAIL;
    }
}

int KIM_API_model::match(const char* teststr, const char* modelstr){
   int error;
    //redirecting std::cout > kimlog
   char kimlog[2048] = "./kim.log";
    std::streambuf * psbuf, * backup; std::ofstream filekimlog;
    filekimlog.open(kimlog);
    backup = std::cout.rdbuf();psbuf = filekimlog.rdbuf();std::cout.rdbuf(psbuf);

    //check test-model match and preinit test-model-API
    KIM_API_model test,mdl;

    //preinit test and model API object
    test.name_temp = (char*) "simulator_name";
    error = test.prestring_init(teststr);
    if(error != KIM_STATUS_OK)
    {
       test.free();
       //redirecting back to > std::cout
       std::cout.rdbuf(backup); filekimlog.close();

       return KIM_STATUS_FAIL;
    }

    mdl.name_temp = (char*) "model_name";
    error = mdl.prestring_init(modelstr);
    if(error != KIM_STATUS_OK)
    {
       test.free();
       mdl.free();
       //redirecting back to > std::cout
       std::cout.rdbuf(backup); filekimlog.close();

       return KIM_STATUS_FAIL;
    }

    //check if they match
    bool match;
    match = is_it_match(test,mdl);
    mdl.free();
    test.free();

    //redirecting back to > std::cout
    std::cout.rdbuf(backup); filekimlog.close();

    if (match)
    {
       error = KIM_STATUS_OK;
    }
    else
    {
       error = KIM_STATUS_FAIL;
    }

    return error;
}


int KIM_API_model::model_reinit(){
   int error;
   int reinit_ind = get_index((char*) "reinit", &error);
   if (error != KIM_STATUS_OK) return error;

   KIM_API_model *pkim = this;
   typedef int (*Model_Reinit)(void *);//prototype for model_reinit
   Model_Reinit mdl_reinit = (Model_Reinit)(*this)[reinit_ind].data.fp;
   if (mdl_reinit == NULL) return KIM_STATUS_FAIL;
   return (*mdl_reinit)(&pkim);
}

#if KIM_LINK_VALUE != KIM_LINK_DYNAMIC_LOAD
extern "C" {
#include "model_init_include.h"
}
int KIM_API_model::model_init(){
    char modelname[KIM_KEY_STRING_LENGTH]="";
    KIM_API_model * kim;
    void ** pkim;
    strcpy(modelname,this->model.name);
    kim=this;
    pkim =(void**) &kim;

    //redirecting std::cout > kimlog
    char kimlog[2048] = "./kim.log";
    std::streambuf * psbuf, * backup; std::ofstream filekimlog;
    filekimlog.open(kimlog,std::ofstream::app);
    backup = std::cout.rdbuf();psbuf = filekimlog.rdbuf();std::cout.rdbuf(psbuf);

std::cout<< "* Info: (KIM_API_model::model_init): call statically linked initialize routine for::"<<modelname<<std::endl;
    //redirecting back to > std::cout
    std::cout.rdbuf(backup); filekimlog.close();

#include "model_init_include.cpp"

    //redirecting std::cout > kimlog
    filekimlog.open(kimlog,std::ofstream::app);
    backup = std::cout.rdbuf();psbuf = filekimlog.rdbuf();std::cout.rdbuf(psbuf);

    std::cout<< "* Info: (KIM_API_model::model_init): model initiliser failed for ";
    std::cout<<modelname<<std::endl;

     //redirecting back to > std::cout
    std::cout.rdbuf(backup); filekimlog.close();

    return KIM_STATUS_FAIL;
}
#else
int KIM_API_model::model_init(){
    char modelname[KIM_KEY_STRING_LENGTH]="";
    KIM_API_model * kim;
    void ** pkim;
    char model_init_routine_name[2048];
    strcpy(modelname,this->model.name);
    kim=this;
    pkim =(void**) &kim;

//redirecting std::cout > kimlog
    char kimlog[2048] = "./kim.log";
    std::streambuf * psbuf, * backup; std::ofstream filekimlog;
    filekimlog.open(kimlog, std::ofstream::app);
    backup = std::cout.rdbuf();psbuf = filekimlog.rdbuf();std::cout.rdbuf(psbuf);

//    sprintf(model_slib_file,"%s/%s/%s.so",directoryPath(srcdir)/$(modelsdir),modelname,modelname);
    std::list<std::string> lst;
    directoryPath(KIM_MODELS_DIR, &lst);
    std::list<std::string>::iterator itr;
    for (itr = lst.begin(); itr != lst.end(); ++itr)
    {
       itr->append("/");
       itr->append(modelname); itr->append("/");
       itr->append(MODELLIBFILE); itr->append(".so");
       //std::cout<< "* Info (KIM_API_model::model_init): Looking for Model shared library file " << itr->c_str() <<std::endl;
       if (0 == access(itr->c_str(), F_OK))
       {
         model_lib_handle = dlopen(itr->c_str(), RTLD_NOW);
       }
       if (NULL != model_lib_handle) break;
    }
    if(NULL == model_lib_handle) {
       //redirecting back to > std::cout
       std::cout<< "* Error (KIM_API_model::model_init): A problem occurred with the Model shared library file for Model name: " << modelname << std::endl;
       std::cout<<modelname<<std::endl<<dlerror()<<std::endl;
       std::cout.rdbuf(backup); filekimlog.close();
       fprintf(stderr,"A problem occurred with the Model shared library file for Model name: %s.\n",modelname);
       return KIM_STATUS_FAIL;
    }

std::cout<<"* Info: (KIM_API_model::model_init): call dynamically linked initialize routine for:"<<modelname<<std::endl;
std::cout<<"               from the shared library:"<<itr->c_str()<<std::endl;
    sprintf(model_init_routine_name,"%s_init_pointer",modelname);

    typedef int (*Model_Init)(void **);//prototype for model_init
    Model_Init mdl_init = *((Model_Init*)dlsym(model_lib_handle,model_init_routine_name));
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
        std::cout << "* Error (KIM_API_model::model_init): Cannot load symbol: " << dlsym_error <<std::endl;
        dlclose(model_lib_handle);

        //redirecting back to > std::cout
        std::cout.rdbuf(backup); filekimlog.close();

        return KIM_STATUS_FAIL;
    }

    //redirecting back to > std::cout
    std::cout.rdbuf(backup); filekimlog.close();


    return (*mdl_init)(pkim);
}
#endif

int KIM_API_model::model_destroy(){
  typedef int (*Model_Destroy)(void *);//prototype for model_destroy
  Model_Destroy mdl_destroy = (Model_Destroy) (*this)[(char*) "destroy"].data.fp;
  //call model_destroy
  KIM_API_model *pkim = this;

  int error = KIM_STATUS_OK;
  if (mdl_destroy != NULL) {
     error = (*mdl_destroy)((void *)&pkim);
  }

#if KIM_LINK_VALUE == KIM_LINK_DYNAMIC_LOAD
  dlclose(model_lib_handle);
#endif
  return error;
}
int KIM_API_model::model_compute(){
  // set model_compute pointer
  typedef int (*Model_Compute)(void *);//prototype for model_compute
  int error = KIM_STATUS_FAIL;
  Model_Compute mdl_compute = (Model_Compute) (*this)[compute_index].data.fp;
  if (mdl_compute == NULL) return error;

  //initialize virials if needed

  if (process_dEdr_ind >=0 || process_d2Edr2_ind >= 0){
     KIM_AUX::Process_DE::init2zero(this,&error);
     if(error != KIM_STATUS_OK) return error;
  }

  //call model_compute
  KIM_API_model *pkim = this;
  error = (*mdl_compute)((void *)&pkim);

  return error;
}

int KIM_API_model::get_neigh(int mode, int request, int *part,
        int *numnei, int** nei1part, double** Rij){
    int locrequest=request;
    int locmode = mode;

    if(mode!=0 && mode!=1) return KIM_STATUS_NEIGH_INVALID_MODE;
    if(this == NULL) return KIM_STATUS_API_OBJECT_INVALID;
    typedef int (*Get_Neigh)(void **, int *, int *, int *, int *, int **,double **);

    if (get_neigh_index < 0) return KIM_STATUS_API_OBJECT_INVALID;
    Get_Neigh get_neigh = (Get_Neigh)(*this)[get_neigh_index].data.fp;
    KIM_API_model *pkim = this;

    if (model_index_shift==0) {
       if (mode==0 && request == 0) { // reset iterator
            return (*get_neigh)((void **)&pkim,&locmode, &locrequest, part, numnei, nei1part, Rij ) ;
        }else{

            int erkey = (*get_neigh)((void **)&pkim,&locmode, &locrequest, part, numnei, nei1part, Rij );
            return erkey;
        }
    }else if (model_index_shift == 1 || model_index_shift == -1){

        int req=request;
        if (mode ==1) req = request - model_index_shift;
        int at = *part;

        if (mode==0 && request == 0) { // reset iterator
            return (*get_neigh)((void **)&pkim,&locmode, &req, &at, numnei, nei1part, Rij );
        }else{
            int erkey = (*get_neigh)((void **)&pkim,&locmode, &req, &at, numnei, nei1part, Rij );
            if (erkey == 1){
                *part= at + model_index_shift;
                if (neiOfAnAtomSize < *numnei) {
                   delete [] neiOfAnAtom;
                   neiOfAnAtom = new int[*numnei];
                   if (neiOfAnAtom == NULL) {
                      neiOfAnAtomSize = 0;
                      std::cout << std::endl << "* Error (KIM_API_model::get_neigh): numnei too big to allocate memory for index conversion: " << *numnei << std::endl;
                      return KIM_STATUS_NEIGH_TOO_MANY_NEIGHBORS;
                   }
                   neiOfAnAtomSize = *numnei;
                }
                for (int i = 0; i<(*numnei);i++){
                   neiOfAnAtom[i] = (*nei1part)[i] + model_index_shift;
                }
                *nei1part = &(neiOfAnAtom[0]);
            }
            return erkey;
        }
    }else{
        std::cout<<std::endl<< "* Error (KIM_API_model::get_neigh): wrong base convert key,model_index_shift =";
        std::cout<< model_index_shift <<"  (must be 0,1 or -1)"<<std::endl;
        return KIM_STATUS_API_OBJECT_INVALID;
    }
}

bool KIM_API_model::irrelevantVars2donotcompute(KIM_API_model & test, KIM_API_model & mdl){
   if(! is_it_match_noFlagCount(test,mdl.inlines,mdl.numlines,false)) {
        std::cout<<"* Error (KIM_API_model::irrelevantVars2donotcompute): Simulator and Model descriptor files are incompatible (do not match)."<<std::endl;
        return false;
    }
    for(int i=0; i<mdl.numlines;i++){
        if(mdl.inlines[i].isitoptional()) {
            mdl[i].flag->calculate = 0;
            for (int j=0;j<test.model.size;j++){
               if(test[j].equiv(mdl.inlines[i],false)) mdl[i].flag->calculate = 1;
            }
        }
    }

    return true;
}

void KIM_API_model::allocate( int nparts, int nspecies, int * error){
    // in process
    if ( this->model.data.p == NULL) {
        std::cout<<"* Error (KIM_API_model::allocate): KIM API object not initialized with KIM_API_file_init()."<<std::endl;
        *error = KIM_STATUS_FAIL;
        return;
    }
    for(int i=0; i<this->model.size;i++){
        intptr_t rank = (intptr_t)this->inlines[i].get_rank();
        int *shape = this->inlines[i].get_shape(nparts,nspecies);
        int calculate = (*this)[i].flag->calculate;
        bool isitparam = this->is_it_par((*this)[i].name);
        intptr_t sz=0;
        int c=1;
        if (shape!=NULL) {
            for(int k=0;k<rank;k++) c=c*shape[k];
            sz=c;
        }else{
            sz = 1;
            if (strcmp((*this)[i].type,"pointer")==0 || strcmp((*this)[i].type,"method")==0) sz=0;
            if (strcmp((*this)[i].type,"flag")==0 ) sz=0;
        }
        if((this->inlines[i].isitoptional() && (calculate == 0)) || isitparam) {
            sz=0;
            if(shape!=0) shape[0]=0;
        }
        (*this)[i].free();
        (*this)[i].init(this->inlines[i].name,this->inlines[i].type,sz,rank,shape);
        strncpy((*this)[i].unit->dim,this->inlines[i].dim,strlen(this->inlines[i].dim)+1);
        (*this)[i].flag->calculate=calculate;
         (*this)[i].flag->peratom = 1;
        if(this->inlines[i].isitperatom()) (*this)[i].flag->peratom = 0;
        delete [] shape;
    }
    *error=KIM_STATUS_OK;
}

std::ostream &operator<<(std::ostream &stream, KIM_API_model &a){
    stream<<"*************************************"<<std::endl;
    stream<<"KIM API Object details:" << std::endl << std::endl;
    stream << a.model;
    stream<<"Active Units" << std::endl;
    stream<< a.unit_h << std::endl;
    stream<<"List of items in KIM API Ojbect" << std::endl;
    stream<<"-------------------------------------"<<std::endl;
    KIMBaseElement **pel =  (KIMBaseElement **)  a.model.data.p;
    for(int i=0;i<a.model.size;i++)
    {
       stream << "index : " << i << std::endl
              << *(pel[i]) << std::endl;
    }
    stream<<"*************************************"<<std::endl;

    return stream;
}
bool KIM_API_model::init_AtomsTypes(){
    nAtomsTypes=0;
    for(int i=0;i < numlines;i++){
        if (strcmp(inlines[i].type, "spec")==0) nAtomsTypes++;
    }
    if (nAtomsTypes==0) return true;

    AtomsTypes = new Atom_Map[nAtomsTypes];
    int ii=0;
    for(int i=0;i < numlines;i++){
        if (strcmp(inlines[i].type, "spec")==0){
            strncpy(AtomsTypes[ii].symbol,inlines[i].name,strlen(inlines[i].name)+1);
            if(inlines[i].get_rank() !=1){
                ErrorCode = -30;
                std::cout <<" atom code error";
                return false;
            }
            int * shp = inlines[i].get_shape();
            AtomsTypes[ii].code = shp[0];
            AtomsTypes[ii].readOnly = (shp[0] != -1);
            delete [] shp;
            ii++;
        }
    }
    qsort((void *) AtomsTypes,(size_t) nAtomsTypes,sizeof(Atom_Map),&(Atom_Map::comparator));
    ErrorCode=1;
    return true;
}

int KIM_API_model::get_num_model_species(int* numberSpecies,
                                         int* maxStringLength)
{
  *numberSpecies = nAtomsTypes;
  *maxStringLength = 0;
  for (int i=0; i<nAtomsTypes; ++i)
  {
    int len = strlen(AtomsTypes[i].symbol);
    if (len > *maxStringLength)
    {
      *maxStringLength = len;
    }
  }

  return KIM_STATUS_OK;
}
int KIM_API_model::get_model_species(const int index,
                                     const char** const speciesString)
{
  if (0 == nAtomsTypes) return KIM_STATUS_FAIL;
  if ((index < 0) || (index>nAtomsTypes))
  {
    return KIM_STATUS_FAIL;
  }

  *speciesString = AtomsTypes[index].symbol;
  return KIM_STATUS_OK;
}

int KIM_API_model::get_num_sim_species(int* numberSpecies,
                                       int* maxStringLength)
{
  *numberSpecies = 0;
  *maxStringLength = 0;
  for (int i=0; i<nAtomsTypes; ++i)
  {
    if (AtomsTypes[i].requestedByTest)
    {
      ++(*numberSpecies);
      int len = strlen(AtomsTypes[i].symbol);
      if (len > *maxStringLength)
      {
        *maxStringLength = len;
      }
    }
  }

  return KIM_STATUS_OK;
}

int KIM_API_model::get_sim_species(const int index,
                                   const char** const speciesString)
{
  if (0 == nAtomsTypes) return KIM_STATUS_FAIL;

  int count = 0;
  for (int i=0; i<nAtomsTypes; ++i)
  {
    if (AtomsTypes[i].requestedByTest) ++count;
  }
  if ((index < 0) || (index>count))
  {
    return KIM_STATUS_FAIL;
  }

  int correspondingIndex = -1;
  for (int i=0; i<nAtomsTypes; ++i)
  {
    if (AtomsTypes[i].requestedByTest) ++correspondingIndex;
    if (correspondingIndex == index)
    {
      *speciesString = AtomsTypes[i].symbol;
      return KIM_STATUS_OK;
    }
  }

  // should never get here
  return KIM_STATUS_FAIL;
}

int KIM_API_model::get_NBC_method(const char** const NBC_String)
{
  if(strcmp(this->NBC_method_current,"none")==0) {
    // no NBC methods are specified
    *NBC_String = NULL;
    return KIM_STATUS_FAIL;
  }
  *NBC_String = NBC_method_current;
  return KIM_STATUS_OK;
}

int KIM_API_model::is_half_neighbors(int* kimerr){
   const int is_half = 1;
   const int is_full = 0;
    *kimerr=KIM_STATUS_FAIL;
    const char* method;
    *kimerr = get_NBC_method(&method);

    if(*kimerr!=1){
        return is_half;
    }

    int answer = is_half;
    if (strcmp(method,"NEIGH_PURE_F")==0) answer = is_full;
    if (strcmp(method,"NEIGH_RVEC_F")==0) answer = is_full;
    if (strcmp(method,"MI_OPBC_F")==0) answer = is_full;
    *kimerr=KIM_STATUS_OK;
    return answer;
}

int KIM_API_model::get_num_params(int* numberParameters, int* maxStringLength)
{
  *numberParameters = 0;
  *maxStringLength = 0;
  for (int i=0; i<model.size; ++i)
  {
    if (is_it_par((*this)[i].name))
    {
      ++(*numberParameters);
      int len = strlen((*this)[i].name);
      if (len > *maxStringLength)
      {
        *maxStringLength = len;
      }
    }
  }

  return KIM_STATUS_OK;
}

int KIM_API_model::get_parameter(const int index, const char** const parameterString)
{
  int correspondingIndex = -1;
  for (int i=0; i<model.size; ++i)
  {
    if (is_it_par((*this)[i].name)) ++correspondingIndex;
    if (correspondingIndex == index)
    {
      *parameterString = (*this)[i].name;
      return KIM_STATUS_OK;
    }
  }

  // In case the requested index is not valid
  return KIM_STATUS_FAIL;
}

int KIM_API_model::get_num_free_params(int* numberParameters,
                                       int* maxStringLength)
{
  *numberParameters = 0;
  *maxStringLength = 0;
  for (int i=0; i<model.size; ++i)
  {
    if (is_it_free_par((*this)[i].name))
    {
      ++(*numberParameters);
      int len = strlen((*this)[i].name);
      if (len > *maxStringLength)
      {
        *maxStringLength = len;
      }
    }
  }

  return KIM_STATUS_OK;
}

int KIM_API_model::get_free_parameter(const int index,
                                      const char** const parameterString)
{
  int correspondingIndex = -1;
  for (int i=0; i<model.size; ++i)
  {
    if (is_it_free_par((*this)[i].name)) ++correspondingIndex;
    if (correspondingIndex == index)
    {
      *parameterString = (*this)[i].name;
      return KIM_STATUS_OK;
    }
  }

  // In case the requested index is not valid
  return KIM_STATUS_FAIL;
}

int KIM_API_model::get_num_fixed_params(int* numberParameters,
                                        int* maxStringLength)
{
  *numberParameters = 0;
  *maxStringLength = 0;
  for (int i=0; i<model.size; ++i)
  {
    if (is_it_fixed_par((*this)[i].name))
    {
      ++(*numberParameters);
      int len = strlen((*this)[i].name);
      if (len > *maxStringLength)
      {
        *maxStringLength = len;
      }
    }
  }

  return KIM_STATUS_OK;
}

int KIM_API_model::get_fixed_parameter(const int index,
                                       const char** const parameterString)
{
  int correspondingIndex = -1;
  for (int i=0; i<model.size; ++i)
  {
    if (is_it_fixed_par((*this)[i].name)) ++correspondingIndex;
    if (correspondingIndex == index)
    {
      *parameterString = (*this)[i].name;
      return KIM_STATUS_OK;
    }
  }

  // In case the requested index is not valid
  return KIM_STATUS_FAIL;
}

int  KIM_API_model::get_neigh_mode(int*kimerr){
    *kimerr=KIM_STATUS_OK;

    if(locator_neigh_mode && !iterator_neigh_mode && !both_neigh_mode){
        return 2;
    }else if(!locator_neigh_mode && iterator_neigh_mode && !both_neigh_mode){
        return 1;
    }else if(!locator_neigh_mode && !iterator_neigh_mode && both_neigh_mode){
        return 3;
    }else if(locator_neigh_mode && iterator_neigh_mode && !both_neigh_mode){
        return 1;
    }else{
        *kimerr = KIM_STATUS_FAIL;
        return -1;
    }
}

int KIM_API_model::get_species_code(const char* species, int * error){
    *error =KIM_STATUS_FAIL;
    if (species == NULL)  {
        *error = KIM_STATUS_PARTICLE_SPECIES_UNDEFINED;
        return KIM_STATUS_PARTICLE_SPECIES_UNDEFINED; //no atom symbol provided
    }
    Atom_Map key, *res=NULL;
    strcpy(key.symbol,species);
    res = (Atom_Map *)bsearch((void *)&key,AtomsTypes,nAtomsTypes,sizeof(Atom_Map),&(Atom_Map::comparator));
    if (res == NULL) {
        *error = KIM_STATUS_PARTICLE_INVALID_SPECIES;
        return  KIM_STATUS_PARTICLE_INVALID_SPECIES; //did not find atom symbol among atom species
    }
    *error=KIM_STATUS_OK;
    return res->code;
}

void KIM_API_model::set_species_code(const char* species, int code, int* error){
   *error = KIM_STATUS_FAIL;
    if (species == NULL)  {
        *error = KIM_STATUS_PARTICLE_SPECIES_UNDEFINED;
        return; //no atom symbol provided
    }
    Atom_Map key, *res=NULL;
    strcpy(key.symbol,species);
    res = (Atom_Map *)bsearch((void *)&key,AtomsTypes,nAtomsTypes,sizeof(Atom_Map),&(Atom_Map::comparator));
    if (res == NULL) {
        *error = KIM_STATUS_PARTICLE_INVALID_SPECIES;
        return; //did not find atom symbol among atom species
    }
    if (res->readOnly) {
       *error = KIM_STATUS_FAIL;
       return;
    }

    res->code = code;

    *error=KIM_STATUS_OK;
    return;
}

bool KIM_API_model::NBC_methods_match(KIM_API_model& test, KIM_API_model& mdl){
   int error;


    bool NBC_method_mdl[number_NBC_methods];
    bool NBC_method_test[number_NBC_methods];
    for (int i=0;i<number_NBC_methods; i++){
        NBC_method_mdl[i] = is_it_in_and_is_it_flag(mdl,NBC_methods[i]);
        NBC_method_test[i] = is_it_in_and_is_it_flag(test,NBC_methods[i]);
    }

    int indexes[number_NBC_methods];
    for (int i=0;i<number_NBC_methods; i++) indexes[i]=1000;
    bool match=false;
    for (int i=0;i<number_NBC_methods; i++) {
        if(NBC_method_test[i]&&NBC_method_mdl[i]){
            match=true;
            indexes[i]=test.get_index(NBC_methods[i], &error);
        }
    }
    if(!match) return false;
    int min=1000;
    for (int i=0;i<number_NBC_methods; i++) if(indexes[i]<min) min=indexes[i];
    strcpy(test.NBC_method_current,test[min].name);
    strcpy(mdl.NBC_method_current,test[min].name);
    return true;
}
bool KIM_API_model::check_consistance_NBC_method(){
   int error;
    //will check current NBC method if all arguments are in the object
    bool match=false;
    int i;
    for (i=0; i<n_NBC_methods; i++){
        if(strcmp(NBC_methods[i],NBC_method_current)==0){
            match =true;
            break;
        }
    }
    if (!match) {
        std::cout<<"* Error (KIM_API_model::check_consistance_NBC_method):"<<NBC_method_current
                <<" is unknown."<<std::endl;
        return false;
    }
    for (int j=0;j<nnarg_NBC[i]; j++){
       if (get_index(arg_NBC_methods[i][j], &error) == -1){
            std::cout<<"* Error (KIM_API_model::check_consistance_NBC_method): Argument "<< arg_NBC_methods[i][j];
            std::cout<<" required for NBC method " << NBC_method_current;
            std::cout<<" is not in KIM API object."<<std::endl;
            return false;
        }
    }
    return true;
}
int KIM_API_model::get_status_msg(const int status_code,
                                  const char** const status_msg)
{
    int mincode=-24,maxcode=3,offset=24;

    static const char KIM_STATUS_MSG[][KIM_KEY_STRING_LENGTH]=
   {
    {"configuration is not supported by the Model"},
    {"base units: are not supported or not the same phys.dimensions"},
    {"unsupported Unit_time"},
    {"unsupported Unit_temperature"},
    {"unsupported Unit_charge"},
    {"unsupported Unit_energy"},
    {"unsupported Unit_length"},
    {"Unit_Handling must be \"flexible\" or \"fixed\" "},
    {"group argument must be 1 or 0 (in KIM_API...multiple routine)"},
    {"numargs is not divisiable by 4(in KIM_API...multiple routine)"},
    {"wrong optional arguments (in a kim_api_...multiple routine)"},
    {"numargs is not divisible by 2 (in KIM_API...multiple routine)"},
    {"numargs is not divisiable by 3 (in KIM_API...multiple routine)"},
    {"invalid value for `request' provided"},
    {"get_neigh method in KIM API object is not set (NULL value)"},
    {"number of neighs of particle too big to allocate for conversion"},
    {"invalid KIM API object"},
    {"negative index in shape"},
    {"invalid mode value"},
    {"no particle species have been specified by the Simulator or Model"},
    {"provided rank does not match KIM API argument rank"},
    {"invalid particle id requested (request out of range)"},
    {"symbol is not among supported particle symbols"},
    {"argument name provided is not in KIM API object"},
    {"unsuccessful completion"},
    {"successful completion"},
    {"iterator has been incremented past end of list"},
    {"iterator has been successfully initialized"}};

    if (status_code < mincode || status_code > maxcode) {
      *status_msg = "the error code is not among KIM_STATUS codes";
      return KIM_STATUS_FAIL;
    }else{
        int ind = offset + status_code;
        *status_msg = KIM_STATUS_MSG[ind];
        return KIM_STATUS_OK;
    }
}

int KIM_API_model::report_error(int ln,const char * fl,const char * usermsg,int ier){
    if(ier <= 0){
      const char* kimstatus;
      get_status_msg(ier, &kimstatus);
        std::cout<<"* Error: at line "<<ln<<" in "<<fl<< std::endl<<"\tMessage: "<<usermsg<<std::endl;
        std::cout<<"\tKIM_STATUS_MSG: "<<kimstatus<<std::endl;
        return KIM_STATUS_FAIL;
    }
    return KIM_STATUS_OK;
}

int KIM_API_model::get_model_index_shift(){
    return this->model_index_shift;
}

void KIM_API_model::set_model_buffer(void* o, int* ier){
    *ier = KIM_STATUS_OK;
    model_buffer = o;
}
void * KIM_API_model::get_model_buffer(int* ier){
    *ier = KIM_STATUS_FAIL;
    if (model_buffer == NULL) return NULL;
    *ier = KIM_STATUS_OK;
    return model_buffer;
}
void KIM_API_model::set_sim_buffer(void* o, int* ier){
    *ier = KIM_STATUS_OK;
    test_buffer = o;
}

void * KIM_API_model::get_sim_buffer(int* ier){
    *ier = KIM_STATUS_FAIL;
    if (test_buffer == NULL) return NULL;
    *ier = KIM_STATUS_OK;
    return test_buffer;
}


bool KIM_API_model::fij_related_things_match(KIM_API_model& test, KIM_API_model& mdl){

   bool tst_process_dEdr = is_it_in(test,(char*) "process_dEdr");
   bool tst_process_d2Edr2 = is_it_in(test,(char*) "process_d2Edr2");
   bool tst_virial_required  = is_it_in(test,(char*) "virial");
   bool tst_particleVirial_required = is_it_in(test,(char*) "particleVirial");
   bool tst_hessian_required =     is_it_in(test,(char*)"hessian");

    if ((tst_process_dEdr || tst_process_d2Edr2) && (tst_virial_required ||
                                                     tst_particleVirial_required ||
                                                     tst_hessian_required)) {
       std::cout << "* Error (KIM_API_model::fij_related_things_match): "
          "Simulator descriptor file cannot list both `process_dEdr' or "
          "`process_d2Edr2'  and any of `virial', `particleVirial', or `hessian'"
            << std::endl;
       return false;
    }

    bool mdl_process_dEdr = is_it_in(mdl,(char*) "process_dEdr");
    bool mdl_process_d2Edr2 = is_it_in(mdl,(char*) "process_d2Edr2");
    bool mdl_virial  = is_it_in(mdl,(char*) "virial");
    bool mdl_particleVirial = is_it_in(mdl,(char*) "particleVirial");
    bool mdl_hessian =     is_it_in(mdl,(char*) "hessian");

    if ((mdl_process_dEdr || mdl_process_d2Edr2) && (mdl_virial ||
                                                     mdl_particleVirial ||
                                                     mdl_hessian)) {
       std::cout << "* Error (KIM_API_model::fij_related_things_match): "
          "Model descriptor file cannot list both `process_dEdr' or "
          "`process_d2Edr2'  and any of `virial', `particleVirial', or `hessian'"
            << std::endl;
       return false;
    }

    bool virial_comp_possible = mdl_virial || mdl_process_dEdr;
    bool particleVirial_comp_possible = mdl_particleVirial || mdl_process_dEdr;
    bool hessian_comp_possible =  mdl_hessian || (mdl_process_dEdr && mdl_process_d2Edr2);

    //do test & model match?
    bool match = true;
    if (tst_virial_required ) if( !virial_comp_possible) match=false;
    if (tst_particleVirial_required ) if( !particleVirial_comp_possible) match=false;
    if (tst_hessian_required ) if( !hessian_comp_possible) match=false;
    if (tst_process_dEdr) if(!mdl_process_dEdr) match=false;
    if (tst_process_d2Edr2) if(!mdl_process_d2Edr2) match=false;

    // the match is set, now set flaggs

    if (tst_virial_required ) if (!mdl_virial) virial_need2add = true;

    if (tst_particleVirial_required ) if (!mdl_particleVirial) particleVirial_need2add = true;

    if (tst_hessian_required ) if (!mdl_hessian) hessian_need2add = true;

    if (tst_process_dEdr) test_doing_process_dEdr = true;
    if (tst_process_d2Edr2) test_doing_process_d2Edr2 = true;

    return match;

}

bool KIM_API_model::add_element(const char* instring){
        KIM_IOline inln;


        //open string as stream from char *
        std::string in_strstream=instring;
        std::stringstream myfile (in_strstream, std::stringstream::in|std::stringstream::out);
        if(!myfile){
            std::cout<<"* Error (KIM_API_model::add_element): can not access input string."<<std::endl;
            return false;
        }

        myfile.seekp(std::stringstream::beg);//set to the begining
        myfile >> inln;
        if(inln.goodformat) {
            this->inlines[numlines]=inln;
        }else{
            std::cout<<"* Error (KIM_API_model::add_element): bad format input string."<<std::endl;
            return false;
        }

        int *shape=NULL;

        KIMBaseElement *el = new KIMBaseElement ;
        int rank=inlines[numlines].get_rank();
        shape =inlines[numlines].get_shape();
        char * name =& (inlines[numlines].name[0]);
        char * type =& (inlines[numlines].type[0]);

        if (el->init(name,type,0,rank,shape)) //preinit element with zero size
        {
           strncpy(el->unit->dim,inlines[numlines].dim,strlen(inlines[numlines].dim)+1);

           el->flag->calculate = 1;
           el->flag->peratom = 1;//per something else
           if(inlines[numlines].isitperatom()) el->flag->peratom = 0; //per atom
           KIMBaseElement **pel =(KIMBaseElement**) model.data.p;
           pel[(int)model.size] =  el;
           delete [] shape;
           numlines ++;
           model.size++;
        }
        else
        {
           return false;
        }

        return true;
}

bool KIM_API_model::fij_related_things_add_set_index(){
    //add part
    if(virial_need2add){
        char instr[512] = "virial            double       pressure     ";
        strcat(instr,"    [6]           # automatically generated");
        if (!(this->add_element(instr))) return false;
    }
    if(particleVirial_need2add){
        char instr[512] = "particleVirial            double       pressure     ";
        strcat(instr,"    [numberOfParticles,6]           # automatically generated");
        if (!(this->add_element(instr))) return false;
    }
    if(hessian_need2add){
        char instr[512] = "hessian            double       pressure     ";
        strcat(instr,"    [numberOfParticles,numberOfParticles,3,3]     # automatically generated");
        if (!(this->add_element(instr))) return false;
    }


    //get index
    int error;
    virial_ind = get_index((char*) "virial", &error);
    particleVirial_ind = get_index((char*)"particleVirial", &error);
    hessian_ind =get_index((char*) "hessian", &error);
    process_dEdr_ind =get_index((char*) "process_dEdr", &error);
    process_d2Edr2_ind =get_index((char*) "process_d2Edr2", &error);


    // Set calculate flags for process_* if the API is doing the computations.
    if (virial_need2add || particleVirial_need2add || hessian_need2add) (*this)[process_dEdr_ind].flag->calculate=1;
    if (hessian_need2add) (*this)[process_d2Edr2_ind].flag->calculate=1;

    // Set calculate flags for process_* if the Test is doing the computations.
    if (test_doing_process_dEdr) (*this)[process_dEdr_ind].flag->calculate=1;
    if (test_doing_process_d2Edr2) (*this)[process_d2Edr2_ind].flag->calculate=1;

    return true;
}
int KIM_API_model::process_dEdr(KIM_API_model** ppkim, double* dE, double* r,
        double** dx,int *i, int *j){
   int ier = KIM_STATUS_OK;;
    KIM_API_model * pkim= *ppkim;
    typedef int (*Process_d1Edr)(KIM_API_model **, double *, double *, double **,int *,int *);

    Process_d1Edr process = (Process_d1Edr) (*pkim)[pkim->process_dEdr_ind].data.fp;
    int process_flag =0;
    process_flag = (*pkim)[pkim->process_dEdr_ind].flag->calculate;

    if (process != NULL && process_flag == 1 && pkim->model_index_shift == 0) {
        ier = (*process)(ppkim,dE,r,dx,i,j);
     }else if (process != NULL && process_flag == 1){
        int i2send = *i-pkim->model_index_shift;
        int j2send = *j-pkim->model_index_shift;
        ier = (*process)(ppkim,dE,r,dx,&i2send,&j2send);
    }else if (process_flag == 1 && pkim->AUX_index_shift == 0){
        ier = KIM_AUX::Process_DE::process_dEdr(ppkim,dE,r,dx,i,j);
    } else if(process_flag == 1){
        int i2send = *i-1;
        int j2send = *j-1;
        ier = KIM_AUX::Process_DE::process_dEdr(ppkim,dE,r,dx,&i2send,&j2send);
    }

    return ier;
}

int KIM_API_model::process_d2Edr2(KIM_API_model **ppkim,double *de,double **r,double ** pdx,int **i,int **j){
   int ier = KIM_STATUS_OK;
    KIM_API_model * pkim= *ppkim;
    typedef int (*Process_d2Edr)(KIM_API_model **, double *, double **, double **,int **,int **);

    Process_d2Edr process = (Process_d2Edr) (*pkim)[pkim->process_d2Edr2_ind].data.fp;
    int process_flag =0;
    process_flag = (*pkim)[pkim->process_d2Edr2_ind].flag->calculate;

    if (process != NULL && process_flag == 1 && pkim->model_index_shift == 0) {
       ier = (*process)(ppkim,de,r,pdx,i,j);
    }else if (process != NULL && process_flag == 1) {
        int k=pkim->model_index_shift;
        int i2send[2];   i2send[0]=(*i)[0]-k; i2send[1]=(*i)[1]-k;
        int j2send[2];   j2send[0]=(*j)[0]-k; j2send[1]=(*j)[1]-k;
        int *pi = &i2send[0];
        int *pj = &j2send[0];
        ier = (*process)(ppkim,de,r,pdx,&pi,&pj);
    } else if(process_flag == 1 && pkim->AUX_index_shift == 0){
       ier = KIM_AUX::Process_DE::process_d2Edr2(ppkim,de,r,pdx,i,j);
    }else if(process_flag == 1 ){
        int i2send[2];   i2send[0]=(*i)[0]-1; i2send[1]=(*i)[1]-1;
        int j2send[2];   j2send[0]=(*j)[0]-1; j2send[1]=(*j)[1]-1;
        int *pi = &i2send[0];
        int *pj = &j2send[0];
        ier = KIM_AUX::Process_DE::process_d2Edr2(ppkim,de,r,pdx,&pi,&pj);
    }

    return ier;
}


//related to Unit_Handling
double KIM_API_model::get_scale_conversion( const char* u_from,const char * u_to, int *error){
    return Unit_Handling::get_scale_conversion(u_from,u_to,error);
}
int KIM_API_model::get_unit_handling(int *error){
    return unit_h.get_unit_handling(error);
}
char * KIM_API_model::get_unit_length(int *error){
    return unit_h.get_unit_length(error);
}
char * KIM_API_model::get_unit_energy(int *error){
    return unit_h.get_unit_energy(error);
}
char * KIM_API_model::get_unit_charge(int *error){
    return unit_h.get_unit_charge(error);
}
char * KIM_API_model::get_unit_temperature(int *error){
    return unit_h.get_unit_temperature(error);
}
char * KIM_API_model::get_unit_time(int *error){
    return unit_h.get_unit_time(error);
}

double KIM_API_model::convert_to_act_unit(
   const char * length, const char* energy, const char* charge,
   const char * temperature, const char * time,
   double length_exponent, double energy_exponent, double charge_exponent,
   double temperature_exponent, double time_exponent, int* kimerror){
   return Unit_Handling::convert_to_act_unit((void *)this, length, energy, charge, temperature, time,
   length_exponent, energy_exponent, charge_exponent,  temperature_exponent, time_exponent, kimerror);
}

//multiple data set/get methods
//
void KIM_API_model::setm_data(int *err, int numargs, ... ){
    *err=KIM_STATUS_FAIL;
    va_list listPointer;
    va_start(listPointer,numargs);
    if(numargs % 4 != 0) {
        std::cout<<"setm_data: numargs must be multiple of 4"<<std::endl;
        *err=KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_4;
        va_end(listPointer);
        return;
    }

    for (int i=0; i<numargs/4; i++){
        char *nm      = va_arg(listPointer, char *);
        intptr_t size = va_arg(listPointer, intptr_t);
        void *dt      = va_arg(listPointer, void *);

        int key       =va_arg(listPointer, int);
        if (key != 1 && key != 0 ){
            *err= KIM_STATUS_WRONG_GROUP_ARGUMENT_KEY;
            va_end(listPointer);
            return;
        }else if(key ==0) continue;

        if(dt==NULL) std::cout<<"setm_data: WARNING: for "<<nm<<" data is NULL\n";
        if(!this->set_data(nm,size,dt)){
            std::cout<<"setm_data: set data for "<<nm<<" failed\n";
            va_end(listPointer);
            return;
        }
    }

    *err=KIM_STATUS_OK;
    va_end(listPointer);

}
void KIM_API_model::setm_method(int *err, int numargs, ... ){
    *err=KIM_STATUS_FAIL;
    va_list listPointer;
    va_start(listPointer,numargs);
    if(numargs % 4 != 0) {
        std::cout<<"setm_method: numargs must be multiple of 4"<<std::endl;
        *err=KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_4;
        va_end(listPointer);
        return;
    }

    for (int i=0; i<numargs/4; i++){
        char *nm      = va_arg(listPointer, char *);
        intptr_t size = va_arg(listPointer, intptr_t);
        func_ptr dt      = va_arg(listPointer, func_ptr);

        int key       =va_arg(listPointer, int);
        if (key != 1 && key != 0 ){
            *err= KIM_STATUS_WRONG_GROUP_ARGUMENT_KEY;
            va_end(listPointer);
            return;
        }else if(key ==0) continue;

        if(dt==NULL) std::cout<<"setm_method: WARNING: for "<<nm<<" data is NULL\n";
        if(!this->set_method(nm,size,dt)){
            std::cout<<"setm_method: set data for "<<nm<<" failed\n";
            va_end(listPointer);
            return;
        }
    }

    *err=KIM_STATUS_OK;
    va_end(listPointer);
}

void KIM_API_model::setm_data_by_index(int *err, int numargs, ... ){
    *err=KIM_STATUS_FAIL;
    va_list listPointer;
    va_start(listPointer,numargs);
    if(numargs % 4 != 0) {
        std::cout<<"setm_data_by_index: numargs must be multiple of 4"<<std::endl;
        *err=KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_4;
        va_end(listPointer);
        return;
    }

    for (int i=0; i<numargs/4; i++){
        int ind      = va_arg(listPointer, int);
        intptr_t size = va_arg(listPointer, intptr_t);
        void *dt      = va_arg(listPointer, void *);
        int key       =va_arg(listPointer, int);
        if (key != 1 && key != 0 ){
            *err= KIM_STATUS_WRONG_GROUP_ARGUMENT_KEY;
            va_end(listPointer);
            return;
        }else if(key ==0) continue;

        if(dt==NULL) std::cout<<"setm_data_by_index: WARNING: for argument group "<<i<<" data is NULL\n";

        if(!this->set_data_by_index(ind,size,dt)){
            std::cout<<"setm_data_by_index: set data for argument group"<<i<<" failed\n";
            va_end(listPointer);
            return;
        }
    }
    *err=KIM_STATUS_OK;
    va_end(listPointer);
}
void KIM_API_model::setm_method_by_index(int *err, int numargs, ... ){
    *err=KIM_STATUS_FAIL;
    va_list listPointer;
    va_start(listPointer,numargs);
    if(numargs % 4 != 0) {
        std::cout<<"setm_method_by_index: numargs must be multiple of 4"<<std::endl;
        *err=KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_4;
        va_end(listPointer);
        return;
    }

    for (int i=0; i<numargs/4; i++){
        int ind      = va_arg(listPointer, int);
        intptr_t size = va_arg(listPointer, intptr_t);
        func_ptr dt      = va_arg(listPointer, func_ptr);
        int key       =va_arg(listPointer, int);
        if (key != 1 && key != 0 ){
            *err= KIM_STATUS_WRONG_GROUP_ARGUMENT_KEY;
            va_end(listPointer);
            return;
        }else if(key ==0) continue;

        if(dt==NULL) std::cout<<"setm_data_by_index: WARNING: for argument group "<<i<<" data is NULL\n";

        if(!this->set_method_by_index(ind,size,dt)){
            std::cout<<"setm_data_by_index: set data for argument group"<<i<<" failed\n";
            va_end(listPointer);
            return;
        }
    }
    *err=KIM_STATUS_OK;
    va_end(listPointer);
}

void KIM_API_model::getm_data(int *err,int numargs, ...){

    *err=KIM_STATUS_FAIL;
    va_list listPointer;
    va_start(listPointer,numargs);
    if(numargs % 3 != 0) {
        std::cout<<"getm_data: numargs must be multiple of 3"<<std::endl;
        *err=KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_3;
        va_end(listPointer);
        return;
    }

    for (int i=0; i<numargs/3; i++){
        char *nm      = va_arg(listPointer, char *);
        void **dt      = va_arg(listPointer, void **);
        int key       =va_arg(listPointer, int);
        if (key != 1 && key != 0 ){
            *err= KIM_STATUS_WRONG_GROUP_ARGUMENT_KEY;
            va_end(listPointer);
            return;
        }else if(key ==0) continue;

        *dt = this->get_data(nm,err);
        if(*err != KIM_STATUS_OK){
            std::cout<<"getm_data: get data for "<<nm<<" failed\n";
            va_end(listPointer);
            return;
        }
    }

    *err=KIM_STATUS_OK;
    va_end(listPointer);
}
void KIM_API_model::getm_method(int *err,int numargs, ...){

    *err=KIM_STATUS_FAIL;
    va_list listPointer;
    va_start(listPointer,numargs);
    if(numargs % 3 != 0) {
        std::cout<<"getm_data: numargs must be multiple of 3"<<std::endl;
        *err=KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_3;
        va_end(listPointer);
        return;
    }

    for (int i=0; i<numargs/3; i++){
        char *nm      = va_arg(listPointer, char *);
        func_ptr *dt      = va_arg(listPointer, func_ptr *);
        int key       =va_arg(listPointer, int);
        if (key != 1 && key != 0 ){
            *err= KIM_STATUS_WRONG_GROUP_ARGUMENT_KEY;
            va_end(listPointer);
            return;
        }else if(key ==0) continue;

        *dt = this->get_method(nm,err);
        if(*err != KIM_STATUS_OK){
            std::cout<<"getm_data: get data for "<<nm<<" failed\n";
            va_end(listPointer);
            return;
        }
    }

    *err=KIM_STATUS_OK;
    va_end(listPointer);
}

void KIM_API_model::getm_data_by_index(int *err,int numargs, ...){
    *err=KIM_STATUS_FAIL;
    va_list listPointer;
    va_start(listPointer,numargs);
    if(numargs % 3 != 0) {
        std::cout<<"getm_data_by_index: numargs must be multiple of 3"<<std::endl;
        *err=KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_3;
        va_end(listPointer);
        return;
    }

    for (int i=0; i<numargs/3; i++){
        int ind      = va_arg(listPointer, int);
        void **dt      = va_arg(listPointer, void **);

        int key       =va_arg(listPointer, int);
        if (key != 1 && key != 0 ){
            *err= KIM_STATUS_WRONG_GROUP_ARGUMENT_KEY;
            va_end(listPointer);
            return;
        }else if(key ==0) continue;

        *dt = this->get_data_by_index(ind,err);
        if(*err != KIM_STATUS_OK){
            std::cout<<"getm_data_by_index: get data for argument group "<<i<<" failed\n";
            va_end(listPointer);
            return;
        }
    }

    *err=KIM_STATUS_OK;
    va_end(listPointer);
}
void KIM_API_model::getm_method_by_index(int *err,int numargs, ...){
    *err=KIM_STATUS_FAIL;
    va_list listPointer;
    va_start(listPointer,numargs);
    if(numargs % 3 != 0) {
        std::cout<<"getm_data_by_index: numargs must be multiple of 3"<<std::endl;
        *err=KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_3;
        va_end(listPointer);
        return;
    }

    for (int i=0; i<numargs/3; i++){
        int ind      = va_arg(listPointer, int);
        func_ptr *dt      = va_arg(listPointer, func_ptr *);

        int key       =va_arg(listPointer, int);
        if (key != 1 && key != 0 ){
            *err= KIM_STATUS_WRONG_GROUP_ARGUMENT_KEY;
            va_end(listPointer);
            return;
        }else if(key ==0) continue;

        *dt = this->get_method_by_index(ind,err);
        if(*err != KIM_STATUS_OK){
            std::cout<<"getm_data_by_index: get data for argument group "<<i<<" failed\n";
            va_end(listPointer);
            return;
        }
    }

    *err=KIM_STATUS_OK;
    va_end(listPointer);
}

void KIM_API_model::getm_index(int *err, int numargs, ...){
     *err=KIM_STATUS_FAIL;
    va_list listPointer;
    va_start(listPointer,numargs);

    if(numargs % 3 != 0) {
        std::cout<<"getm_index: numargs must be multiple of 3"<<std::endl;
        *err=KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_3;
        va_end(listPointer);
        return;
    }

    for (int i=0; i<numargs/3; i++){
        char *nm      = va_arg(listPointer, char *);
        int *ind      = va_arg(listPointer, int *);

        int key       =va_arg(listPointer, int);
        if (key != 1 && key != 0 ){
            *err= KIM_STATUS_WRONG_GROUP_ARGUMENT_KEY;
            va_end(listPointer);
            return;
        }else if(key ==0) continue;

        *ind = this->get_index(nm,err);
        if(*err != KIM_STATUS_OK){
            std::cout<<"getm_index: get index for "<<nm<<" failed\n";
            va_end(listPointer);
            return;
        }
    }

    *err=KIM_STATUS_OK;
    va_end(listPointer);

}

void KIM_API_model::setm_compute(int *err, int numargs, ...){
     *err=KIM_STATUS_FAIL;
    va_list listPointer;
    va_start(listPointer,numargs);
    if(numargs % 3 != 0) {
        std::cout<<"setm_compute: numargs must be multiple of 3"<<std::endl;
        *err=KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_3;
        va_end(listPointer);
        return;
    }

    for (int i=0; i<numargs/3; i++){
        char *nm      = va_arg(listPointer, char *);
        int compute_flag = va_arg(listPointer, int);

        int key       =va_arg(listPointer, int);
        if (key != 1 && key != 0 ){
            *err= KIM_STATUS_WRONG_GROUP_ARGUMENT_KEY;
            va_end(listPointer);
            return;
        }else if(key ==0) continue;

        int index = this->get_index(nm,err);
        if (*err != KIM_STATUS_OK){
           std::cout<<"setm_compute:  name "<<nm<<" not in KIM\n";
           va_end(listPointer);
           return;
        }
        if (compute_flag ==1){
            (*this)[index].flag->calculate = 1;
        }else if (compute_flag ==0){
            (*this)[index].flag->calculate = 0;
        }else{
            std::cout<<"setm_compute:  for "<<nm<<" failed: compute_flag must be 0 or 1\n";
            va_end(listPointer);
            return;
        }
    }

    *err=KIM_STATUS_OK;
    va_end(listPointer);
}

void KIM_API_model::setm_compute_by_index(int* err, int numargs, ...){
      *err=KIM_STATUS_OK;
    va_list listPointer;
    va_start(listPointer,numargs);
    if(numargs % 3 != 0) {
        std::cout<<"setm_compute_by_index: numargs must be multiple of 3"<<std::endl;
        *err=KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_3;
        va_end(listPointer);
        return;
    }

    for (int i=0; i<numargs/3; i++){
        int index      = va_arg(listPointer, int);
        int compute_flag = va_arg(listPointer, int);

        int key       =va_arg(listPointer, int);
        if (key != 1 && key != 0 ){
            *err= KIM_STATUS_WRONG_GROUP_ARGUMENT_KEY;
            va_end(listPointer);
            return;
        }else if(key ==0) continue;

        if (index < 0 || index >= this->model.size) *err=KIM_STATUS_FAIL;
        if (*err != KIM_STATUS_OK){
           std::cout<<"setm_compute_by_index:  for argument group "<<i<<" failed\n";
           va_end(listPointer);
           return;
        }
        if (compute_flag ==1){
            (*this)[index].flag->calculate = 1;
        }else if (compute_flag ==0){
            (*this)[index].flag->calculate = 0;
        }else{
            std::cout<<"setm_compute_by_index:  for argument group "<<i<<" failed: compute_flag must be 0 or 1\n";
            *err=KIM_STATUS_FAIL;
            va_end(listPointer);
            return;
        }
    }

    *err=KIM_STATUS_OK;
    va_end(listPointer);
}

void KIM_API_model::getm_compute(int *err,int numargs, ...){
     *err=KIM_STATUS_FAIL;
    va_list listPointer;
    va_start(listPointer,numargs);
    if(numargs % 3 != 0) {
        std::cout<<"getm_compute: numargs must be multiple of 3"<<std::endl;
        *err=KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_3;
        va_end(listPointer);
        return;
    }

    for (int i=0; i<numargs/3; i++){
        char *nm      = va_arg(listPointer, char *);
        int *compute_flag = va_arg(listPointer, int*);

        int key       =va_arg(listPointer, int);
        if (key != 1 && key != 0 ){
            *err= KIM_STATUS_WRONG_GROUP_ARGUMENT_KEY;
            va_end(listPointer);
            return;
        }else if(key ==0) continue;

        int index = this->get_index(nm,err);
        if (*err != KIM_STATUS_OK){
           std::cout<<"getm_compute:  name "<<nm<<" not in KIM\n";
           va_end(listPointer);
           return;
        }
        *compute_flag =(*this)[index].flag->calculate;
    }

    *err=KIM_STATUS_OK;
    va_end(listPointer);
}

void KIM_API_model::getm_compute_by_index(int* err, int numargs, ...){
    *err=KIM_STATUS_OK;
    va_list listPointer;
    va_start(listPointer,numargs);
    if(numargs % 3 != 0) {
        std::cout<<"getm_compute_by_index: numargs must be multiple of 3"<<std::endl;
        *err=KIM_STATUS_NUMARGS_NOT_DIVISIBLE_BY_3;
        va_end(listPointer);
        return;
    }

    for (int i=0; i<numargs/3; i++){
        int index      = va_arg(listPointer, int);
        int *compute_flag = va_arg(listPointer, int*);

        int key       =va_arg(listPointer, int);
        if (key != 1 && key != 0 ){
            *err= KIM_STATUS_WRONG_GROUP_ARGUMENT_KEY;
            va_end(listPointer);
            return;
        }else if(key ==0) continue;

        if (index < 0 || index >= this->model.size) *err=KIM_STATUS_FAIL;
        if (*err != KIM_STATUS_OK){
           std::cout<<"getm_compute_by_index:  for argument group "<<i<<" failed\n";
           va_end(listPointer);
           return;
        }
        *compute_flag = (*this)[index].flag->calculate;

    }

    *err=KIM_STATUS_OK;
    va_end(listPointer);
}

void KIM_API_model::print(int* error){
    *error =KIM_STATUS_FAIL;
    if (this==NULL) return;
    std::cout<<(*this);
    *error=KIM_STATUS_OK;
}

intptr_t KIM_API_model::get_size_by_index(int I,int *error){
    *error =KIM_STATUS_FAIL;
    if (this == NULL) return 0;
    *error =KIM_STATUS_OK;
    return (*this)[I].size;
}

intptr_t KIM_API_model::get_rank_by_index(int I,int *error){
    *error =KIM_STATUS_FAIL;
    if (this == NULL) return 0;
    *error =KIM_STATUS_OK;
    return (*this)[I].rank;
}

intptr_t KIM_API_model::get_shape_by_index(int I, int * shape,int *error){
     *error =KIM_STATUS_OK;
    if (this == NULL) return -2;
    *error =1;
     if((*this)[I].rank == 0){
            return 0;
        }else if((*this)[I].rank ==1){
            shape[0] = (int)(*this)[I].size;
            return 1;
        }else if((*this)[I].rank>1){
            for (int i=0; i< (*this)[I].rank; i++) shape[i] =(*this)[I].shape[i];
            return (*this)[I].rank;
        }else{
            *error =KIM_STATUS_FAIL;
            return -1;
        }
}

int KIM_API_model::get_compute_by_index(int I,int * error){
    *error = KIM_STATUS_FAIL;
    if ((I < 0) || (I >= model.size)) return KIM_STATUS_ARG_UNKNOWN;
    *error = KIM_STATUS_OK;
    return (*this)[I].flag->calculate;
 }
