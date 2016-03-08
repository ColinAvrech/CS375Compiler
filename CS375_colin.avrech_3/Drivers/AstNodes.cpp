/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#include "AstNodes.hpp"

AbstractNode::~AbstractNode()
{
}

void AbstractNode::Walk(Visitor* visitor, bool visit)
{
}
