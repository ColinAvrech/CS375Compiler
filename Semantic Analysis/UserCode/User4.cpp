/******************************************************************\
 * Author: 
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/
#include "../Drivers/Driver4.hpp"
\
#include <stack>
typedef std::unordered_map<std::string, Symbol*> SymbolMap;
typedef std::pair<std::string, Symbol*> SymbolPair;
SymbolMap g_GlobalSymbols;

enum ScopeType {
    ST_SCOPE, //Used for if/else and loops
    ST_CLASS,
    ST_FUNCTION,
    ST_LOOP
};

class ScopeEntry
{
public:
    ScopeEntry(AbstractNode* node, const ScopeType& type)
    {
        mNode = node;
        mType = type;
		mLocals.clear();
    }
    ScopeType mType;
    AbstractNode* mNode;
	std::vector<Symbol*> mLocals;
};

typedef std::vector<ScopeEntry> ScopeStack;
ScopeStack scopeStack;

template<typename T>
void AddSymbolToLibrary(Library* lib, T* sym, const bool& isGlobal)
{
	if (isGlobal)
	{
		if (g_GlobalSymbols.find(sym->mName) != g_GlobalSymbols.end())
		{
			ErrorSameName(sym->mName);
		}
		else
		{
			lib->mGlobalsByName[sym->mName] = sym;
			lib->mGlobals.push_back(sym);
			g_GlobalSymbols[sym->mName] = sym;
		}
	}
	else
	{
		ScopeStack::reverse_iterator top = scopeStack.rbegin();
		top->mLocals.push_back(sym);

		while(top->mType == ST_SCOPE)
		{
			++top;
		}

		if (top->mType == ST_FUNCTION)
		{
			Function* pFunc = static_cast<FunctionNode*>(top->mNode)->mSymbol;
			for (auto local : pFunc->mLocals)
			{
				if (local->mName.compare(sym->mName) == 0)
					ErrorSameName(sym->mName);
			}

			pFunc->mLocals.push_back(sym);
			sym->mParentFunction = pFunc;
		}
		else if (top->mType == ST_CLASS)
		{
			Type* pClass = static_cast<ClassNode*>(top->mNode)->mSymbol;

			for (auto member : pClass->mMembers)
			{
				if (member->mName.compare(sym->mName) == 0)
					ErrorSameName(sym->mName);
			}

			pClass->mMembersByName[sym->mName] = sym;
			pClass->mMembers.push_back(sym);
			sym->mParentType = pClass;
		}
	}
	lib->mAllSymbols.push_back(std::make_unique<T>(*sym));
}

#pragma region VisitorBase
// All of our node types inherit from this node and implement the Walk function
enum VisitResult
{
    Continue,
    Stop
};

class Visitor
{
public:
    virtual VisitResult Visit(AbstractNode*   node) { return Continue; }
    virtual VisitResult Visit(BlockNode* node) { return this->Visit((AbstractNode*)node); }
    virtual VisitResult Visit(StatementNode*  node) { return this->Visit((AbstractNode*)node); }
    virtual VisitResult Visit(ExpressionNode* node) { return this->Visit((StatementNode*)node); }
    //virtual VisitResult Visit(LiteralNode*    node)   { return this->Visit((ExpressionNode*)node);     }
    virtual VisitResult Visit(UnaryOperatorNode* node) { return this->Visit((ExpressionNode*)node); }
    virtual VisitResult Visit(BinaryOperatorNode* node) { return this->Visit((ExpressionNode*)node); }
    virtual VisitResult Visit(PostExpressionNode* node) { return this->Visit((ExpressionNode*)node); }
    virtual VisitResult Visit(MemberAccessNode*   node) { return this->Visit((PostExpressionNode*)node); }
    virtual VisitResult Visit(ClassNode*      node) { return this->Visit((AbstractNode*)node); }
    //virtual VisitResult Visit(MemberNode*     node)   { return this->Visit((AbstractNode*)node);       }
    virtual VisitResult Visit(VariableNode*   node) { return this->Visit((StatementNode*)node); }
    virtual VisitResult Visit(TypeNode*       node) { return this->Visit((AbstractNode*)node); }
    //virtual VisitResult Visit(NamedReference* node)   { return this->Visit((ExpressionNode*)node);     }
    virtual VisitResult Visit(ValueNode* node) { return this->Visit((ExpressionNode*)node); }
    virtual VisitResult Visit(FunctionNode*   node) { return this->Visit((AbstractNode*)node); }
    virtual VisitResult Visit(ParameterNode*  node) { return this->Visit((VariableNode*)node); }
    virtual VisitResult Visit(LabelNode* node) { return this->Visit((StatementNode*)node); }
    virtual VisitResult Visit(GotoNode* node) { return this->Visit((StatementNode*)node); }
    virtual VisitResult Visit(CallNode* node) { return this->Visit((PostExpressionNode*)node); }
    virtual VisitResult Visit(CastNode* node) { return this->Visit((PostExpressionNode*)node); }
    virtual VisitResult Visit(IndexNode* node) { return this->Visit((PostExpressionNode*)node); }
    virtual VisitResult Visit(ForNode* node) { return this->Visit((StatementNode*)node); }
    virtual VisitResult Visit(WhileNode* node) { return this->Visit((StatementNode*)node); }
    virtual VisitResult Visit(ScopeNode* node) { return this->Visit((StatementNode*)node); }
    virtual VisitResult Visit(IfNode* node) { return this->Visit((StatementNode*)node); }
    virtual VisitResult Visit(BreakNode* node) { return this->Visit((StatementNode*)node); }
    virtual VisitResult Visit(ContinueNode* node) { return this->Visit((StatementNode*)node); }
    virtual VisitResult Visit(ReturnNode* node) { return this->Visit((StatementNode*)node); }
};

#pragma endregion 

#pragma region Phase1Visitor
class Phase1Visitor : public Visitor
{
public:
    Phase1Visitor(Library* lib = nullptr) { mLib = lib;}

    VisitResult Visit(AbstractNode* node) override;
    VisitResult Visit(ClassNode* node) override;
    VisitResult Visit(BlockNode* node) override;
    VisitResult Visit(UnaryOperatorNode* node) override;
    VisitResult Visit(BinaryOperatorNode* node) override;
    VisitResult Visit(PostExpressionNode* node) override;
    VisitResult Visit(MemberAccessNode*   node) override;
    VisitResult Visit(VariableNode*   node) override;
    VisitResult Visit(FunctionNode*   node) override;
    VisitResult Visit(CallNode* node) override;
    VisitResult Visit(CastNode* node) override;
    VisitResult Visit(IndexNode* node) override;
    VisitResult Visit(ForNode* node) override;
    VisitResult Visit(WhileNode* node) override;
    VisitResult Visit(ScopeNode* node) override;
    VisitResult Visit(IfNode* node) override;
    VisitResult Visit(ReturnNode* node) override;

    Library* mLib;
};
VisitResult Phase1Visitor::Visit(AbstractNode* node)
{
    return Continue;
}

VisitResult Phase1Visitor::Visit(ClassNode* node)
{
    if (node->mSymbol == nullptr)
    {
       for (SymbolMap::iterator it = g_GlobalSymbols.begin(); it != g_GlobalSymbols.end(); ++it)
       {
      	  if (it->first.compare(node->mName.mText) == 0 && node->mSymbol != it->second)
      		  ErrorSameName(it->first);
       }
    
       node->mSymbol = mLib->CreateType(node->mName.mText, true);

       for (unsigned i = 0; i < node->mMembers.size(); ++i)
       {
	       node->mMembers[i]->mParent = node;
       }
    }

    return Continue;
}

VisitResult Phase1Visitor::Visit(BlockNode* node)
{
    for (unsigned i = 0; i < node->mGlobals.size(); ++i)
    {
        node->mGlobals[i]->mParent = node;
    }
    return Continue;
}

VisitResult Phase1Visitor::Visit(UnaryOperatorNode* node) 
{
    node->mRight->mParent = node;
    return Continue;
}

VisitResult Phase1Visitor::Visit(BinaryOperatorNode* node) 
{
    node->mRight->mParent = node->mLeft->mParent = node;
    return Continue;
}

VisitResult Phase1Visitor::Visit(PostExpressionNode* node) 
{
    node->mLeft->mParent = node;
    return Continue;
}

VisitResult Phase1Visitor::Visit(MemberAccessNode*   node) 
{
    node->mLeft->mParent = node;
    return Continue;
}

VisitResult Phase1Visitor::Visit(VariableNode*   node) 
{
    node->mType->mParent = node;

    if (node->mInitialValue)
        node->mInitialValue->mParent = node;

    return Continue;
}

VisitResult Phase1Visitor::Visit(FunctionNode*   node)
{
    for (unsigned i = 0; i < node->mParameters.size(); ++i)
    {
	      node->mParameters[i]->mParent = node;
    }

    node->mScope->mParent = node;

    if (node->mReturnType)
        node->mReturnType->mParent = node;

    return Continue;
}

VisitResult Phase1Visitor::Visit(CallNode* node)
{
    for (unsigned i = 0; i < node->mArguments.size(); ++i)
    {
        node->mArguments[i]->mParent = node;
    }
    return Continue;
}

VisitResult Phase1Visitor::Visit(CastNode* node)
{
    node->mLeft->mParent = node;
    return Continue;
}

VisitResult Phase1Visitor::Visit(IndexNode* node)
{
    node->mLeft->mParent = node->mIndex->mParent = node;
    return Continue;
}

VisitResult Phase1Visitor::Visit(ForNode* node)
{
    if (node->mInitialExpression)
        node->mInitialExpression->mParent = node;

    if (node->mInitialVariable)
        node->mInitialVariable->mParent = node;

    if (node->mIterator)
        node->mIterator->mParent = node;

    if (node->mCondition)
        node->mCondition->mParent = node;

    node->mScope->mParent = node;

    return Continue;
}

VisitResult Phase1Visitor::Visit(WhileNode* node)
{
    node->mCondition->mParent = node->mScope->mParent = node;
    return Continue;
}

VisitResult Phase1Visitor::Visit(ScopeNode* node)
{
    for (unsigned i = 0; i < node->mStatements.size(); ++i)
    {
        node->mStatements[i]->mParent = node;
    }
    return Continue;
}

VisitResult Phase1Visitor::Visit(IfNode* node)
{
    if (node->mCondition)
        node->mCondition->mParent = node;

    if (node->mElse)
        node->mElse->mParent = node;

    node->mScope->mParent = node;
    return Continue;
}

VisitResult Phase1Visitor::Visit(ReturnNode* node)
{
    if(node->mReturnValue)
        node->mReturnValue->mParent = node;

    return Continue;
}
#pragma endregion

#pragma region Phase2Visitor
class Phase2Visitor : public Visitor
{
public:
    Phase2Visitor(Library* lib = nullptr) { mLib = lib; }
    VisitResult Visit(FunctionNode* node) override;
    
    VisitResult Visit(TypeNode* node) override;
    
    Library* mLib;
};

VisitResult Phase2Visitor::Visit(TypeNode* node)
{
    std::string name = node->mName.mText;
    Type* givenType = static_cast<Type*>(g_GlobalSymbols[name]);
    if(givenType)
        node->mSymbol = mLib->GetPointerType(givenType, node->mPointerCount);
    else
        ErrorSymbolNotFound(name);

    return Continue;
}

VisitResult Phase2Visitor::Visit(FunctionNode* node)
{
    std::vector<Type*> parameterTypes;

    TypeNode* paramNode;
    Type* baseType;
    for (unsigned i = 0; i < node->mParameters.size(); ++i)
    {
        paramNode = node->mParameters[i]->mType.get();
        
        paramNode->Walk(this);

        if(paramNode->mSymbol == nullptr)
            paramNode->mSymbol = mLib->CreateType(paramNode->mName.mText, paramNode->mParent == nullptr);
        
        baseType = static_cast<Type*>(g_GlobalSymbols[paramNode->mName.mText]);
        parameterTypes.push_back(mLib->GetPointerType(baseType, paramNode->mPointerCount));
    }

    node->mScope->Walk(this);
    
    Type* returnType;
    if (node->mReturnType)
    {
        node->mReturnType->Walk(this);
        returnType = node->mReturnType->mSymbol;
    }
    else
    {
        returnType = static_cast<Type*>(g_GlobalSymbols["Void"]);
    }

    node->mSignatureType = mLib->GetFunctionType(parameterTypes, returnType);
    return Stop;
}

#pragma endregion

#pragma region Phase3Visitor
class Phase3Visitor : public Visitor
{
public:
    Phase3Visitor(Library* lib = nullptr) { mLib = lib; }

    VisitResult Visit(LabelNode* node) override;

    VisitResult Visit(VariableNode* node) override;

    //Get added to scope stack
    VisitResult Visit(ClassNode* node) override;
    VisitResult Visit(FunctionNode* node) override;
	VisitResult Visit(ScopeNode* node) override;

    Library* mLib;
};

VisitResult Phase3Visitor::Visit(LabelNode* node)
{
    bool isGlobal = scopeStack.size() == 0;
	std::string name = node->mName.mText;
    node->mSymbol = mLib->CreateLabel(name, isGlobal);    
	bool error = false;

	if (isGlobal)
	{
		if (mLib->mGlobalsByName.find(name) != mLib->mGlobalsByName.end())
		{
			error = true;
		}
		else
		{
			mLib->mGlobalsByName[name] = node->mSymbol;
			mLib->mGlobals.push_back(node->mSymbol);
			g_GlobalSymbols[name] = node->mSymbol;
		}
	}
	else
	{
		ScopeStack::reverse_iterator topFunc;
		for (topFunc = scopeStack.rbegin(); topFunc->mType != ST_FUNCTION && topFunc != scopeStack.rend(); ++topFunc);

		Function* pFunc = static_cast<FunctionNode*>(topFunc->mNode)->mSymbol;
		if (pFunc->mLabelsByName.find(name) == pFunc->mLabelsByName.end())
		{
			node->mSymbol->mParentFunction = pFunc;
			pFunc->mLabelsByName[name] = node->mSymbol;
			pFunc->mLocals.push_back(node->mSymbol);
		}
		else
			error = true;
	}
	mLib->mAllSymbols.push_back(std::make_unique<Label>(*node->mSymbol));

	if (error)
		ErrorSameName(name);

    return Stop;
}

VisitResult Phase3Visitor::Visit(VariableNode* node)
{
    bool isGlobal = scopeStack.size() == 0;
	node->mSymbol = mLib->CreateVariable(node->mName.mText, isGlobal);

    node->mType->Walk(this);
    node->mSymbol->mType = node->mType->mSymbol;
    
    if (node->mInitialValue)
    {
        node->mInitialValue->Walk(this);
        node->mInitialValue->mResolvedType = node->mType->mSymbol;
    }
    
    AddSymbolToLibrary(mLib, node->mSymbol, isGlobal);

	return Stop;
}

VisitResult Phase3Visitor::Visit(ClassNode* node)
{
	scopeStack.push_back(ScopeEntry(node, ST_CLASS));
	for (unsigned i = 0; i < node->mMembers.size(); ++i)
	{
		node->mMembers[i]->Walk(this);
	}
    scopeStack.pop_back();
	return Stop;
}

VisitResult Phase3Visitor::Visit(FunctionNode* node)
{
	bool isGlobal = scopeStack.size() == 0;
	node->mSymbol = mLib->CreateFunction(node->mName.mText, isGlobal);
    node->mSymbol->mType = node->mSignatureType;
    node->mSymbol->mExecutableFunction = node;

	scopeStack.push_back(ScopeEntry(node, ST_FUNCTION));
    for (unsigned i = 0; i < node->mParameters.size(); ++i)
	{
        node->mParameters[i]->Walk(this);
        node->mParameters[i]->mSymbol->mIsParameter = true;
    }
    
    if (node->mReturnType)
    {
        node->mReturnType->Walk(this);
    }

    node->mScope->Walk(this);
    scopeStack.pop_back();

    AddSymbolToLibrary(mLib, node->mSymbol, isGlobal);

	return Stop;
}

VisitResult Phase3Visitor::Visit(ScopeNode* node)
{
	scopeStack.push_back(ScopeEntry(node, ST_SCOPE));
	for (unsigned i = 0; i < node->mStatements.size(); ++i)
	{
		node->mStatements[i]->Walk(this);
	}
	scopeStack.pop_back();

	return Stop;
}
#pragma endregion

#pragma region Phase4Visitor
class Phase4Visitor : public Visitor
{
public:
	Phase4Visitor(Library* lib = nullptr) { mLib = lib; }
	VisitResult Visit(ValueNode* node) override;

	VisitResult Visit(IndexNode* node) override;

	VisitResult Visit(MemberAccessNode* node) override;

    VisitResult Visit(BinaryOperatorNode* node) override;
    
    VisitResult Visit(UnaryOperatorNode* node) override;
    
    VisitResult Visit(CallNode* node) override;
    
    VisitResult Visit(CastNode* node) override;
    
    VisitResult Visit(IfNode* node) override;
    
    VisitResult Visit(ForNode* node) override;
    
    VisitResult Visit(WhileNode* node) override;
    
    VisitResult Visit(GotoNode* node) override;
    
    VisitResult Visit(ReturnNode* node) override;

    VisitResult Visit(VariableNode* node) override;

    VisitResult Visit(BreakNode* node) override;

    VisitResult Visit(ContinueNode* node) override;

    //Get added to scope stack
    VisitResult Visit(ClassNode* node) override;
    VisitResult Visit(FunctionNode* node) override;
	VisitResult Visit(ScopeNode* node) override;
    
    Library* mLib;
};

//Get added to scope stack
VisitResult Phase4Visitor::Visit(ClassNode* node)
{
    scopeStack.push_back(ScopeEntry(node, ST_CLASS));
    for (unsigned i = 0; i < node->mMembers.size(); ++i)
    {
        node->mMembers[i]->Walk(this);
    }
    scopeStack.pop_back();
    return Stop;
}


VisitResult Phase4Visitor::Visit(FunctionNode* node)
{
    scopeStack.push_back(ScopeEntry(node, ST_FUNCTION));
    for (unsigned i = 0; i < node->mParameters.size(); ++i)
    {
        node->mParameters[i]->Walk(this);
    }

    if (node->mReturnType)
    {
        node->mReturnType->Walk(this);
    }

    node->mScope->Walk(this);
    scopeStack.pop_back();

    return Stop;
}
VisitResult Phase4Visitor::Visit(ScopeNode* node)
{
	scopeStack.push_back(ScopeEntry(node, ST_SCOPE));
	for (unsigned i = 0; i < node->mStatements.size(); ++i)
	{
		node->mStatements[i]->Walk(this);
	}
	scopeStack.pop_back();
	return Stop;
}
VisitResult Phase4Visitor::Visit(ValueNode* node)
{
    if (node->mResolvedType == nullptr)
    {
        switch (node->mToken.mEnumTokenType)
        {
        case TokenType::IntegerLiteral:
            node->mResolvedType = static_cast<Type*>(g_GlobalSymbols["Integer"]);
            break;

        case TokenType::FloatLiteral:
			node->mResolvedType = static_cast<Type*>(g_GlobalSymbols["Float"]);
            break;

        case TokenType::StringLiteral:
			if (g_GlobalSymbols.find("Byte*") == g_GlobalSymbols.end())
				node->mResolvedType = mLib->CreateType("Byte*", true);
			else
				node->mResolvedType = static_cast<Type*>(g_GlobalSymbols["Byte*"]);
            break;

        case TokenType::CharacterLiteral:
			node->mResolvedType = static_cast<Type*>(g_GlobalSymbols["Byte"]);
            break;

        case TokenType::True:
        case TokenType::False:
			node->mResolvedType = static_cast<Type*>(g_GlobalSymbols["Boolean"]);
            break;

        case TokenType::Null:
			node->mResolvedType = static_cast<Type*>(g_GlobalSymbols["Null*"]);
            break;

        case TokenType::Identifier:
            std::string name = node->mToken.mText;
            ScopeStack::reverse_iterator top;
            bool fStop = false;
            for ( top = scopeStack.rbegin(); top != scopeStack.rend() && !fStop ; ++top)
            {
                for (unsigned i = 0; i < top->mLocals.size() && node->mResolvedType == nullptr; ++i)
                {
                    if (top->mLocals[i]->mName.compare(name) == 0)
                        node->mResolvedType = top->mLocals[i]->mType;
                }

                fStop = node->mResolvedType != nullptr
                        || (top->mType == ST_FUNCTION || top->mType == ST_CLASS);
            }

            if (node->mResolvedType == nullptr)
            {
                SymbolMap::iterator it = g_GlobalSymbols.find(name);
                if (it != g_GlobalSymbols.end())
					node->mResolvedType = it->second->mType;
                else
                    ErrorSymbolNotFound(name);
            }
            break;
        }
    }

    return Stop;
}

//TODO
VisitResult Phase4Visitor::Visit(IndexNode* node)
{
    node->mLeft->Walk(this);
    node->mIndex->Walk(this);

    if (node->mLeft->mResolvedType->mMode == TypeMode::Pointer
     && node->mIndex->mResolvedType == g_GlobalSymbols["Integer"])
	{
		node->mResolvedType = node->mLeft->mResolvedType->mPointerToType;
	}
	else
        ErrorInvalidIndexer(node);
	
    return Stop;
}

VisitResult Phase4Visitor::Visit(MemberAccessNode* node)
{
    node->mLeft->Walk(this);

	Type* pClass = nullptr;
	if (node->mOperator.mEnumTokenType == TokenType::Arrow && node->mLeft->mResolvedType->mMode == TypeMode::Pointer
	 && node->mLeft->mResolvedType->mPointerToType->mMode == TypeMode::Class)
		pClass = node->mLeft->mResolvedType->mPointerToType;
	else if (node->mOperator.mEnumTokenType == TokenType::Dot && node->mLeft->mResolvedType->mMode == TypeMode::Class)
		pClass = node->mLeft->mResolvedType;

	if(pClass)
	{
		if (pClass->mMembersByName.find(node->mName.mText) != pClass->mMembersByName.end())
		{
			node->mResolvedMember = pClass->mMembersByName[node->mName.mText];
			node->mResolvedType = node->mResolvedMember->mType;
		}
		else
			ErrorSymbolNotFound(node->mName.mText);
	}
	else
		ErrorInvalidMemberAccess(node);
	
	return Stop;
}

VisitResult Phase4Visitor::Visit(BinaryOperatorNode* node)
{
	node->mRight->Walk(this);
	node->mLeft->Walk(this);

    switch (node->mOperator.mEnumTokenType)
    {
        case TokenType::Minus:
            if (node->mLeft->mResolvedType->mMode == TypeMode::Pointer
            && node->mRight->mResolvedType->mMode == TypeMode::Pointer)
            {
                node->mResolvedType = static_cast<Type*>(g_GlobalSymbols["Integer"]);
                break;
            }

        case TokenType::Plus:
            if (node->mLeft->mResolvedType == g_GlobalSymbols["Integer"] && node->mRight->mResolvedType->mMode == TypeMode::Pointer)
            {
                node->mResolvedType = node->mRight->mResolvedType;
                break;
            }
            else if (node->mLeft->mResolvedType->mMode == TypeMode::Pointer && node->mRight->mResolvedType == g_GlobalSymbols["Integer"])
            {
                node->mResolvedType = node->mLeft->mResolvedType;
                break;
            }

        case TokenType::Asterisk:
        case TokenType::Divide:
        case TokenType::Modulo:
            if (node->mLeft->mResolvedType == node->mRight->mResolvedType
            && (node->mLeft->mResolvedType == g_GlobalSymbols["Byte"]
             || node->mLeft->mResolvedType == g_GlobalSymbols["Integer"]
             || node->mLeft->mResolvedType == g_GlobalSymbols["Float"]))
            {
                node->mResolvedType = node->mLeft->mResolvedType;
            }
            break;
       
        case TokenType::LessThan:
        case TokenType::GreaterThan:       
        case TokenType::LessThanOrEqualTo:
        case TokenType::GreaterThanOrEqualTo:
        case TokenType::Equality:
        case TokenType::Inequality:
           node->mResolvedType = static_cast<Type*>(g_GlobalSymbols["Boolean"]);
           break;

        case TokenType::LogicalOr:
        case TokenType::LogicalAnd:
            if( (node->mLeft->mResolvedType == g_GlobalSymbols["Boolean"] 
		      || node->mLeft->mResolvedType->mMode == TypeMode::Pointer)
             && (node->mRight->mResolvedType == g_GlobalSymbols["Boolean"]
			  || node->mRight->mResolvedType->mMode == TypeMode::Pointer))
                node->mResolvedType = static_cast<Type*>(g_GlobalSymbols["Boolean"]);
            break;

        case TokenType::Assignment:
            if (node->mLeft->mResolvedType == node->mRight->mResolvedType
             && node->mRight->mResolvedType->mMode != TypeMode::Function)
            {
				node->mResolvedType = node->mLeft->mResolvedType = node->mRight->mResolvedType;
            }
            break;
    }
    
    if(node->mResolvedType == nullptr)
        ErrorInvalidBinaryOperator(node);

    return Stop;
}

VisitResult Phase4Visitor::Visit(UnaryOperatorNode* node) 
{
	node->mRight->Walk(this);
    Type* rightType = node->mRight->mResolvedType;

    switch (node->mOperator.mEnumTokenType)
    {
        case TokenType::Asterisk               :
            if (rightType->mMode == TypeMode::Pointer)
                node->mResolvedType = rightType->mPointerToType;
            break;

        case TokenType::Plus                   :
        case TokenType::Minus                  :
            if (rightType == g_GlobalSymbols["Byte"]
             || rightType == g_GlobalSymbols["Integer"]
             || rightType == g_GlobalSymbols["Float"])
                node->mResolvedType = rightType;
            break;

        case TokenType::Increment              :
        case TokenType::Decrement              :
            if (rightType == g_GlobalSymbols["Byte"]
             || rightType == g_GlobalSymbols["Integer"]
             || rightType == g_GlobalSymbols["Float"]
             || rightType->mMode == TypeMode::Pointer)
                node->mResolvedType = rightType;
            break;

        case TokenType::LogicalNot             :
            if (rightType->mMode == TypeMode::Pointer
             || rightType == g_GlobalSymbols["Boolean"])
                node->mResolvedType = static_cast<Type*>(g_GlobalSymbols["Boolean"]);
            break;

        case TokenType::BitwiseAndAddressOf    :
            node->mResolvedType = mLib->GetPointerType(rightType);
            break;
    }

    if(node->mResolvedType == nullptr)
        ErrorInvalidUnaryOperator(node);

	return Stop;
}

VisitResult Phase4Visitor::Visit(CallNode* node) 
{
    node->mLeft->Walk(this);
    
    if (node->mLeft->mResolvedType->mMode != TypeMode::Function)
        ErrorNonCallableType(node->mLeft->mResolvedType);

    const std::vector<Type*>& params = node->mLeft->mResolvedType->mParameterTypes;
    bool fError = node->mArguments.size() != params.size();
    for (unsigned i = 0; i < node->mArguments.size(); ++i)
    {
        node->mArguments[i]->Walk(this);
        if (node->mArguments[i]->mResolvedType != params[i])
            fError = true;
    }

    if (fError)
        ErrorInvalidCall(node);
    else
		node->mResolvedType = node->mLeft->mResolvedType->mReturnType;

//    if (node->mArguments.size() == 0)
//        node->mResolvedType->mParameterTypes.push_back(static_cast<Type*>(g_GlobalSymbols["Void"]));

    return Stop;
}

VisitResult Phase4Visitor::Visit(CastNode* node) 
{
	node->mLeft->Walk(this);
	node->mType->Walk(this);

    node->mResolvedType = node->mType->mSymbol;

	if ( node->mLeft->mResolvedType->mMode == TypeMode::Function
	  || node->mType->mSymbol->mMode == TypeMode::Function
	  || node->mLeft->mResolvedType->mMode == TypeMode::Pointer
      && node->mType->mSymbol->mMode == TypeMode::Class
      && node->mType->mSymbol != g_GlobalSymbols["Boolean"]
      && node->mType->mSymbol != g_GlobalSymbols["Integer"]
      || node->mType->mSymbol->mMode == TypeMode::Pointer
      && node->mLeft->mResolvedType->mMode == TypeMode::Class
      && node->mLeft->mResolvedType != g_GlobalSymbols["Integer"])
		ErrorInvalidCast(node->mLeft->mResolvedType, node->mType->mSymbol);
		
	return Stop;
}

VisitResult Phase4Visitor::Visit(IfNode* node) 
{
	bool error = false;
    if (node->mCondition)
    {
        node->mCondition->Walk(this);

		if (node->mCondition == nullptr
			|| !(node->mCondition->mResolvedType->mMode == TypeMode::Pointer
				|| node->mCondition->mResolvedType == g_GlobalSymbols["Boolean"]))
			error = true;
    }

	node->mScope->Walk(this);

	if (node->mElse)
		node->mElse->Walk(this);

	if(error)
		ErrorConditionExpectedBooleanOrPointer(node->mCondition->mResolvedType);

	return Stop;
}

VisitResult Phase4Visitor::Visit(ForNode* node) 
{
    scopeStack.push_back(ScopeEntry(node, ST_LOOP));
    if (node->mInitialExpression)
        node->mInitialExpression->Walk(this);

    if (node->mInitialVariable)
        node->mInitialVariable->Walk(this);

    if (node->mCondition)
    {
        node->mCondition->Walk(this);

        if (node->mCondition == nullptr
            || !(node->mCondition->mResolvedType->mMode == TypeMode::Pointer
                || node->mCondition->mResolvedType == g_GlobalSymbols["Boolean"]))
            ErrorConditionExpectedBooleanOrPointer(node->mCondition->mResolvedType);
    }

    if (node->mIterator)
        node->mIterator->Walk(this);

    node->mScope->Walk(this);
    scopeStack.pop_back();

    return Stop;
}

VisitResult Phase4Visitor::Visit(WhileNode* node) 
{
    scopeStack.push_back(ScopeEntry(node, ST_LOOP));
    if (node->mCondition)
    {
        node->mCondition->Walk(this);

        if (node->mCondition == nullptr
            || !(node->mCondition->mResolvedType->mMode == TypeMode::Pointer
                || node->mCondition->mResolvedType == g_GlobalSymbols["Boolean"]))
            ErrorConditionExpectedBooleanOrPointer(node->mCondition->mResolvedType);
    }

    node->mScope->Walk(this);
    scopeStack.pop_back();

    return Stop;
}

VisitResult Phase4Visitor::Visit(GotoNode* node) 
{
	bool found = false;
	FunctionNode* topFunc = nullptr;
	for (ScopeStack::reverse_iterator it = scopeStack.rbegin(); !found && it != scopeStack.rend(); ++it)
	{
		if (it->mType == ST_FUNCTION)
		{
			if (!topFunc)
				topFunc = static_cast<FunctionNode*>(it->mNode);

			Function* pFunc = static_cast<FunctionNode*>(it->mNode)->mSymbol;
			if (pFunc->mLabelsByName.find(node->mName.mText) != pFunc->mLabelsByName.end())
				node->mResolvedLabel = pFunc->mLabelsByName[node->mName.mText];
		}
	}

	return Stop;
}

VisitResult Phase4Visitor::Visit(ReturnNode* node) 
{
    FunctionNode* parentFunc = static_cast<FunctionNode*>(node->mParent->mParent);
    Type* got;
    Type* expected;
    got = expected = nullptr;
    if (node->mReturnValue)
    {
        node->mReturnValue->Walk(this);
        got = node->mReturnValue->mResolvedType;
    }
    
    if (parentFunc->mReturnType)
        expected = parentFunc->mReturnType->mSymbol;
    else
        expected = static_cast<Type*>(g_GlobalSymbols["Void"]);

    if(( !got && expected != g_GlobalSymbols["Void"])
    || (got != expected))
    {
        ErrorTypeMismatch(expected, got);
    }
	return Stop;
}

VisitResult Phase4Visitor::Visit(VariableNode* node)
{	
	node->mType->Walk(this);

	if (node->mInitialValue)
	{
		node->mInitialValue->Walk(this);
		node->mInitialValue->mResolvedType = node->mType->mSymbol;
	}

	bool isGlobal = scopeStack.size() == 0;
	if (node->mSymbol == nullptr)
	{
		node->mSymbol = mLib->CreateVariable(node->mName.mText, isGlobal);
		node->mSymbol->mType = node->mType->mSymbol;
		AddSymbolToLibrary(mLib, node->mSymbol, isGlobal);
	}
	else if(!isGlobal)
	{
		scopeStack.back().mLocals.push_back(node->mSymbol);
	}
	return Stop;
}


VisitResult Phase4Visitor::Visit(BreakNode* node)
{   
    for (ScopeStack::reverse_iterator top = scopeStack.rbegin(); top != scopeStack.rend(); ++top)
    {
        if (top->mType == ST_LOOP)
            return Stop;
    }
    
    ErrorBreakContinueMustBeInsideLoop();
    return Stop;
}

VisitResult Phase4Visitor::Visit(ContinueNode* node)
{
    for (ScopeStack::reverse_iterator top = scopeStack.rbegin(); top != scopeStack.rend(); ++top)
    {
        if (top->mType == ST_LOOP)
            return Stop;
    }

    ErrorBreakContinueMustBeInsideLoop();
    return Stop;
}
#pragma endregion


Type* Library::CreateType(const std::string& name, bool isGlobal)
{
    unsigned pos = name.find("*");
    Type* newSymbol;
    if (pos != name.npos)
    {
        size_t numPtrs = name.size() - pos;
		std::string givenname = name.substr(0, pos);
        Type* givenType = nullptr;

		if (g_GlobalSymbols.find(givenname) != g_GlobalSymbols.end())
		{
			givenType = static_cast<Type*>(g_GlobalSymbols[givenname]);
		}
		else
		{
			for (unsigned i = 0; i < mAllSymbols.size(); ++i)
			{
				if (mAllSymbols[i]->mName.compare(givenname) == 0)
				{
					givenType = static_cast<Type*>(mAllSymbols[i].get());
				}
			}
		}
        
        newSymbol = GetPointerType(givenType, numPtrs);
        newSymbol->mName = name;
    }
    else
    {
        newSymbol = new Type();
        newSymbol->mLibrary = this;
        newSymbol->mName = name;

        //Items to fill
        newSymbol->mParentFunction = nullptr;
        newSymbol->mParentType = nullptr;
        newSymbol->mType = nullptr;
    
        if (isGlobal)
        {
            if (mGlobalsByName.find(name) != mGlobalsByName.end())
            {
                ErrorSameName(name);
            }
            else
            {
                mGlobalsByName[name] = newSymbol;
                mGlobals.push_back(newSymbol);
            }
        }

        mAllSymbols.push_back(std::make_unique<Type>(*newSymbol));
        g_GlobalSymbols[name] = newSymbol;

    }
    return newSymbol;
}

Variable* Library::CreateVariable(const std::string& name, bool isGlobal)
{
    Variable* newSymbol = new Variable();
    newSymbol->mLibrary = this;
    newSymbol->mName = name;

    return newSymbol;
}

Function* Library::CreateFunction(const std::string& name, bool isGlobal)
{
    Function* newSymbol = new Function();
    newSymbol->mLibrary = this;
    newSymbol->mName = name;
    
    return newSymbol;
}

Label* Library::CreateLabel(const std::string& name, bool isGlobal)
{
    Label* newSymbol = new Label();
    newSymbol->mLibrary = this;
    newSymbol->mName = name;
    newSymbol->mType = nullptr;
    newSymbol->mParentFunction = nullptr;
	newSymbol->mParentType = nullptr;
    return newSymbol;
}

Type* Library::GetPointerType(Type* givenType)
{
    std::string name = givenType->mName;
    name.append("*");
    
    Symbol* symbol;
    for (unique_vector<Symbol>::iterator it = mAllSymbols.begin(); it != mAllSymbols.end(); ++it)
    {
        symbol = it->get();
        if (name.compare(symbol->mName) == 0)
        {
            return static_cast<Type*>(symbol);
        }
    }
    
    std::unique_ptr<Type> uptrType = std::make_unique<Type>();
    Type* type = uptrType.get();
    type->mLibrary = this;
    type->mType = nullptr;
    type->mName = name;
    type->mParentFunction = nullptr;
    type->mParentType = nullptr;
    type->mMode = TypeMode::Pointer;
    type->mPointerToType = givenType;

    mGlobalsByName[name] = type;
    mGlobals.push_back(type);
    mAllSymbols.push_back(std::move(uptrType));
    g_GlobalSymbols.insert(SymbolPair(name, type));
    return type;
}

Type* Library::GetPointerType(Type* pointerToType, size_t pointerCount)
{
    Type* current = pointerToType;
    for (unsigned i = 0; i < pointerCount; ++i)
    {
        current = GetPointerType(current);
    }
    
    return current;
}

Type* Library::GetFunctionType(std::vector<Type*> parameterTypes, Type* returnType)
{
    Type* symbol;
    for (unique_vector<Symbol>::iterator it = mAllSymbols.begin(); it != mAllSymbols.end(); ++it)
    {
        symbol = static_cast<Type*>(it->get());
        if ( symbol->mMode == TypeMode::Function
          && symbol->mParameterTypes == parameterTypes 
          && symbol->mReturnType == returnType)
        {
            return symbol;
        }
    }

    symbol = new Type();
    symbol->mLibrary = this;
    symbol->mMode = TypeMode::Function;    
    symbol->mParameterTypes = parameterTypes;
    symbol->mType = nullptr;
    symbol->mParentFunction = nullptr;
    symbol->mParentType = nullptr;

    std::string funcSigStr("function(");
    Type* paramType;
    for (unsigned i = 0; i < parameterTypes.size(); ++i)
    {
        paramType = parameterTypes[i];
        funcSigStr.append(paramType->mName);
        
        if (parameterTypes.size() - i > 1)
            funcSigStr.append(", ");
    }

    funcSigStr.append(") : ");

    funcSigStr.append(returnType->mName);
    symbol->mReturnType = returnType;

    symbol->mName = funcSigStr;

    mGlobalsByName[funcSigStr] = symbol; //Do we need to do this?
    mGlobals.push_back(symbol);
    g_GlobalSymbols[funcSigStr] = symbol;
    mAllSymbols.push_back(std::make_unique<Type>(*symbol));
    
    return symbol;
}

// Run semantic analysis over the tree
// You should initialize your symbol table with the libraries that you depend upon given here
// This will run multiple passes to collect symbols (module based compilation)
// This will also compute types of expressions and maintain the variable scope stack
// If an error occurs within analysis the above 'SemanticException' must be thrown with the correct error
// The library should contain all the proper symbols after completion
// A partial library may be used to print out a tree after an error occurs (for your own debugging)
void SemanticAnalyize(AbstractNode* node, std::vector<Library*>& dependencies, Library* library)
{
    scopeStack.clear();
    g_GlobalSymbols.clear();
    for (auto lib : dependencies)
    {
        for (SymbolMap::iterator it = lib->mGlobalsByName.begin(); it != lib->mGlobalsByName.end(); ++it)
        {
            g_GlobalSymbols[it->first] = it->second;
        }
    }

    std::vector<Visitor*> passes;
    
    passes.push_back(new Phase1Visitor(library));
    passes.push_back(new Phase2Visitor(library));
    passes.push_back(new Phase3Visitor(library));
    passes.push_back(new Phase4Visitor(library));
    
    for (auto pass : passes)
        node->Walk(pass);
    
}

#pragma region PrintSymbolVisitor

#define PRINT_NODE(outputStr)                            \
NodePrinter printer;\
printer << outputStr; 

class PrintSymbolVisitor : public Visitor
{
public:
    /*Specials*/
    VisitResult Visit(ClassNode* node) override;
    
    VisitResult Visit(VariableNode* node) override;
    
    VisitResult Visit(ParameterNode* node) override;
    
    VisitResult Visit(FunctionNode* node) override;
    
    VisitResult Visit(TypeNode* node) override;
    
    VisitResult Visit(LabelNode* node) override;
    
    VisitResult Visit(GotoNode* node) override;
    
    VisitResult Visit(ExpressionNode* node) override;
    
    VisitResult Visit(ValueNode* node) override;
    
    VisitResult Visit(BinaryOperatorNode* node) override;
    
    VisitResult Visit(UnaryOperatorNode* node) override;
    
    VisitResult Visit(PostExpressionNode* node) override;
    
    VisitResult Visit(MemberAccessNode* node) override;
    
    VisitResult Visit(CallNode* node) override;
    
    VisitResult Visit(CastNode* node) override;
    
    VisitResult Visit(IndexNode* node) override;
    
    /*Normals*/
    VisitResult Visit(AbstractNode* node) override;
    
    VisitResult Visit(BlockNode* node) override;
    
    VisitResult Visit(IfNode* node) override;
    
    VisitResult Visit(WhileNode* node) override;
    
    VisitResult Visit(ForNode* node) override;
    
    VisitResult Visit(ScopeNode* node) override;
    
    VisitResult Visit(BreakNode* node) override;
    
    VisitResult Visit(ContinueNode* node) override;
    
    VisitResult Visit(ReturnNode* node) override;
};

VisitResult PrintSymbolVisitor::Visit(ClassNode* node)
{
    PRINT_NODE("ClassNode(" << node->mSymbol << ")")
    
    for (unsigned i = 0; i < node->mMembers.size(); ++i)
    {
      node->mMembers[i]->Walk(this);
    }
    
    return Stop;
}

VisitResult PrintSymbolVisitor::Visit(VariableNode* node)
{
	PRINT_NODE("VariableNode(" << node->mSymbol << ")")

	node->mType->Walk(this);
	
	if(node->mInitialValue)
		node->mInitialValue->Walk(this);

	return Stop;
}

VisitResult PrintSymbolVisitor::Visit(ParameterNode* node)
{
	PRINT_NODE("ParameterNode(" << node->mSymbol << ")")

	node->mType->Walk(this);

	if (node->mInitialValue)
		node->mInitialValue->Walk(this);

	return Stop;
}

VisitResult PrintSymbolVisitor::Visit(FunctionNode* node)
{
	PRINT_NODE("FunctionNode(" << node->mSymbol << ")")

	for (unsigned i = 0; i < node->mParameters.size(); ++i)
	{
		node->mParameters[i]->Walk(this);
	}

	if (node->mReturnType)
		node->mReturnType->Walk(this);

	node->mScope->Walk(this);

	return Stop;
}

VisitResult PrintSymbolVisitor::Visit(TypeNode* node)
{
	PRINT_NODE("TypeNode(" << node->mSymbol << ")")
	return Stop;
}

VisitResult PrintSymbolVisitor::Visit(LabelNode* node)
{
	PRINT_NODE("LabelNode(" << node->mSymbol << ")")
	return Stop;
}

VisitResult PrintSymbolVisitor::Visit(GotoNode* node)
{
	PRINT_NODE("GotoNode(" << node->mResolvedLabel << ")")
	return Stop;
}

VisitResult PrintSymbolVisitor::Visit(ExpressionNode* node)
{
	PRINT_NODE("ExpressionNode(" << node->mResolvedType << ")")
	return Stop;
}

VisitResult PrintSymbolVisitor::Visit(ValueNode* node)
{
	PRINT_NODE("ValueNode(" << node->mToken << ", " << node->mResolvedType << ")")
	return Stop;
}

VisitResult PrintSymbolVisitor::Visit(BinaryOperatorNode* node)
{
	PRINT_NODE("BinaryOperatorNode(" << node->mOperator << ", " << node->mResolvedType << ")")

	node->mRight->Walk(this);
	node->mLeft->Walk(this);
	return Stop;
}


VisitResult PrintSymbolVisitor::Visit(UnaryOperatorNode* node)
{
	PRINT_NODE("UnaryOperatorNode(" << node->mOperator << ", " << node->mResolvedType << ")")
	node->mRight->Walk(this);
	return Stop;
}

VisitResult PrintSymbolVisitor::Visit(PostExpressionNode* node)
{
	PRINT_NODE("PostExpressionNode(" << node->mResolvedType << ")")

	node->mLeft->Walk(this);
	return Stop;
}

VisitResult PrintSymbolVisitor::Visit(MemberAccessNode* node)
{
	//TODO print correctly
	PRINT_NODE("MemberAccessNode(" << node->mOperator << ", " << node->mResolvedMember << ", " << node->mResolvedType << ")")
	node->mLeft->Walk(this);
	return Stop;
}

VisitResult PrintSymbolVisitor::Visit(CallNode* node)
{
	PRINT_NODE("CallNode(" << node->mResolvedType << ")")
	node->mLeft->Walk(this);
    for (unsigned i = 0; i < node->mArguments.size(); ++i)
	{
		node->mArguments[i]->Walk(this);
    }

    return Stop;
}

VisitResult PrintSymbolVisitor::Visit(CastNode* node)
{
	PRINT_NODE("CastNode(" << node->mResolvedType << ")")

	node->mLeft->Walk(this);
	node->mType->Walk(this);
	return Stop;
}

VisitResult PrintSymbolVisitor::Visit(IndexNode* node)
{
	PRINT_NODE("IndexNode(" << node->mResolvedType << ")")

	node->mLeft->Walk(this);
	node->mIndex->Walk(this);
	return Stop;
}

VisitResult PrintSymbolVisitor::Visit(AbstractNode* node)
{
	return Continue;
}

VisitResult PrintSymbolVisitor::Visit(BlockNode* node)
{
	PRINT_NODE("BlockNode")
	
	for (unsigned i = 0; i < node->mGlobals.size(); ++i)
	{
		node->mGlobals[i]->Walk(this);
	}
	return Stop;
}

VisitResult PrintSymbolVisitor::Visit(IfNode* node)
{
    PRINT_NODE("IfNode")
    if (node->mCondition)
        node->mCondition->Walk(this);
    
    node->mScope->Walk(this);
    
    if (node->mElse)
        node->mElse->Walk(this);
    
    return Stop;
}

VisitResult PrintSymbolVisitor::Visit(WhileNode* node)
{
    PRINT_NODE("WhileNode")
    node->mCondition->Walk(this);
    node->mScope->Walk(this);
    return Stop;
}

VisitResult PrintSymbolVisitor::Visit(ForNode* node)
{
    PRINT_NODE("ForNode")
    if (node->mInitialVariable)
        node->mInitialVariable->Walk(this);
    
    if (node->mInitialExpression)
        node->mInitialExpression->Walk(this);
    
    if (node->mCondition)
        node->mCondition->Walk(this);
    
    node->mScope->Walk(this);
    
    if (node->mIterator)
        node->mIterator->Walk(this);
    
    return Stop;
}

VisitResult PrintSymbolVisitor::Visit(ScopeNode* node)
{
    PRINT_NODE("ScopeNode")
    for (unsigned i = 0; i < node->mStatements.size(); ++i)
    {
        node->mStatements[i]->Walk(this);
    }
    return Stop;
}

VisitResult PrintSymbolVisitor::Visit(BreakNode* node)
{
    PRINT_NODE("BreakNode")
    return Stop;
}

VisitResult PrintSymbolVisitor::Visit(ContinueNode* node)
{
    PRINT_NODE("ContinueNode")
    return Stop;
}

VisitResult PrintSymbolVisitor::Visit(ReturnNode* node)
{
  PRINT_NODE("ReturnNode")
  if (node->mReturnValue)
        node->mReturnValue->Walk(this);

  return Stop;
}
#pragma endregion

void PrintTreeWithSymbols(AbstractNode* node)
{
    PrintSymbolVisitor printSymbols;
    node->Walk(&printSymbols);
}
