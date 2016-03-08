/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#include "AstNodes.hpp"

AbstractNode::AbstractNode() :
  mParent(nullptr)
{
}

AbstractNode::~AbstractNode()
{
}

void AbstractNode::Walk(Visitor* visitor, bool visit)
{
}

ClassNode::ClassNode() :
  mSymbol(nullptr)
{
}

TypeNode::TypeNode() :
  mSymbol(nullptr)
{
}

VariableNode::VariableNode() :
  mSymbol(nullptr)
{
}

FunctionNode::FunctionNode() :
  mSymbol(nullptr),
  mSignatureType(nullptr)
{
}

GotoNode::GotoNode() :
  mResolvedLabel(nullptr)
{
}

ExpressionNode::ExpressionNode() :
  mResolvedType(nullptr)
{
}

MemberAccessNode::MemberAccessNode() :
  mResolvedMember(nullptr)
{
}