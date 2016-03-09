/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#include "SymbolTable.hpp"
#include <sstream>

Symbol::Symbol() :
  mType(nullptr),
  mParentType(nullptr),
  mParentFunction(nullptr),
  mLibrary(nullptr)
{
}

Symbol::~Symbol()
{
}

void Symbol::Print(size_t depth)
{
  for (size_t i = 0; i < depth; ++i)
    printf("| ");

  printf("%s\n", Dump().c_str());
}

std::ostream& operator<<(std::ostream& stream, const Symbol* symbol)
{
  if (symbol == nullptr)
  {
    stream << "(nullptr)";
  }
  else
  {
    stream << symbol->mName;
  }

  return stream;
}

std::ostream& operator<<(std::ostream& stream, const Symbol& symbol)
{
  return stream << &symbol;
}

Variable::Variable() :
  mIsParameter(false)
{
}

std::string Variable::Dump()
{
  std::stringstream stream;
  if (mIsParameter)
    stream << "Parameter";
  else
    stream << "Variable";

  stream << "(";
  stream << mName;
  stream << ", ";
  stream << mType;
  stream << ")";
  return stream.str();
}

Function::Function() :
  mExecutableFunction(nullptr)
{
}

void Function::Print(size_t depth)
{
  Symbol::Print(depth);

  for (Symbol* symbol : mLocals)
    symbol->Print(depth + 1);
}

std::string Function::Dump()
{
  std::stringstream stream;
  stream << "Function(";
  stream << mName;
  stream << ", ";
  stream << mType;
  stream << ")";
  return stream.str();
}

Label::Label()
{
}

std::string Label::Dump()
{
  std::stringstream stream;
  stream << "Label(";
  stream << mName;
  stream << ")";
  return stream.str();
}

Type::Type() :
  mMode(TypeMode::Class),
  mPointerToType(nullptr),
  mReturnType(nullptr)
{
}

void Type::Print(size_t depth)
{
  Symbol::Print(depth);

  for (Symbol* member : mMembers)
    member->Print(depth + 1);
}

std::string Type::Dump()
{
  std::stringstream stream;
  stream << "Type(";
  stream << mName;
  stream << ")";
  return stream.str();
}

void Library::Print()
{
  for (Symbol* symbol : mGlobals)
    symbol->Print();
}
