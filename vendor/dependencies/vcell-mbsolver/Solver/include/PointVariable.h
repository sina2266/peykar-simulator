#ifndef PointVariable_h
#define PointVariable_h
#include <string>
#include <Variable.h>
#include <SExpression.h>
using std::string;

namespace moving_boundary
{
	struct PointVariable : public Variable {

		PointVariable(const string & name);
		~PointVariable();

		VariableType getType()
		{
			return vartype_point;
		}
	};
}

#endif
