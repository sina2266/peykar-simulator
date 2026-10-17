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
	~JumpCondition();

	VCell::Expression *getExpression() const { return expression; }
	Membrane* getMembrane() const { return membrane; }

	void bindExpression(SymbolTable*);
	double evaluateExpression(double* values);
	double evaluateExpression(SimulationExpression*, MembraneElement*);
	void reinitConstantValues(SimulationExpression* sim);

private:
	Membrane* membrane;
	VCell::Expression *expression;
	double* constantValue;
	bool bNeedsXYZ;

	bool isConstantExpression(SimulationExpression *sim);
};

#endif
