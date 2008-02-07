#ifndef HSHGTEST_H
#define HSHGTEST_H

// Copyright (c) 2007 Charles Bailey

// Distributed under the Boost Software License, Version 1.0
// See: http://boost.org/LICENSE_1_0.txt

#include <string>
#include <cstdlib>
#include <ostream>

//! namespace for the test framework
namespace HSHGTest
{

//! An exception class representing a test assertion failure.
class TestFailure
{
public:

	//! Constructs an exception with the given details.
	TestFailure( const char* message, const char* file, unsigned int line )
		: m_message( message )
		, m_file( file )
		, m_line( line )
	{
	}

	//! Returns a description of the test that failed.
	const std::string& GetMessage() const  { return m_message; }

	//! Returns the name of the source file where the failed test resides.
	const std::string& GetFile() const     { return m_file; }

	//! Returns the source line number where the failed test resides.
	unsigned int GetLine() const           { return m_line; }

private:

	std::string m_message;
	std::string m_file;
	unsigned int m_line;
};

//! Throws a TestFailure exception if the given condition is false.
inline void TestAssert( bool condition, const char* message, const char* file
			   , unsigned int line )
{
	if( !condition )
	{
		throw TestFailure( message, file, line );
	}
}

//! Helper structure for defining named test functions
struct TestFn
{
	//! the name of the test function
	const char* name;
	//! test function pointer
	void (*fptr)();
};

//! Runs a sequence of tests

//! \returns EXIT_SUCCESS if the list of tests was non-empty and there were no
//!          test failures, EXIT_FAILURE otherwise.
//! \param fnarray an array of TestFn structs terminated by a final entry whose
//!        fptr member is NULL
//! \param log an output stream for writing test output
inline int RunTests(TestFn fnarray[], std::ostream& log)
{
	int fail = 0, count = 0;
	for (TestFn* p = fnarray; p->fptr != NULL; ++p)
	{
		try
		{
			++count;
			(*p->fptr)();
		}
		catch (TestFailure& tf)
		{
#ifdef WIN32
			log << tf.GetFile() << '(' << tf.GetLine() << ") : error TST0000: Test "
#else
			log << tf.GetFile() << ':' << tf.GetLine() << ": Test "
#endif
				<< p->name << " failed. ( " << tf.GetMessage() << " )\n";
			++fail;
		}
		catch (...)
		{
#ifdef WIN32
			log << __FILE__ << '(' << __LINE__ << ") : error TST0001: Test "
#else
			log << __FILE__ << ':' << __LINE__ << ": Test "
#endif
			<< p->name << " failed. ( An unexpected exception was thrown. )\n";
			++fail;
		}
	}
	log << count - fail << " out of " << count << " tests passed.\n";
	return count == 0 || fail != 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

} // namespace HSHGTest

//! Begins the declaration of a block of tests.
#define HSHG_BEGIN_TESTS namespace { HSHGTest::TestFn tests[] = {

//! Defines a test function.

//! The test function must have a definition or a prototype in scope at the
//! point at which this macro is used.
#define HSHG_TEST_ENTRY(t) { #t, t },

//! Ends the declaration of a block of tests.
#define HSHG_END_TESTS { NULL, NULL } }; }

//! Defines a default main function for a test executable.
#define HSHG_TEST_MAIN int main() { return HSHGTest::RunTests( tests, std::cout ); }

//! Calls the HSHGTest::TestAssert function with the given condition, a
//! stringized version of the condition for a description and appropriate
//! file and line numbers.
#define HSHG_ASSERT( x ) HSHGTest::TestAssert( x, #x, __FILE__, __LINE__ )

//! Calls the HSHGTest::TestAssert function with the given condition, a
//! supplied description and appropriate file and line numbers.
#define HSHG_ASSERT_DESC( x, d ) HSHGTest::TestAssert( x, d, __FILE__, __LINE__ )

//! Calls the HSHGTest::TestAssert function with false if the given expression
//! doesn't cause an exception (other than a TestFailure) that can be caught
//! by a catch block of the form "catch (except&)" for the given macro parameter
//! except.
#define HSHG_ASSERT_THROWS( x, except ) try { x;\
	HSHGTest::TestAssert( false, "Exception " #except " expected."\
		, __FILE__, __LINE__ ); } catch( except & ) {}

#endif//HSHGTEST_H
