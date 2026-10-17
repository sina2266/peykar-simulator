#include <stdexcept>
#include "gtest/gtest.h"
#include "Logger.h"
#include "VCellException.h"
using vcell_util::Logger;

namespace {
	const char *badCall( ) {
		throw std::logic_error("don't call me");
	}

}
TEST(logger,macro) {
	Logger & logger = Logger::get( );
	logger.set(Logger::debug);
	ASSERT_TRUE(logger.enabled(Logger::debug));
	ASSERT_TRUE(logger.enabled(Logger::warn));
	ASSERT_TRUE(logger.enabled(Logger::fatal));
	ASSERT_TRUE(logger.enabled(Logger::info));
	ASSERT_FALSE(logger.enabled(Logger::trace));
	ASSERT_FALSE(logger.enabled(Logger::verbose));
	VCELL_LOG(debug,"howdy")
	VCELL_LOG(verbose,"don't do this")
	VCELL_LOG(trace,badCall( ))
	VCELL_LOG(info, "x is " << 5)
	VCELL_COND_LOG(debug, 2 + 2 == 4, "math works")
	VCELL_COND_LOG(debug, 1 == 0, "not true")
	//VCELL_LOG_N(info, "begin the message")
	//VCELL_LOG(info, ", end the message")
}
TEST(logger,key) {
	Logger & logger = Logger::get( );
	logger.set(Logger::debug);
	logger.set(Logger::Key::extra1,true);
	ASSERT_TRUE(logger.enabled(Logger::verbose,Logger::Key::extra1));
	logger.set(Logger::Key::extra1,false);
	ASSERT_FALSE(logger.enabled(Logger::verbose,Logger::Key::extra1));

	ASSERT_FALSE(logger.enabled(Logger::verbose,Logger::Key::extra2));
	ASSERT_TRUE(logger.enabled(Logger::debug,Logger::Key::extra2));
	ASSERT_TRUE(logger.enabled(Logger::warn,Logger::Key::extra2));

	logger.set(Logger::warn);
	ASSERT_FALSE(logger.enabled(Logger::verbose,Logger::Key::extra2));
	ASSERT_FALSE(logger.enabled(Logger::debug,Logger::Key::extra2));
	ASSERT_TRUE(logger.enabled(Logger::warn,Logger::Key::extra2));
	ASSERT_TRUE(logger.enabled(Logger::fatal,Logger::Key::extra2));

	ASSERT_FALSE(logger.enabled(Logger::fatal,Logger::Key::extra1));
	logger.set(Logger::verbose);
	ASSERT_FALSE(logger.enabled(Logger::fatal,Logger::Key::extra1));

//	logger.set(Logger::debug, "fish");
//	logger.set(Logger::info, "fowl");
//	VCELL_KEY_LOG(debug,"fish","pike swims in the " << "water")
//	VCELL_KEY_LOG(debug,"fowl","should not see this")
//	VCELL_KEY_LOG(verbose,"koala","should not see this")
//	VCELL_KEY_LOG(fatal,"koala","should not see this")
	
}
TEST(logger,level) {
	std::cout 
		<< Logger::debug << ' ' 
		<< Logger::warn << ' ' 
		<< Logger::fatal << ' ' 
		<< Logger::info << ' ' 
		<< Logger::verbose << ' ' 
		<< Logger::trace << std::endl; 
	Logger::Level lvl = Logger::readLevel("debug");
	ASSERT_TRUE(lvl == Logger::debug);
//	ASSERT_THROW( lvl = Logger::read("ignore"),std::domain_error);
}

TEST(logger,exception) {
	ASSERT_THROW( VCELL_EXCEPTION(domain_error, 3 << " is bigger than " << 2),std::domain_error);
}
TEST(logger,position) {
	try {
		VCELL_EXCEPTION(domain_error, 3 << " is bigger than " << 2);
	}
	catch (std::exception &e) {
		std::cerr << e.what( ) << std::endl;
	}
}