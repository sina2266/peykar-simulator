#ifndef MTExpression_h
#define MTExpression_h
#include <typeinfo>
#include <map>
#include <Expression.h>
namespace VCell {
	struct MapTable;

	/**
	* map backed expression 
	*/
	struct MTExpression {
		/**
		* @param exp string representation of function
		* @param m must remain in memory 
		*/
		MTExpression(const std::string & exp, MapTable &m);

		/**
		* @param exp string representation of function
		* @param m must remain in memory 
		*/
		MTExpression(const char * const exp, MapTable &);

		/**
		* @return new generated string representation
		*/
		std::string infix( ) const {
			return expression.infix( );
		}

		/**
		* is expression constant?
		* @return true if is
		*/
		bool isConstant( ) const {
			return expression.isConstant( );
		}

		/**
		* return constant value
		* @throws VCell::ExpressionException if #isConstant( ) != true
		*/ 
		double constantValue( ) const {
			return expression.evaluateConstant( );
		}

		double evaluate( ) const;

		MapTable & ourTable( ) const {
			return mapTable;
		}


	private:
		friend struct MapTable;
		void bind(const MapTable &mt);
		mutable VCell::Expression expression;
		MapTable &mapTable;

	};


}
#endif
