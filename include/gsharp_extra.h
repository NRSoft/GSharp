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
 */
#ifndef GSHARP_EXTRA_H_INCLUDED
#define GSHARP_EXTRA_H_INCLUDED

#include <string>

namespace gsharp
{

using namespace std;

/////////  class  E x t r a I n f o  //////////
// Manages messages produced during the normal run of the interpreter
class ExtraInfo
{
public:
   enum Type: unsigned int {MSG=0, PRN, DBG, LOG, TOTAL_TYPES}; // keep the order for iteration

   /////////  R e t r i e v e  ////////
   // If no messages of a particular type during the last operation, returns nullptr
   inline const char* Retrieve(Type type) noexcept
   {
      if(type >= TOTAL_TYPES || _msg[type].empty())
         return nullptr;
      _output.clear(); // erase msg
      _output.swap(_msg[type]);
      return _output.c_str();
   }

   /////////  A s s i g n  ////////
   inline void Assign(Type type, const string& msg)
   {
      if(type < TOTAL_TYPES) _msg[type] = msg;
   }

   //////////  C l e a r  //////////
   inline void Clear(Type type = TOTAL_TYPES)
   {
      if(type < TOTAL_TYPES)
         _msg[type].clear();
      else
         for(string& m: _msg) m.clear();
   }

   ///////  F i r s t  N o n  E m p t y  ///////
   inline bool FirstNonEmpty(Type* type = nullptr) const
   {
      for(unsigned int t=0; t<TOTAL_TYPES; ++t)
         if(!_msg[t].empty()){
            if(type != nullptr)
               *type = static_cast<Type>(t);
            return true;
         }
      return false;
   }

protected:
   string _msg[TOTAL_TYPES];
   string _output; // to keep the last
};


} // namespace

#endif // GSHARP_EXTRA_H_INCLUDED
