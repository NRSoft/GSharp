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
#ifndef GSHARP_PROGRAM_H_INCLUDED
#define GSHARP_PROGRAM_H_INCLUDED

#include <cmath>
#include <array>
#include <vector>
#include <string>
#include <stack>
#include <unordered_map>
#include "gsharp_extra.h"

#ifdef TEST_BUILD
#include "../test/gsharp_test.h"
#endif // TEST_BUILD

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace gsharp
{

using namespace std;

typedef unsigned int ONumber;
typedef unsigned int LineNumber; // all valid LineNumbers start from 1, anyhwere in the code !!!

typedef struct
{
   enum {UNDEF, SUB, IF, DO, WHILE, REPEAT} type;
   LineNumber start_line; // where this code block starts
   vector<LineNumber> mid_line; // all internal lines (elseif, else)
   LineNumber end_line; // the first line after(!) the block
   int run_times;
} CodeBlock;


///////  class  P r o g r a m  ////////
// storage for the program code
// each instance keeps one subroutine (or main program)
class Program
{
public:
   const static size_t TOTAL_CNC_PARAMETERS = 5602; // defined in LinuxCNC and RS274/NGC standard
   const static size_t TOTAL_LOCAL_PARAMETERS = 30; // within the TOTAL_CNC_PARAMETERS number
   const static size_t TOTAL_INTERNAL_PARAMETERS = 50; // max possible to parse a single line
   const static size_t TOTAL_PARAMETERS = TOTAL_CNC_PARAMETERS + TOTAL_INTERNAL_PARAMETERS;
   const static size_t INTERNAL_PARAMETERS_START = TOTAL_CNC_PARAMETERS; // above the valid range of CNC parameters
   const static size_t MAX_STACK_LEVELS = 1000; // TODO: define real & safe stack depth
   const static size_t RETURN_VALUE_PARAMETER = 5000; // if any sub returns value, it is stored here
   const double TOLERANCE_EQUAL = 0.0001; // defined in LinuxCNC for comparison of doubles
   const static bool USE_BLOCK_DELETE = false; // disabled by default
   const static bool USE_PRETTY_FORMAT = true; // enabled: add spaces between g-words
   const static bool CONVERT_TO_UPPER = true; // enabled: all output characters are in upper case

public:
    // stores the program, extracts sub-routines as separate routines
   Program();
   virtual ~Program() {}

   // load the program code
   void Load(const string& code);

   // produce next g-code line in sequence
   // false on return means that there are no more lines left
   // extra messages can be present, even if the line is empty
   bool Step(string& line, ExtraInfo& extra);

   void Rewind(); // to start program over again

   inline void Clear() {_params.fill(0);} // clears global paramteres

   inline void EnableBlockDelete(bool enable=true) {_block_delete = enable;}
   inline void EnablePrettyFormat(bool enable=true) {_format_pretty = enable;}
   inline void EnableConvertToUpper(bool enable=true) {_convert_to_upper = enable;}

   void SetParam(unsigned int number, double value);
   double GetParam(unsigned int number) const;

   inline LineNumber GetCurrentLineNumber() const {return _last_used_line;}
   const string GetSourceLine(LineNumber num) const;

   // Levels of debug output to stdout:
   // 0:   no debug messages
   // 1:   basic messages
   // 2:   relatively verbouse
   // 3:   detailed messaging
   // >3:  all messages, including very specific local ones (may flood stdout)
   inline void DebugLevel(unsigned int level) {_debug_level = level;}

protected:
   vector<string> _code; // the program code split into lines (incl. empty)

   LineNumber _current_line; // current line number in the code for parsing
   LineNumber _last_used_line; // the number of the last line at which execution paused

   unordered_map<ONumber, CodeBlock> _blocks; // list of o-blocks with parameters

   array<double, TOTAL_PARAMETERS> _params; // global access (valid id > TOTAL_LOCAL_PARAMETERS)
   array<double, TOTAL_LOCAL_PARAMETERS> _local_params;
   size_t _current_internal_param; // only for the interpreter use itself

   // call stack for subroutines
   stack<array<double, TOTAL_LOCAL_PARAMETERS>> _param_stack;
   stack<LineNumber> _return_stack;

   ExtraInfo _extra; // any active comments during execution? They are stores here

   bool _block_delete; // disable lines starting with '/'?
   bool _format_pretty; // place spaces between g-code words?
   bool _convert_to_upper; // output charcters in upper case?

   LineNumber _percent_start;
   LineNumber _percent_stop;
   bool _percent_active;

   unsigned int _debug_level;

private:
   // major parsing functions
   void _ProcessComments(string& line, ExtraInfo* extra=nullptr);
   void _PrepareLine(string& line); // check for abnormal symbols, remove whitespaces, convert to lowercase
   void _RemoveNword(string& str); // if there is an N-word infront, remove it
   bool _ReadOword(string& str, ONumber& number, string& command);
   void _SimplifyOperators(string& line); // replace with single-character operators
   void _CalculateExpressions(string& line, vector<double>* output = nullptr);
   void _ResolveParameters(string& line, int precision=4);
   const string _ParseLine(const string& line, int precision=4); // for test routines (no o-codes)

   // expression parsing support functions
   double _ParseLogical(const string& expr);        // AND, OR, XOR
   double _ParseConditional(const string& expr);    // EQ, NE, GT, GE, LT, LE
   double _ParseSummation(const string& expr);      // +, -
   double _ParseMultiplication(const string& expr); // *, /, MOD
   double _ParsePower(const string& expr);          // **
   double _ParseOperand(const string& expr);        // direct value or parameter

   // other parsing support functions
   void   _ReplaceWithParameter(double value, string& line, size_t start, size_t len);
   double _ReadParameter(const string& line, size_t pos, size_t& len);
   void   _AssignParameter(const string& line, size_t pos, size_t& len);
   void   _ReplaceSingleOperator(string& line, const string& operation, char replacement);
   double _CalculateExpressionFromBracket(const string& line, size_t start, size_t& len);
   double _ApplyAsFunctionArgument(double arg, const string& line, size_t& start, size_t& len);
   double _EvaluateExpression(string& expr);
   double _ParseExpression(const string& expr);
   void   _FormatPretty(string& line);

   // degrees - radian conversion
   inline double _radians (double degrees) const { return degrees * M_PI / 180; }
   inline double _degrees (double radians) const { return radians * 180 / M_PI; }

private:

#ifdef TEST_BUILD
   // allow unit tests to access private members
   FRIEND_TEST(GSharpTest, ParseExpression);
   FRIEND_TEST(GSharpTest, ParseParameters);
#endif // TEST_BUILD
};

} // namespace gsharp

#endif // GSHARP_PROGRAM_H_INCLUDED
