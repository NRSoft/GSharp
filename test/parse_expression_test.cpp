#include "gsharp_test.h"
#include "../src/gsharp_program.h"
#include "../src/gsharp_except.h"

/*
from "The NIST RS274/NGC Interpreter - Version 3":
 (https://www.nist.gov/customcf/get_pdf.cfm?pub_id=823374)

C.2 Sample Program to Test Expressions

This file is to test the interpretation of expressions. It tests all unary and binary functions
implemented. It also tests parameter setting and referencing. The last few lines test more
complicated expressions. This program is not intended to be run on a machining center. The tool
path is just random back and forth on the X-axis.

n0010 g21 g1 x3 f20 (expression test)
n0020 x [1 + 2] (x should be 3)
n0030 x [1 - 2] (x should be -1)
n0040 x [1 --3] (x should be 4)
n0050 x [2/5] (x should be 0.40)
n0060 x [3.0 * 5] (x should be 15)
n0070 x [0 OR 0] (x should be 0)
n0080 x [0 OR 1] (x should be 1)
n0090 x [2 or 2] (x should be 1)
n0100 x [0 AND 0] (x should be 0)
n0110 x [0 AND 1] (x should be 0)
n0120 x [2 and 2] (x should be 1)
n0130 x [0 XOR 0] (x should be 0)
n0140 x [0 XOR 1] (x should be 1)
n0150 x [2 xor 2] (x should be 0)
n0160 x [15 MOD 4.0] (x should be 3)
n0170 x [1 + 2 * 3 - 4 / 5] (x should be 6.2)
n0180 x sin[30] (x should be 0.5)
n0190 x cos[0.0] (x should be 1.0)
n0200 x tan[60.0] (x should be 1.7321)
n0210 x sqrt[3] (x should be 1.7321)
n0220 x atan[1.7321]/[1.0] (x should be 60.0)
n0230 x asin[1.0] (x should be 90.0)
n0240 x acos[0.707107] (x should be 45.0000)
n0250 x abs[20.0] (x should be 20)
n0260 x abs[-1.23] (x should be 1.23)
n0270 x round[-0.499] (x should be 0)
n0280 x round[-0.5001] (x should be -1.0)
n0290 x round[2.444] (x should be 2)
n0300 x round[9.975] (x should be 10)
n0310 x fix[-0.499] (x should be -1.0)
n0320 x fix[-0.5001] (x should be -1.0)
n0330 x fix[2.444] (x should be 2)
n0340 x fix[9.975] (x should be 9)
n0350 x fup[-0.499] (x should be 0.0)
n0360 x fup[-0.5001] (x should be 0.0)
n0370 x fup[2.444] (x should be 3)
n0380 x fup[9.975] (x should be 10)
n0390 x exp[2.3026] (x should be 10)
n0400 x ln[10.0] (x should be 2.3026)
n0410 x [2 ** 3.0] #1=2.0 (x should be 8.0)
n0420 ##1 = 0.375 (#1 is 2, so parameter 2 is set to 0.375)
n0430 x #2 (x should be 0.375) #3=7.0
n0440 #3=5.0 x #3 (parameters set in parallel, so x should be 7, not 5)
n0450 x #3 #3=1.1 (parameters set in parallel, so x should be 5, not 1.1)
n0460 x [2 + asin[1/2.1+-0.345] / [atan[fix[4.4] * 2.1 * sqrt[16.8]] /[-18]]**2]
n0470 x sqrt[3**2 + 4**2] (x should be 5.0)
n0480 m2

*/

namespace gsharp
{

TEST_F(GSharpTest, ParseExpression)
{
   gsharp::Program r;

   r.DebugLevel(0);

   try{
      // whitespaces and comments
      EXPECT_STREQ("Y2", r._ParseLine("Y[ ROU N(com (ment) ) D (co) [2. 37() 4571]] ").c_str());
      // plus and minus signs
      EXPECT_STREQ("Z-3", r._ParseLine("z [-1*+3]").c_str());
      // parameter assignment
      EXPECT_STREQ("X0", r._ParseLine("#1=.8 x#1").c_str());
      EXPECT_STREQ("X0.8", r._ParseLine("#1=[#1+1] x#1").c_str());
      EXPECT_STREQ("Y1.8", r._ParseLine("y#1").c_str());
      // spaces between g-words
      r.EnablePrettyFormat(false); // disable
      EXPECT_STREQ("G21G1X3F20", r._ParseLine("n0010 g21 g1 x3 f20").c_str());
      // upper-case output
      r.EnableConvertToUpper(false); // disable
      EXPECT_STREQ("g21g1x3f20", r._ParseLine("n0010 g21 g1 x3 f20").c_str());
      r.EnableConvertToUpper();
      r.EnablePrettyFormat();

      // the recommended tests
      EXPECT_STREQ("G21 G1 X3 F20", r._ParseLine("n0010 g21 g1 x3 f20").c_str());
      EXPECT_STREQ("X3", r._ParseLine("n0020 x [1 + 2]").c_str());
      EXPECT_STREQ("X-1", r._ParseLine("n0030 x [1 - 2]").c_str());
      EXPECT_STREQ("X4", r._ParseLine("n0040 x [1 --3]").c_str());
      EXPECT_STREQ("X0.4", r._ParseLine("n0050 x [2/5]").c_str());
      EXPECT_STREQ("X15", r._ParseLine("n0060 x [3.0 * 5]").c_str());
      EXPECT_STREQ("X0", r._ParseLine("n0070 x [0 OR 0]").c_str());
      EXPECT_STREQ("X1", r._ParseLine("n0080 x [0 OR 1]").c_str());
      EXPECT_STREQ("X1", r._ParseLine("n0090 x [2 or 2]").c_str());
      EXPECT_STREQ("X0", r._ParseLine("n0100 x [0 AND 0]").c_str());
      EXPECT_STREQ("X0", r._ParseLine("n0110 x [0 AND 1]").c_str());
      EXPECT_STREQ("X1", r._ParseLine("n0120 x [2 and 2]").c_str());
      EXPECT_STREQ("X0", r._ParseLine("n0130 x [0 XOR 0]").c_str());
      EXPECT_STREQ("X1", r._ParseLine("n0140 x [0 XOR 1]").c_str());
      EXPECT_STREQ("X0", r._ParseLine("n0150 x [2 xor 2]").c_str());
      EXPECT_STREQ("X3", r._ParseLine("n0160 x [15 MOD 4.0]").c_str());
      EXPECT_STREQ("X6.2", r._ParseLine("n0170 x [1 + 2 * 3 - 4 / 5]").c_str());
      EXPECT_STREQ("X0.5", r._ParseLine("n0180 x sin[30]").c_str());
      EXPECT_STREQ("X1", r._ParseLine("n0190 x cos[0.0]").c_str());
      EXPECT_STREQ("X1.7321", r._ParseLine("n0200 x tan[60.0]").c_str());
      EXPECT_STREQ("X1.7321", r._ParseLine("n0210 x sqrt[3]").c_str());
      EXPECT_STREQ("X60", r._ParseLine("n0220 x atan[1.73205]/[1.0]").c_str());
      EXPECT_STREQ("X90", r._ParseLine("n0230 x asin[1.0]").c_str());
      EXPECT_STREQ("X45", r._ParseLine("n0240 x acos[0.707107]").c_str());
      EXPECT_STREQ("X20", r._ParseLine("n0250 x abs[20.0]").c_str());
      EXPECT_STREQ("X1.23", r._ParseLine("n0260 x abs[-1.23]").c_str());
      EXPECT_STREQ("X0", r._ParseLine("n0270 x round[-0.499]").c_str());
      EXPECT_STREQ("X-1", r._ParseLine("n0280 x round[-0.5001]").c_str());
      EXPECT_STREQ("X2", r._ParseLine("n0290 x round[2.444]").c_str());
      EXPECT_STREQ("X10", r._ParseLine("n0300 x round[9.975]").c_str());
      EXPECT_STREQ("X-1", r._ParseLine("n0310 x fix[-0.499]").c_str());
      EXPECT_STREQ("X-1", r._ParseLine("n0320 x fix[-0.5001]").c_str());
      EXPECT_STREQ("X2", r._ParseLine("n0330 x fix[2.444]").c_str());
      EXPECT_STREQ("X9", r._ParseLine("n0340 x fix[9.975]").c_str());
      EXPECT_STREQ("X0", r._ParseLine("n0350 x fup[-0.499]").c_str());
      EXPECT_STREQ("X0", r._ParseLine("n0360 x fup[-0.5001]").c_str());
      EXPECT_STREQ("X3", r._ParseLine("n0370 x fup[2.444]").c_str());
      EXPECT_STREQ("X10", r._ParseLine("n0380 x fup[9.975]").c_str());
      EXPECT_STREQ("X10", r._ParseLine("n0390 x exp[2.302585]").c_str());
      EXPECT_STREQ("X2.3026", r._ParseLine("n0400 x ln[10.0]").c_str());
      EXPECT_STREQ("X8", r._ParseLine("n0410 x [2 ** 3.0] #1=2.0").c_str());
      EXPECT_STREQ("", r._ParseLine("n0420 ##1 = 0.375").c_str());
      EXPECT_STREQ("X0.375", r._ParseLine("n0430 x #2 #3=7.0").c_str());
      EXPECT_STREQ("X7", r._ParseLine("n0440 #3=5.0 x #3").c_str());
      EXPECT_STREQ("X5", r._ParseLine("n0450 x #3 #3=1.1").c_str());
      // this line makes a bit more sense compared to the original below
      EXPECT_STREQ("X0.0005", r._ParseLine("n0460 x [asin[1/2.1+-0.345] / [atan[fix[4.4] * 2.1 * sqrt[16.8]] /[-18]]**2]").c_str());
//      EXPECT_STREQ("x2", r._ParseLine("n0460 x [2 + asin[1/2.1+-0.345] / [atan[fix[4.4] * 2.1 * sqrt[16.8]] /[-18]]**2]").c_str());
      EXPECT_STREQ("X5", r._ParseLine("n0470 x sqrt[3**2 + 4**2]").c_str());
      EXPECT_STREQ("M2", r._ParseLine("m2").c_str());

   }
   catch(ErrorMsg& err){
      FAIL() << "Due to exception: " << err.what();
   }

//TODO: test failure cases to catch the expected exceptions (line number? exact phrase?)
}

} // namespace
