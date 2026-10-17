#include <cstring>
#include <stdexcept>
#include "OdeResultSet.h"
#include "Exception.h"
#include "Expression.h"

OdeResultSet::OdeResultSet(): columnWeights(nullptr), rowData(nullptr), numRowsAllocated(0), numRowsUsed(0), numFunctionColumns(0), numDataColumns(0)
{}

OdeResultSet::~OdeResultSet(){
	for (const Column& column : this->columns) delete column.expression;
	this->columns.clear();
	delete[] rowData;
	delete[] columnWeights;
}

void OdeResultSet::addColumn(const std::string& aColumn) {
	if (0 != this->numRowsAllocated) throw VCell::Exception("Can't add column when rowData is not empty");
	this->columns.emplace_back(aColumn, nullptr);
	this->numDataColumns++;
}

void OdeResultSet::bindFunctionExpression(SymbolTable* symbolTable) const {
	for (const Column& column : this->columns) {
		if (column.expression == nullptr) continue;
		column.expression->bindExpression(symbolTable);
	}
}

void OdeResultSet::addFunctionColumn(const std::string& aColumn, const std::string& columnExpression) {
	this->columns.push_back(Column(aColumn, new VCell::Expression(columnExpression)));
	this->numFunctionColumns ++;
}

void OdeResultSet::addRow(const double* aRow) {
	if (this->numRowsAllocated == 0) {
		this->numRowsAllocated = 500;
		this->rowData = new double[this->numRowsAllocated * this->numDataColumns];
		memset(this->rowData, 0, this->numRowsAllocated * this->numDataColumns * sizeof(double));
	} else if (this->numRowsAllocated == this->numRowsUsed) {
		const int oldNumRowsAllocated = this->numRowsAllocated;
		const double* oldRowData = this->rowData;
		this->numRowsAllocated += 500;
		this->rowData = new double[this->numRowsAllocated * this->numDataColumns];
		memset(this->rowData, 0, this->numRowsAllocated * this->numDataColumns * sizeof(double));
		memcpy(this->rowData, oldRowData, oldNumRowsAllocated * this->numDataColumns * sizeof(double));
		delete[] oldRowData;
	}
	int index = this->numRowsUsed * this->numDataColumns;
	for (int i = 0; i < this->numDataColumns; i ++, index ++) {
		this->rowData[index] = aRow[i];
	}
	this->numRowsUsed++;
}

void OdeResultSet::setColumnWeights(const double* weights){
	delete[] this->columnWeights;
	this->columnWeights = new double[this->columns.size()];
	memcpy(this->columnWeights, weights, this->columns.size() * sizeof(double));
}

double* OdeResultSet::getRowData(const int index) {
	if (index >= this->numRowsUsed) {
		throw VCell::Exception("OdeResultSet::getRowData(int index), row index is out of bounds");
	}
	return this->rowData + index * this->numDataColumns;
}

void OdeResultSet::clearData() {
	this->numRowsUsed = 0;
	memset(this->rowData, 0, this->numRowsAllocated * this->numDataColumns * sizeof(double));
}

int OdeResultSet::findColumn(const std::string& aColumn) const {
	int columnIndex = 0;
	for (const Column& column : this->columns) {
		if (column.name == aColumn) break;
		columnIndex++;
	}
	if (columnIndex == this->columns.size()) columnIndex = -1;
	return columnIndex;
}

double OdeResultSet::getColumnWeight(int index) {
	if (index >= (int)this->columns.size()) {
		throw std::out_of_range("OdeResultSet::getColumnWeight(int index), column index is out of bounds");
	}
	return columnWeights[index];
}

int OdeResultSet::getNumColumns() const {
	return static_cast<int>(this->columns.size());
}

std::string& OdeResultSet::getColumnName(const int index) {
	if (index >= static_cast<int>(this->columns.size())) {
		throw std::out_of_range("OdeResultSet::getColumnName(int index), column index is out of bounds");
	}
	return this->columns[index].name;
}

int OdeResultSet::getNumRows() const {
	return this->numRowsUsed;
}

VCell::Expression* OdeResultSet::getColumnFunctionExpression(const int columnIndex) const {
	if (columnIndex >= static_cast<int>(this->columns.size())) {
		throw std::out_of_range("OdeResultSet::getColumnFunctionExpression(), column index is out of bounds");
	}
	return this->columns[columnIndex].expression;
}

void OdeResultSet::getColumnData(const int index, const int numParams, const double* paramValues, double* colData) const {
	if (index < 0 || index >= this->getNumColumns()){
		throw std::out_of_range("OdeResultSet::getColumnData(int columnIndex), columnIndex out of bounds");
	}
	if (this->columns[index].expression == nullptr) {
		for (int i = 0; i < this->numRowsUsed; i++){
			colData[i] = this->rowData[i * this->numDataColumns + index];
		}
	} else { // Function
		auto* values = new double[this->numDataColumns + numParams];
		memcpy(values + this->numDataColumns, paramValues, numParams * sizeof(double));
		for (int i = 0; i < this->numRowsUsed; i++){
			memcpy(values, this->rowData + i * this->numDataColumns, this->numDataColumns * sizeof(double));
			colData[i] = this->columns[index].expression->evaluateVector(values);
		}
		delete[] values;
	}
}

void OdeResultSet::copyInto(OdeResultSet* otherOdeResultSet) const {
	// columns
	if (otherOdeResultSet->columns.size() != this->columns.size()) {
		for (const Column& column : otherOdeResultSet->columns) {
			delete column.expression;
		}
		otherOdeResultSet->columns.clear();
		for (const Column& column : this->columns) {
			if (nullptr == column.expression) {
				otherOdeResultSet->addColumn(column.name);
			} else {
				otherOdeResultSet->addFunctionColumn(column.name, column.expression->infix());
			}
		}
		if (nullptr != this->columnWeights) {
			otherOdeResultSet->setColumnWeights(this->columnWeights);
		}
	}
	// rows
	if (otherOdeResultSet->numRowsAllocated != this->numRowsAllocated) {
		delete[] otherOdeResultSet->rowData;
		otherOdeResultSet->rowData = new double[this->numRowsAllocated * this->numDataColumns];
		otherOdeResultSet->numRowsAllocated = this->numRowsAllocated;
	}
	otherOdeResultSet->numRowsUsed = this->numRowsUsed;
	memcpy(otherOdeResultSet->rowData, this->rowData, this->numRowsAllocated * this->numDataColumns * sizeof(double));
}

void OdeResultSet::addEmptyRows(int numRowsToAdd) {
	if (this->numRowsAllocated < this->numRowsUsed + numRowsToAdd) {
		int oldNumRowsAllocated = this->numRowsAllocated;
		double* oldRowData = this->rowData;
		this->numRowsAllocated = this->numRowsUsed + numRowsToAdd;
		this->rowData = new double[this->numRowsAllocated * this->numDataColumns];
		memset(this->rowData, 0, this->numRowsAllocated * this->numDataColumns * sizeof(double));
		memcpy(this->rowData, oldRowData, oldNumRowsAllocated *  this->numDataColumns * sizeof(double));
		delete[] oldRowData;
	} 
	this->numRowsUsed += numRowsToAdd;
}
