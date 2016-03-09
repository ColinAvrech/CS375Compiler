/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#include "Driver4.hpp"
#include "DriverShared.hpp"
#include <stdio.h>

#if DRIVER4
int main(int argc, char* argv[])
{
  typedef void (*TestFn)();
  TestFn tests[] = 
  {
    Driver4Part1Test0,
    Driver4Part1Test1,
    Driver4Part1Test2,
    Driver4Part1Test3,
    Driver4Part1Test4,
    Driver4Part2Test5,
    Driver4Part2Test6,
    Driver4Part2Test7,
    Driver4Part2Test8,
    Driver4Part2Test9,
    Driver4Part3Test10,
    Driver4Part3Test11,
    Driver4Part3Test12,
    Driver4Part3Test13,
    Driver4Part3Test14,
    Driver4Part3Test15,
    Driver4Part3Test16,
    Driver4Part3Test17,
    Driver4Part3Test18,
    Driver4Part3Test19,
    Driver4Part4Test20,
    Driver4Part4Test21,
    Driver4Part4Test22,
    Driver4Part4Test23,
    Driver4Part4Test24,
    Driver4Part4Test25,
    Driver4Part4Test26,
    Driver4Part4Test27,
    Driver4Part4Test28,
    Driver4Part4Test29
  };

  return DriverMain(argc, argv, tests, DriverArraySize(tests));
}
#endif

//*********************************************************************************************
SemanticException::SemanticException()
{
}

SemanticException::SemanticException(const std::string& error) :
  mError(error)
{
}

const char* SemanticException::what() const
{
  return mError.c_str();
}

void ErrorSameName(const std::string& name)
{
  std::stringstream stream;
  stream << "Duplicate symbols with the same name '" << name << "' in the same scope";
  throw SemanticException(stream.str());
}

void ErrorSymbolNotFound(const std::string& name)
{
  std::stringstream stream;
  stream << "The symbol '" << name << "' was not found";
  throw SemanticException(stream.str());
}

void ErrorIncorrectSymbolType(const std::string& name)
{
  std::stringstream stream;
  stream << "The symbol '" << name << "' was a different type than what we expected";
  throw SemanticException(stream.str());
}

void ErrorConditionExpectedBooleanOrPointer(Type* type)
{
  std::stringstream stream;
  stream << "Conditions expect Boolean or Pointer types, and we got '" << type << "'";
  throw SemanticException(stream.str());
}


void ErrorMayOnlyReferToFunctionsOrVariables(const std::string& name)
{
  std::stringstream stream;
  stream << "When used in an expression an identifier '" << name << "' can only refer to a Function or Variable";
  throw SemanticException(stream.str());
}

void ErrorTypeMismatch(Type* expected, Type* got)
{
  std::stringstream stream;
  stream << "We expected the type '" << expected << "' but instead we got '" << got << "'";
  throw SemanticException(stream.str());
}

void ErrorNonCallableType(Type* type)
{
  std::stringstream stream;
  stream << "Attempting to perform a call on a type '" << type << "' that is non-callable (must be a Function type)";
  throw SemanticException(stream.str());
}

void ErrorInvalidMemberAccess(MemberAccessNode* node)
{
  std::stringstream stream;
  stream << "Unable to access member '" << node->mName << "' on type '" << node->mLeft->mResolvedType << "' with operator '" << node->mOperator << "'";
  throw SemanticException(stream.str());
}

void ErrorInvalidCall(CallNode* node)
{
  std::stringstream stream;

  Type* functionType = node->mLeft->mResolvedType;

  stream << "Expected call arguments (";
  
  bool emitComma = false;
  for (Type* parameterType : functionType->mParameterTypes)
  {
    if (emitComma)
      stream << ", ";
    stream << parameterType;
    emitComma = true;
  }

  stream << ") but we got (";

  emitComma = false;
  for (auto& argumentNode : node->mArguments)
  {
    if (emitComma)
      stream << ", ";
    stream << argumentNode->mResolvedType;
    emitComma = true;
  }

  stream << ")";
  
  throw SemanticException(stream.str());
}

void ErrorInvalidCast(Type* from, Type* to)
{
  std::stringstream stream;
  stream << "Invalid cast from '" << from << "' to '" << to << "'";
  throw SemanticException(stream.str());
}

void ErrorInvalidIndexer(IndexNode* node)
{
  std::stringstream stream;
  stream << "Index must only be applied to the Pointer type (got " << node->mLeft->mResolvedType << ") with an Integer index (got " << node->mIndex->mResolvedType << ")";
  throw SemanticException(stream.str());
}

void ErrorBreakContinueMustBeInsideLoop()
{
  throw SemanticException("The break or continue statements must be nested within a loop");
}

void ErrorGotoLabelNotFound(const std::string& name)
{
  std::stringstream stream;
  stream << "A goto was unable to find the label " << name;
  throw SemanticException(stream.str());
}

void ErrorInvalidUnaryOperator(UnaryOperatorNode* node)
{
  std::stringstream stream;
  stream << "The unary operator '" << node->mOperator << "' is not valid with the type '" << node->mRight->mResolvedType << "'";
  throw SemanticException(stream.str());
}

void ErrorInvalidBinaryOperator(BinaryOperatorNode* node)
{
  std::stringstream stream;
  Type* leftType = node->mLeft->mResolvedType;
  Type* rightType = node->mRight->mResolvedType;
  stream << "The binary operator '" << node->mOperator << "' is not valid between the types '" << leftType << "'" << " and '" << rightType << "'";
  throw SemanticException(stream.str());
}

Type* VoidType = nullptr;
Type* NullType = nullptr;
Type* IntegerType = nullptr;
Type* FloatType = nullptr;
Type* BooleanType = nullptr;
Type* ByteType = nullptr;

Library* InitializeCoreLibrary()
{
  static Library core;
  static bool initialized = false;
  
  if (initialized)
    return &core;
  initialized = true;
  
  VoidType      = core.CreateType("Void", true);
  NullType      = core.CreateType("Null", true);
  IntegerType   = core.CreateType("Integer", true);
  FloatType     = core.CreateType("Float", true);
  BooleanType   = core.CreateType("Boolean", true);
  ByteType      = core.CreateType("Byte", true);

  return &core;
}

void RunSemanticTest(int part, int test, const char* stream)
{
  printf("************** PART %d TEST %d **************\n", part, test);
  DfaState* root = CreateLanguageDfa();
  std::vector<Token> tokens;
  TokenizeAndDeleteRoot(root, stream, TokenNames, &tokens, ReadLanguageToken);
  printf("\n");

  Library* core = InitializeCoreLibrary();

  try
  {
    RemoveWhitespaceAndComments(tokens);
    auto rootNode = ParseBlock(tokens);
    auto rootNodePointer = rootNode.get();
    if (rootNodePointer)
    {
      std::vector<Library*> dependencies;
      dependencies.push_back(core);
      
      auto library = std::make_unique<Library>();
      try
      {
        SemanticAnalyize(rootNodePointer, dependencies, library.get());

        PrintTreeWithSymbols(rootNodePointer);
        printf("\n");

        library->Print();
      }
      catch (SemanticException& e)
      {
        PrintTreeWithSymbols(rootNodePointer);
        printf("\n");

        printf("%s\n", e.what());
      }
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

void Driver4Part1Test0()
{
  RunSemanticTest(1, 0, STRINGIZE(class Player { }));
}

void Driver4Part1Test1()
{
  RunSemanticTest(1, 1, STRINGIZE(
    class Player
    {
      var Lives : Integer;
      var Health : Float;
    }
  ));
}

void Driver4Part1Test2()
{
  RunSemanticTest(1, 2, STRINGIZE(
    class Player
    {
      function Speak() { }
      var Lives : Integer;
    }
  ));
}

void Driver4Part1Test3()
{
  RunSemanticTest(1, 3, STRINGIZE(
    function Debug(value : Integer*)
    {
      var next : Integer**;
      
      label Foo;

      var isAlive : Boolean;
    }
  ));
}

void Driver4Part1Test4()
{
  RunSemanticTest(1, 4, STRINGIZE(
    var ThreadCount : Integer;
    var LoaderLock : ThreadLock;
    function Allocate(size : Integer) : Byte* {}
    class ThreadLock {}
    var Derp : Integer**;
  ));
}

void Driver4Part2Test5()
{
  RunSemanticTest(2, 5, STRINGIZE(
    class Player
    {
      var Lives : Integer;
      var Lives : Float;
    }
  ));
}

void Driver4Part2Test6()
{
  RunSemanticTest(2, 6, STRINGIZE(
    var MainThread : Thread;
  ));
}

void Driver4Part2Test7()
{
  RunSemanticTest(2, 7, STRINGIZE(
    var GameEngine : Engine*;
  ));
}


void Driver4Part2Test8()
{
  RunSemanticTest(2, 8, STRINGIZE(
    function Run()
    {
      label End;
      label End;
    }
  ));
}

void Driver4Part2Test9()
{
  RunSemanticTest(2, 9, STRINGIZE(
    var Bob : Print;
  ));
}

void Driver4Part3Test10()
{
  RunSemanticTest(3, 10, STRINGIZE(
    var A : Byte*     = "hello world";
    var B : Integer   = 42;
    var C : Byte      = 'a';
    var D : Float     = 3.14159f;
    var E : Boolean   = true;
    var F : Boolean   = false;
    var G : Null*     = null;
  ));
}

void Driver4Part3Test11()
{
  RunSemanticTest(3, 11, STRINGIZE(
    function Test()
    {
      if (true)
      {
      }
      else
      {
      }

      while (true)
      {
      }

      for (;true;)
      {
      }
    }
  ));
}

void Driver4Part3Test12()
{
  RunSemanticTest(3, 12, STRINGIZE(
    function Test()
    {
      var i : Integer;

      if (true)
      {
        var f : Float;

        i;
        f;
      }
    }
  ));
}

void Driver4Part3Test13()
{
  RunSemanticTest(3, 13, STRINGIZE(
    class Player
    {
      var Lives : Integer;
      var Health : Float;
    }

    function Test()
    {
      var player : Player;
      player.Lives;
      player.Health;

      var pointer : Player*;
      pointer->Lives;
      pointer->Health;
    }
  ));
}

void Driver4Part3Test14()
{
  RunSemanticTest(3, 14, STRINGIZE(
    class Player
    {
      function Speak(volume : Float)
      {
      }
    }

    function ABC(a : Integer, b : Float, c : Byte*) : Byte
    {
      return '\n';
    }

    function Test()
    {
      var c : Byte = ABC(5, 6.0f, "hello");

      var player : Player;
      player.Speak(100.0f);
    }
  ));
}

void Driver4Part3Test15()
{
  RunSemanticTest(3, 15, STRINGIZE(
    function Test()
    {
      var i : Integer;
      i as Float;
      i as Byte*;

      var f : Float;
      f as Integer;

      var p : Integer*;
      p as Float*;
      p as Integer;
      p as Boolean;
    }
  ));
}


void Driver4Part3Test16()
{
  RunSemanticTest(3, 16, STRINGIZE(
    function Test()
    {
      var firstCharacter : Byte = "hello"[0];
      var array : Float*;
      array[42];
    }
  ));
}

void Driver4Part3Test17()
{
  RunSemanticTest(3, 17, STRINGIZE(
    function Test()
    {
      while (true)
      {
        label InsideWhile;
        break;
      }

      goto End;

      for (var i : Integer = 0; false;)
      {
        continue;
      }
      
      label End;
    }
  ));
}

void Driver4Part3Test18()
{
  RunSemanticTest(3, 18, STRINGIZE(
    function Test()
    {
      var i : Integer;
      + - --i;

      var f : Float;
      + - ++f;

      var p : Integer*;
      - * ++p;
      !!p;

      var p2 : Integer* = &i;
    }
  ));
}

void Driver4Part3Test19()
{
  RunSemanticTest(3, 19, STRINGIZE(
    function Test()
    {
      var i : Integer;
      i = i + 1;
      i = (3 - 1) * 5;

      var f : Float;
      f = f + 1.0f;
      f = (3.0f - 1.0f) * 5.0f;

      var p : Integer*;
      p = p + 5;
      p = p - 5;

      i = *p;
      i = p - p;

      var b : Boolean;
      b = 5 > 3;
      b = 9 < 2;
      b = 1 <= 5 || false;
      b = 1 >= 5 && false;
      b = true == false;
      b = 3 != 5;
    }
  ));
}

void Driver4Part4Test20()
{
  RunSemanticTest(4, 20, STRINGIZE(
    function Test()
    {
      return 5;
    }
  ));
}

void Driver4Part4Test21()
{
  RunSemanticTest(4, 21, STRINGIZE(
    function Test()
    {
      if (3.0f)
      {
      }
      else if (5)
      {
      }
    }
  ));
}

// Tests errors in resolving a local variable by name/scope
void Driver4Part4Test22()
{
  RunSemanticTest(4, 22, STRINGIZE(
    function Test()
    {
      if (true)
      {
        var a : Integer;
      }

      a = 5;
    }
  ));
}

void Driver4Part4Test23()
{
  RunSemanticTest(4, 23, STRINGIZE(
    class Player
    {
      var Lives : Integer;
    }

    function Test()
    {
      var p : Player*;
      p.Lives = 9;
    }
  ));
}

void Driver4Part4Test24()
{
  RunSemanticTest(4, 24, STRINGIZE(
    function ABC(a : Integer, b : Float, c : Byte*) : Byte
    {
      return '\n';
    }

    function Test()
    {
      ABC(6, "hello", 3.0f);
    }
  ));
}

void Driver4Part4Test25()
{
  RunSemanticTest(4, 25, STRINGIZE(
    function Test()
    {
      5.0f as Integer*;
    }
  ));
}

void Driver4Part4Test26()
{
  RunSemanticTest(4, 26, STRINGIZE(
    function Test()
    {
      5.0f[0];
    }
  ));
}

void Driver4Part4Test27()
{
  RunSemanticTest(4, 27, STRINGIZE(
    function Test()
    {
      break;
    }
  ));
}

void Driver4Part4Test28()
{
  RunSemanticTest(4, 28, STRINGIZE(
    function Test()
    {
      *5;
    }
  ));
}

void Driver4Part4Test29()
{
  RunSemanticTest(4, 29, STRINGIZE(
    function Test()
    {
      var i : Integer;
      &i % &i;
    }
  ));
}