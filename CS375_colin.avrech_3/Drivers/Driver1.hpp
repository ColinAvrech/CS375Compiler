/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#pragma once
#ifndef COMPILER_CLASS_DRIVER_1
#define COMPILER_CLASS_DRIVER_1

#include <string>
#include <vector>

/***************************** PART 1 *****************************/

// The token types that we will return in part two
// It is advised that your own code just uses the same enum, or an enum with the same numeric ids
namespace TokenType
{
  enum Enum
  {
    #define TOKEN(Name, Value) Name,
    #include "Tokens.inl"
    #undef TOKEN
    EnumCount
  };
}

// An array of names for each of the tokens (the index is the token id)
const char* TokenNames[];

// The token type that the driver expects to be returned
// The user must fill out the text of the parsed token and the accepted token type (defaults to 0 / invalid)
class Token
{
public:
  Token();
  Token(const char* str, size_t length, int type);
  const char* mText;
  size_t mLength;

  // Allow streams to output our token
  friend std::ostream& operator<<(std::ostream& out, const Token& token);

  // A convenience function to get a sliced version of the string
  std::string str() const;

  // A conversion to bool (only true if the token type is not Invalid)
  // This is useful for shorthanding some 'Expect' operations
  operator bool() const;

  // We use a union because for Part 1 of the assignment the driver will be making up fake token types
  // that don't belong in the TokenType enumeration. Having the enumeration there makes it easier to debug and visualize
  union
  {
    int mTokenType;
    TokenType::Enum mEnumTokenType;
  };
};

// You must implement this state in your own code
class DfaState;

// Creates a state and returns the created state (whatever structure the user wants to create)
// If the 'acceptingToken' is 0, then this is NOT an accepting state
// Any value other than 0 means this state accepts a token with that numeric id
DfaState* AddState(int acceptingToken);

// Add an edge transition from one state to another on the given character (the user must cast from/to into their own state types)
// The driver will never add two edges on the same character from the same state
void AddEdge(DfaState* from, DfaState* to, char c);

// Add an edge transition from one state to another on ANY character (the user must cast from/to into their own state types)
// The driver will never add two default edges from the same state
void AddDefaultEdge(DfaState* from, DfaState* to);

// Read a token starting from the given character stream and root/starting state
// If an error occurs with tokenizing, the token type of 0 should be returned as well
// as the text that it read up to (excluding the last read character)
// Users code can internally read tokens with any structure they prefer, but
// should return the 'Token' with all members filled out correctly
void ReadToken(DfaState* startingState, const char* stream, Token& outToken);

// This is only ever called on the root or starting state
// This should delete all states in the state machine, as well as any edge structure (if the user created them)
void DeleteStateAndChildren(DfaState* root);


/***************************** PART 2 *****************************/

// Same as the ReadToken above, except the user is expected to detect keywords after parsing
// an Identifier and set the correct token type (generally should call the above ReadToken)
void ReadLanguageToken(DfaState* startingState, const char* stream, Token& outToken);

// Create the entire state machine for part two, and return the root / starting state
DfaState* CreateLanguageDfa();

/***************************** TESTS  *****************************/

// Stream: " abbbba abaaa abbbbbbbbbba  aba abb aa"
// Token type 1: 'a', with any number of 'b's, and then ending with 'a'
// Token type 2: ' ' (space)
void Driver1Part1Test0();

// Stream: "==>(hello)+(world)=>>(=>)++++(+ => hello)"
// Token type 1: one or more '=' followed by a '>'
// Token type 2: '(' with zero or more characters of any type inside followed by ')' and then an optional '+'
// Token type 3: '+'
// Token type 4: '++'
void Driver1Part1Test1();

// Stream: "&@attr@aintin-+++&+++@+hello|+world"
// This test creates states and edges but many of them are NOT accepting (only one token, the '&', is accepted)
// This technique can be useful for detecting invalid tokens and giving good errors and tests stability of your tokenizer
// Token: '@at'
// Token: 'int'
// Token: '+' followed by any number of '+', or any number of any other character but ending with '|'
// Token type 1: '&'
void Driver1Part1Test2();

// Identifiers and whitespace:
// Stream: "hello world__ This is __a Test _of_ \t identifiers123    AND  whitespace\r\n  sUch aS\t\r   TABS_and_NEWLINEs"
void Driver1Part2Test3();

// Numeric literals with identifers, whitespace, and a member access operator:
// Stream: "123 540.99 3.14195f5 7 65.4321e10 1.1e+9 2.2e-8 3.ToString 0.0.0"
void Driver1Part2Test4();

// String and character literals (because of the string literals, we did NOT quote the stream):
// Careful, your default edge should never switch on the null terminator at the end of the string
// Stream:
//"hello" "world""this is a test\r\nof string\tliterals" 'a' '\n' 'c' "can you handle \"this\"" "error\j "error two!
void Driver1Part2Test5();

// Stream (because of the string literals, we did NOT quote the stream):
//class Player
//{
//  var Health : float = 99.0f;
//}
//
//function main()
//{
//  // Do some simple test /*
//  var a = 5;
//  var b : int = 1;
//  if (a > b)
//  {
//    ++a;
//    b +=-(a+ b) *+2;
//    Print("hello\tworld"); /* multi-line "comment"! */
//    Print("b's value is: ", b);
//    Print('c');
//    Print(5.ToString());
//    
//    for (int i = 0; i < 99 / 2; ++i)
//    {
//      Print(i);
//    }
//    
//    var p = Player();
//    Print(p.Health);
//  }
//}
void Driver1Part2Test6();

// When submitting your assignment, we will run extra tests outside the ones presented here
// Take your time and make sure your code is correct! The following are unknown tests...
void Driver1Part3Test7();

/***************************** INTERNAL *****************************/
// This is a helper macro we use for testing string literals within a C++ string literal (because they are extermely annoying to escape!)
#define STRINGIZE(...) #__VA_ARGS__

// These are for usage by the other drivers
typedef void (*TokenReaderFn)(DfaState* startingState, const char* stream, Token& outToken);
void TokenizeAndDeleteRoot(DfaState* root, const char* stream, const char* tokenNames[], std::vector<Token>* tokensOut, TokenReaderFn reader);

#endif
