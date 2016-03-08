/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#include "Driver1.hpp"
#include "DriverShared.hpp"
#include <stdio.h>

#if DRIVER1
int main(int argc, char* argv[])
{
  typedef void (*TestFn)();
  TestFn tests[] = 
  {
    Driver1Part1Test0,
    Driver1Part1Test1,
    Driver1Part1Test2,
    Driver1Part2Test3,
    Driver1Part2Test4,
    Driver1Part2Test5,
    Driver1Part2Test6,
    Driver1Part3Test7
  };

  return DriverMain(argc, argv, tests, DriverArraySize(tests));
}
#endif

//*********************************************************************************************

Token::Token() :
  mText(""),
  mLength(0),
  mTokenType(0)
{
}

Token::Token(const char* text, size_t length, int type) :
  mText(text),
  mLength(length),
  mTokenType(type)
{
}

std::ostream& operator<<(std::ostream& out, const Token& token)
{
  out << token.str();
  return out;
}

std::string Token::str() const
{
  return std::string(mText, mLength);
}

Token::operator bool() const
{
  return this->mTokenType != TokenType::Invalid;
}

const char* TokenNames[] =
{
  #define TOKEN(Name, Value) #Name,
  #include "Tokens.inl"
  #undef TOKEN
};

// When printing a character it can get pretty annoying if we print actual newlines (we want to print escaped versions instead)
const char* Escape(char c)
{
  unsigned char index = (unsigned char)c;

  // Make an entry for every character + null terminator
  static char characterStrings[256 * 2] = {0};
  static bool initialized = false;

  // Initilaize our table of character strings (this is so we don't have to allocate strings)
  if (initialized == false)
  {
    initialized = true;
    for (size_t i = 0; i < 256; ++i)
      characterStrings[i * 2] = (char)i;
  }

  // If we encounter these special characters, we want to print out the escaped version
  switch (c)
  {
  case '\0':
    return "\\0";
  case '\a':
    return "\\a";
  case '\b':
    return "\\b";
  case '\f':
    return "\\f";
  case '\n':
    return "\\n";
  case '\r':
    return "\\r";
  case '\t':
    return "\\t";
  case '\v':
    return "\\v";
  default:
    return characterStrings + index * 2;
  }
}

void TokenizeAndDeleteRoot(DfaState* root, const char* stream, const char* tokenNames[], std::vector<Token>* tokensOut, TokenReaderFn reader)
{
  // Read until we exhaust the stream
  while (*stream != '\0')
  {
    Token token;
    reader(root, stream, token);
    
    std::string escapedText;
    for (size_t i = 0; i < token.mLength; ++i)
      escapedText += Escape(token.mText[i]);

    printf("Token: '%s' of type %d (%s)\n", escapedText.c_str(), token.mTokenType, tokenNames[token.mTokenType]);
    stream += token.mLength;

    if (token.mLength == 0)
    {
      printf("Skipping one character of input: '%s'\n", Escape(*stream));
      ++stream;
    }
    else if (tokensOut != nullptr)
    {
      tokensOut->push_back(token);
    }
  }

  DeleteStateAndChildren(root);
}

void RunTest(int part, int test, DfaState* root, const char* stream, const char* tokenNames[])
{
  printf("************** PART %d TEST %d **************\n", part, test);
  TokenReaderFn reader = ReadLanguageToken;
  if (part == 1)
    reader = ReadToken;
  TokenizeAndDeleteRoot(root, stream, tokenNames, nullptr, reader);
  printf("*******************************************\n\n");
}

void Driver1Part1Test0()
{
  const char* stream = " abbbba abaaa abbbbbbbbbba  aba abb aa";

  DfaState* root = AddState(0);

  // aba
  DfaState* s1 = AddState(0);
  DfaState* s2 = AddState(1);
  AddEdge(root, s1, 'a');
  AddEdge(s1, s1, 'b');
  AddEdge(s1, s2, 'a');

  // ' '
  DfaState* s3 = AddState(2);
  AddEdge(root, s3, ' ');

  const char* tokenNames[] =
  {
    "Invalid",
    "AbaIdentifier",
    "Space"
  };

  RunTest(1, 0, root, stream, tokenNames);
}

void Driver1Part1Test1()
{
  const char* stream = "==>(hello)+(world)=>>(=>)++++(+ => hello)";

  DfaState* root = AddState(0);

  // ==>
  DfaState* s1 = AddState(0);
  DfaState* s2 = AddState(1);
  AddEdge(root, s1, '=');
  AddEdge(s1, s1, '=');
  AddEdge(s1, s2, '>');

  // (...)+
  DfaState* s3 = AddState(0);
  DfaState* s4 = AddState(2);
  DfaState* s5 = AddState(2);
  AddEdge(root, s3, '(');
  AddDefaultEdge(s3, s3);
  AddEdge(s3, s4, ')');
  AddEdge(s4, s5, '+');

  // + and ++
  DfaState* s6 = AddState(3);
  DfaState* s7 = AddState(4);
  AddEdge(root, s6, '+');
  AddEdge(s6, s7, '+');

  const char* tokenNames[] =
  {
    "Invalid",
    "RightArrow",
    "Group",
    "Plus",
    "Increment"
  };
  
  RunTest(1, 1, root, stream, tokenNames);
}

void Driver1Part1Test2()
{
  const char* stream = "&@attr@aintin-+++&+++@+hello|+world";

  DfaState* root = AddState(0);

  // @at
  DfaState* s1 = AddState(0);
  DfaState* s2 = AddState(0);
  DfaState* s3 = AddState(0);
  AddEdge(root, s1, '@');
  AddEdge(s1, s2, 'a');
  AddEdge(s2, s3, 't');

  // int
  DfaState* s4 = AddState(0);
  DfaState* s5 = AddState(0);
  DfaState* s6 = AddState(0);
  AddEdge(root, s4, 'i');
  AddEdge(s4, s5, 'n');
  AddEdge(s5, s6, 't');

  // ++++++++ or +...|
  DfaState* s7 = AddState(0);
  DfaState* s8 = AddState(0);
  DfaState* s9 = AddState(0);
  DfaState* s10 = AddState(0);
  AddEdge(root, s7, '+');
  AddEdge(s7, s8, '+');
  AddEdge(s8, s8, '+');
  AddDefaultEdge(s7, s9);
  AddDefaultEdge(s9, s9);
  AddEdge(s9, s10, '|');
  
  // &
  DfaState* s11 = AddState(1);
  AddEdge(root, s11, '&');

  const char* tokenNames[] =
  {
    "Invalid",
    "And"
  };
  
  RunTest(1, 2, root, stream, tokenNames);
}

void Driver1Part2Test3()
{
  const char* stream = "hello world__ This is __a Test _of_ \t identifiers123    AND  whitespace\r\n  sUch aS\t\r   TABS_and_NEWLINEs";
  DfaState* root = CreateLanguageDfa();
  RunTest(2, 3, root, stream, TokenNames);
}

void Driver1Part2Test4()
{
  const char* stream = "123 540.99 3.14195f5 7 65.4321e10 1.1e+9 2.2e-8 3.ToString 0.0.0";
  DfaState* root = CreateLanguageDfa();
  RunTest(2, 4, root, stream, TokenNames);
}

void Driver1Part2Test5()
{
  const char* stream = STRINGIZE("hello" "world""this is a test\r\nof string literals" 'a' '\n' 'c' "can you handle \"this\"" "error\j "error two!);
  DfaState* root = CreateLanguageDfa();
  RunTest(2, 5, root, stream, TokenNames);
}

void Driver1Part2Test6()
{
  const char* stream = 
    "class Player\n"
    "{\n"
    "  var Health : float = 99.0f;\n"
    "}\n"
    "\n"
    "function main()\n"
    "{\n"
    "  // Do some simple test /*\n"
    "  var a = 5;\n"
    "  var b : int = 1;\n"
    "  if (a > b)\n"
    "  {\n"
    "    ++a;\n"
    "    b +=-(a+ b) *+2;\n"
    "    Print(\"hello\tworld\"); /* multi-line \"comment\"! */\n"
    "    Print(\"b's value is: \", b);\n"
    "    Print('c');\n"
    "    Print(5.ToString());\n"
    "    \n"
    "    for (int i = 0; i < 99 / 2; ++i)\n"
    "    {\n"
    "      Print(i);\n"
    "    }\n"
    "    \n"
    "    var p = Player();\n"
    "    Print(p.Health);\n"
    "  }\n"
    "}";
  
  DfaState* root = CreateLanguageDfa();
  RunTest(2, 6, root, stream, TokenNames);
}
