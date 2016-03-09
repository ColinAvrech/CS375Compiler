/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#pragma once
#ifndef COMPILER_CLASS_SYMBOL_TABLE
#define COMPILER_CLASS_SYMBOL_TABLE

#include "Driver1.hpp"
#include "DriverShared.hpp"

// Forward declarations
class Symbol;
class Type;
class Variable;
class Function;
class Label;
class Library;

// A symbol is anything that must be identified by name
class Symbol
{
public:
  Symbol();
  virtual ~Symbol();

  // Each symbol knows how to print itself and its children
  // The depth allows us to visualize the hierarchy
  virtual void Print(size_t depth = 0);
  virtual std::string Dump() = 0;

  // The name of this symbol
  // This name may be generated / normalized
  // For example, a pointer to an Integer may be declared with whitespace as 'Integer *'
  // The normalized version of this name would not include spaces unlessed needed: 'Integer*'
  // That way, all declarations of a pointer to Integer would be resolved to the same symbol name
  std::string mName;

  // The type that this symbol resolves to
  // For a Variable, this is the type it is declared as
  // For a Function, this is the signature type (TypeMode will be Function)
  // For Type and Label, this should be left as null
  Type* mType;
  
  // The type that this symbol is nested within
  // (or null if this symbol is global or local to a function)
  Type* mParentType;

  // The function that this symbol is nested within
  // (or null if the symbol is global or a member of a class)
  Function* mParentFunction;

  // The library that owns this symbol
  Library* mLibrary;
};

// A helper operator for outputting a symbol to the ostream
// This also handles the case for null (and is safer/easier to use when debugging)
std::ostream& operator<<(std::ostream& stream, const Symbol* symbol);
std::ostream& operator<<(std::ostream& stream, const Symbol& symbol);

// Could be a global, local, or even class member variable
class Variable : public Symbol
{
public:
  Variable();
  std::string Dump() override;

  // Whether or not this variable is a parameter
  // This is mostly for debugging and visualization, but must be filled out by semantic analysis
  bool mIsParameter;
};

class Function : public Symbol
{
public:
  Function();
  void Print(size_t depth = 0) override;
  std::string Dump() override;

  // Local variables and labels defined within this function
  std::vector<Symbol*> mLocals;

  // Labels mapped by name
  // Note that it is an error to have two labels defined with the same name (but not a variable and label)
  std::unordered_map<std::string, Label*> mLabelsByName;

  // Its quite common in a scripting language to have your function symbol
  // also point back at the executable version of the function
  // This may include a direct machine code compiled pointer (function pointer)
  // Or possibly a pointer to your IR / byte code
  // Or if you're using an interpreter, a direct pointer to the function node
  void* mExecutableFunction;
};

class Label : public Symbol
{
public:
  Label();
  std::string Dump() override;
};

namespace TypeMode
{
  enum Enum
  {
    // Used for classes and any primitives like Integer and Float
    Class,

    // A pointer to another type (Integer*, Player*, etc)
    Pointer,

    // A function signature with return type and parameter types
    Function
  };
}

class Type : public Symbol
{
public:
  Type();
  void Print(size_t depth = 0) override;
  std::string Dump() override;

  // Whether this is a class (or primitive), pointer, or a function pointer type
  // Default mode is Class
  TypeMode::Enum mMode;
  
  //** Only relevant if this mMode is Class **//
  // The members that belong to this type mapped by name
  std::vector<Symbol*> mMembers;
  std::unordered_map<std::string, Symbol*> mMembersByName;

  //** Only relevant if this mMode is Pointer **//
  // The type that we point to
  // Every Type marked as 'Pointer' is assumed to be one level deep
  // An Integer** is a Type(Pointer) -> Type(Pointer) -> Type(Integer)
  // Note that for the symbols, we do not use the PointerCount
  Type* mPointerToType;

  //** Only relevant if this mMode is Function **//
  std::vector<Type*> mParameterTypes;
  Type* mReturnType;
};

// The library owns all symbols and is responsible for destroying them
class Library
{
public:
  // The library owns all symbols and will cleanup their memory upon destruction
  // This is essentially a flattened array of all symbols (no hierarchy)
  unique_vector<Symbol> mAllSymbols;

  // All symbols that should be made available globally (mapped by name)
  // You will typically use this to initialize your symbol scope stack (the first push)
  std::vector<Symbol*> mGlobals;
  std::unordered_map<std::string, Symbol*> mGlobalsByName;
  
  // Prints out the symbols recursively, showing the hierarchy
  void Print();

  //**************** USER IMPLEMENTED ****************//

  // Create a Type, Variable, Function, or Label symbol
  // The library is responsible for the destruction of these, therefore they must be added to 'mAllSymbols'
  // If the symbol is not nested within a class or function then it must be added to 'mGlobals' and 'mGlobalsByName'
  // The name and library for the symbol should also be filled out
  // If global, this should throw ErrorSameName for duplicate symbol names
  // I recommend using templates to simplify this code
  Type*     CreateType    (const std::string& name, bool isGlobal = false);
  Variable* CreateVariable(const std::string& name, bool isGlobal = false);
  Function* CreateFunction(const std::string& name, bool isGlobal = false);
  Label*    CreateLabel   (const std::string& name, bool isGlobal = false);

  // Add a pointer to the given type
  // This function should find an existing symbol by normalized name or return a new symbol
  // Any newly created symbols should be added to 'mGlobals'
  // Passing 'Integer' with 'TypeMode::Value' would return 'Integer*' with 'TypeMode::Pointer'
  // Passing 'Float*' with 'TypeMode::Pointer' would return 'Float**' with 'TypeMode::Pointer'
  // The return should always be 'TypeMode::Pointer'
  Type* GetPointerType(Type* pointerToType);

  // Get a series of pointers given a base Class or Function type, and a number of pointers
  // If 'pointerCount' is 0, this should return the same type that was passed in
  Type* GetPointerType(Type* pointerToType, size_t pointerCount);
  
  // Get a function symbol for the given types and return value
  // This function should find an existing symbol by normalized name or return a new symbol
  // Any newly created symbols should be added to 'mGlobals'
  // The return should always be 'TypeMode::Function'
  Type* GetFunctionType(std::vector<Type*> parameterTypes, Type* returnType);
};

#endif
