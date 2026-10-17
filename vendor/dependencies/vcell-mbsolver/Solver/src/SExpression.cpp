#include <SExpression.h>
#include <Physiology.h>
#include <VCellException.h>
#include <vector>
#include <algorithm>
using moving_boundary::SExpression;
using moving_boundary::Physiology;

SExpression::SExpression()
	:constValue(nullptr),
	 bConcentrationDependent(false)
{
}

SExpression::SExpression(const string& exp)
	:expression(exp),
	 constValue(nullptr),
	 bConcentrationDependent(false)
{
	tryConstant();
}

SExpression::SExpression(const string& exp, const SymbolTable *symTable)
	:expression(exp),
	 constValue(nullptr),
	 bConcentrationDependent(false)
{
	tryConstant();
	bindExpression(symTable);
}

void SExpression::bindExpression(const SymbolTable *symbolTable) {
	if (constValue == nullptr)
	{
		expression.bindExpression(const_cast<SymbolTable *>(symbolTable));
		vector<std::string> symbols;
		expression.getSymbols(symbols);
		bConcentrationDependent = false;
		for (vector<std::string>::iterator iter = symbols.begin(); iter != symbols.end(); ++ iter)
		{
			std::string& s = *iter;
			auto findIter = std::find(Physiology::fixedTimeSpatialSymbols.begin(), Physiology::fixedTimeSpatialSymbols.end(), s);
			if (findIter == std::end(Physiology::fixedTimeSpatialSymbols))
			{
				bConcentrationDependent = true;
				break;
			}
		}
	}
}

double SExpression::evaluate(double* values) const
{
	if (constValue != nullptr)
	{
		return *constValue;
	}
	return expression.evaluateVector(values);
}

void SExpression::tryConstant()
{
	if (expression.isConstant())
	{
		constValue = new double;
		*constValue = expression.evaluateConstant();
		bConcentrationDependent = false;
	}
}

double SExpression::constantValue( ) const
{
	if (constValue == nullptr)
	{
		std::stringstream ss;
		ss << "Expression " << infix() << " is not a constant." << std::endl;
		throw ss.str();
	}
	return *constValue;
}



	
