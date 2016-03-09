/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#include "Driver2.hpp"
#include "DriverShared.hpp"
#include <stdio.h>

#if DRIVER2
int main(int argc, char* argv[])
{
  typedef void (*TestFn)();
  TestFn tests[] = 
  {
    Driver2Part1Test0,
    Driver2Part1Test1,
    Driver2Part1Test2,
    Driver2Part1Test3,
    Driver2Part2Test4,
    Driver2Part2Test5,
    Driver2Part2Test6,
    Driver2Part2Test7,
    Driver2Part3Test8,
    Driver2Part3Test9,
    Driver2Part3Test10,
    Driver2Part3Test11,
    Driver2Part3Test12,
    Driver2Part3Test13
  };

  return DriverMain(argc, argv, tests, DriverArraySize(tests));
}
#endif

//*********************************************************************************************

ParsingException::ParsingException()
{
}

ParsingException::ParsingException(const std::string& error) :
  mError(error)
{
}

const char* ParsingException::what() const
{
  return mError.c_str();
}

std::vector<PrintRule*> PrintRule::ActiveRules;

void PrintRule::PrintTabs()
{
  for (size_t i = 0; i < ActiveRules.size(); ++i)
    mStream << "| ";
}

PrintRule::PrintRule(const char* rule) :
  mAccepted(false),
  mName(rule)
{
  this->PrintTabs();
  this->mStream << rule << "\n";
  ActiveRules.push_back(this);
}

PrintRule::~PrintRule()
{
  ActiveRules.pop_back();

  if (this->mAccepted || std::uncaught_exception())
  {
    this->PrintTabs();

    if (std::uncaught_exception())
      this->mStream << "End" << this->mName << "*\n";
    else
      this->mStream << "End" << this->mName << "\n";

    if (ActiveRules.empty())
      printf("%s\n", this->mStream.str().c_str());
    else
      ActiveRules.back()->mStream << this->mStream.str();
  }
}

bool PrintRule::Accept()
{
  this->mAccepted = true;
  return true;
}

void PrintRule::AcceptedToken(TokenType::Enum type)
{
  PrintRule* rule = GetLatestRule();
  if (rule == nullptr)
  {
    printf("Accepting a token when we haven't entered a rule yet (construct a PrintRule on the stack)");
    return;
  }

  rule->PrintTabs();
  rule->mStream << "Accepted: " << TokenNames[type] << "\n";
}

void PrintRule::DebugPrintLine(const char* text)
{
  this->PrintTabs();
  this->mStream << text << "\n";
}

PrintRule* PrintRule::GetLatestRule()
{
  if (ActiveRules.empty())
    return nullptr;

  return ActiveRules.back();
}

void RunParserTest(std::vector<Token>& tokens)
{
  try
  {
    RemoveWhitespaceAndComments(tokens);
    Recognize(tokens);
    printf("Parsing Successful\n");
  }
  catch (ParsingException&)
  {
    printf("Parsing Failed (Exception)\n");
  }
}

void RunTest(int part, int test, std::vector<Token>& tokens)
{
  printf("************** PART %d TEST %d **************\n", part, test);
  RunParserTest(tokens);
  printf("*******************************************\n\n");
}

void RunTest(int part, int test, const char* stream)
{
  printf("************** PART %d TEST %d **************\n", part, test);
  DfaState* root = CreateLanguageDfa();
  std::vector<Token> tokens;
  TokenizeAndDeleteRoot(root, stream, TokenNames, &tokens, ReadLanguageToken);
  printf("\n");
  RunParserTest(tokens);
  printf("*******************************************\n\n");
}

void AddToken(std::vector<Token>& tokens, const char* text, TokenType::Enum type)
{
  tokens.push_back(Token(text, strlen(text), type));
}

void Driver2Part1Test0()
{
  std::vector<Token> tokens;
  RunTest(1, 0, tokens);
}

void Driver2Part1Test1()
{
  std::vector<Token> tokens;
  AddToken(tokens, "class", TokenType::Class);
  AddToken(tokens, "Player", TokenType::Identifier);
  AddToken(tokens, "{", TokenType::OpenCurley);
  AddToken(tokens, "}", TokenType::CloseCurley);

  RunTest(1, 1, tokens);
}

void Driver2Part1Test2()
{
  std::vector<Token> tokens;
  AddToken(tokens, "class", TokenType::Class);
  AddToken(tokens, "Enemy", TokenType::Identifier);
  AddToken(tokens, "{", TokenType::OpenCurley);
  AddToken(tokens, "var", TokenType::Var);
  AddToken(tokens, "Lives", TokenType::Identifier);
  AddToken(tokens, ":", TokenType::Colon);
  AddToken(tokens, "Integer", TokenType::Identifier);
  AddToken(tokens, ";", TokenType::Semicolon);
  AddToken(tokens, "}", TokenType::CloseCurley);

  RunTest(1, 2, tokens);
}

void Driver2Part1Test3()
{
  std::vector<Token> tokens;
  AddToken(tokens, "class", TokenType::Class);
  AddToken(tokens, "Frog", TokenType::Identifier);
  AddToken(tokens, "{", TokenType::OpenCurley);
  
  AddToken(tokens, "function", TokenType::Function);
  AddToken(tokens, "EatFly", TokenType::Identifier);
  AddToken(tokens, "(", TokenType::OpenParentheses);
  AddToken(tokens, "target", TokenType::Identifier);
  AddToken(tokens, ":", TokenType::Colon);
  AddToken(tokens, "Fly", TokenType::Identifier);
  AddToken(tokens, ",", TokenType::Comma);
  AddToken(tokens, "speed", TokenType::Identifier);
  AddToken(tokens, ":", TokenType::Colon);
  AddToken(tokens, "Float", TokenType::Identifier);
  AddToken(tokens, ")", TokenType::CloseParentheses);
  AddToken(tokens, ":", TokenType::Colon);
  AddToken(tokens, "Boolean", TokenType::Identifier);
  AddToken(tokens, "{", TokenType::OpenCurley);
  AddToken(tokens, "}", TokenType::CloseCurley);

  AddToken(tokens, "var", TokenType::Var);
  AddToken(tokens, "FliesEaten", TokenType::Identifier);
  AddToken(tokens, ":", TokenType::Colon);
  AddToken(tokens, "Integer", TokenType::Identifier);
  AddToken(tokens, ";", TokenType::Semicolon);
  AddToken(tokens, "}", TokenType::CloseCurley);

  RunTest(1, 3, tokens);
}

void Driver2Part2Test4()
{
  RunTest(2, 4, "class Player { }");
}

void Driver2Part2Test5()
{
  RunTest(2, 5, "class Player { var Lives : Integer; function Jump(height : Float, doubleJump : Boolean) : Void { if (true) { } else if (true) { } else { } } }");
}

void Driver2Part2Test6()
{
  RunTest(2, 6, "class Player { function Test() { var test : Integer; test = 5 + 9 * (3 - 1); this.Test(); } }");
}

void Driver2Part2Test7()
{
  RunTest(2, 7, "function X() : Integer { label X; goto X; while(true) { continue; } for(var i : Integer = 0; i < 10 && !(i == 5 || i != 12); i = i + 1) { i = 3.0 as Integer; break; } X(\"hello\", 3.0, null); array[0] = 5; return 0; }");
}

void Driver2Part3Test8()
{
  RunTest(3, 8, "class Player { function");
}

void Driver2Part3Test9()
{
  RunTest(3, 9, "class Player { function Test(hello : Integer,) { } }");
}

void Driver2Part3Test10()
{
  RunTest(3, 10, "class Player { function Test() { \"hello\".Print(); var i : Integer; i = 5 +; } }");
}

void Driver2Part3Test11()
{
  RunTest(3, 11, "class Player { function Test() {  } } }");
}

void Driver2Part3Test12()
{
  RunTest(3, 12, "class Player { var Lives : Integer = [0]; }");
}

void Driver2Part3Test13()
{
  RunTest(3, 13, "function Test() { for (;;) { this.Test(this.Test[0]) } }");
}
