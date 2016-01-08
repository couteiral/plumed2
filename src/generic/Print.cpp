/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Copyright (c) 2011-2014 The plumed team
   (see the PEOPLE file at the root of the distribution for a list of names)

   See http://www.plumed.org for more information.

   This file is part of plumed, version 2.

   plumed is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   plumed is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with plumed.  If not, see <http://www.gnu.org/licenses/>.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#include "core/ActionPilot.h"
#include "core/ActionWithArguments.h"
#include "core/ActionRegister.h"

using namespace std;

namespace PLMD{
namespace generic{

//+PLUMEDOC ANALYSIS PRINT
/*
Print quantities to a file.

This directive can be used multiple times
in the input so you can print files with different strides or print different quantities 
to different files.  You can control the buffering of output using the \subpage FLUSH keyword.

\par Examples
The following input instructs plumed to print the distance between atoms 3 and 5 on a file 
called COLVAR every 10 steps, and the distance and total energy on a file called COLVAR_ALL
every 1000 steps.
\verbatim
DISTANCE ATOMS=2,5 LABEL=distance
ENERGY             LABEL=energy
PRINT ARG=distance          STRIDE=10   FILE=COLVAR
PRINT ARG=distance,energy   STRIDE=1000 FILE=COLVAR_ALL
\endverbatim
(See also \ref DISTANCE and \ref ENERGY).

*/
//+ENDPLUMEDOC

class Print :
public ActionPilot,
public ActionWithArguments
{
  string file;
  OFile ofile;
  string fmt;
// small internal utility
/////////////////////////////////////////
// these are crazy things just for debug:
// they allow to change regularly the
// printed argument
  int rotate;
  int rotateCountdown;
  int rotateLast;
  vector<Value*> rotateArguments;
/////////////////////////////////////////
public:
  void calculate(){}
  void prepare();
  Print(const ActionOptions&);
  static void registerKeywords(Keywords& keys);
  void apply(){}
  void update();
  ~Print();
};

PLUMED_REGISTER_ACTION(Print,"PRINT")

void Print::registerKeywords(Keywords& keys){
  Action::registerKeywords(keys);
  ActionPilot::registerKeywords(keys);
  ActionWithArguments::registerKeywords(keys);
  keys.use("ARG");
  keys.add("compulsory","STRIDE","1","the frequency with which the quantities of interest should be output");
  keys.add("optional","FILE","the name of the file on which to output these quantities");
  keys.add("optional","FMT","the format that should be used to output real numbers");
  keys.add("hidden","_ROTATE","some funky thing implemented by GBussi");
}

Print::Print(const ActionOptions&ao):
Action(ao),
ActionPilot(ao),
ActionWithArguments(ao),
fmt("%f"),
rotate(0)
{
  ofile.link(*this);
  parse("FILE",file);
  if(file.length()>0){
    ofile.open(file);
    log.printf("  on file %s\n",file.c_str());
  } else {
    log.printf("  on plumed log file\n");
    ofile.link(log);
  }
  parse("FMT",fmt);
  fmt=" "+fmt;
  log.printf("  with format %s\n",fmt.c_str());
  for(unsigned i=0;i<getNumberOfArguments();++i) ofile.setupPrintValue( getPntrToArgument(i) );
/////////////////////////////////////////
// these are crazy things just for debug:
// they allow to change regularly the
// printed argument
  parse("_ROTATE",rotate);
  if(rotate>0){
    rotateCountdown=rotate;
    for(unsigned i=0;i<getNumberOfArguments();++i) rotateArguments.push_back( getPntrToArgument(i) );
    vector<Value*> a(1,rotateArguments[0]);
    requestArguments(vector<Value*>(1,rotateArguments[0]));
    rotateLast=0;
  }
/////////////////////////////////////////
  checkRead();
}

void Print::prepare(){
/////////////////////////////////////////
// these are crazy things just for debug:
// they allow to change regularly the
// printed argument
  if(rotate>0){
    rotateCountdown--;
    if(rotateCountdown==0){
      rotateCountdown=rotate;
      rotateLast++;
      rotateLast%=rotateArguments.size();
      requestArguments(vector<Value*>(1,rotateArguments[rotateLast]));
    }
  }
/////////////////////////////////////////
}

void Print::update(){
      ofile.fmtField(" %f");
      ofile.printField("time",getTime());
      for(unsigned i=0;i<getNumberOfArguments();i++){
        ofile.fmtField(fmt);
        ofile.printField( getPntrToArgument(i), getArgument(i) );
        //ofile.printField(getPntrToArgument(i)->getName(),getArgument(i));
      }
      ofile.printField();
}

Print::~Print(){
}

}


}
