/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#pragma once
#ifndef COMPILER_CLASS_DRIVER_2
#define COMPILER_CLASS_DRIVER_2

#include "Driver1.hpp"
#include <string>
#include <sstream>
#include <vector>
#include <exception>
#include <memory>

/***************************** DRIVER *****************************/

// Throw this exception type when a parsing error occurrs
class ParsingException : public std::exception
{
public:
  ParsingException();
  ParsingException(const std::string& error);
  const char* what() const override;
  std::string mError;
};

// Called before we attempt to parse or recognize a stream of tokens
// This should remove all whitespace and comments from the token stream
void RemoveWhitespaceAndComments(std::vector<Token>& tokens);

// Determine whether the input stream of tokens represents a valid parsable program
// As soon as a rule is accepted, the name of the rule should be printed to stdout
// If we fail parsing, we should throw ParsingException (otherwise success is assumed)
void Recognize(std::vector<Token>& tokens);

// Every time we enter a rule in our recursive descent parser we should
// create one of these on the stack at the very beginning
// At the end of a rule we should call Accept if the rule successfully parsed
// It is assuemed that a rule is not accepted if Accept is not called
// If an exception is being thrown then the rule will print out with a closing bracket ']'
class PrintRule
{
public:
  PrintRule(const char* rule);
  ~PrintRule();

  // Should be called when we complete a rule and does not need to be called if we fail
  // If the passed in value is true, then the rule will be accepted and printed
  // The return value is always the same as the passed in value (to help simplify and shorten code)
  template <typename T>
  T Accept(T result)
  {
    if (result)
      this->Accept();
    return std::move(result);
  }
  bool Accept();

  // This should be called every time we accept a token
  // Note that this is static, so we don't actually need to know the rule we are within
  static void AcceptedToken(TokenType::Enum type);

  // Debug prints any text the user wants to the current position in the tree
  // Do not use this in the final submission
  void DebugPrintLine(const char* text);

  // Gets the latest print rule, or returns null if none are active (generally used for debugging)
  static PrintRule* GetLatestRule();

private:
  bool mAccepted;
  void PrintTabs();
  static std::vector<PrintRule*> ActiveRules;
  std::stringstream mStream;
  std::string mName;
};

/***************************** TESTS  *****************************/

// Tests an empty token stream
void Driver2Part1Test0();

// Tests an empty class token stream
void Driver2Part1Test1();

// Tests a class with a member variable inside of it
void Driver2Part1Test2();

// Tests a class with a member function and member variable inside of it
void Driver2Part1Test3();

// Using your tokenizer, tests an empty class
void Driver2Part2Test4();

// Using your tokenizer, tests a class with a member variable and member function with an if statement
void Driver2Part2Test5();

// Using your tokenizer, tests a class with a member function with a local variable, expression, and function call
void Driver2Part2Test6();

// Using your tokenizer, tests a majority of the constructs of the grammar
void Driver2Part2Test7();

// Failing test: Missing curley brace on class
void Driver2Part3Test8();

// Failing test: Extra comma in function parameters
void Driver2Part3Test9();

// Failing test: Missing expression after a + binary operator
void Driver2Part3Test10();

// Failing test: We didn't parse everything; extra closing curley brace at the end
void Driver2Part3Test11();

// Failing test: Unexpected expression due to a random indexing operator [0]
void Driver2Part3Test12();

// Failing test: Missing semicolon
void Driver2Part3Test13();

/***************************** INTERNAL *****************************/
// These are for usage by the other drivers

#endif
