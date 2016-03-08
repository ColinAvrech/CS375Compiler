/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#pragma once
#ifndef COMPILER_CLASS_DRIVER_4
#define COMPILER_CLASS_DRIVER_4

#include "Driver1.hpp"
#include "Driver2.hpp"
#include "Driver3.hpp"
#include "AstNodes.hpp"
#include "SymbolTable.hpp"

/***************************** DRIVER *****************************/

// When an error occurs within semantic analysis you must throw this exception
// The different possible error messages are standardized and listed below
class SemanticException : public std::exception
{
public:
  SemanticException();
  SemanticException(const std::string& error);
  const char* what() const override;
  std::string mError;
};

// Thrown when we detect that two members/locals that are
// defined within a class/function that have the same name
void ErrorSameName(const std::string& name);

// Thrown when we fail to find a symbol by name when walking the scope stack
// This should even be thrown if we fail to resolve a type name
void ErrorSymbolNotFound(const std::string& name);

// Thrown when we're looking for a particular kind of symbol but we find out it is the wrong type
void ErrorIncorrectSymbolType(const std::string& name);

// All of our conditionals expect a pointer or boolean type (if, while, for)
void ErrorConditionExpectedBooleanOrPointer(Type* type);

// Thrown when a ValueNode resolves its Symbol to anything that isn't a Function or Variable
void ErrorMayOnlyReferToFunctionsOrVariables(const std::string& name);

// Thrown whenever the type of an expression does not match what is expected
// In the case of ReturnNode, either type passed in here may be null
void ErrorTypeMismatch(Type* expected, Type* got);

// Thrown if a member access is attempted on a type that is not a Class or Pointer
// Also thrown if '.' is attempted on a Pointer or '->' is attempted on a class
void ErrorInvalidMemberAccess(MemberAccessNode* node);

// Thrown when we attempt to perform a call on a non-callable type (only valid type is Function)
void ErrorNonCallableType(Type* type);

// Thrown if a call was invalid (either from invalid number of parameters or mismatched parameter types)
void ErrorInvalidCall(CallNode* node);

// Thrown when we attempt to perform an invalid cast between two types
void ErrorInvalidCast(Type* from, Type* to);

// Thrown when the left type of an index is not a Pointer
// Also thrown if the index of the index is not of type Integer
void ErrorInvalidIndexer(IndexNode* node);

// Thrown when a break or continue statement is detected outside of a loop
void ErrorBreakContinueMustBeInsideLoop();

// Thrown when a goto is unable to find a label within the same function
// Note: Labels do not use the scope stack. They are only scoped to the entire function (eg jumping backwards)
void ErrorGotoLabelNotFound(const std::string& name);

// Thrown when a unary operation is done on an invalid type
void ErrorInvalidUnaryOperator(UnaryOperatorNode* node);

// Thrown when a binary operation is done between invalid types
void ErrorInvalidBinaryOperator(BinaryOperatorNode* node);

// Run semantic analysis over the tree
// You should initialize your symbol table with the libraries that you depend upon given here
// This will run multiple passes to collect symbols (module based compilation)
// This will also compute types of expressions and maintain the variable scope stack
// If an error occurs within analysis the above 'SemanticException' must be thrown with the correct error
// The library should contain all the proper symbols after completion
// A partial library may be used to print out a tree after an error occurs (for your own debugging)
void SemanticAnalyize(AbstractNode* node, std::vector<Library*>& dependencies, Library* library);

// Print the tree out with semantic symbols starting from the given node
// The printed tree must match the tree from the driver
void PrintTreeWithSymbols(AbstractNode* node);

// Symbols that we guarantee exist (within the core library)
extern Type* VoidType;
extern Type* NullType;

// All these types can be cast to each other
extern Type* IntegerType;
extern Type* FloatType;
extern Type* BooleanType;
extern Type* ByteType;

/***************************** TESTS  *****************************/

// Parse an empty class with the name 'Player'
void Driver4Part1Test0();

// Parse a class 'Player' with member variables 'Health' and 'Lives'
void Driver4Part1Test1();

// Parse a class 'Player' with member variables and member functions
void Driver4Part1Test2();

// Parse locals and labels nested within functions along with pointer symbols
void Driver4Part1Test3();

// Parse global functions, variables, and classes
void Driver4Part1Test4();

// Parse a class 'Player' with two member variables of the same name
void Driver4Part2Test5();

// Parse a global variable with an Type that was never defined
void Driver4Part2Test6();

// Parse a global variable with a pointer to a Type that was never defined
void Driver4Part2Test7();

// Parse a function with labels of the same name
void Driver4Part2Test8();

// A global variable attempts to use a global function as its type
void Driver4Part2Test9();

// Tests type resolution for all the literals we support
void Driver4Part3Test10();

// Tests if, else, while, and for (scoped constructs)
void Driver4Part3Test11();

// Tests local variable references (ValueNodes) including scoped resolving
void Driver4Part3Test12();

// Tests member access resolution with functions and variables (value and pointer)
void Driver4Part3Test13();

// Tests the CallNode and ReturnNode (function calling)
void Driver4Part3Test14();

// Tests type casting between different types
void Driver4Part3Test15();

// Tests indexing on pointers
void Driver4Part3Test16();

// Tests breaks, continues, and gotos within a for and while loop
void Driver4Part3Test17();

// Tests unary operators on many types
void Driver4Part3Test18();

// Tests binary operators on many types
void Driver4Part3Test19();

// Tests errors in returning
void Driver4Part4Test20();

// Tests errors in an IfNode's conditional
void Driver4Part4Test21();

// Tests errors in resolving a local variable by name/scope
void Driver4Part4Test22();

// Tests errors in member access resolution
void Driver4Part4Test23();

// Tests errors in incorrect call parameters
void Driver4Part4Test24();

// Tests type casting invalid types
void Driver4Part4Test25();

// Tests errors with indexing on a non pointer
void Driver4Part4Test26();

// Tests a break statement outside of a loop
void Driver4Part4Test27();

// Tests an invalid unary operator
void Driver4Part4Test28();

// Tests an invalid binary operator
void Driver4Part4Test29();

#endif
