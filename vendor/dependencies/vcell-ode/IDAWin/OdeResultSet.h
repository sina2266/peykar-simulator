#ifndef ODERESULTSET_H
#define ODERESULTSET_H

#include <string>
#include <utility>
#include <vector>

namespace VCell {
	class Expression;
}

class SymbolTable;

struct Column {
	std::string name;
	VCell::Expression *expression;

	Column(std::string arg_name, VCell::Expression *exp) {
		this->name = std::move(arg_name);
		this->expression = exp;
	}
};

class OdeResultSet {
	public:
		OdeResultSet();

		~OdeResultSet();

		// Column Methods
		[[nodiscard]] int getNumColumns() const;

		std::vector<Column> getColumns() { return this->columns; }

		void addColumn(const std::string &aColumn);

		[[nodiscard]] int findColumn(const std::string &aColumn) const;

		std::string &getColumnName(int index);

		double getColumnWeight(int index);

		void setColumnWeights(const double *weights);

		void getColumnData(int index, int numParams, const double *paramValues, double *colData) const;

		// Function Methods
		void addFunctionColumn(const std::string &aColumn, const std::string &columnExpression);

		[[nodiscard]] int getNumFunctionColumns() const { return this->numFunctionColumns; }
		[[nodiscard]] int getNumDataColumns() const { return this->numDataColumns; }

		[[nodiscard]] VCell::Expression *getColumnFunctionExpression(int columnIndex) const;

		void bindFunctionExpression(SymbolTable *symbolTable) const;

		// Row Methods
		[[nodiscard]] int getNumRows() const;

		void addRow(const double *aRow);

		double *getRowData(int index);

		[[nodiscard]] double *getAllRowData() const { return this->rowData; }

		void addEmptyRows(int numRowsToAdd);

		// Misc.
		void copyInto(OdeResultSet *otherOdeResultSet) const;

		void clearData();

	private:
		// 0 : t
		// 1 ~ N : variable names;
		std::vector<Column> columns;
		double *columnWeights;
		double *rowData;
		int numRowsAllocated;
		int numRowsUsed;
		int numFunctionColumns;
		int numDataColumns;
};

#endif
