/******************************************************************\
 * Author: 
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/
#include "../Drivers/Driver3.hpp"

#include <iostream>
#include <cassert>
#include <typeinfo> //typeid for classname printing

using std::unique_ptr;
using std::make_unique;

class Parser 
{
public:
#define RETURN_NODE(rule, node) return rule.Accept(std::move(node));

	Parser(std::vector<Token>& tokens)
	{
		m_tokenPos = 0;
		m_tokenStream = &tokens;
		GetCurrentToken();
	}

	~Parser() {}

    unique_ptr<BlockNode> Block()//  
    {
      PrintRule rule("Block");
      unique_ptr<BlockNode> node = make_unique<BlockNode>();
      
      //TODO Implement root body
      if (m_tokenStream->size())
      {
        bool fNewArg = true;
        while( node->mGlobals.push_back(Class()) 
            || node->mGlobals.push_back(Function()) 
            || node->mGlobals.push_back(Var()) && this->Expect(TokenType::Semicolon));
      }
      if (m_tokenPos > m_tokenStream->size())
        throw ParsingException("Too few tokens. Check syntax.");

      RETURN_NODE(rule, node)
    }

    unique_ptr<ExpressionNode> Expression()
    {
      PrintRule rule("Expression");
      unique_ptr<ExpressionNode> node = Expression1();
      if (node == nullptr)
        return false;

      unique_ptr<BinaryOperatorNode> opNode = make_unique<BinaryOperatorNode>();
      if(  this->Accept(TokenType::Assignment, &opNode->mOperator)
        || this->Accept(TokenType::AssignmentPlus, &opNode->mOperator)
        || this->Accept(TokenType::AssignmentMinus, &opNode->mOperator)
        || this->Accept(TokenType::AssignmentMultiply, &opNode->mOperator)
        || this->Accept(TokenType::AssignmentDivide, &opNode->mOperator)
        || this->Accept(TokenType::AssignmentModulo, &opNode->mOperator))
      {
        opNode->mLeft = Expression();
        if (opNode->mLeft == nullptr)
          throw ParsingException();

        opNode->mRight = std::move(node);

        RETURN_NODE(rule, opNode)
      }

      RETURN_NODE(rule, node)
    }

private:
	std::vector<Token>* m_tokenStream;
	unsigned m_tokenPos;
	Token m_currentToken;
	Token m_lastDesiredToken;
	std::string lastError;
	PrintRule* m_lastRule;
	/*
	bool ParseTokenStream(std::vector<Token>& tokens)
	{
		m_tokenStream = tokens;
		bool fSuccess = false;
		try
		{
			switch (m_tokenStream[m_tokenPos].mTokenType)
			{
				case TokenType::Class:
					fSuccess = Class();
					break;

				case TokenType::Function:
					fSuccess = Function();
					break;
			}
		}
		catch (ParsingException exception)
		{
			std::cout << exception.what() << std::tempOpl;
		}
	}
	*/

    void ThrowError(const TokenType::Enum& desiredType)
    {
        char cstr[64];
        sprintf_s(cstr, sizeof(cstr), "Couldn't accept token of type: %d", (int)desiredType);
        lastError = cstr;
        throw ParsingException(cstr);
    }

	//Helper Functions

    bool Expect(const TokenType::Enum& desiredType, Token* token = nullptr) // 
    {
      bool result = this->Accept(desiredType, token);
      if (!result)
        ThrowError(desiredType);

      return result;
    }

    bool Expect(const Token* desiredType, Token* token = nullptr) // 
	{
		bool result = this->Accept(desiredType->mEnumTokenType, token);
		if (!result)
          ThrowError(desiredType->mEnumTokenType);

		return result;
	}

    bool Accept(const TokenType::Enum& desiredType, Token* token = nullptr) // 
    {
        bool result = m_currentToken.mEnumTokenType == desiredType;
        if (result)
        {
          if(token)
            *token = m_currentToken;

          PrintRule::AcceptedToken(desiredType);

          ++m_tokenPos;
          GetCurrentToken();
        }

        return result;
    }

	void GetCurrentToken()
	{
		if (m_tokenStream && m_tokenPos < m_tokenStream->size())
			m_currentToken = (*m_tokenStream)[m_tokenPos];
	}

    #pragma region ParserRules
    //Rule Functions


    unique_ptr<StatementNode> Statement()  
    {
	    PrintRule rule("Statement");

        unique_ptr<StatementNode> node = FreeStatement();
        if (node == nullptr)
        {
          node = DelimitedStatement();
          if(node)
            this->Expect(TokenType::Semicolon);
        }

        RETURN_NODE(rule, node);
    }

    unique_ptr<ClassNode> Class()//  
    {
	    PrintRule rule("Class");
	    if (!this->Accept(TokenType::Class))
		    return false;

        unique_ptr<ClassNode> node = make_unique<ClassNode>();

	    this->Expect(TokenType::Identifier, &node->mName);
	    this->Expect(TokenType::OpenCurley);

        for (unsigned i = 0; i == node->mMembers.size(); ++i)
        {
          if (node->mMembers.push_back(Var()))
            this->Expect(TokenType::Semicolon);
          else
            node->mMembers.push_back(Function());
        }

        this->Expect(TokenType::CloseCurley);
	    RETURN_NODE(rule, node)
    }

    unique_ptr<VariableNode> Var()
    {
	    PrintRule rule("Var");
	    if (!this->Accept(TokenType::Var))
		    return false;

        unique_ptr<VariableNode> node = make_unique<VariableNode>();
	
	    this->Expect(TokenType::Identifier, &node->mName);
	
        node->mType = SpecifiedType();

        if (node->mType == nullptr)
          throw ParsingException();

        if (this->Accept(TokenType::Assignment))
        {
          node->mInitialValue = Expression();
          
          if (node->mInitialValue == nullptr)
            throw ParsingException();
        }
        else
        {
          node->mInitialValue = nullptr;
        }

        RETURN_NODE(rule, node)
    }

    unique_ptr<FunctionNode> Function()  
    {
	    PrintRule rule("Function");
	    if (!this->Accept(TokenType::Function))
		    return false;

        unique_ptr<FunctionNode> node = make_unique<FunctionNode>();

        this->Expect(TokenType::Identifier, &node->mName);
	    this->Expect(TokenType::OpenParentheses);
		
	    if (node->mParameters.push_back(Parameter()))
	    {
            unsigned i = 1; 
		    while (this->Accept(TokenType::Comma) && node->mParameters.size() == i)
		    {
                ++i;
                node->mParameters.push_back(Parameter());
		    }
			
            if (node->mParameters.size() != i)
			    throw ParsingException("Missing a parameter in function.");
	    }

	    this->Expect(TokenType::CloseParentheses);

	    node->mReturnType = SpecifiedType(); //return type

        node->mScope = Scope();
        if (node->mScope == nullptr)
            throw ParsingException("Function missing scope.");

	    return rule.Accept(std::move(node));
    }

    unique_ptr<ParameterNode> Parameter()
    {
	    PrintRule rule("Parameter");
        unique_ptr<ParameterNode> node = make_unique<ParameterNode>();

        if (this->Accept(TokenType::Identifier, &node->mName))
        {
          node->mType = SpecifiedType();
          if (node->mType == nullptr)
              throw ParsingException("Parameter missing specified type.");
        }
        else
        {
          node = nullptr;
        }

        RETURN_NODE(rule, node);
    }

    unique_ptr<TypeNode> SpecifiedType()
    {
	    PrintRule rule("SpecifiedType");
	    if (!this->Accept(TokenType::Colon))
		    return nullptr;

        unique_ptr<TypeNode> node = Type();

        if (node == nullptr)
          throw ParsingException();

        RETURN_NODE(rule, node);
    }

    unique_ptr<ScopeNode> Scope()  
    {
	    PrintRule rule("Scope");
	    if (!this->Accept(TokenType::OpenCurley))
		    return nullptr;

        unique_ptr<ScopeNode> node = make_unique<ScopeNode>();

	    while(node->mStatements.push_back(Statement()));

        this->Expect(TokenType::CloseCurley);
        RETURN_NODE(rule, node);
    }

    unique_ptr<StatementNode> DelimitedStatement()  
    {
	    PrintRule rule("DelimitedStatement");
        unique_ptr<StatementNode> node = Label();

        if (node == nullptr)
          node = Goto();

        if (node == nullptr)
          node = Return();

        if (node == nullptr && this->Accept(TokenType::Break))
          node = make_unique<BreakNode>();

        if (node == nullptr && this->Accept(TokenType::Continue))
          node = make_unique<ContinueNode>();

        if (node == nullptr)
          node = Var();

        if (node == nullptr)
          node = Expression();

        RETURN_NODE(rule, node);
    }

    unique_ptr<StatementNode> FreeStatement()  
    {
	    PrintRule rule("FreeStatement");

        unique_ptr<StatementNode> node = If();
        if(node == nullptr)
          node = While();
        
        if(node == nullptr)
          node = For();

        RETURN_NODE(rule, node)
    }

    unique_ptr<LabelNode> Label()  
    {
	    PrintRule rule("Label");
	    if (!this->Accept(TokenType::Label))
		    return nullptr;

        unique_ptr<LabelNode> node = make_unique<LabelNode>();
        this->Expect(TokenType::Identifier, &node->mName);
        RETURN_NODE(rule, node)
    }

    unique_ptr<GotoNode> Goto()  
    {
	    PrintRule rule("Goto");
	    if (!this->Accept(TokenType::Goto))
		    return false;

        unique_ptr<GotoNode> node = make_unique<GotoNode>();
        this->Expect(TokenType::Identifier, &node->mName);
        RETURN_NODE(rule, node)
    }

    unique_ptr<ReturnNode> Return()
    {
	    PrintRule rule("Return");
	    if (!this->Accept(TokenType::Return))
		    return nullptr;

        unique_ptr<ReturnNode> node = make_unique<ReturnNode>();
	    node->mReturnValue = Expression();

        RETURN_NODE(rule, node)
    }

    unique_ptr<IfNode> If() 
    {
	    PrintRule rule("If");
        unique_ptr<IfNode> node = nullptr;
        if (this->Accept(TokenType::If))
        {
          node = make_unique<IfNode>();

          node->mCondition = GroupedExpression();
          node->mScope = Scope();

          if (node->mScope == nullptr)
            throw ParsingException("If statement is missing scope.");
          
          node->mElse = Else();
        }
        RETURN_NODE(rule, node)
    }

    unique_ptr<IfNode> Else() 
    {
	    PrintRule rule("Else");
        unique_ptr<IfNode> node = nullptr;
        if (this->Accept(TokenType::Else))
        {
          node = If();
          if (node == nullptr)
          {
            node = make_unique<IfNode>();
            node->mScope = Scope();
            if (node->mScope == nullptr)
              throw ParsingException();
          }
        }

        RETURN_NODE(rule, node);
    }

    unique_ptr<WhileNode> While() 
    {
	    PrintRule rule("While");
	    if (!this->Accept(TokenType::While))
		    return nullptr;

        unique_ptr<WhileNode> node = make_unique<WhileNode>();
        
        node->mCondition = GroupedExpression();

        if (node->mCondition == nullptr)
            throw ParsingException("While statement missing condition.");

        node->mScope = Scope();

        if (node->mScope == nullptr)
            throw ParsingException("While statement missing scope.");

        RETURN_NODE(rule, node)
    }

    unique_ptr<ForNode> For()  
    {
	    PrintRule rule("For");
	    if (!this->Accept(TokenType::For))
		    return nullptr;

        unique_ptr<ForNode> node = make_unique<ForNode>();

	    this->Expect(TokenType::OpenParentheses);

        node->mInitialVariable = Var();
	    if (node->mInitialVariable == nullptr)
		    node->mInitialExpression = Expression();

	    this->Expect(TokenType::Semicolon); //first
        node->mCondition = Expression();

	    this->Expect(TokenType::Semicolon); //second
	    node->mIterator = Expression();

        this->Expect(TokenType::CloseParentheses);

        node->mScope = Scope();

        if (node->mScope == nullptr)
            throw ParsingException("For statement missing scope.");

        RETURN_NODE(rule, node)
    }

    unique_ptr<ExpressionNode> GroupedExpression()  
    {
	    PrintRule rule("GroupedExpression");
	    if (!this->Accept(TokenType::OpenParentheses))
		    return nullptr;

        unique_ptr<ExpressionNode> node = Expression();
        this->Expect(TokenType::CloseParentheses);
	    RETURN_NODE(rule, node)
    }

    #pragma region expressions

    unique_ptr<ExpressionNode> Expression1()
    {
      PrintRule rule("Expression1");
      unique_ptr<ExpressionNode> node = Expression2();
      if (node != nullptr)
      {
        bool fOp = false;
        unique_ptr<BinaryOperatorNode> binaryOp, tempOp;
        do
        {
          if (fOp)
          {
            tempOp->mLeft = Expression2();

            if (tempOp->mLeft == nullptr)
              throw ParsingException("BinaryOperator missing left expression.");

            if (binaryOp)
              tempOp->mRight = std::move(binaryOp);
            else
              tempOp->mRight = std::move(node);

            binaryOp = std::move(tempOp);
          }

          tempOp = make_unique<BinaryOperatorNode>();
          fOp = Accept(TokenType::LogicalOr, &tempOp->mOperator);
        } while (fOp);

        if (binaryOp)
        {
          RETURN_NODE(rule, binaryOp)
        }
      }
      RETURN_NODE(rule, node)
    }

    unique_ptr<ExpressionNode> Expression2()
    {
	    PrintRule rule("Expression2");
        unique_ptr<ExpressionNode> node = Expression3();

        if (node != nullptr)
        {
          bool fOp = false;
          unique_ptr<BinaryOperatorNode> binaryOp, tempOp;
          do
          {
            if (fOp)
            {
              tempOp->mLeft = Expression3();

              if (tempOp->mLeft == nullptr)
                throw ParsingException();

              if (binaryOp)
                tempOp->mRight = std::move(binaryOp);
              else
                tempOp->mRight = std::move(node);

              binaryOp = std::move(tempOp);
            }

            tempOp = make_unique<BinaryOperatorNode>();
            fOp = Accept(TokenType::LogicalAnd, &tempOp->mOperator);
          } while (fOp);

          if (binaryOp)
          {
            RETURN_NODE(rule, binaryOp)
          }
        }

        RETURN_NODE(rule, node)
    }

    unique_ptr<ExpressionNode> Expression3()
    {
	    PrintRule rule("Expression3");
        unique_ptr<ExpressionNode> node = Expression4();
        if (node != nullptr)
        {
          bool fOp = false;
          unique_ptr<BinaryOperatorNode> binaryOp, tempOp;
          do
          {
            if (fOp)
            {
              tempOp->mLeft = Expression4();

              if (tempOp->mLeft == nullptr)
                throw ParsingException();

              if (binaryOp)
                tempOp->mRight = std::move(binaryOp);
              else
                tempOp->mRight = std::move(node);

              binaryOp = std::move(tempOp);
            }

            tempOp = make_unique<BinaryOperatorNode>();

            fOp = this->Accept(TokenType::LessThan, &tempOp->mOperator)
              || this->Accept(TokenType::GreaterThan, &tempOp->mOperator)
              || this->Accept(TokenType::LessThanOrEqualTo, &tempOp->mOperator)
              || this->Accept(TokenType::GreaterThanOrEqualTo, &tempOp->mOperator)
              || this->Accept(TokenType::Equality, &tempOp->mOperator)
              || this->Accept(TokenType::Inequality, &tempOp->mOperator);
          } while (fOp);

          if (binaryOp)
          {
            RETURN_NODE(rule, binaryOp)
          }
        }

        RETURN_NODE(rule, node)
    }

    unique_ptr<ExpressionNode> Expression4()
    {
	    PrintRule rule("Expression4");
        unique_ptr<ExpressionNode> node = Expression5();
        if (node != nullptr)
        {

          bool fOp = false;
          unique_ptr<BinaryOperatorNode> binaryOp, tempOp;
          do
          {
            if (fOp)
            {
              tempOp->mLeft = Expression5();

              if (tempOp->mLeft == nullptr)
                throw ParsingException();

              if (binaryOp)
                tempOp->mRight = std::move(binaryOp);
              else
                tempOp->mRight = std::move(node);

              binaryOp = std::move(tempOp);
            }

            tempOp = make_unique<BinaryOperatorNode>();
            fOp = this->Accept(TokenType::Plus, &tempOp->mOperator) || this->Accept(TokenType::Minus, &tempOp->mOperator);
          } while (fOp);

          if (binaryOp)
          {
            RETURN_NODE(rule, binaryOp)
          }
        }

        RETURN_NODE(rule, node)
    }

    unique_ptr<ExpressionNode> Expression5()
    {
	    PrintRule rule("Expression5");
        unique_ptr<ExpressionNode> node = Expression6();
        if (node != nullptr)
        {
          bool fOp = false;
          unique_ptr<BinaryOperatorNode> binaryOp, tempOp;
          do
          {
            if (fOp)
            {
              tempOp->mLeft = Expression6();

              if (tempOp->mLeft == nullptr)
                throw ParsingException();

              if (binaryOp)
                tempOp->mRight = std::move(binaryOp);
              else
                tempOp->mRight = std::move(node);

              binaryOp = std::move(tempOp);
            }

            tempOp = make_unique<BinaryOperatorNode>();
            fOp = this->Accept(TokenType::Asterisk, &tempOp->mOperator)
              || this->Accept(TokenType::Divide, &tempOp->mOperator)
              || this->Accept(TokenType::Modulo, &tempOp->mOperator);
          } while (fOp);

          if (binaryOp)
          {
            RETURN_NODE(rule, binaryOp)
          }
        }

        RETURN_NODE(rule, node)
    }

    unique_ptr<ExpressionNode> Expression6()
    {
	    PrintRule rule("Expression6");
        
        bool fOp = false;
        unique_ptr<UnaryOperatorNode> unaryOp = make_unique<UnaryOperatorNode>();

        UnaryOperatorNode* tempOp = unaryOp.get();
        UnaryOperatorNode* end = tempOp;
        do
        {
          fOp = this->Accept(  TokenType::Asterisk, &tempOp->mOperator)
             || this->Accept(     TokenType::Minus, &tempOp->mOperator)
             || this->Accept(TokenType::LogicalNot, &tempOp->mOperator)
             || this->Accept( TokenType::Increment, &tempOp->mOperator)
             || this->Accept( TokenType::Decrement, &tempOp->mOperator);
        
          if (fOp && tempOp->mOperator)
          {
            end = tempOp;
            tempOp->mRight = make_unique<UnaryOperatorNode>();
            tempOp = dynamic_cast<UnaryOperatorNode*>(tempOp->mRight.get());
          }
        } while (fOp);

        end->mRight = Expression7();
        
        if (end->mRight == nullptr)
          RETURN_NODE(rule, end->mRight)
        else
        {
          if (unaryOp->mOperator)
            RETURN_NODE(rule, unaryOp)
          else
            RETURN_NODE(rule, tempOp->mRight)
        }
    }

    unique_ptr<ExpressionNode> Expression7()
    {
	    PrintRule rule("Expression7");
        unique_ptr<ExpressionNode> node = Value();
        
        if (node != nullptr)
        {
          unique_ptr<PostExpressionNode> postExpr, tempPE;
          do
          {
            if (tempPE)
            {
              if (postExpr)
                tempPE->mLeft = std::move(postExpr);
              else
                tempPE->mLeft = std::move(node);

              postExpr = std::move(tempPE);
            }

            tempPE = MemberAccess();
            if (tempPE)
              continue;

            tempPE = Call();
            if (tempPE)
              continue;

            tempPE = Cast();
            if (tempPE)
              continue;

            tempPE = Index();
          } while (tempPE);

          if (postExpr)
            RETURN_NODE(rule, postExpr)
        }

        RETURN_NODE(rule, node)
    }

    unique_ptr<ExpressionNode> Value()  
    {
	    PrintRule rule("Value"); 
        unique_ptr<ValueNode> node = make_unique<ValueNode>();

	    bool fIsValue = this->Accept(TokenType::True, &node->mToken)
		    || this->Accept(TokenType::False, &node->mToken)
		    || this->Accept(TokenType::Null, &node->mToken)
		    || this->Accept(TokenType::IntegerLiteral, &node->mToken)
		    || this->Accept(TokenType::FloatLiteral, &node->mToken)
		    || this->Accept(TokenType::StringLiteral, &node->mToken)
            || this->Accept(TokenType::Identifier, &node->mToken);
       
        //Talk to trevor about
        if (!fIsValue)
        {
          unique_ptr<ExpressionNode> expr = GroupedExpression();
          RETURN_NODE(rule, expr)
        }
        
        RETURN_NODE(rule, node);
    }

    unique_ptr<MemberAccessNode> MemberAccess()
    {
	    PrintRule rule("MemberAccess");
        unique_ptr<MemberAccessNode> node = make_unique<MemberAccessNode>();
        if (!(this->Accept(TokenType::Dot, &node->mOperator) || this->Accept(TokenType::Arrow, &node->mOperator)))
          node = nullptr;
        else
          this->Expect(TokenType::Identifier, &node->mName);

        RETURN_NODE(rule, node)
    }

    unique_ptr<CallNode> Call()  
    {
	    PrintRule rule("Call");
        unique_ptr<CallNode> node = nullptr;
        if (this->Accept(TokenType::OpenParentheses))
        {
          node = make_unique<CallNode>();

          bool fComma = false;
	      do
		  {
            if (node->mArguments.push_back(Expression()) == false && fComma)
              throw ParsingException("Missing a parameter in function.");

            fComma = this->Accept(TokenType::Comma);
          } while (fComma);

          this->Expect(TokenType::CloseParentheses);
        }
        RETURN_NODE(rule, node)
    }

    unique_ptr<TypeNode> Type()
    {
      PrintRule rule("Type");
      unique_ptr<TypeNode> node = make_unique<TypeNode>();
      if (this->Accept(TokenType::Identifier, &node->mName))
      {
        node->mPointerCount = 0;
        while (this->Accept(TokenType::Asterisk))
        {
          node->mPointerCount++;
        }
      }
      else
        node = nullptr;

      RETURN_NODE(rule, node)
    }

    unique_ptr<CastNode> Cast()
    {
	    PrintRule rule("Cast");
        unique_ptr<CastNode> node = nullptr;
        if (this->Accept(TokenType::As))
        {
          node = make_unique<CastNode>();
          node->mType = Type();
          if (node->mType == nullptr)
            throw ParsingException();
        }
        RETURN_NODE(rule, node)
    }

    unique_ptr<IndexNode> Index()
    {
	    PrintRule rule("Index");
        unique_ptr<IndexNode> node = nullptr;
        if (this->Accept(TokenType::OpenBracket))
        {
          node = make_unique<IndexNode>();
          node->mIndex = Expression();
          if (node->mIndex == nullptr)
            throw ParsingException();

          this->Expect(TokenType::CloseBracket);
        }

        RETURN_NODE(rule, node)
    }
};

#pragma region Visitor
// All of our node types inherit from this node and implement the Walk function
enum VisitResult
{
  Continue,
  Stop
};

#define PRINT_NODE(outputStr)                            \
NodePrinter printer;\
printer << outputStr; 

class Visitor
{
public:
  virtual VisitResult Visit(AbstractNode*   node)     { return Continue;                               }
  virtual VisitResult Visit(BlockNode* node)          { return this->Visit((AbstractNode*)node);       }
  virtual VisitResult Visit(StatementNode*  node)     { return this->Visit((AbstractNode*)node);       }
  virtual VisitResult Visit(ExpressionNode* node)     { return this->Visit((StatementNode*)node);      }
  //virtual VisitResult Visit(LiteralNode*    node)   { return this->Visit((ExpressionNode*)node);     }
  virtual VisitResult Visit(UnaryOperatorNode* node)  { return this->Visit((ExpressionNode*)node);     }
  virtual VisitResult Visit(BinaryOperatorNode* node) { return this->Visit((ExpressionNode*)node);     }
  virtual VisitResult Visit(PostExpressionNode* node) { return this->Visit((ExpressionNode*)node);     }
  virtual VisitResult Visit(MemberAccessNode*   node) { return this->Visit((PostExpressionNode*)node); }
  virtual VisitResult Visit(ClassNode*      node)     { return this->Visit((AbstractNode*)node);       }
  //virtual VisitResult Visit(MemberNode*     node)   { return this->Visit((AbstractNode*)node);       }
  virtual VisitResult Visit(VariableNode*   node)     { return this->Visit((StatementNode*)node);      }
  virtual VisitResult Visit(TypeNode*       node)     { return this->Visit((AbstractNode*)node);       }
  //virtual VisitResult Visit(NamedReference* node)   { return this->Visit((ExpressionNode*)node);     }
  virtual VisitResult Visit(ValueNode* node)          { return this->Visit((ExpressionNode*)node);     }
  virtual VisitResult Visit(FunctionNode*   node)     { return this->Visit((AbstractNode*)node);       }
  virtual VisitResult Visit(ParameterNode*  node)     { return this->Visit((AbstractNode*)node);       }
  virtual VisitResult Visit(LabelNode* node)          { return this->Visit((StatementNode*)node);      }
  virtual VisitResult Visit(GotoNode* node)           { return this->Visit((StatementNode*)node);      }
  virtual VisitResult Visit(CallNode* node)           { return this->Visit((PostExpressionNode*)node); }
  virtual VisitResult Visit(CastNode* node)           { return this->Visit((PostExpressionNode*)node); }
  virtual VisitResult Visit(IndexNode* node)          { return this->Visit((PostExpressionNode*)node); }
  virtual VisitResult Visit(ForNode* node)            { return this->Visit((StatementNode*)node);      }
  virtual VisitResult Visit(WhileNode* node)          { return this->Visit((StatementNode*)node);      }
  virtual VisitResult Visit(ScopeNode* node)          { return this->Visit((StatementNode*)node);      }
  virtual VisitResult Visit(IfNode* node)             { return this->Visit((StatementNode*)node);      }
  virtual VisitResult Visit(BreakNode* node)          { return this->Visit((StatementNode*)node);      }
  virtual VisitResult Visit(ContinueNode* node)       { return this->Visit((StatementNode*)node);      }
  virtual VisitResult Visit(ReturnNode* node)         { return this->Visit((StatementNode*)node);      }
};



class PrintVisitor : public Visitor
{
public:
  VisitResult Visit(AbstractNode* node) override;

  VisitResult Visit(BlockNode* node) override;

  VisitResult Visit(ClassNode* node) override;

  VisitResult Visit(CallNode* node) override;
  
  VisitResult Visit(CastNode* node) override;

  VisitResult Visit(IfNode* node) override;

  VisitResult Visit(IndexNode* node) override;

  VisitResult Visit(VariableNode* node) override;

  VisitResult Visit(WhileNode* node) override;

  VisitResult Visit(ForNode* node) override;
  
  VisitResult Visit(ParameterNode* node) override;

  VisitResult Visit(FunctionNode* node) override;

  VisitResult Visit(ScopeNode* node) override;

  VisitResult Visit(TypeNode* node) override;

  VisitResult Visit(LabelNode* node) override;

  VisitResult Visit(GotoNode* node) override;

  VisitResult Visit(ValueNode* node) override;

  VisitResult Visit(BinaryOperatorNode* node) override;

  VisitResult Visit(UnaryOperatorNode* node) override;

  VisitResult Visit(MemberAccessNode* node) override;

  VisitResult Visit(BreakNode* node) override;

  VisitResult Visit(ContinueNode* node) override;

  VisitResult Visit(ReturnNode* node) override;
  /*
  VisitResult Visit(PostExpressionNode* node) override;
  VisitResult Visit(CallNode* node) override;

  VisitResult Visit(CastNode* node) override;

  VisitResult Visit(IndexNode* node) override;
  */
};

VisitResult PrintVisitor::Visit(AbstractNode* node)
{
  return Continue;
}

VisitResult PrintVisitor::Visit(BlockNode* node)
{
  PRINT_NODE("BlockNode")
  for (unsigned i = 0; i < node->mGlobals.size(); ++i)
  {
    node->mGlobals[i]->Walk(this);
  }

  return Stop;
}
VisitResult PrintVisitor::Visit(ClassNode* node)
{
  PRINT_NODE("ClassNode(" << node->mName << ")")

  for (unsigned i = 0; i < node->mMembers.size(); ++i)
  {
    node->mMembers[i]->Walk(this);
  }

  return Stop;
}

VisitResult PrintVisitor::Visit(CallNode* node)
{
  PRINT_NODE("CallNode")
  node->mLeft->Walk(this);

  for (unsigned i = 0; i < node->mArguments.size(); ++i)
  {
    node->mArguments[i]->Walk(this);
  }

  return Stop;
}

VisitResult PrintVisitor::Visit(CastNode* node)
{
  PRINT_NODE("CastNode")
  
  node->mLeft->Walk(this);
  node->mType->Walk(this);
  return Stop;
}

VisitResult PrintVisitor::Visit(IfNode* node)
{
  PRINT_NODE("IfNode")
  if (node->mCondition)
    node->mCondition->Walk(this);

  node->mScope->Walk(this);

  if (node->mElse)
    node->mElse->Walk(this);

  return Stop;
}

VisitResult PrintVisitor::Visit(IndexNode* node)
{
  PRINT_NODE("IndexNode")

  node->mLeft->Walk(this);
  node->mIndex->Walk(this);
  return Stop;
}

VisitResult PrintVisitor::Visit(VariableNode* node)
{
  PRINT_NODE("VariableNode(" << node->mName << ")")
  node->mType->Walk(this);
  node->mInitialValue->Walk(this);
  return Stop;
}

VisitResult PrintVisitor::Visit(WhileNode* node)
{
  PRINT_NODE("WhileNode")
  node->mCondition->Walk(this);
  node->mScope->Walk(this);
  return Stop;
}

VisitResult PrintVisitor::Visit(ForNode* node)
{
  PRINT_NODE("ForNode")
  if(node->mInitialVariable)
    node->mInitialVariable->Walk(this);

  if(node->mInitialExpression)
    node->mInitialExpression->Walk(this);
  
  if(node->mCondition)
    node->mCondition->Walk(this);

  node->mScope->Walk(this);

  if(node->mIterator)
    node->mIterator->Walk(this);

  return Stop;
}

VisitResult PrintVisitor::Visit(ParameterNode* node)
{
  PRINT_NODE("ParameterNode(" << node->mName << ")")

  node->mType->Walk(this);

  return Stop;
}

VisitResult PrintVisitor::Visit(FunctionNode* node)
{
  PRINT_NODE("FunctionNode(" << node->mName << ")")
  
  for (unsigned i = 0; i < node->mParameters.size(); ++i)
  {
    node->mParameters[i]->Walk(this);
  }
  
  if(node->mReturnType)
    node->mReturnType->Walk(this);

  node->mScope->Walk(this);

  return Stop;
}
VisitResult PrintVisitor::Visit(ScopeNode* node)
{
  PRINT_NODE("ScopeNode")
  for (unsigned i = 0; i < node->mStatements.size(); ++i)
  {
    node->mStatements[i]->Walk(this);
  }
  return Stop;
}

VisitResult PrintVisitor::Visit(TypeNode* node)
{
  PRINT_NODE("TypeNode(" << node->mName << "," << node->mPointerCount << ")")

  return Stop;
}

VisitResult PrintVisitor::Visit(LabelNode* node)
{
  PRINT_NODE("LabelNode(" << node->mName << ")")

  return Stop;
}

VisitResult PrintVisitor::Visit(GotoNode* node)
{
  PRINT_NODE("GotoNode(" << node->mName << ")")

  return Stop;
}

VisitResult PrintVisitor::Visit(ValueNode* node)
{
  PRINT_NODE("ValueNode(" << node->mToken << ")")

  return Stop;
}

VisitResult PrintVisitor::Visit(BinaryOperatorNode* node)
{
  PRINT_NODE("BinaryOperatorNode(" << node->mOperator << ")")
  node->mRight->Walk(this);
  node->mLeft->Walk(this);

  return Stop;
}

VisitResult PrintVisitor::Visit(UnaryOperatorNode* node)
{
  PRINT_NODE("UnaryOperatorNode(" << node->mOperator << ")")

  node->mRight->Walk(this);

  return Stop;
}
/*
VisitResult PrintVisitor::Visit(PostExpressionNode* node)
{
NodePrinter printer;
node->mLeft->Walk(this);
PRINT_END
}
*/
VisitResult PrintVisitor::Visit(MemberAccessNode* node)
{
  //TODO print correctly
  PRINT_NODE( "MemberAccessNode(" << node->mOperator << "," << node->mName << ")")
  node->mLeft->Walk(this);
  return Stop;
}

VisitResult PrintVisitor::Visit(BreakNode* node)
{
  PRINT_NODE("BreakNode")
  return Stop;
}

VisitResult PrintVisitor::Visit(ContinueNode* node)
{
  PRINT_NODE("ContinueNode")
  return Stop;
}

VisitResult PrintVisitor::Visit(ReturnNode* node)
{
  PRINT_NODE("ReturnNode")
  if (node->mReturnValue)
    node->mReturnValue->Walk(this);

  return Stop;
}
#pragma region walk

#define WALK_INIT if (visit && visitor->Visit(this) == VisitResult::Stop)\
                    return;

void BlockNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

    AbstractNode::Walk(visitor, false);

  for (unsigned i = 0; i < mGlobals.size(); ++i)
  {
    mGlobals[i]->Walk(visitor);
  }
}

void ClassNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  AbstractNode::Walk(visitor, false);

  for (unsigned i = 0; i < mMembers.size(); ++i)
  {
    mMembers[i]->Walk(visitor);
  }
}

void StatementNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT
}

void TypeNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT
}

void VariableNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  StatementNode::Walk(visitor, false);

  mType->Walk(visitor);

  if (mInitialValue)
    mInitialValue->Walk(visitor);
}

void ScopeNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  StatementNode::Walk(visitor, false);

  for (unsigned i = 0; i < mStatements.size(); ++i)
  {
    mStatements[i]->Walk(visitor);
  }
}

void ParameterNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  AbstractNode::Walk(visitor, false);
  
  mType->Walk(visitor);
}

void FunctionNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  AbstractNode::Walk(visitor, false);

  for (unsigned i = 0; i < mParameters.size(); ++i)
  {
    mParameters[i]->Walk(visitor);
  }

  mReturnType->Walk(visitor);
  mScope->Walk(visitor);
}

void LabelNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  StatementNode::Walk(visitor, false);
}

void GotoNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  StatementNode::Walk(visitor, false);
}

void ReturnNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  StatementNode::Walk(visitor, false);

  if (mReturnValue)
    mReturnValue->Walk(visitor);
}

void BreakNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  StatementNode::Walk(visitor, false);
}

void ContinueNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  StatementNode::Walk(visitor, false);
}

void ExpressionNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT
    
  StatementNode::Walk(visitor, false);
}

void IfNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  StatementNode::Walk(visitor, false);

  if (mCondition)
    mCondition->Walk(visitor);

  mScope->Walk(visitor);

  if (mElse)
    mElse->Walk(visitor);
}

void WhileNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

    StatementNode::Walk(visitor, false);

  if (mCondition)
    mCondition->Walk(visitor);

  mScope->Walk(visitor);
}

void ForNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  StatementNode::Walk(visitor, false);
  
  if (mInitialVariable)
    mInitialVariable->Walk(visitor);

  if (mInitialExpression)
    mInitialExpression->Walk(visitor);

  if (mCondition)
    mCondition->Walk(visitor);

  if (mIterator)
    mIterator->Walk(visitor);

  mScope->Walk(visitor);
}

void ValueNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  ExpressionNode::Walk(visitor, false);
}

void BinaryOperatorNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  ExpressionNode::Walk(visitor, false);

  if (mLeft)
    mLeft->Walk(visitor);

  if (mRight)
    mRight->Walk(visitor);
}

void UnaryOperatorNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  ExpressionNode::Walk(visitor, false);

  if (mRight)
    mRight->Walk(visitor);
}

void PostExpressionNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  ExpressionNode::Walk(visitor, false);

  if (mLeft)
    mLeft->Walk(visitor);
}

void MemberAccessNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  PostExpressionNode::Walk(visitor, false);
}

void CallNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  PostExpressionNode::Walk(visitor, false);

  for (unsigned i = 0; i < mArguments.size(); ++i)
  {
    mArguments[i]->Walk(visitor);
  }
}

void CastNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

  PostExpressionNode::Walk(visitor, false);

  mType->Walk(visitor);
}

void IndexNode::Walk(Visitor* visitor, bool visit)
{
  WALK_INIT

    PostExpressionNode::Walk(visitor, false);

  if (mIndex != nullptr)
    mIndex->Walk(visitor);
}

void PrintTree(AbstractNode* node)
{
  PrintVisitor printer;
  node->Walk(&printer);
}

unique_ptr<ExpressionNode> ParseExpression(std::vector<Token>& tokens)
{
  Parser myParser(tokens);

  return myParser.Expression();
}

unique_ptr<BlockNode> ParseBlock(std::vector<Token>& tokens)
{
  Parser myParser(tokens);
  
  return myParser.Block();
}

void RemoveWhitespaceAndComments( std::vector<Token>& tokens)
{
	TokenType::Enum type;
	unsigned i = 0;
	while(i < tokens.size())
	{
		type = static_cast<TokenType::Enum>(tokens[i].mTokenType);
		if (type == TokenType::Whitespace
			|| type == TokenType::SingleLineComment
			|| type == TokenType::MultiLineComment)
		{
			tokens.erase(tokens.begin() + i);
		}
		else
		{
			++i;
		}
	}
}

void Recognize(std::vector<Token>& tokens)
{
	bool fSuccess = false;
	Parser parser(tokens);
}
