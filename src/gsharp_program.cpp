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
#include <algorithm>
#include "gsharp_program.h"
#include "gsharp_except.h"

using namespace gsharp;
using namespace std;


//////  c o n s t r u c t o r  ///////
Program::Program()
{
   _debug_level = 0;

//TODO: store/retrieve persistent parameters (check for early exceptions!)
   _params.fill(0); // for now at the very start all are zero
   _block_delete = USE_BLOCK_DELETE;
   _format_pretty = USE_PRETTY_FORMAT;
   _convert_to_upper = CONVERT_TO_UPPER;
   _percent_start = 0;
   _percent_stop = 0;
   Rewind();
}


/////////  R e w i n d  /////////
void Program::Rewind()
{
   _current_line = 1;
   _current_internal_param = INTERNAL_PARAMETERS_START-1;
   _local_params.fill(0);
   while(!_param_stack.empty())
      _param_stack.pop();
   while(!_return_stack.empty())
      _return_stack.pop();
   _percent_active = false;
}


//////////  S e t  P a r a m  ////////
void Program::SetParam(unsigned int number, double value)
{
   if(number == 0 || number > TOTAL_CNC_PARAMETERS)
      throw ErrorMsg(this, "Attempt to set unexisting parameter #%d", number);

   if(number <= TOTAL_LOCAL_PARAMETERS)
      _local_params[number-1] = value;
   else
      _params[number-1] = value;
}


//////////  G e t  P a r a m  ////////
double Program::GetParam(unsigned int number) const
{
   if(number == 0 || number > TOTAL_CNC_PARAMETERS)
      throw ErrorMsg(this, "Attempt to read unexisting parameter #%d", number);

   if(number <= TOTAL_LOCAL_PARAMETERS)
      return _local_params[number-1];
   return _params[number-1];
}


///////  G e t  S o u r c e  L i n e  ///////
const string Program::GetSourceLine(LineNumber num) const
{
   if(num >= _code.size())
      throw ErrorMsg(this, "Attempt to read non-existing code line #%d", num);
   return _code[num];
}


/////////////  L o a d  ///////////
void Program::Load(const string& code)
{
   // fresh restart
   _code.clear();
   _code.push_back("you should not access line 0"); // line numbers start from 1
   _current_line = 1; // starts from 1
   _blocks.clear();

   _percent_start = 0;
   _percent_stop = 0;

   // do analysis line by line
   string line;
   stringstream ss(code);
   for(; getline(ss, line); ++_current_line){
      _code.push_back(line);

      // prepare the string for processing
      if(_debug_level > 0)
         cout << "String to load: " << line << endl;

      if(!line.empty() && line[0] == '%'){ // percent delimiter
         if(_percent_start == 0)
            _percent_start = _current_line;
         else if(_percent_stop == 0)
            _percent_stop = _current_line;
         else //TODO: maybe allow many lines with %, but stop at the second instance?
            throw ErrorMsg(this, "Two many '%%' characters");
         continue;
      }

      _ProcessComments(line); // remove comments
      _PrepareLine(line); // whitespaces, lowcase
      _RemoveNword(line);

      if(line.empty())
         continue;

      // detect and process control lines (which starts with O-word)
      ONumber o_num;
      string cmd;
      if(_ReadOword(line, o_num, cmd)){
         if(_debug_level > 1)
            cout << "Found o-word 'o" << o_num << "' with command: '" << cmd << "'" << endl;
         // check if we have not used this o-number before
         if(cmd != "call"){ // 'call' can appear anywhere, don't process it yet
            if(_blocks.count(o_num) == 0){
               // then create the new code block
               CodeBlock block{CodeBlock::UNDEF, _current_line, vector<LineNumber>()};
               if(cmd == "sub")
                  block.type = CodeBlock::SUB;
               else if(cmd == "if")
                  block.type = CodeBlock::IF;
               else if(cmd == "do")
                  block.type = CodeBlock::DO;
               else if(cmd == "while")
                  block.type = CodeBlock::WHILE;
               else if(cmd == "repeat")
                  block.type = CodeBlock::REPEAT;
               else if(cmd == "endsub" || cmd == "return" || cmd == "elseif" || cmd == "else" ||
                       cmd == "endif" || cmd == "break" || cmd == "continue" ||
                       cmd == "endwhile" || cmd == "endrepeat")
                  throw ErrorMsg(this, "Unexpected o-code command '%s'", cmd.c_str());
               else
                  throw ErrorMsg(this, "Unrecognised o-code command '%s'", cmd.c_str());

               pair<ONumber, CodeBlock> entry(o_num, block);
               _blocks.insert(entry);
               if(_debug_level > 0)
                  cout << "Created o-block {" << block.start_line << "," <<
                           block.end_line << "," << block.type << "}" << endl;
            }
            else{ // the block with this o-code already exists
               CodeBlock& block = _blocks[o_num]; // the block with this o-code

                // has the end line been already defined? shouldn't happen
               if(block.end_line != 0 && cmd != "call") // but 'call' can appear anywhere
                  throw ErrorMsg(this, "O-code block already finished in line %d", block.end_line-1);

               if(cmd == "sub" || cmd == "if" || cmd == "do" || cmd == "repeat" ||
                  (cmd == "while" && _blocks[o_num].type != CodeBlock::DO))
                     throw ErrorMsg(this, "O-number %d is alredy used in line %d",
                                    o_num, block.start_line);

               // check if the command matches the corresponding block
               if((cmd == "return" && block.type != CodeBlock::SUB) ||
                  (cmd == "endsub" && block.type != CodeBlock::SUB) ||
                  (cmd == "elseif" && block.type != CodeBlock::IF)  ||
                  (cmd == "else"   && block.type != CodeBlock::IF)  ||
                  (cmd == "endif"  && block.type != CodeBlock::IF)  ||
                  (cmd == "while"  && block.type != CodeBlock::DO)  ||
                  (cmd == "endwhile" && block.type != CodeBlock::WHILE) ||
                  (cmd == "break" && block.type != CodeBlock::DO && block.type != CodeBlock::WHILE) ||
                  (cmd == "continue" && block.type != CodeBlock::DO && block.type != CodeBlock::WHILE) ||
                  (cmd == "endrepeat" && block.type != CodeBlock::REPEAT))
                     throw ErrorMsg(this, "Unexpected command for o-code block %d", o_num);

               if(cmd == "elseif" || cmd == "else")
                  block.mid_line.push_back(_current_line);

               // set the end line for this o-block
               if(cmd == "endsub" || cmd == "endif" || cmd == "while" ||
                  cmd == "endwhile" || cmd == "endrepeat"){
                     block.end_line = _current_line + 1;
               if(_debug_level > 0)
                  cout << "Finished o-block {" << block.start_line << "," <<
                           block.end_line << "," << block.type << "}" << endl;
               }
            }
         }
      }
   }
   // check if percent delimiters are formed correctly
   if(_percent_start > 0 && _percent_stop <= _percent_start)
      throw ErrorMsg(this, "No closing '%%' character");
   if(_debug_level > 1)
      cout << "Percent (%) demarcation lines from " << _percent_start << " to " << _percent_stop << endl;

   // check if all blocks have been formed correctly
   for(auto it=_blocks.begin(); it != _blocks.end(); ++it)
      if(it->second.end_line == 0)
         throw ErrorMsg(this, "o-block %d in line %d doesn't have the end", it->first, it->second.start_line);

   Rewind(); // prepare for the next steps
}


///////////  S t e p  ///////////
bool Program::Step(string& line, ExtraInfo& extra)
{
   extra.Clear();
   while(_current_line < _code.size()){
      // program runs only within % delimiters, if they are present
      if(_percent_start > 0){
         if(_current_line == _percent_start){
            _percent_active = true; // activate processing
            ++_current_line;
            continue;
         }
         else if(_current_line == _percent_stop){
            _current_line = _code.size(); // finished: move to the last line
            continue;
         }
         else if(!_percent_active){
            ++_current_line; // skip non-active lines
            continue;
         }
      }

      line = _code[_current_line];
      if(_debug_level > 0)
         cout << "Step to line (" << _current_line << "): " << line << endl;

      // standard operations before processing
      _current_internal_param = INTERNAL_PARAMETERS_START-1;
      _ProcessComments(line, &extra); // remove comments, check for commands
      _PrepareLine(line); // whitespaces, lowcase
      _RemoveNword(line);

      if(line.empty()){
         ++_current_line;
         if(extra.FirstNonEmpty()) // there were messages to process
            return true;
         continue;
      }

      // detect and process control lines (which starts with O-word)
      string cmd;
      ONumber o_num;
      if(_ReadOword(line, o_num, cmd)){ // flow control
         LineNumber next_line = _current_line + 1;
         if(_blocks.count(o_num) == 0)
            throw ErrorMsg(this, "O-block number %d is not found", o_num);
         CodeBlock& block = _blocks[o_num]; // the block with this o-code
         // commands that don't have a parameter following
         if(cmd == "sub")
            next_line = block.end_line; // skip the subroutine completely
         else if(cmd == "break")
            next_line = block.end_line; // exit the current block
         else if(cmd == "continue")
            next_line = block.end_line - 1; // to the last line of the loop
         else if(cmd == "endwhile")
            next_line = block.start_line; // back to check the loop condition again
         else if(cmd == "endrepeat"){
            if(--block.run_times > 0) // have we finished repeating?
               next_line = block.start_line + 1; // if not: go straight after 'repeat'
         }
         else if(cmd == "else"){
            if(block.run_times != 0) // just finished with 'if' body
               next_line = block.end_line;
         }
         else if(cmd != "do" && cmd != "endif"){
            // the following commands may contain a parameter after the command
            vector<double> arguments; // expressions as arguments
            _SimplifyOperators(line);
            _CalculateExpressions(line, &arguments);
            if(_debug_level > 0)
               cout << "Found " << arguments.size() << " argument(s)" << endl;
            if(_debug_level > 1){
               cout << "Arguments values:";
               for(const auto& value: arguments)
                  cout << " " << value;
               cout << endl;
            }

            if(cmd == "call"){
               if(block.type != CodeBlock::SUB)
                  throw ErrorMsg(this, "Cannot find sub %d to call", o_num);
               if(_return_stack.size() >= MAX_STACK_LEVELS)
                  throw ErrorMsg(this, "Stack overflow calling sub %d", o_num);
               _return_stack.push(next_line);
               _param_stack.push(_local_params); // preserve local params for after return from subroutine
               // _local_params.fill(0); // LinuxCNC OCodes: keep "the same value as in the calling context"
               if(!arguments.empty()){
                  size_t to_copy = min(arguments.size(), _local_params.size());
                  for(size_t i=0; i<to_copy; ++i)
                     _local_params[i] = arguments[i]; // assign arguments from the 'call' line
               }
               next_line = block.start_line + 1; // next after 'sub' declaration
            }
            else if(cmd == "return" || cmd == "endsub"){
               if(!arguments.empty()) // any return value?
                  _params[RETURN_VALUE_PARAMETER-1] = arguments[0];
               if(_return_stack.empty())
                  throw ErrorMsg(this, "Stack underrun returning form sub %d", o_num);
               _local_params = _param_stack.top(); _param_stack.pop();
               next_line = _return_stack.top(); _return_stack.pop();
            }
            else{
               // the following commands require at least one parameter after the command
               if(line.empty() || arguments.empty())
                  throw ErrorMsg(this, "No arguments specified for '%s' command", cmd.c_str());

               double argument = arguments[0]; //stod(line);

               if(cmd == "repeat")
                  block.run_times = argument;
               else if(cmd == "while"){
                  if(argument == 0.0)
                     next_line = block.end_line; // finished with the loop
                  else if(_current_line != block.start_line)
                     next_line = block.start_line + 1; // this is 'do-while' loop
               }
               else if(cmd == "if"){
                  block.run_times = (argument != 0.0)? 1: 0;
                  if(argument == 0.0)
                     next_line = (block.mid_line.empty())? block.end_line: block.mid_line[0];
               }
               else if(cmd == "elseif"){
                  if(block.run_times != 0)
                     next_line = block.end_line; // just finished the previous 'if' body
                  else{
                     block.run_times = (argument != 0.0)? 1: 0;
                     if(argument == 0.0){ // goto the next mid-line
                        auto it = find(block.mid_line.begin(), block.mid_line.end(), _current_line);
                        next_line = (it<block.mid_line.end()-1)? *(it+1): block.end_line;
                     }
                  }
               }
            }
         }
         _current_line = next_line;
         line.clear();
         if(extra.FirstNonEmpty())
            return true; // G-code line is ready to go! (or active comment)
      }
      else{ // non-control G-code
         ++_current_line;
         _SimplifyOperators(line);
         _CalculateExpressions(line);
         _ResolveParameters(line, 3);

         if(line.substr(0, 2) == "m2" || line.substr(0, 3) == "m30")
            _current_line = _code.size(); // make it the last line

         _FormatPretty(line);
         if(!line.empty() || extra.FirstNonEmpty())
            return true; // G-code line is ready to go! (or active comment)
      }
   }
   line.clear();
   return false; // current line is beyond the code lines
}
