#ifndef OstreamSpy_h
#define OstreamSpy_h
#include <streambuf>
namespace vcell_util {

	/**
	* specialized streambuf for uses in OStreamSpy
	* @tparam Spy class which implements intercept(char_type)
	*/
	template <class Spy, typename char_type, typename traits = std::char_traits<char_type> >
	struct Spybuf: public std::basic_streambuf<char_type, traits> {
		// based on public domain code by Thomas Guest
		// http://wordaligned.org/articles/cpp-streambufs

		typedef typename traits::int_type int_type;

		/**
		@param spy_ -- reference must remain valid
		@param sb streambuf to spy upon 
		*/
		Spybuf(Spy & spy_,std::basic_streambuf<char_type, traits> * sb)
			:spy(spy_),
			realbuf(sb)
		{
		}

	private:    
		virtual int sync()
		{
			return realbuf->pubsync( );
		}

		virtual int_type overflow(int_type c)
		{
			int_type const eof = traits::eof();

			if (traits::eq_int_type(c, eof))
			{
				return traits::not_eof(c);
			}
			else
			{
				char_type const ch = traits::to_char_type(c);
				spy.intercept(ch);
				return realbuf->sputc(ch);
			}
		}

	private:
		Spy & spy;
		std::basic_streambuf<char_type, traits> * realbuf;
	};

	/**
	* class to intercept writes to an existing ostream
	* @tparam Spy class which implements intercept(char_type)
	*/
	template <class Spy, typename char_type = char, typename traits = std::char_traits<char_type> >
	struct OStreamSpy {
		/**
		@param target stream to intercept
		@param spy -- reference must remain valid
		*/
		OStreamSpy( std::basic_ostream<char_type,traits> & target_, Spy & spy_ )
			:target(target_),
			spy(spy_),
			targetbuf(target_.rdbuf( )),
			spybuf(spy_,targetbuf)
		{
			target.rdbuf(&spybuf);
		}
		/**
		* restores previous streambuf
		*/
		~OStreamSpy(  ) {
			target.rdbuf(targetbuf);
		}
	private:
		std::basic_ostream<char_type,traits> & target;
		Spy & spy;
		std::basic_streambuf<char,traits> * targetbuf; 
		// must come after targetbuf in class declaration due to initialization order
		Spybuf<Spy, char_type, traits> spybuf;
	};
}
#endif
