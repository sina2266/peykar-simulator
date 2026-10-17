#include <string>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <typeinfo>
#include <map>
#include <limits>
#include <persist.h>
#include <VCellException.h>
#include <ManagedArrayPtr.h>
#include <vcellstring.h>
#include <Logger.h>
using std::type_info;
using namespace vcell_persist;
using namespace vcell_util;
namespace {
	const long magicWord = 15661403; //a randomly generate unsigned int (random.org)

	struct TokenMap : public std::map<const type_info *,std::string> {
		TokenMap( ) {
			//sample expansion: Registrar::reg<double>("double");

			#define REGISTER_MACRO(T) Registrar::reg<T>(#T);
			REGISTER_MACRO(char)
			REGISTER_MACRO(signed char)
			REGISTER_MACRO(unsigned char)

			REGISTER_MACRO(short) //by the C++ standard, these type are signed (i.e. short == signed short)
			REGISTER_MACRO(int)
			REGISTER_MACRO(long)

			REGISTER_MACRO(unsigned short)
			REGISTER_MACRO(unsigned int)
			REGISTER_MACRO(unsigned long)

			REGISTER_MACRO(float)
			REGISTER_MACRO(double)

			#undef REGISTER_MACRO
		}
	};

	TokenMap typeTokens;

	bool isDictionaryInUse = false;
	bool writeInProgress = false;
	//bool usingReadDictionary = false;
	typedef unsigned short TokenKeyType;
	//typedef std::map<TokenKeyType,std::string> DictionaryType;

	//DictionaryType readTokenDictionary;
	std::map<const std::string,TokenKeyType> tokenDictionary;

	typedef unsigned char TokenLengthType;
}

void TokenT<bool>::insert(std::ostream &os, const type_info & ti) {
	const std::string & token = getTypeToken(ti);
	if (!isDictionaryInUse) {
		os << token; 
	}
	else {
		const TokenKeyType tokenKey = tokenDictionary[token];
		binaryWrite(os,tokenKey);
	}
}


void TokenT<bool>::check(std::istream &is, const type_info & ti) {
	const std::string & token = getTypeToken(ti);
	if (!isDictionaryInUse) {
		const size_t ts = token.size( );
		StackPtr<char,100> b(ts);
		char * buffer = b.get( );
		is.read(buffer,ts);
		const bool match = strncmp(buffer,token.c_str( ),ts ) == 0;
		if (!match) {
			std::string tokenRead = convertNonPrintable(std::string(buffer,ts) );
			std::string expected = convertNonPrintable(token);
			VCELL_EXCEPTION(invalid_argument,"Read token " << tokenRead << ", " << expected << " expected");
		}
	}
	else {
		TokenKeyType expectedKey = tokenDictionary[token];
		TokenKeyType key; 
		binaryRead(is,key);
		if (key != expectedKey) {
			VCELL_EXCEPTION(invalid_argument,"Read token key " << key << ", mapped  to " << " tbd "  << ", expected "
				<< expectedKey << " mapped to " << token);
		}
	}
}

void Registrar::registerTypeToken(const char * token,const type_info & ti) {
		TokenMap::iterator iter = typeTokens.find(&ti);
		if (iter == typeTokens.end( )) {
			if (writeInProgress) {
				VCELL_EXCEPTION(runtime_error,"Invalid registration attempt during streaming out");
			}
			typeTokens[&ti] = token; 
			return;
		}
		if (token != typeTokens[&ti]) {
			VCELL_EXCEPTION(invalid_argument,"type " << ti.name( ) << " registration token " << token
				<< " differs from previously registered " << typeTokens[&ti]);
		}
}

const std::string & vcell_persist::getTypeToken(const type_info & ti) {
	TokenMap::iterator iter = typeTokens.find(&ti);
	if (iter != typeTokens.end( )) {
		return iter->second;
	}
	VCELL_EXCEPTION(invalid_argument,"type " << ti.name( ) << " not registered with call to registerTypeToken");
}

void Registrar::registerTypeToken(const char *classname, const type_info &clzz, const std::type_info &templateParameter, int dim) {
	std::ostringstream oss;
	oss << classname << '<' << getTypeToken(templateParameter) << ',' << dim << '>';
	registerTypeToken(oss.str( ).c_str( ),clzz);
}

void Registrar::registerTypeToken(const char *classname, const type_info &clzz, int dim) {
	std::ostringstream oss;
	oss << classname << '<' <<  dim << '>';
	registerTypeToken(oss.str( ).c_str( ),clzz);
}

void Registrar::registerTypeToken(const char *classname, const type_info &clzz, const std::type_info &templateParameterA, const std::type_info &templateParameterB, int dim) {
	std::ostringstream oss;
	oss << classname << '<' << getTypeToken(templateParameterA) << ',' << getTypeToken(templateParameterB) << ',' << dim << '>';
	registerTypeToken(oss.str( ).c_str( ),clzz);
}

WriteFormatter::WriteFormatter(std::ostream &os, unsigned short version, bool dictionary) {
	if (dictionary && isDictionaryInUse) {
			throw std::runtime_error("attempt to create second formatter object before destruction of first (on writing)");
	}
	using std::ios;
	os.exceptions(ios::badbit|ios::failbit|ios::eofbit);
	isDictionaryInUse = writeInProgress = dictionary;

	binaryWrite(os,magicWord);
	binaryWrite(os,version);
	binaryWrite(os,isDictionaryInUse);
	if (isDictionaryInUse) {
		std::vector<std::string> tokensByKey(typeTokens.size( ) + 1); //zero slot unused
		tokenDictionary.clear( );
		TokenKeyType index = 1; 
		for (TokenMap::const_iterator iter = typeTokens.begin( ); iter != typeTokens.end( ); ++iter) {
			const TokenKeyType key = index++;
			const std::string & token = iter->second;
			tokenDictionary[iter->second] = key; 
			tokensByKey[key] = token;
			if (index == 0) {
				VCELL_EXCEPTION(domain_error,"too many tokens, exceeds limit " << std::numeric_limits<TokenKeyType>::max( ) - 1 );
			}
		}
		binaryWrite(os,index);

		const size_t maxTokenLength = std::numeric_limits<TokenLengthType>::max( ); 
		for (TokenKeyType i = 1 ; i < index; i++) {
			const std::string & token = tokensByKey.at(i); 
			if (token.size( ) > maxTokenLength) {
				VCELL_EXCEPTION(out_of_range, token << " length " << token.size( ) << " > max size " << maxTokenLength);
			}
			StdString<TokenLengthType>::save(os,token);
			VCELL_LOG(info,"token " << token << " index " << i);
		}
	}
	unsigned char zero =  0;
	binaryWrite(os,zero); //add check char
}

ReadFormatter::ReadFormatter(std::istream &is, unsigned short version) {
	if (isDictionaryInUse) {
		throw std::runtime_error("attempt to create second formatter object before destruction of first (on reading)");
	}
	using std::ios;
	is.exceptions(ios::badbit|ios::failbit);
	long word; 
	binaryRead(is,word);
	if (word != magicWord) {
		VCELL_EXCEPTION(invalid_argument,"file does not begin with correct magic word, " << word << " read, " << magicWord << " expected");
	}
	unsigned short ver;
	binaryRead(is,ver);
	if (ver != version) {
		VCELL_EXCEPTION(invalid_argument,"file does not begin with correct version, " << ver << " read, " << version << " expected");
	}
	binaryRead(is,isDictionaryInUse);
	if (isDictionaryInUse) {
		tokenDictionary.clear( );
		TokenKeyType limit; 
		binaryRead(is,limit);
		//char buffer[ml];
		for (TokenKeyType i = 1 ; i < limit; i++) {
			std::string token = StdString<TokenLengthType>::restore(is);
			tokenDictionary[token] = i;
		}
	}
	unsigned char shouldBeZero;
	binaryRead(is,shouldBeZero);
	if (shouldBeZero != 0) {
		VCELL_EXCEPTION(invalid_argument,"expected zero marker, read " << shouldBeZero);
	}
}

WriteFormatter::~WriteFormatter( ) {
	isDictionaryInUse = writeInProgress = false;
	tokenDictionary.clear( );
}
ReadFormatter::~ReadFormatter( ) {
	isDictionaryInUse =false;
	tokenDictionary.clear( );
}


template <typename S>
typename std::enable_if< std::is_same<S, unsigned char>::value || std::is_same<S,unsigned short>::value, void>::type
StdString<S>::save(std::ostream &os,  const std::string &str) {
	const size_t length = str.size( );
	if (length > std::numeric_limits<S>::max( )) {
		const size_t mLength = std::numeric_limits<S>::max( );
		VCELL_EXCEPTION(out_of_range,"string " << str << " length " << length << " greater than max allowed " << mLength); 
	}
	S slen = static_cast<S>(length);
	binaryWrite(os,slen);
	os.write(str.c_str( ),slen);
}

template <typename S>
typename std::enable_if< std::is_same<S, unsigned char>::value || std::is_same<S,unsigned short>::value, void>::type
StdString<S>::restore(std::istream &is, std::string &out) {
	out = restore(is);
}

template <typename S>
typename std::enable_if< std::is_same<S, unsigned char>::value || std::is_same<S,unsigned short>::value, std::string>::type
StdString<S>::restore(std::istream &is) {
	char buffer[std::numeric_limits<S>::max()];
	S slen;
	binaryRead(is,slen);
	is.read(buffer,slen);
	return std::string(buffer,slen);
}

template struct vcell_persist::StdString<unsigned char>;
template struct vcell_persist::StdString<unsigned short>;
