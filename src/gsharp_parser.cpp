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
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "gsharp_program.h"
#include "gsharp_except.h"

using namespace std;
using namespace gsharp;


///////  P a r s e L i n e  ///////
// for test cases only: the main parser is in Step() function
const string Program::_ParseLine(const string& line, int precision)
{
   string str(line);
   if(_debug_level > 0)
      cout << "Initial line to parse: " << str << endl;
   _current_internal_param = INTERNAL_PARAMETERS_START-1;

   _ProcessComments(str, &_extra);
   _PrepareLine(str);
   _RemoveNword(str);
   _SimplifyOperators(str);
   _CalculateExpressions(str);
   _ResolveParameters(str, precision);
   _FormatPretty(str);

   if(_debug_level > 0)
      cout << "Final parsed line: " << str << endl;
   return str;
}


//////  P r o c e s s C o m m e n t s  //////
// finds and removes comments from the input line
// if comments have specific command, execute them
// comments may have a pair of brackets () inside, but not just a single non-matching bracket
void Program::_ProcessComments(string& line, ExtraInfo* extra)
{
   while(1){
      // search for comments
      size_t pos = line.find_first_of(";(");
      if(pos == string::npos) // no comments left?
         break; // finished

      size_t len = 1;
      if(line[pos] == '('){
         // find the corresponding closing bracket
         for(int bracket_cnt = 0 ; bracket_cnt >= 0; ++len){
            if(pos+len >= line.size())
               throw ErrorMsg(this, "No closing bracket for comment");
            if(line[pos+len] == '(') // start of the internal bracket pair
               ++bracket_cnt;
            if(line[pos+len] == ')') // internal bracket pair ended
               --bracket_cnt;
         }
      }
      else // it's a comment starting form semicolon ';'
         len = line.size() - pos + 1; // everything until the end of the line

      // extract comment content, clearing leading and trailing zeros
      string comment;
      size_t start = line.find_first_not_of(" \t", pos+1);
      if(start < pos+len-1){ // if didn't hit the closing bracket
         size_t finish = line.find_last_not_of(" \t", pos+len-2);
         if(finish > pos) // if after the opening bracket
            comment = line.substr(start, finish-start+1);
      }
      if(_debug_level > 2)
         cout << "Comment extracted: " << comment << endl;

      // remove comment from the line
      line.erase(pos, len);

      // process commands in the comment, if present
      if(extra){
         string active(comment.size(), '\0'); // convert to lowercase for comparison
         ::transform(comment.begin(), comment.end(), active.begin(), ::tolower);
         if(_debug_level > 2)
            cout << "Checking for active comment in: " << active << endl;

         if(active.compare(0, 4, "msg,") == 0){
            comment = comment.substr(4); // extract the message
            extra->Assign(ExtraInfo::MSG, comment);
         }
         else if(active.compare(0, 6, "print,") == 0){
            comment = comment.substr(6);
            _ResolveParameters(comment);
            extra->Assign(ExtraInfo::PRN, comment);
         }
         else if(active.compare(0, 6, "debug,") == 0){
            comment = comment.substr(6);
            _ResolveParameters(comment);
            extra->Assign(ExtraInfo::DBG, comment);
         }
         else if(active.compare(0, 4, "log,") == 0){
            comment = comment.substr(4);
            _ResolveParameters(comment);
            extra->Assign(ExtraInfo::LOG, comment);
         }
      }
   }

   if(_debug_level > 1)
      cout << "Str after removing comments: " << line << endl;
}


///////  P r e p a r e L i n e  ///////
void Program::_PrepareLine(string& str)
{
   // check for not-allowed symbols outside comments
   if(str.find_first_of("%&|^@~!<>{}()$?,\"`':;_") != string::npos)
      throw ErrorMsg(this, "Unexpected character");

   // remove whitespaces and convert to lowercase
   str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());
   transform(str.begin(), str.end(), str.begin(), ::tolower);
   if(_block_delete && !str.empty() && str[0] == '/'){
      if(_debug_level > 2)
         cout << "Str has 'block_delete' character: " << str << endl;
      str.clear(); // ignore the string completely
   }
   if(_debug_level > 1)
      cout << "Str prepared for processing: " << str << endl;
}


///////  R e m o v e N w o r d  ///////
// N-word is expected only in the beginning of the line
void Program::_RemoveNword(string& str)
{
   if(str.empty() || str[0] != 'n') return; // no N-word in front
   if(str.size() <= 1 || !::isdigit(str[1]))
      throw ErrorMsg(this, "Ill-formed N-word");
   size_t pos = str.find_first_not_of("0123456789", 1);
   str = str.substr((pos!=string::npos)? pos: str.size());
}


////////  R e a d O w o r d  ////////
// O-word is expected only in the beginning of the line, after N-word has been removed
// the whole o-word with the number and command is removed from the input string on exit
bool Program::_ReadOword(string& str, ONumber& number, string& command)
{
   if(str.empty() || str[0] != 'o') return false; // no O-word in front
   if(str.size() <= 1 || !::isdigit(str[1]))
      throw ErrorMsg(this, "Ill-formed O-word");

   size_t pos1;
   number = ::stoul(str.substr(1), &pos1);
   ++pos1; // pos1 was counted from the begiining of the number, has to be from 'o'
   if(_debug_level > 2)
      cout << "O-number = " << number << endl;
   size_t pos2 = str.find_first_not_of("abcdefghijklmnopqrstuvwxyz", pos1);
   command = str.substr(pos1, (pos2!=string::npos)? pos2-pos1: string::npos);
   if(_debug_level > 2)
      cout << "O-command [" << pos1 << "," << ((pos2!=string::npos)? pos2: str.size()) <<
              "]: '" << command << "'" << endl;
   str = str.substr((pos2!=string::npos)? pos2: str.size()); // extract remaining part
   return true;
}


///////  R e p l a c e S i n g l e O p e r a t o r  ///////
// replace multi-character operator with the single-character one
void Program::_ReplaceSingleOperator(string& line, const string& operation, char replacement)
{
   size_t pos = 0;
   while(pos != string::npos){
      pos = line.find(operation, pos);
      if(pos != string::npos)
         line.replace(pos, operation.length(), 1, replacement);
   }
}


////////  S i m p l i f y O p e r a t o r s  ///////
// replace all multi-character operators with single-character ones for simplicity
void Program::_SimplifyOperators(string& line)
{
   _ReplaceSingleOperator(line, "mod", '%');
   _ReplaceSingleOperator(line, "and", '&');
   _ReplaceSingleOperator(line, "xor", '^');
   _ReplaceSingleOperator(line, "or",  '|');
   _ReplaceSingleOperator(line, "**",  '@'); // use as power symbol
   _ReplaceSingleOperator(line, "eq",  '~');
   _ReplaceSingleOperator(line, "ne",  '!');
   _ReplaceSingleOperator(line, "gt",  '>');
   _ReplaceSingleOperator(line, "ge",  '}');
   _ReplaceSingleOperator(line, "lt",  '<');
   _ReplaceSingleOperator(line, "le",  '{');
   if(_debug_level > 1)
      cout << "Str after simplifying operators: " << line << endl;
}


////////  P a r s e O p e r a n d  ////////
// immediate value or parameter
double Program::_ParseOperand(const string& expr)
{
   if(_debug_level > 2)
      cout << "Expr to parse operand: " << expr << endl;

   if(expr.empty())
      throw ErrorMsg(this, "Empty operand");

   if(expr[0] == '+') // ignore positive sign
      return _ParseOperand(expr.substr(1));

   if(expr[0] == '-') // invert value with the negative sign
      return -_ParseOperand(expr.substr(1));

   // check if this is a parameter
   if(expr[0] == '#'){
      size_t len;
      double value = _ReadParameter(expr, 0, len);
      if(len != expr.size()) // other characters present
         throw ErrorMsg(this, "Unexpected character after the parameter");
      return value;
   }

   if(!::isdigit(expr[0]) && expr[0] != '.') // should start with a decimal digit or decimal dot
      throw ErrorMsg(this, "Unexpected operand");

   // directly read the value
   // read the parameter index
   size_t last_pos;
   double value = stod(expr, &last_pos);
   if(last_pos != expr.size())
      throw ErrorMsg(this, "Unexpected character after the value");

   return value;
}


////////  P a r s e M u l t i p l i c a t i o n  ////////
// operators: POW (@)
double Program::_ParsePower(const string& expr)
{
   if(_debug_level > 2)
      cout << "Expr to parse power: " << expr << endl;
   size_t delimiter = expr.find_last_of('@'); // "last" to ensure left-to-right pocessing
   if(delimiter != string::npos){
      double lhs = _ParsePower(expr.substr(0, delimiter)); // may contain power ops
      double rhs = _ParseOperand(expr.substr(delimiter+1)); // should not contain any power ops
      // power
      return pow(lhs, rhs);
   }
   return _ParseOperand(expr);
}


////////  P a r s e M u l t i p l i c a t i o n  ////////
// operators: (*), (/), MOD (%)
double Program::_ParseMultiplication(const string& expr)
{
   if(_debug_level > 2)
      cout << "Expr to parse multiplication: " << expr << endl;
   size_t delimiter = expr.find_last_of("*/%"); // "last" to ensure left-to-right pocessing
   if(delimiter != string::npos){
      double lhs = _ParseMultiplication(expr.substr(0, delimiter)); // may contain mult ops
      double rhs = _ParsePower(expr.substr(delimiter+1)); // should not contain any mult ops
      if(expr[delimiter] == '*') // multiply
         return lhs * rhs;
      else if(expr[delimiter] == '/') // divide
         return lhs / rhs;
      // modulus on doubles
      return fmod(lhs, rhs);
   }
   return _ParsePower(expr);
}


////////  P a r s e S u m m a t i o n  ////////
// operators: (+), (-)
double Program::_ParseSummation(const string& expr)
{
   if(_debug_level > 2)
      cout << "Expr to parse summation: " << expr << endl;
   size_t delimiter = expr.find_last_of("+-"); // "last" to ensure left-to-right pocessing
   if(delimiter != string::npos){
      // check if it is unary plus or minus
      if(delimiter > 0 && (::isdigit(expr[delimiter-1]) || expr[delimiter-1] == '.')){
         // proceed with the binary operation
         double lhs = _ParseSummation(expr.substr(0, delimiter)); // may contain summation ops
         double rhs = _ParseMultiplication(expr.substr(delimiter+1)); // should not contain any sum ops
         if(expr[delimiter] == '+') // add
            return lhs + rhs;
         // subtract
         return lhs - rhs;
      }
      // unary sign operation
      if(_debug_level > 2)
         cout << "Unary operand found in: " << expr << endl;
      // replace the signed value with parameter
      string str(expr);
      size_t pos = str.find_first_not_of("0123456789.", delimiter+1);
      size_t len = (pos == string::npos)? str.size()-delimiter: pos-delimiter;
      if(_debug_level > 3)
         cout << "Unary operand starts from pos " << delimiter << " with " << len << " chars" << endl;
      double value = _ParseOperand(expr.substr(delimiter, len));
      _ReplaceWithParameter(value, str, delimiter, len);
      // continue evaluation
      return _ParseSummation(str);
   }
   return _ParseMultiplication(expr);
}


////////  P a r s e C o n d i t i o n a l  ////////
// operators: EQ (~), NE (!), LT (<), LE ({), GE (}), GT (>)
double Program::_ParseConditional(const string& expr)
{
   if(_debug_level > 2)
      cout << "Expr to parse conditional: " << expr << endl;
   size_t delimiter = expr.find_last_of("~!{<>}"); // "last" to ensure left-to-right pocessing
   if(delimiter != string::npos){
      double lhs = _ParseConditional(expr.substr(0, delimiter)); // may contain other cond ops
      double rhs = _ParseSummation(expr.substr(delimiter+1)); // should not contain any cond ops

      // simple tolerance-based comparison may not be effective in all cases
      //  but let's follow LinuxCNC approach for now
      if(expr[delimiter] == '~') // eq
         return (fabs(lhs - rhs) < TOLERANCE_EQUAL)? 1.0: 0.0;
      else if(expr[delimiter] == '!') // ne
         return (fabs(lhs - rhs) >= TOLERANCE_EQUAL)? 1.0: 0.0;
      else if(expr[delimiter] == '{') // le
         return (lhs <= rhs)? 1.0: 0.0;
      else if(expr[delimiter] == '<') // lt
         return (lhs < rhs)? 1.0: 0.0;
      else if(expr[delimiter] == '>') // gt
         return (lhs > rhs)? 1.0: 0.0;
      // ge
      return (lhs >= rhs)? 1.0: 0.0;
   }
   return _ParseSummation(expr);
}


////////  P a r s e L o g i c a l  ////////
// operators: AND (&), OR (|), XOR (^)
double Program::_ParseLogical(const string& expr)
{
   if(_debug_level > 2)
      cout << "Expr to parse logical: " << expr << endl;
   size_t delimiter = expr.find_last_of("&|^"); // "last" to ensure left-to-right pocessing
   if(delimiter != string::npos){
      double lhs = _ParseLogical(expr.substr(0, delimiter)); // may contain logical ops
      double rhs = _ParseConditional(expr.substr(delimiter+1)); // should not contain any logical ops
      if(expr[delimiter] == '&') // and
         return (lhs!=0.0 && rhs!=0.0)? 1.0: 0.0;
      else if(expr[delimiter] == '|') // or
         return (lhs!=0.0 || rhs!=0.0)? 1.0: 0.0;
      // xor
      return ((lhs!=0.0 && rhs==0.0) || (lhs==0.0 && rhs!=0.0))? 1.0: 0.0;
   }
   return _ParseConditional(expr);
}


///////  E v a l u a t e E x p r e s s i o n  ///////
double Program::_EvaluateExpression(string& expr) // evaluate espression inside [.]
{
   // safety checks
   if(expr.empty())
      throw ErrorMsg(this, "Ill-formed expression");

   // if there are internal expressions, process them
   _CalculateExpressions(expr);

   if(expr.find_first_of('=') != string::npos)
      throw ErrorMsg(this, "Assignments are not allowed inside expressions");

   if(_debug_level > 2)
      cout << "Clean expr to parse: " << expr << endl;

   // now there should be no other expressions inside <expr>, only operations and operands
   return _ParseLogical(expr); // start parsing from logical operators
}


//////  C a l c u l a t e E x p r e s s i o n F r o m B r a c k e t  //////
// <start> should point to the opening bracket (assumed, not checked)
// the function finds the corresponding closing bracket and calculates the expression inside
// <len> is set to the length of the expression, including brackets
double Program::_CalculateExpressionFromBracket(const string& line, size_t start, size_t& len)
{
   // find corresponding closing bracket
   len = 1;
   for(int bracket_cnt = 0 ; bracket_cnt >= 0; ++len){
      if(start+len >= line.size())
         throw ErrorMsg(this, "No closing bracket for expression");
      if(line[start+len] == '[') // found internal sub-expression
         ++bracket_cnt;
      if(line[start+len] == ']') // internal sub-expression ended
         --bracket_cnt;
   }

   // evaluate expression inside the brackets
   string expr = line.substr(start+1, len-2); // internal part of the expression
   return _EvaluateExpression(expr);
}


//////  A p p l y A s F u n c t i o n A r g u m e n t  //////
// check if this expression is a function argument ( e.g. abs[..] )
//  then apply the function and use it as a single expression
double Program::_ApplyAsFunctionArgument(double arg, const string& line, size_t& start, size_t& len)
{
   double result = arg; // default: no function
   if(start == 0 || !::isalpha(line[start-1])) // not a character - not a function!
      return arg;

   size_t pos = start; // new start position
   size_t last = start + len; // just after the closing bracket

   if(_debug_level > 1)
      cout << "Check Fn before expr: " << line.substr(start, len) << endl;

   // start with the longest possible function name
   if(start >= 5){
      if(_debug_level > 2)
         cout << "Fn5 to parse: " << line.substr(pos-5) << endl;
      if(line.compare(pos-5, 5, "round") == 0){
         result = round(arg);
         pos -= 5; // new start position
      }
   }

   // try 4 character function names first to avoid detecting sin() before asin()
   if(pos == start && start >= 4){
      pos -= 4;
      if(_debug_level > 2)
         cout << "Fn4 to parse: " << line.substr(pos) << endl;
      if(line.compare(pos, 4, "acos") == 0){
         if(arg < -1.0 || arg > 1.0)
            throw ErrorMsg(this, "Out of range ACOS argument");
         result = _degrees(acos(arg)); // output in degrees (RS274/NGC)
      }
      else if(line.compare(pos, 4, "asin") == 0){
         if(arg < -1.0 || arg > 1.0)
            throw ErrorMsg(this, "Out of range ASIN argument");
         result = _degrees(asin(arg)); // output in degrees (RS274/NGC)
      }
      else if(line.compare(pos, 4, "sqrt") == 0){
         if(arg < 0.0)
            throw ErrorMsg(this, "Negative SQRT argument");
         result = sqrt(arg);
      }
      else if(line.compare(pos, 4, "atan") == 0){ // special case in LinuxCNC
      if(_debug_level > 3)
         cout << "Last Fn char: '" << line[last] << "' then: '" << line[last+1] << "'" << endl;
         if(line[last] != '/' || line[last+1] != '[') // double argument expression
            throw ErrorMsg(this, "Ill-formed ATAN expression");
         if(_debug_level > 1)
            cout << "ATAN second expr to parse: " << line.substr(last+1) << endl;
         size_t arg2_len;
         double arg2 = _CalculateExpressionFromBracket(line, last+1, arg2_len);
         result = _degrees(atan2(arg, arg2)); // output in degrees (RS274/NGC)
         last += arg2_len + 1; // adjust the last element
      }
      else
         pos += 4; // if no function found, restore original position
   }

   // 3-character functions
   if(pos == start && start >= 3){
      pos -= 3;
      if(_debug_level > 2)
         cout << "Fn3 to parse: " << line.substr(pos) << endl;
      if(line.compare(pos, 3, "abs") == 0)
         result = fabs(arg);
      else if(line.compare(pos, 3, "cos") == 0)
         result = cos(_radians(arg)); // argument in degrees (RS274/NGC)
      else if(line.compare(pos, 3, "fix") == 0)
         result = floor(arg);
      else if(line.compare(pos, 3, "fup") == 0)
         result = ceil(arg);
      else if(line.compare(pos, 3, "sin") == 0)
         result = sin(_radians(arg)); // argument in degrees (RS274/NGC)
      else if(line.compare(pos, 3, "tan") == 0)
         result = tan(_radians(arg)); // argument in degrees (RS274/NGC)
      else if(line.compare(pos, 3, "exp") == 0){
         result = exp(arg);
         if(result == HUGE_VAL)
            throw ErrorMsg(this, "EXP argument is too big");
      }
      else
         pos += 3; // restore original position
   }

   // 2-character functions
   if(pos == start && start >= 2){
      if(_debug_level > 2)
         cout << "Fn2 to parse: " << line.substr(pos-2) << endl;
      if(line.compare(pos-2, 2, "ln") == 0){
         result = log(arg);
         pos -= 2; // new start position
      }
   }

   start = pos;
   if(_debug_level > 3)
      cout << "Ended Fn with start pos: " << start << endl;
   len = last - start; // adjust expression length accordingly
   return result;
}


///////  C a l c u l a t e E x p r e s s i o n s  ////////
void Program::_CalculateExpressions(string& line, vector<double>* output)
{
   while(1){
      // search for any (remaining) expressions
      size_t expr_start = line.find_first_of('[');
      if(expr_start == string::npos) // no expressions?
         break; // then no changes to the input line, ready for parsing

      size_t expr_len;
      double result = _CalculateExpressionFromBracket(line, expr_start, expr_len);
      if(_debug_level > 1)
         cout << "Raw expr " << line.substr(expr_start, expr_len) << " = " << result << endl;

      // is this expression a function argument? e.g. abs[..]
      result = _ApplyAsFunctionArgument(result, line, expr_start, expr_len);
      if(_debug_level > 1)
         cout << "Expression: " << line.substr(expr_start, expr_len) << " equals " << result << endl;

      // substitute expression with the internal parameter equal to the evaluated expression
      _ReplaceWithParameter(result, line, expr_start, expr_len);

      if(output != nullptr)
         output->push_back(result); // only first order expressions
   }
   if(_debug_level > 1)
      cout << "Str after calculating expressions: " << line << endl;
}


///////  R e p l a c e  W i t h  P a r a m e t e r  ///////
void Program::_ReplaceWithParameter(double value, string& line, size_t start, size_t len)
{
   // #indices start from 1, while c++ arrays start from 0, so there is a unit diff
   if(_debug_level > 1)
      cout << "Saving value " << value << " into parameter #" << (_current_internal_param+1) << endl;
   _params[_current_internal_param] = value;
   line.replace(start, len, string("#") + to_string(++_current_internal_param));
   if(_current_internal_param >= TOTAL_PARAMETERS)
      throw ErrorMsg(this, "Max number of internal parameters reached");
}


////////  R e a d P a r a m e t e r  ////////
// assumes that the "#" character is located at <pos>
// updates the <len> of the whole parameter string (## and number)
double Program::_ReadParameter(const string& line, size_t pos, size_t& len)
{
   // process recursive parameter referencing (aka ###..)
   int nref = 1; // number of "#" as recursive parameters
   for(;pos + nref < line.size(); ++nref)
      if(line[pos + nref] != '#')
         break;

   // expect a digit after the last '#'
   if(!::isdigit(line[pos + nref]))
      throw ErrorMsg(this, "Error in parameter index");

   // read the parameter index
   char* last_ptr;
   double value = strtod(line.c_str() + pos + nref, &last_ptr);
   len = last_ptr - (line.c_str() + pos); // length of the parameter string

   // unwind references
   for(;nref > 0; --nref){
      size_t idx = static_cast<size_t>(round(value));
      if(idx == 0 || idx > TOTAL_PARAMETERS)
         throw ErrorMsg(this, "Parameter #%d does not exist", idx);
      value = (idx <= TOTAL_LOCAL_PARAMETERS)? _local_params[idx-1]: _params[idx-1];
   }

   return value;
}


////////  A s s i g n  P a r a m e t e r  ////////
// assumes that the "#" character is located at <pos>
// updates the <len> of the whole assignment string (##, number, '=' and the value)
void Program::_AssignParameter(const string& line, size_t pos, size_t& len)
{
   // process recursive parameter referencing (aka ###..)
   int nref = 1; // number of "#" as recursive parameters
   for(;pos + nref < line.size(); ++nref)
      if(line[pos + nref] != '#')
         break;

   // expect a digit after the last '#'
   if(!::isdigit(line[pos + nref]))
      throw ErrorMsg(this, "Error in parameter index");

   // read the parameter index
//TODO: Replace with string-based coversion stod?
   char* last_ptr;
   double index = strtod(line.c_str() + pos + nref, &last_ptr);

   // read the new parameter value
   last_ptr++; // one after the '=' character
   if(*last_ptr == '\0' || (!::isdigit(*last_ptr) && *last_ptr != '.' && *last_ptr != '-'))
      throw ErrorMsg(this, "Error in the value to assign");
   double new_value = strtod(last_ptr, &last_ptr);
   len = last_ptr - (line.c_str() + pos); // length of the whole assignment expression

   // unwind references
   size_t idx;
   for(;nref > 0; --nref){
      idx = static_cast<size_t>(round(index));
      if(idx == 0 || idx > TOTAL_PARAMETERS)
         throw ErrorMsg(this, "Parameter #%d does not exist", idx);
      index = (idx <= TOTAL_LOCAL_PARAMETERS)? _local_params[idx-1]: _params[idx-1];
   }
   if(_debug_level > 0)
      cout << "Assigning value " << new_value << " to parameter #" << idx << endl;

   if(idx <= TOTAL_LOCAL_PARAMETERS)
      _local_params[idx-1] = new_value;
   else // global
      _params[idx-1] = new_value;
}


////////  R e s o l v e P a r a m e t e r s  ////////
// substitute parameters in the string with their values, using certain precision
void Program::_ResolveParameters(string& line, int precision)
{
   if(_debug_level > 1)
      cout << "Str to resolve parameters: " << line << endl;

   size_t len, pos = 0;
   int assignments = 0;
   while(1){
      if(_debug_level > 3)
         cout << "Searching for '#' in line '" << line << "' from pos " << pos << endl;
      pos = line.find_first_of('#', pos);
      if(pos == string::npos)
         break; // finished

      double value = _ReadParameter(line, pos, len);
      if(value == -0.0) value = 0.0; // explicit check for negative zero

      if(line[pos+len] == '=') // don't assign anything yet
         ++assignments;
      else{ // substitute parameter with its' value string
         stringstream ss;
         ss << fixed << setprecision(precision) << value; // format up to precision
         string str = ss.str();
         str.erase(str.find_last_not_of('0') + 1, string::npos); // remove trailing zeros
         if(*str.rbegin() == '.') // decimal dot at the very end?
            str.erase(str.size()-1); // remove trailing dot
         if(str.empty()) // nothing left after removing all zeros and dots?
            str = "0"; // this could be the only reason
         // substitute parameter with value
         if(_debug_level > 1)
            cout << "Parameter " << line.substr(pos, len) << " is replaced by value " << str << endl;
         line.replace(pos, len, str);
         len = str.size(); // new length after replacement

      }
      pos += len;

   }

   if(_debug_level > 2)
      cout << assignments << " parameter(s) to assign in line " << line << endl;

   // assign parameters only as the last step (LinuxCNC requirement)
   if(assignments > 0){
      pos = 0;
      while(1){ // redo the search
         pos = line.find_first_of('#', pos);
         if(pos == string::npos)
            break;

         _AssignParameter(line, pos, len);
         line.erase(pos, len);
      }
   }
}


/////////  F o r m a t  P r e t t y  ///////
// insert spaces between digits and alpha characters
// convert to upper case
void Program::_FormatPretty(string& line)
{
   if(_format_pretty)
      for(size_t i=1; i<line.size(); ++i){
         if(::isdigit(line[i-1]) && ::isalpha(line[i]))
            line.insert(i, 1, ' ');
      }

   if(_convert_to_upper)
      ::transform(line.begin(), line.end(), line.begin(), ::toupper);
}

