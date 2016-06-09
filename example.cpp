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
 *  Stand-alone converter of G# macro-programming language into plain G-Code
 *
 *  G# is a subset of LinuxCNC (https://github.com/LinuxCNC/linuxcnc)
 *    and RS274/NGC (https://www.nist.gov/customcf/get_pdf.cfm?pub_id=823374)
 *
 *  For compatibility with the above standards please visit
 *    https://github.com/nrsoft/gsharp or check "gsharp.h" header
 *
 *  To compile this example run:
 *    g++ example.cpp -Iinclude -Llib -lgsharp -og#2g -Wall -std=c++11
 *
 */

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "gsharp.h"
#include "gsharp_extra.h"

using namespace std;

int main(int argc, char* argv[])
{
   gsharp::Interpreter r;

   cout << "G#2G: CNC G# macro-code converter to plain G-Code" << endl;
   cout << " Using libgsharp ver " << r.GetVersionStr() << "" << endl;
   cout << " More info at https://github.com/nrsoft/gsharp" << endl << endl;
   if(argc < 3){
      cout << "Usage: " << endl;
      cout << " g#2g <input_file> <output_file>" << endl << endl;
      return 1;
   }

   // open input file
   string filename(argv[1]);
   ifstream file_in(filename, ifstream::in);
   if(!file_in.good()){
      cout << "Cannot open file: " << filename << endl;
      return 1;
   }

   // read file into string stream
   stringstream ss;
   ss << file_in.rdbuf();

   // Load program
   try{
      r.Load(ss.str());
   }
   catch(exception& e){
      cout << "File parsing error: " << e.what() << endl;
      return 1;
   }

   // create output file
   filename = argv[2];
   ofstream file_out(filename, ifstream::out);
   if(!file_out.good()){
      cout << "Cannot create file: " << filename << endl;
      return 1;
   }

   // run interpreter
   string str;
   gsharp::ExtraInfo extra;
   try{
      while(r.Step(str, extra)){
         // record next G-Code line
         if(!str.empty()){
            file_out << str << endl;
            //cout << "(" << r.GetCurrentLineNumber() << "): " << str << endl; // console check
         }
         // any messages to display?
         gsharp::ExtraInfo::Type t;
         while(extra.FirstNonEmpty(&t)){
            switch(t){
               case gsharp::ExtraInfo::PRN:
                  cout << "PRN";
                  break;
               case gsharp::ExtraInfo::DBG:
                  cout << "DBG";
                  break;
               case gsharp::ExtraInfo::LOG:
                  cout << "LOG";
                  break;
               default:
                  cout << "MSG";
            }
            cout << ": " << extra.Retrieve(t) << endl;
         }
      }
   }
   catch(exception& e){
      cout << "Interpreter error: " << e.what() << endl << endl;
      return 1;
   }

   return 0;
}
