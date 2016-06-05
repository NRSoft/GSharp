#include <fstream>
#include <sstream>
#include <string>
#include "gsharp_test.h"
#include "../src/gsharp_program.h"
#include "../src/gsharp_except.h"


namespace gsharp
{

using namespace std;

void GSharpTest_PrintProgramOutput(Program& p)
{
   string str;
   ExtraInfo extra;
   p.DebugLevel(0);
   while(p.Step(str, extra)){
      if(!str.empty())
         cout << " *** " << str << " *** ";
      ExtraInfo::Type t;
      while(extra.FirstNonEmpty(&t)){
         switch(t){
            case ExtraInfo::MSG:
               cout << "MSG";
               break;
            case ExtraInfo::PRN:
               cout << "PRN";
               break;
            case ExtraInfo::DBG:
               cout << "DBG";
               break;
            case ExtraInfo::LOG:
               cout << "LOG";
               break;
            default:
               cout << "???";
               break;
         }
         cout << " '" << extra.Retrieve(t) << "' ";
      }
      cout << endl;
   }
}


TEST_F(GSharpTest, ParseOCode)
{
   Program r;
   ExtraInfo extra;
   string str;

   // find relative path to g-code test files
   string path = __FILE__; // assume the test files are in the same directory as this file
   size_t slash = path.find_last_of("/\\");
   path = path.substr(0, slash+1);


//////////////  "loops.ngc"  ///////////////
   string filename = "loops.ngc";
   ifstream file(path + filename, ifstream::in);
   ASSERT_TRUE(file.good()) << "Cannot open file: " << path + filename;
   stringstream ss;
   ss << file.rdbuf();
//cout << ss.str() << endl; // the file content

   r.DebugLevel(0);
   try{
      r.Load(ss.str());
   }
   catch(ErrorMsg& err){
      FAIL() << "Due to exception: " << err.what();
   }

   r.DebugLevel(0);
   try{
//      GSharpTest_PrintProgramOutput(r);
//      r.Rewind();
//      r.Clear();

      r.DebugLevel(0);
      // the lines expected from stepping through "loops.ngc"
      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_TRUE(str.empty()) << "String: " << str.c_str();
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::MSG), "testing while loop using #1");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "X0");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "X1");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "X2");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_TRUE(str.empty()) << "String: " << str.c_str();
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::MSG), "testing do-while loop using #2");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Y2.3");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Z3");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_TRUE(str.empty()) << "String: " << str.c_str();
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::DBG), "Param 2 equals 2.2");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Y2.2");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Z2");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_TRUE(str.empty()) << "String: " << str.c_str();
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::DBG), "Param 2 equals 2.1");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Y2.1");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Y2");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Y1.9");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Y1.8");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_TRUE(str.empty()) << "String: " << str.c_str();
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::LOG), "testing if statement");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_TRUE(str.empty()) << "String: " << str.c_str();
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::PRN), "Condition uses param 1: 1");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Y1");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_TRUE(str.empty()) << "String: " << str.c_str();
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::MSG), "testing repeat statement");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "X1.4");
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::MSG), "run");
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::DBG), "using param 2 = 1.4");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "X1.1");
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::MSG), "run");
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::DBG), "using param 2 = 1.1");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "X0.8");
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::MSG), "run");
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::DBG), "using param 2 = 0.8");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "M30");
      EXPECT_FALSE(extra.FirstNonEmpty());
    }
   catch(ErrorMsg& err){
      FAIL() << "Due to exception: " << err.what();
   }
   file.close();


////////////  "subroutine.ngc"  ////////////
   filename = "subroutine.ngc";
   file.open(path + filename, std::ifstream::in);
   ASSERT_TRUE(file.good()) << "Cannot open file: " << path + filename;
   ss.str(""); // clear the stream buffer
   ss << file.rdbuf();
//cout << ss.str() << endl; // the file content

   r.DebugLevel(0);
   try{
      r.Load(ss.str());
   }
   catch(ErrorMsg& err){
      FAIL() << "Due to exception: " << err.what();
   }

   try{
//      GSharpTest_PrintProgramOutput(r);
//      r.Rewind();
//      r.Clear();

      r.DebugLevel(0);
      // the lines expected from stepping through "subrouting.ngc"
      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_TRUE(str.empty()) << "String: " << str.c_str();
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::MSG), "Testing subroutine.ngc");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "G0 X0 Y0 Z0");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "X11");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "X12");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_TRUE(str.empty()) << "String: " << str.c_str();
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::DBG), "main [1]=11, [30]=12, [5000]=0");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_TRUE(str.empty()) << "String: " << str.c_str();
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::PRN), "sub1 [1]=1, [2]=2");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Y1");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Y2");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Y12");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Y31");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_TRUE(str.empty()) << "String: " << str.c_str();
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::DBG), "main [1]=11, [30]=12, [5000]=456");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "X456");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "X11");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "X12");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "X31");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_TRUE(str.empty()) << "String: " << str.c_str();
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::PRN), "sub2 [1]=6");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Y6");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "Y26");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_TRUE(str.empty()) << "String: " << str.c_str();
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::DBG), "main [1]=11, [30]=12, [5000]=123");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "X123");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "X11");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_TRUE(str.empty()) << "String: " << str.c_str();
      EXPECT_STREQ(extra.Retrieve(ExtraInfo::LOG), "Finished");

      EXPECT_TRUE(r.Step(str, extra));
      EXPECT_STREQ(str.c_str(), "M2");
      EXPECT_FALSE(extra.FirstNonEmpty());

      EXPECT_FALSE(r.Step(str, extra));
   }
   catch(ErrorMsg& err){
      FAIL() << "Due to exception: " << err.what();
   }
   file.close();

//TODO: create ngc files with errors and catch the expected exceptions (line number? exact phrase?)
}

} // namespace
