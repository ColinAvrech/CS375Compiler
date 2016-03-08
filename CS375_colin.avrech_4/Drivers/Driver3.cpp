/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#include "Driver3.hpp"
#include "DriverShared.hpp"
#include <stdio.h>

#if DRIVER3
int main(int argc, char* argv[])
{
  typedef void (*TestFn)();
  TestFn tests[] = 
  {
    Driver3Part1Test0,
    Driver3Part1Test1,
    Driver3Part1Test2,
    Driver3Part1Test3,
    Driver3Part1Test4,
    Driver3Part1Test5,
    Driver3Part1Test6,
    Driver3Part1Test7,
    Driver3Part1Test8,
    Driver3Part2Test9,
    Driver3Part2Test10,
    Driver3Part2Test11,
    Driver3Part2Test12,
    Driver3Part2Test13,
    Driver3Part2Test14,
    Driver3Part2Test15,
    Driver3Part2Test16,
    Driver3Part2Test17,
    Driver3Part3Test18,
    Driver3Part3Test19,
    Driver3Part3Test20
  };

  return DriverMain(argc, argv, tests, DriverArraySize(tests));
}
#endif

//*********************************************************************************************
std::vector<NodePrinter*> NodePrinter::ActiveNodes;

NodePrinter::NodePrinter()
{
  for (size_t i = 0; i < ActiveNodes.size(); ++i)
    (*this) << "| ";

  ActiveNodes.push_back(this);
}

NodePrinter::NodePrinter(const std::string& text)
{
  for (size_t i = 0; i < ActiveNodes.size(); ++i)
    (*this) << "| ";

  ActiveNodes.push_back(this);
  (*this) << text;
}

NodePrinter::~NodePrinter()
{
  ActiveNodes.pop_back();

  if (ActiveNodes.empty())
    printf("%s\n", this->str().c_str());
  else
    (*ActiveNodes.back()) << "\n" << this->str();
}

template <typename T>
void RunAstTest(int part, int test, T (*parser)(std::vector<Token>& tokens), const char* stream)
{
  printf("************** PART %d TEST %d **************\n", part, test);
  DfaState* root = CreateLanguageDfa();
  std::vector<Token> tokens;
  TokenizeAndDeleteRoot(root, stream, TokenNames, &tokens, ReadLanguageToken);
  printf("\n");

  try
  {
    RemoveWhitespaceAndComments(tokens);
    auto rootNode = parser(tokens);
    if (rootNode)
    {
      printf("Parsing Successful\n\n");
      PrintTree(rootNode.get());
    }
    else
    {
      printf("Parsing Failed (Null Node)\n");
    }
  }
  catch (ParsingException&)
  {
    printf("Parsing Failed (Exception)\n");
  }

  printf("*******************************************\n\n");
}

void Driver3Part1Test0()
{
  RunAstTest(1, 0, ParseExpression, STRINGIZE("hello world"));
}

void Driver3Part1Test1()
{
  RunAstTest(1, 1, ParseExpression, STRINGIZE(5 + 3.0));
}

void Driver3Part1Test2()
{
  RunAstTest(1, 2, ParseExpression, STRINGIZE(9 * 2.0f * 1 * 160 / 5 / 10.0));
}

void Driver3Part1Test3()
{
  RunAstTest(1, 3, ParseExpression, STRINGIZE(a = b = c = d));
}

void Driver3Part1Test4()
{
  RunAstTest(1, 4, ParseExpression, STRINGIZE(a = b = 5 + 3 + 9.0));
}

void Driver3Part1Test5()
{
  RunAstTest(1, 5, ParseExpression, STRINGIZE(5 + 3 * 9 - 6 / (2 + 2) || a == b && a != -c));
}

void Driver3Part1Test6()
{
  RunAstTest(1, 6, ParseExpression, STRINGIZE(!--(*++a - -5)));
}

void Driver3Part1Test7()
{
  RunAstTest(1, 7, ParseExpression, STRINGIZE("what"[0].ToString(formatter)->c_str() as Integer + 5));
}

void Driver3Part1Test8()
{
  RunAstTest(1, 8, ParseExpression, STRINGIZE(((99.0 + ("str" + "ing")[a + ++b * 2][0] as Double + player.Health->Lives + enemy.GetMaxLives(3 + a[0], 9 as Double)) > 0 || true) && false));
}

void Driver3Part2Test9()
{
  RunAstTest(2, 9, ParseBlock, "");
}

void Driver3Part2Test10()
{
  RunAstTest(2, 10, ParseBlock, "function Print(value : String*) { }");
}

void Driver3Part2Test11()
{
  RunAstTest(2, 11, ParseBlock, "class Player { }");
}

void Driver3Part2Test12()
{
  RunAstTest(2, 12, ParseBlock, "function DoSomething() { while(true) { } }");
}

void Driver3Part2Test13()
{
  RunAstTest(2, 13, ParseBlock, "function DoSomething() { for (var i : Integer = 0; i < 100; ++i) { Print(\"hello world\"); } }");
}

void Driver3Part2Test14()
{
  RunAstTest(2, 14, ParseBlock, "function DoSomething() { var a : Integer = 10; if (a > 5) { } }");
}

void Driver3Part2Test15()
{
  RunAstTest(2, 15, ParseBlock, "function DoSomething() { var b : Integer = 20; if (b == 10) { } else if (b == 20) { } else { } }");
}

void Driver3Part2Test16()
{
  const char* code =
    "class Player\n"
    "{\n"
    "  var Lives : Integer = 9;\n"
    "  var Health : Float = 100.0f;\n"
    "  function ApplyDamage(amount : Float) : Void\n"
    "  {\n"
    "    this.Health -= amount;\n"
    "    if (this.Health <= 0.0f)\n"
    "    {\n"
    "      --this.Lives;\n"
    "      this.Health = 100.0f;\n"
    "      this.SendDeathEvent();\n"
    "    }\n"
    "  }\n"
    "  function SendDeathEvent()\n"
    "  {\n"
    "    // Not implemented yet\n"
    "  }\n"
    "}\n";

  RunAstTest(2, 16, ParseBlock, code);
}

// Parse several different block level empty classes and functions with various expressions and statements
void Driver3Part2Test17()
{
  const char* code =
    "class Player\n"
    "{\n"
    "  var Lives : Integer = 9 * 1 + 0;\n"
    "  var Health : Float = 100.0f;\n"
    "  function ApplyDamage(amount : Float) : Boolean\n"
    "  {\n"
    "    this.Health -= amount;\n"
    "    if (this.Health <= 0.0f)\n"
    "    {\n"
    "      --this.Lives;\n"
    "      this.Health = 100.0f;\n"
    "      this.SendDeathEvent();\n"
    "      goto End;\n"
    "    }\n"
    "    else\n"
    "    {\n"
    "      while (this.Health < 50.0f)\n"
    "      {\n"
    "        this.Health += 1.0f * 2;\n"
    "        if (this.Health as Integer % 2 == 0)\n"
    "        {\n"
    "          continue;\n"
    "        }\n"
    "        this.Health *= 1.1f;"
    "        if (this.Health > 30.0f)\n"
    "        {\n"
    "          break;\n"
    "        }\n"
    "      }\n"
    "      return false;\n"
    "    }\n"
    "    label End;\n"
    "    return true;\n"
    "  }\n"
    "  function SendDeathEvent()\n"
    "  {\n"
    "    // Not implemented yet\n"
    "  }\n"
    "}\n";

  RunAstTest(2, 17, ParseBlock, code);
}

void Driver3Part3Test18()
{
  RunAstTest(3, 18, ParseExpression, "5 + ");
}

void Driver3Part3Test19()
{
  RunAstTest(3, 19, ParseBlock, "var gCount : Integer = (a + b * 9");
}

void Driver3Part3Test20()
{
  RunAstTest(3, 20, ParseBlock, "function Explode() { if (a > 5) { } else }");
}
