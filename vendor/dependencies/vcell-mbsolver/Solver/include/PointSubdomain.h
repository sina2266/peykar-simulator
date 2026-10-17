#ifndef PointSubdomain_h
#define PointSubdomain_h
#include <Subdomain.h>
#include <SExpression.h>
#include <MovingBoundaryTypes.h>
#include <vector>
using std::vector;

namespace moving_boundary
{
	struct PointSubdomain : public Subdomain
	{
	public:
		PointSubdomain(const string & name, Physiology* physiology, const string& posX, const string& posY);
		virtual ~PointSubdomain();

		SubdomainType getType()
		{
			return subdomain_point;
		}

		void bindExpressions(SymbolTable* symbolTable);
		double* getPositionValues()
		{
			return positionValues;
		}
		void updatePosition(double* values);

	protected:
		string name_;
		SExpression* positions[DIM];
		double positionValues[DIM];
	};
}

#endif
