/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#pragma once
#ifndef COMPILER_CLASS_DRIVER_3
#define COMPILER_CLASS_DRIVER_3

#include "Driver1.hpp"
#include "Driver2.hpp"
#include "AstNodes.hpp"

/***************************** PART 1 *****************************/

// Prints out a node and handles tabbing as you descend into child nodes (to help visualize a tree)
class NodePrinter : public std::stringstream
{
public:
  NodePrinter();
  NodePrinter(const std::string& text);
  ~NodePrinter();

private:
  static std::vector<NodePrinter*> ActiveNodes;
};

// Parse the following stream of tokens into an Expression Tree (starting from the Expression rule)
// This function should return a root node (which holds all of the parsed operators, literals, etc)
// If we fail parsing, we should throw ParsingException (otherwise success is assumed)
std::unique_ptr<ExpressionNode> ParseExpression(std::vector<Token>& tokens);

// Print the tree out starting from the given node
// The printed tree must match the tree from the driver
void PrintTree(AbstractNode* node);

/***************************** PART 2 *****************************/

// Parse the following stream of tokens into an Abstract Syntax Tree (starting from the Block rule)
// This function should return a root node (which holds all of the parsed classes and functions)
// If we fail parsing, we should throw ParsingException (otherwise success is assumed)
std::unique_ptr<BlockNode> ParseBlock(std::vector<Token>& tokens);

/***************************** TESTS  *****************************/

// Parse the single literal '"hello world"' as an expression
void Driver3Part1Test0();

// Parse a binary operator with literal children '5 + 3.0' as an expression
void Driver3Part1Test1();

// Parse left to right associativity with '9 * 2.0f * 1 * 160 / 5 / 10.0' as an expression
void Driver3Part1Test2();

// Parse right to left associativity with 'a = b = c = d' as an expression
void Driver3Part1Test3();

// Parse mixed associativity with 'a = b += 5 + 3 + 9.0' as an expression
void Driver3Part1Test4();

// Parse operator precedence and grouping with '5 + 3 * 9 - 6 / (2 + 2) || a == b && a != -c' as an expression
void Driver3Part1Test5();

// Parse unary operators with '!--(*++a - -5)' as an expression
void Driver3Part1Test6();

// Parse call, member access, indexer, and cast with '"what"[0].ToString(formatter)->c_str() as Integer + 5' as an expression
void Driver3Part1Test7();

// Parse a complex expression '((99.0 + ("str" + "ing")[a + ++b * 2][0] as Double + player.Health->Lives + enemy.GetMaxLives(3 + a[0], 9 as Double)) > 0 || true) && false'
void Driver3Part1Test8();

// Parse an empty block (should return a block node with no children)
void Driver3Part2Test9();

// Parse a global function Print with parameters
void Driver3Part2Test10();

// Parse a an empty class Player
void Driver3Part2Test11();

// Parse a global function with a while loop
void Driver3Part2Test12();

// Parse a global function with a for loop and variables
void Driver3Part2Test13();

// Parse a global function with a single if statement (no else)
void Driver3Part2Test14();

// Parse a global function with several if statements (incuding else if and else)
void Driver3Part2Test15();

// Parse a class Player with variables, functions, and expressions
void Driver3Part2Test16();

// Parse several different block level empty classes and functions with various expressions and statements
void Driver3Part2Test17();

// Failure test where we try to parse an expression with a missing right hand of a binary operator
void Driver3Part3Test18();

// Failure test where we try and parse a variable whose initial value is a grouped expression missing an end parentheses
void Driver3Part3Test19();

// Failure test where we try and parse a global function and a nested if with an else that is missing a scope
void Driver3Part3Test20();

#endif
