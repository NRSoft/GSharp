/*
 *  Copyright 2016, Night Road Software (https://github.com/nrsoft)
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *  
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *      * Neither the name of "Night Road Software" nor the names of its
 *  contributors may be used to endorse or promote products derived from
 *  this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  *********************************************************************
 *
 *  Interpreter library of G# macro-programming language into plain G-code lines
 *
 *  G# is a subset of LinuxCNC (https://github.com/LinuxCNC/linuxcnc)
 *      and RS274/NGC (https://www.nist.gov/customcf/get_pdf.cfm?pub_id=823374)
 *
 *  The major differences to the above standards:
 *   - does NOT support <named_parameters> and correspondingly no EXIST[] funcion
 *   - all numbered parameters are volatile (TODO: make some of them persistent, as required)
 *   - two '%' demarcation lines can appear anywhere in the program,
 *      first % line marks the start, second - stop of the execution
 *   - O-subs can be located anywhere in the code: they are executed only if called and
 *      jumped over in all other cases
 *   - O-call can NOT be issued to a subroutine located in a separate file
 *   - O-endsub and O-return can return a value, but it will be stored in parameter #5000
 *   - comments can be anywhere in any line (except for % lines: only after % character),
 *   - comments can contain pairs of brackets "()", but not a non-matched single bracket
 *   - (PROBE*) comments are ignored as not relevant (should be managed by the machine control system)
 */
#ifndef GSHARP_H_INCLUDED
#define GSHARP_H_INCLUDED

#include <string>
#include "gsharp_extra.h"


namespace gsharp
{

// Any error generated during the interpreter operation(s) will be thrown
//  and have to be caught in the calling application, for example:
//
//  Interpreter interp;
//  std::string program;
//   ... // load ngc file into <program> string
//  try{
//     interp.Load(program);
//  }
//  catch(const char* e){
//     std::cout << "Error: " << e << std::endl;
//  }
//
// The error string contains the current line number in the brackets and the description
// Load() and Step() functions must be wrapped into the try-catch block
//

class Interpreter
{
public:
   Interpreter();
   virtual ~Interpreter();

   // current library version
   const char* GetVersionStr() const;
   void GetVersion(int& major, int& minor, int& build, int& rev) const;

   // load the complete program code, global parameters remain untouched
   void Load(const std::string& code);

   // perform next step in the execution
   // produces the next plain g-code line which is to appear in sequence
   // false on return means that there are no more lines left to process
   // extra messages can be present at any step, even if return is true and the line is empty
   bool Step(std::string& line, ExtraInfo& extra);

   // to be able to restart program execution again, global parameters remain untouched
   void Rewind();

   // reset global parameters to zero
   void Clear();

   // call to enable g-code 'block delete' feature (default = disabled)
   void EnableBlockDelete(bool enable=true);

   // call to enable spaces between g-code words (default = enabled)
   void EnablePrettyFormat(bool enable=true);

   // call to enable generating output in uppercase, otherwise lowercase (default = enabled)
   void EnableConvertToUpper(bool enable=true);

   // assign value to specific parameter (can be called between steps e.g. for debugging)
   void SetParam(unsigned int number, double value);

   // retrieve the current value of specific parameter (e.g. for debging)
   double GetParam(unsigned int number) const;

   // retrieve the source line
   const std::string GetSourceLine(unsigned int num) const;

   // retrieve the current line number (during execution)
   unsigned int GetCurrentLineNumber() const;

private:
   void* _interpreter; // implementation
};

} // namespace

#endif // GSHARP_H_INCLUDED
