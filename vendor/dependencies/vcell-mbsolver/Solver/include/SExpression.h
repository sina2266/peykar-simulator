#ifndef SExpression_h
#define SExpression_h
#include <typeinfo>
#include <Expression.h>
#include <SimpleSymbolTable.h>
using std::string;

namespace moving_boundary {

	struct SExpression {
		SExpression();

		/**
		* @param exp string representation of function
		*/
		SExpression(const string& exp);

		/**
		* @param exp string representation of function
		* @param symTable symbol table -- must remain valid during
		* lifetime of object
		*/
		SExpression(const string& exp, const SymbolTable* symTable);

		/**
		* @param symTable symbol table -- must remain valid during
		* lifetime of object
		*/
		void bindExpression(const SymbolTable* symbolTable);
		/**
		* @return new generated string representation
		*/
		string infix( ) const {
			return expression.infix( );
		}

		/**
		* @tparam CTR std::vector, std::array, et. al.
		*/
		template <class CTR>
		double evaluate(const CTR  &values) const {
			return evaluate(const_cast<double *>(values.data( )));
		}

		double evaluate(double* values) const;
		/**
		* is expression constant?
		* @return true if is
		*/
		bool isConstant( ) const {
			return constValue != nullptr;
		}

		/**
		* return constant value
		* @throws VCell::ExpressionException if #isConstant( ) != true
		*/ 
		double constantValue( ) const;
		bool isConcentrationDependent()
		{
			return bConcentrationDependent;
		}

	private:
		mutable VCell::Expression expression;
		double* constValue;
		void tryConstant();
		bool bConcentrationDependent;
	};

}
#endif
