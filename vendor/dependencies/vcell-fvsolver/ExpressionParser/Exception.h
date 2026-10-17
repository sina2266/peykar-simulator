#ifndef EXCEPTION_H
#define EXCEPTION_H
#include <stdexcept>
#include <string>

namespace VCell {
	class Exception : public std::exception  
	{
	public:
		typedef std::string string;
		Exception();
		Exception(string message);
		Exception(string title, string message);
		/**
		* calls #getMessage
		*/
	    virtual const char * what() const throw( );
		
		virtual string getExactMessage();
		string getMessage(void);
		~Exception(void) throw( );
		static void rethrowException(Exception& ex, string replacementMessage="");
		void replaceMessage(string& replacementMessage);
		static string add_escapes(string str);
	//	static string add_escape(char ch);

	protected:
		string message;	
		string title;	
	};
}

#endif
