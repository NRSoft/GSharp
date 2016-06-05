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
 */
#include <string>
#include "version.h"
#include "gsharp.h"
#include "gsharp_program.h"
#include "gsharp_except.h"

using namespace gsharp;


Interpreter::Interpreter()
{
   _interpreter = new Program;
}


Interpreter::~Interpreter()
{
   delete (Program*)_interpreter;
}


const char* Interpreter::GetVersionStr() const
{
   return GSHARP_FULLVERSION_STRING;
}


void Interpreter::GetVersion(int& major, int& minor, int& build, int& rev) const
{
   major = GSHARP_MAJOR;
   minor = GSHARP_MINOR;
   build = GSHARP_BUILD;
   rev = GSHARP_REVISION;
}


void Interpreter::Load(const string& code)
{
   try{ ((Program*)_interpreter)->Load(code); }
   catch(ErrorMsg& err){ throw err.what(); }
}


bool Interpreter::Step(string& line, ExtraInfo& extra)
{
   try{ return ((Program*)_interpreter)->Step(line, extra); }
   catch(ErrorMsg& err){ throw err.what(); }
}


void Interpreter::Rewind()
{
   try{ ((Program*)_interpreter)->Rewind(); }
   catch(ErrorMsg& err){ throw err.what(); }
}


void Interpreter::Clear()
{
   try{ ((Program*)_interpreter)->Clear(); }
   catch(ErrorMsg& err){ throw err.what(); }
}


void Interpreter::EnableBlockDelete(bool enable)
{
   try{ ((Program*)_interpreter)->EnableBlockDelete(enable); }
   catch(ErrorMsg& err){ throw err.what(); }
}


void Interpreter::EnablePrettyFormat(bool enable)
{
   try{ ((Program*)_interpreter)->EnablePrettyFormat(enable); }
   catch(ErrorMsg& err){ throw err.what(); }
}


void Interpreter::EnableConvertToUpper(bool enable)
{
   try{ ((Program*)_interpreter)->EnableConvertToUpper(enable); }
   catch(ErrorMsg& err){ throw err.what(); }
}


void Interpreter::SetParam(unsigned int number, double value)
{
   try{ ((Program*)_interpreter)->SetParam(number, value); }
   catch(ErrorMsg& err){ throw err.what(); }
}


double Interpreter::GetParam(unsigned int number) const
{
   try{ return ((Program*)_interpreter)->GetParam(number); }
   catch(ErrorMsg& err){ throw err.what(); }
}


const std::string Interpreter::GetSourceLine(unsigned int num) const
{
   try{ return ((Program*)_interpreter)->GetSourceLine(num); }
   catch(ErrorMsg& err){ throw err.what(); }
}


unsigned int Interpreter::GetCurrentLineNumber() const
{
   try{ return ((Program*)_interpreter)->GetCurrentLineNumber(); }
   catch(ErrorMsg& err){ throw err.what(); }
}
