#ifndef SIMPLECHARSTREAM_H
#define SIMPLECHARSTREAM_H

#include <iostream>
#include <string>
using std::istream;
using std::string;

class SimpleCharStream
{
protected:
	int bufsize;
	int available;
	int tokenBegin;

private:
	int* bufline;
	int* bufcolumn;
	int column;
	int line;
	bool prevCharIsCR;
	bool prevCharIsLF;
	istream* inputStream;
	char* buffer;
	int maxNextCharInd;
	int inBuf;
	int tabSize;
	void ExpandBuff(bool wrapAround);
	void FillBuff(void);
	void UpdateLineColumn(char c);
	void init(istream* dstream, int startline,  int startcolumn, int buffersize);

public:
	SimpleCharStream(istream* dstream, int startline,  int startcolumn, int buffersize);
	SimpleCharStream(istream* dstream, int startline,  int startcolumn);
	SimpleCharStream(istream* dstream);
	~SimpleCharStream(void);

	static bool staticFlag;
	int bufpos;

	char BeginToken(void);
	char readChar(void);
	int getColumn(void);
	int getLine(void);
	int getEndColumn(void);
	int getEndLine(void);
	int getBeginColumn(void);
	int getBeginLine(void);
	void backup(int amount);
	string GetImage(void);
	char* GetSuffix(int len);
	void Done(void);
	void adjustBeginLineColumn(int newLine, int newCol);
};

#endif
