/******************************************************************\
 * Author: 
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/
#include "../Drivers/Driver1.hpp"
#include <unordered_map>

#pragma region DFACode
class DfaState
{
public:
	DfaState(int acceptingToken = 0)
	{
		mDefaultEdge = nullptr;
		mAcceptingToken = static_cast<TokenType::Enum>(acceptingToken);
	}

  ~DfaState()
  {
    mEdges.erase(mEdges.begin(), mEdges.end());

    if (this == mDefaultEdge)
    {
      mDefaultEdge = nullptr;
    }
    else
    {
      delete mDefaultEdge;
      mDefaultEdge = nullptr;
    }

    mAcceptingToken = TokenType::Enum::Invalid;
  }

	std::unordered_map<char, DfaState*> mEdges;
	DfaState* mDefaultEdge;

	// Any value other than 'Invalid' means this IS an accepting state
	TokenType::Enum mAcceptingToken;
};

DfaState* AddState(int acceptingToken)
{
  return new DfaState(acceptingToken);
}

void AddEdge(DfaState* from, DfaState* to, char c)
{
  from->mEdges[c] = to;
}

void AddDefaultEdge(DfaState* from, DfaState* to)
{
  from->mDefaultEdge = to;
}

#include<string>
void ReadToken(DfaState* startingState, const char* stream, Token& outToken)
{
  std::string strToken = "";
  DfaState* lastAcceptingState = nullptr;

  if (startingState && stream)
  {
    bool fStop = false;
    std::string strTemp = "";
    DfaState* dest;
    DfaState* walker = startingState;

    char c;
    for (int i = 0; !fStop && stream[i]; ++i)
    {
      c = stream[i];
      
      dest = walker->mDefaultEdge;

      if (walker->mEdges.size() && walker->mEdges.find(c) != walker->mEdges.end())
        dest = walker->mEdges[c];

      if (dest)
      {
        strTemp += c;
        walker = dest;
        
        if (walker->mAcceptingToken)
        {
          lastAcceptingState = walker;
          strToken = strTemp;
        }
      }
      else
        fStop = true;
    }

    
    if (walker->mAcceptingToken != 0)
    {
      lastAcceptingState = walker;
      strToken = strTemp;
    }

    if (lastAcceptingState == nullptr)
    {
      if (strTemp.size())
        strToken = strTemp;
      //else
      //  strToken += c;
    }
    else
      outToken.mTokenType = lastAcceptingState->mAcceptingToken;
  }


  int length = strToken.size();
  char* cStr = new char[length+1];
  *(cStr + length) = 0;
  memcpy(cStr, strToken.c_str(), length);
  
  outToken.mText = cStr;
  outToken.mLength = length;

}

void DeleteStateAndChildren(DfaState* root)
{
  if (root)
  {
    delete root;
    root = nullptr;
  }
}

void ReadLanguageToken(DfaState* startingState, const char* stream, Token& outToken)
{
  ReadToken(startingState, stream, outToken);

  int result;

#pragma region CheckKeywords
  const char* keywords[] =
  {
    // The TOKEN macro is used like TOKEN(Class, "class")
#define TOKEN(Name, Value) Value,
#include "../Drivers/TokenKeywords.inl"
#undef TOKEN
    ""
  };

  unsigned numKeywords = sizeof(keywords) / sizeof(const char*);
  for (unsigned i = 0; i < numKeywords; ++i)
  {
    result = strcmp(keywords[i], outToken.mText);
    if(result == 0)
    {
      outToken.mTokenType = static_cast<TokenType::Enum>(TokenType::KeywordStart + i + 1);
      return;
    }
	/*
    else if (result == 1) //keywords are all greater then the token string
    {
      break;
    }
	*/
  }
#pragma endregion;
}

//#include <regex>
DfaState* CreateLanguageDfa()
{
  /*
  std::smatch m;

  std::regex rWhitespace("[ \r\n\t]+");   // matches whitespace characters
  std::regex rIdentifier("[a-zA-Z_][a-zA-Z0-9_]*");   // matches identifier characters
  
  std::regex rIntegerLiteral("[0-9]+");   // matches identifier characters
  std::regex rFloatLiteral("[0-9]+[.][0-9]+(e[+-]?[0-9]+)?f?");   // matches identifier characters
  //std::regex rStringLiteral("\"([^"\\] | \\[nrt"])*\"");   // matches identifier characters
  std::regex rCharacterLiteral("'[^'\\]|\\[nrt']'");
  std::regex rSingleLineComment("//.*(\r|\n|\0)");

  for (unsigned i = 0; i < rWhitespace.; ++i)
  {

  }
  */

  DfaState* root = AddState(0);

#pragma region Whitespace //must be on top
  DfaState* stateWhitespace = AddState(TokenType::Enum::Whitespace);

  //Whitespace Edges
  AddEdge(root, stateWhitespace, ' ');
  AddEdge(root, stateWhitespace, '\r');
  AddEdge(root, stateWhitespace, '\n');
  AddEdge(root, stateWhitespace, '\t');

  stateWhitespace->mEdges = root->mEdges;
#pragma endregion

// Symbol state
#pragma region Symbols
  const char* symbols[] =
  {
    // The TOKEN macro is used like TOKEN(Class, "class")
#define TOKEN(Name, Value) Value,
#include "../Drivers/TokenSymbols.inl"
#undef TOKEN
    ""
  };

  std::string tempStr;
  unsigned numSymbols = sizeof(symbols) / sizeof(const char*);
  DfaState* walker;
  char c;
  for (unsigned i = 0; i < numSymbols; ++i)
  {
    tempStr = symbols[i];
    walker = root;

    for (unsigned charIndex = 0; charIndex < tempStr.size(); ++charIndex)
    {
      c = tempStr[charIndex];

      if (walker->mEdges.find(c) == walker->mEdges.end())
        AddEdge(walker, AddState(0), c);

      walker = walker->mEdges.find(c)->second;
    }

    if(tempStr.size())
      walker->mAcceptingToken = static_cast<TokenType::Enum>(TokenType::Enum::SymbolStart + i + 1);
  }
#pragma endregion 

#pragma region SingleLineComment
  DfaState* stateSingleLineComment = AddState(TokenType::Enum::SingleLineComment);
  DfaState* stateSingleLineTerminus = AddState(TokenType::Enum::SingleLineComment);
  
  AddEdge(root->mEdges['/'], stateSingleLineComment, '/');
  AddDefaultEdge(stateSingleLineComment, stateSingleLineComment);
  
  AddEdge(stateSingleLineComment, stateSingleLineTerminus, '\r');
  AddEdge(stateSingleLineComment, stateSingleLineTerminus, '\n');
#pragma endregion

#pragma region MultiLineComment
  DfaState* stateMultiLineComment = AddState(0);
  AddEdge(root->mEdges['/'], stateMultiLineComment, '*');
  AddDefaultEdge(stateMultiLineComment, stateMultiLineComment);

  DfaState* stateMultiLineTransition = AddState(0);
  AddDefaultEdge(stateMultiLineTransition, stateMultiLineComment);
  AddEdge(stateMultiLineComment, stateMultiLineTransition, '*');

  DfaState* stateMultiLineTerminus = AddState(TokenType::Enum::MultiLineComment);
  AddEdge(stateMultiLineTransition, stateMultiLineTerminus, '/');
#pragma endregion

#pragma region Identifier
  DfaState* stateIdentifier = AddState(TokenType::Enum::Identifier);

  AddDefaultEdge(root, stateIdentifier);
  //Identifier Edges
  for (unsigned i = 0; i < 26; ++i)
  {
    AddEdge(stateIdentifier, stateIdentifier, 'a' + i);
    AddEdge(stateIdentifier, stateIdentifier, 'A' + i);
    AddEdge(stateIdentifier, stateIdentifier, '_');
  }

  for (unsigned i = 0; i < 10; ++i)
  {
    AddEdge(stateIdentifier, stateIdentifier, '0' + i);
  }
#pragma endregion

#pragma region NumericLiteral
  DfaState* stateIntegerLiteral = AddState(TokenType::Enum::IntegerLiteral);

  DfaState* stateFloatTransition = AddState(TokenType::Enum::Invalid);
  DfaState* stateFloatLiteral = AddState(TokenType::Enum::FloatLiteral);
  DfaState* stateFloatTerminus = AddState(TokenType::Enum::FloatLiteral);

  
  //Numeric Literal Edges

  for (unsigned i = 0; i < 10; ++i)
  {
    AddEdge(root, stateIntegerLiteral, '0' + i);
    AddEdge(stateIntegerLiteral, stateIntegerLiteral, '0' + i);

    AddEdge(stateFloatTransition, stateFloatLiteral, '0' + i);
    AddEdge(stateFloatLiteral, stateFloatLiteral, '0' + i);
  }


  AddEdge(stateIntegerLiteral, stateFloatTransition, '.');

  AddEdge(stateFloatLiteral, stateFloatTransition, 'e');
  AddEdge(stateFloatTransition, stateFloatTransition, '+');
  AddEdge(stateFloatTransition, stateFloatTransition, '-');

  AddEdge(stateFloatLiteral, stateFloatTerminus, 'f');

#pragma endregion

#pragma region CharacterLiteral
  DfaState* stateCharacterTransition = AddState(TokenType::Enum::Invalid);
  DfaState* stateCharacterLiteral = AddState(TokenType::Enum::CharacterLiteral);
  AddEdge(root, stateCharacterTransition, '\'');
  AddEdge(stateCharacterTransition, stateCharacterLiteral, '\'');
  AddDefaultEdge(stateCharacterTransition, stateCharacterTransition);

  DfaState* stateCEscapeSequence = AddState(TokenType::Enum::Invalid);
  AddEdge(stateCharacterTransition, stateCEscapeSequence, '\\');

  AddEdge(stateCEscapeSequence, stateCharacterTransition, 'n');
  AddEdge(stateCEscapeSequence, stateCharacterTransition, 'r');
  AddEdge(stateCEscapeSequence, stateCharacterTransition, 't');
  AddEdge(stateCEscapeSequence, stateCharacterTransition, '\'');

#pragma endregion


#pragma region StringLiteral
  DfaState* stateStringTransition = AddState(TokenType::Enum::Invalid);
  AddEdge(root, stateStringTransition, '\"');
  AddDefaultEdge(stateStringTransition, stateStringTransition);

  DfaState* stateStringLiteral = AddState(TokenType::Enum::StringLiteral);
  AddEdge(stateStringTransition, stateStringLiteral, '\"');

  DfaState* stateSEscapeSequence = AddState(TokenType::Enum::Invalid);
  AddEdge(stateStringTransition, stateSEscapeSequence, '\\');

  AddEdge(stateSEscapeSequence, stateStringTransition, 'n');
  AddEdge(stateSEscapeSequence, stateStringTransition, 'r');
  AddEdge(stateSEscapeSequence, stateStringTransition, 't');
  AddEdge(stateSEscapeSequence, stateStringTransition, '\'');
  AddEdge(stateSEscapeSequence, stateStringTransition, '\"');
#pragma endregion

  return root;
}
#pragma endregion
