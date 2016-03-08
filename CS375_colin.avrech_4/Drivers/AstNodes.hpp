/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#pragma once
#ifndef COMPILER_CLASS_AST_NODES
#define COMPILER_CLASS_AST_NODES

#include "Driver1.hpp"
#include "DriverShared.hpp"
#include <memory>
#include <vector>

// You must implement the base virtual Visitor class
class Visitor;

// Forward declarations of all the node types
class AbstractNode;
class BlockNode;
class ClassNode;
class StatementNode;
class VariableNode;
class ScopeNode;
class ParameterNode;
class FunctionNode;
class TypeNode;
class LabelNode;
class GotoNode;
class ReturnNode;
class BreakNode;
class ContinueNode;
class ExpressionNode;
class IfNode;
class WhileNode;
class ForNode;
class ValueNode;
class BinaryOperatorNode;
class UnaryOperatorNode;
class PostExpressionNode;
class MemberAccessNode;
class CallNode;
class CastNode;
class IndexNode;

// Forward declarations for the symbols
class Symbol;
class Type;
class Variable;
class Function;
class Label;
class Library;

// All of our node types inherit from this node and implement the Walk function
class AbstractNode
{
public:
  AbstractNode();
  virtual ~AbstractNode();
  
  virtual void Walk(Visitor* visitor, bool visit = true);

  //* Semantic Analysis *//
  AbstractNode* mParent;
};

class BlockNode : public AbstractNode
{
public:
  // ClassNode / VariableNode / FunctionNode
  unique_vector<AbstractNode> mGlobals;

  void Walk(Visitor* visitor, bool visit = true) override;
};

class ClassNode : public AbstractNode
{
public:
  ClassNode();
  Token mName;
  
  // VariableNode / FunctionNode
  unique_vector<AbstractNode> mMembers;
  
  void Walk(Visitor* visitor, bool visit = true) override;

  //* Semantic Analysis *//
  // The class node should create the following symbol
  Type* mSymbol;
};

class StatementNode : public AbstractNode
{
public:

  void Walk(Visitor* visitor, bool visit = true) override;
};

class TypeNode : public AbstractNode
{
public:
  TypeNode();
  Token mName;
  size_t mPointerCount;

  void Walk(Visitor* visitor, bool visit = true) override;

  //* Semantic Analysis *//
  // The symbol we resolved
  // Note that symbol resolution may actually need to create a type
  // For example, a pointer to a type may need to create a symbol
  Type* mSymbol;
};

class VariableNode : public StatementNode
{
public:
  VariableNode();
  Token mName;
  std::unique_ptr<TypeNode> mType;

  // Can be null
  std::unique_ptr<ExpressionNode> mInitialValue;

  void Walk(Visitor* visitor, bool visit = true) override;

  //* Semantic Analysis *//
  // The variable node should create the following symbol
  Variable* mSymbol;
};

class ParameterNode : public VariableNode
{
public:
  void Walk(Visitor* visitor, bool visit = true) override;
};

class ScopeNode : public StatementNode
{
public:
  unique_vector<StatementNode> mStatements;
  
  void Walk(Visitor* visitor, bool visit = true) override;
};

class FunctionNode : public AbstractNode
{
public:
  FunctionNode();
  Token mName;
  unique_vector<ParameterNode> mParameters;

  // Can be null
  std::unique_ptr<TypeNode> mReturnType;

  std::unique_ptr<ScopeNode> mScope;

  void Walk(Visitor* visitor, bool visit = true) override;

  //* Semantic Analysis *//
  // The signature type is filled out first and is used later when we create the Function symbol
  Type* mSignatureType;
  // The function node should create the following symbol
  Function* mSymbol;
};

class LabelNode : public StatementNode
{
public:
  Token mName;

  void Walk(Visitor* visitor, bool visit = true) override;

  //* Semantic Analysis *//
  // The label node should create the following symbol
  Label* mSymbol;
};

class GotoNode : public StatementNode
{
public:
  GotoNode();
  Token mName;

  void Walk(Visitor* visitor, bool visit = true) override;

  //* Semantic Analysis *//
  // The label that we have resolved this goto to point at (within the same function)
  Label* mResolvedLabel;
};

class ReturnNode : public StatementNode
{
public:
  // Can be null
  std::unique_ptr<ExpressionNode> mReturnValue;

  void Walk(Visitor* visitor, bool visit = true) override;
};

class BreakNode : public StatementNode
{
public:
  void Walk(Visitor* visitor, bool visit = true) override;
};

class ContinueNode : public StatementNode
{
public:
  void Walk(Visitor* visitor, bool visit = true) override;
};

class ExpressionNode : public StatementNode
{
public:
  ExpressionNode();
  void Walk(Visitor* visitor, bool visit = true) override;

  //* Semantic Analysis *//
  // The type that we've resolved this expression to
  Type* mResolvedType;
};

class IfNode : public StatementNode
{
public:

  // Can be null
  std::unique_ptr<ExpressionNode> mCondition;

  std::unique_ptr<ScopeNode> mScope;

  // Can be null
  std::unique_ptr<IfNode> mElse;

  void Walk(Visitor* visitor, bool visit = true) override;
};

class WhileNode : public StatementNode
{
public:
  std::unique_ptr<ExpressionNode> mCondition;
  std::unique_ptr<ScopeNode> mScope;

  void Walk(Visitor* visitor, bool visit = true) override;
};

class ForNode : public StatementNode
{
public:
  // Can be null
  std::unique_ptr<VariableNode> mInitialVariable;
  // Can be null
  std::unique_ptr<ExpressionNode> mInitialExpression;

  // Can be null
  std::unique_ptr<ExpressionNode> mCondition;
  // Can be null
  std::unique_ptr<ExpressionNode> mIterator;

  std::unique_ptr<ScopeNode> mScope;

  void Walk(Visitor* visitor, bool visit = true) override;
};

class ValueNode : public ExpressionNode
{
public:
  // Includes name references
  Token mToken;

  void Walk(Visitor* visitor, bool visit = true) override;
};

class BinaryOperatorNode : public ExpressionNode
{
public:
  Token mOperator;
  std::unique_ptr<ExpressionNode> mLeft;
  std::unique_ptr<ExpressionNode> mRight;

  void Walk(Visitor* visitor, bool visit = true) override;
};

class UnaryOperatorNode : public ExpressionNode
{
public:
  Token mOperator;
  std::unique_ptr<ExpressionNode> mRight;

  void Walk(Visitor* visitor, bool visit = true) override;
};

class PostExpressionNode : public ExpressionNode
{
public:
  std::unique_ptr<ExpressionNode> mLeft;

  void Walk(Visitor* visitor, bool visit = true) override;
};

class MemberAccessNode : public PostExpressionNode
{
public:
  MemberAccessNode();
  Token mOperator;
  Token mName;

  void Walk(Visitor* visitor, bool visit = true) override;

  //* Semantic Analysis *//
  // The member that we resolved with this member access
  Symbol* mResolvedMember;
};

class CallNode : public PostExpressionNode
{
public:
  unique_vector<ExpressionNode> mArguments;

  void Walk(Visitor* visitor, bool visit = true) override;
};

class CastNode : public PostExpressionNode
{
public:
  std::unique_ptr<TypeNode> mType;

  void Walk(Visitor* visitor, bool visit = true) override;
};

class IndexNode : public PostExpressionNode
{
public:
  std::unique_ptr<ExpressionNode> mIndex;

  void Walk(Visitor* visitor, bool visit = true) override;
};

#endif
