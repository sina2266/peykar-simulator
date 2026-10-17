#ifndef JUMPCONDITION
#define JUMPCONDITION

class Membrane;
namespace VCell {
	class Expression;
}
class SymbolTable;
struct MembraneElement;
class SimulationExpression;

class JumpCondition
{
public:
	JumpCondition(Membrane*, VCell::Expression*);
	~JumpCondition(void);

	VCell::Expression *getExpression() {
		return expression;
	}
	Membrane* getMembrane() {
		return membrane;
	}

	void bindExpression(SymbolTable*);
	double evaluateExpression(double* values);

private:
	Membrane* membrane;
	VCell::Expression *expression;
	double* constantValue;
	bool bNeedsXYZ;

	bool isConstantExpression();
};

#endif
