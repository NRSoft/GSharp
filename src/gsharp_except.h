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
#ifndef GSHARP_EXCEPT_H_INCLUDED
#define GSHARP_EXCEPT_H_INCLUDED

//#include <ctime>
#include <cstdarg>
#include <exception>

#include "gsharp_program.h"

namespace gsharp
{

/////////  class  E r r o r M s g  ////////
// Captured as an exception during abnormal code line or internal interpreter operation
class ErrorMsg: public exception
{
public:
//TODO: convert interface to C++ streaming style (from old C 'printf' style)?
   ErrorMsg(const Program* p, const char* fmt, ...)
   {
      va_list args;
      va_start(args, fmt);
      va_list args2;
      va_copy(args2, args);
      buf.resize(16+std::vsnprintf(NULL, 0, fmt, args));
      int written = snprintf(buf.data(), 16, "(%d): ", p->GetCurrentLineNumber());
      vsnprintf(buf.data()+written, buf.size(), fmt, args2);
   }

   virtual ~ErrorMsg() {}

   virtual const char* what() const noexcept
   {
      return buf.data();
   }

protected:
   vector<char> buf;

private:
};

/*
// SAMPLE WITH THE TIMESTAMP (for reference)
void debug_log(const char *fmt, ...)
{
    std::time_t t = std::time(nullptr);
    char time_buf[100];
    std::strftime(time_buf, sizeof time_buf, "%D %T", std::gmtime(&t));
    va_list args1;
    va_start(args1, fmt);
    va_list args2;
    va_copy(args2, args1);
    std::vector<char> buf(1+std::vsnprintf(NULL, 0, fmt, args1));
    va_end(args1);
    std::vsnprintf(buf.data(), buf.size(), fmt, args2);
    va_end(args2);
    std::printf("%s [debug]: %s\n", time_buf, buf.data());
}
*/

} // namespace
#endif // GSHARP_EXCEPT_H_INCLUDED
