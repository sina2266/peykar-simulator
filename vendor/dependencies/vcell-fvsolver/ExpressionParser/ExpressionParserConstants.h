#ifndef EXPRESSIONPARSERCONSTANTS_H
#define EXPRESSIONPARSERCONSTANTS_H

const int EEOF = 0;
const int RELATIONAL_OPERATOR = 5;
const int LT = 6;
const int GT = 7;
const int LE = 8;
const int GE = 9;
const int EQ = 10;
const int NE = 11;
const int AND = 12;
const int OR = 13;
const int NOT = 14;
const int POWER = 15;
const int ADD = 16;
const int SUB = 17;
const int MULT = 18;
const int DIV = 19;
const int FLOATING_POINT_LITERAL = 20;
const int EXPONENT = 21;
const int INTEGER_LITERAL = 22;
const int IDENTIFIER = 23;
const int ID = 24;
const int LETTER = 25;
const int DIGIT = 26;

const int DEFAULT = 0;

const string tokenImage[] = {
    "<EOF>",
    "\" \"",
    "\"\\t\"",
    "\"\\n\"",
    "\"\\r\"",
    "<RELATIONAL_OPERATOR>",
    "\"<\"",
    "\">\"",
    "\"<=\"",
    "\">=\"",
    "\"==\"",
    "\"!=\"",
    "<AND>",
    "<OR>",
    "<NOT>",
    "\"^\"",
    "\"+\"",
    "\"-\"",
    "\"*\"",
    "\"/\"",
    "<FLOATING_POINT_LITERAL>",
    "<EXPONENT>",
    "<INTEGER_LITERAL>",
    "<IDENTIFIER>",
    "<ID>",
    "<LETTER>",
    "<DIGIT>",
    "\";\"",
    "\"(\"",
    "\")\"",
    "\",\"",
};

#endif
