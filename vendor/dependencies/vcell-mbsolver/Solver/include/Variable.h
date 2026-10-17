#ifndef Variable_h
#define Variable_h
#include <string>
#include <SExpression.h>
using std::string;

namespace moving_boundary
{
	struct Physiology;
	enum VariableType
	{
		vartype_unknown=0,
		vartype_volume=1,
		vartype_point=10,
	};
	enum ExpressionIndex
	{
		expr_initial=0,
		expr_source,
		expr_diffusion,
		expr_advection_x,
		expr_advection_y,
		expr_size,
	};

	const char ExpressionDescription[][50] = {"Initial", "Rate", "Diffusion", "VelocityX", "VelocityY"};

	class Variable {
public:
		Variable(const string & name);
		Variable(const string & name, int size);
		virtual ~Variable();

		const string & name( ) const {
			return name_;
		}

		void setExpression(ExpressionIndex exprIndex, const string& expr);
//		const SExpression* getExpression(ExpressionIndex exprIndex ) const
//		{
//			return expressions[exprIndex];
//		}

		bool isAdvecting() const
		{
			return bAdvecting;
		}
		virtual VariableType getType()=0;
		double evaluateExpression(ExpressionIndex exprIndex, double* inputValues) const;
		bool isExpressionConstant(ExpressionIndex exprIndex) const;
		bool isExpressionNonZero(ExpressionIndex exprIndex) const;
		double getExpressionConstantValue(ExpressionIndex exprIndex) const;
		void setCurrSol(int index, double v);
		double* getCurrSol();
		double getCurrSol(int index);

	protected:
		friend Physiology;
		bool bAdvecting;
		void bindExpressions(const SimpleSymbolTable* symTable);
		string name_;
		SExpression** expressions;
		int size;
		double* currSol;

	private:
		void initExpressions();
		void forbidNullExpression(ExpressionIndex exprIndex) const;
	};
}

#endif
