#include <functional>
#include <cppunit/extensions/HelperMacros.h>

#define MAX_SIZE 512
#define MAP_SIZE 32

#define FLAG_FIXSTR       1
#define FLAG_NIL          2
#define FLAG_BOOLEAN      4
#define FLAG_NUMBER       8
#define FLAG_START_MAP    16
#define FLAG_STOP_MAP     32
#define FLAG_START_ARRAY  64
#define FLAG_STOP_ARRAY   128
#define FLAG_ERROR        256

extern "C" {
#include <string.h>
#include <stdint.h>

#include "../event-msgpack.h"

static msgpk_t msgpk;
static char _str_val[MAX_SIZE];
static int  _flags;
static int  _error;

void _strappend(char *dest, char c)
{
	int i = strlen(dest);
	dest[i] = c;
	dest[i + 1] = '\0';
}

char writer(char c)
{
	_strappend(_str_val, c);
	return 0;
}

void write_error(char error_code)
{
	_flags |= FLAG_ERROR;
	_error = error_code;
}
} // extern "C"

class WriteTest : public CppUnit::TestFixture  {
CPPUNIT_TEST_SUITE( WriteTest );
CPPUNIT_TEST( testNil );
CPPUNIT_TEST( testBoolean );
CPPUNIT_TEST( testFixInt );
CPPUNIT_TEST( testFixStr );
CPPUNIT_TEST( testFixArray );
CPPUNIT_TEST( testFixMap );
CPPUNIT_TEST( testError );
CPPUNIT_TEST_SUITE_END();

private:

public:
	void setUp()
	{
		msgpk_init(&msgpk);
		memset(_str_val, 0, MAX_SIZE);
		msgpk.vt.writer = writer;
		msgpk.vt.write_error = write_error;
		_flags = 0;
		_error = 0;
	}

	void tearDown() 
	{
	}

	void testNil()
	{
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_nil(&msgpk);
		CPPUNIT_ASSERT(strcmp(_str_val, "\xc0") == 0);
	}

	void testBoolean()
	{
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_boolean(&msgpk, 0);
		CPPUNIT_ASSERT(strcmp(_str_val, "\xc2") == 0);
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_boolean(&msgpk, 1);
		CPPUNIT_ASSERT(strcmp(_str_val, "\xc3") == 0);
	}

	void testFixInt()
	{
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_number(&msgpk, 1);
		CPPUNIT_ASSERT(strcmp(_str_val, "\x01") == 0);
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_number(&msgpk, 127);
		CPPUNIT_ASSERT(strcmp(_str_val, "\x7f") == 0);
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_number(&msgpk, -1);
		CPPUNIT_ASSERT(strcmp(_str_val, "\xff") == 0);
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_number(&msgpk, -32);
		CPPUNIT_ASSERT(strcmp(_str_val, "\xe0") == 0);
	}

	void testFixStr()
	{	
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_str(&msgpk, "test");
		CPPUNIT_ASSERT(strcmp(_str_val, "\xa4""test") == 0);
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_str(&msgpk, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
		CPPUNIT_ASSERT(strcmp(_str_val, "\xbf""aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") == 0);
	}

	void testFixArray()
	{
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_start_array(&msgpk, 0);
		CPPUNIT_ASSERT(strcmp(_str_val, "\x90") == 0);
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_start_array(&msgpk, 4);
		msgpk_write_str(&msgpk, "test");
		msgpk_write_number(&msgpk, 1);
		msgpk_write_nil(&msgpk);
		msgpk_write_boolean(&msgpk, 0);
		msgpk_write_boolean(&msgpk, 1);// After array
		CPPUNIT_ASSERT(strcmp(_str_val, "\x94\xa4\x74\x65\x73\x74\x01\xc0\xc2\xc3") == 0);
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_start_array(&msgpk, 15);
		int i;

		for (i = 0; i < 15; i++) {
			msgpk_write_number(&msgpk, 1);
		}

		CPPUNIT_ASSERT(strcmp(_str_val, "\x9F\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01") == 0);
	}

	void testFixMap()
	{
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_start_map(&msgpk, 0);
		CPPUNIT_ASSERT(strcmp(_str_val, "\x80") == 0);
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_start_map(&msgpk, 4);
		msgpk_write_number_entry(&msgpk, "s1", 1);
		msgpk_write_boolean_entry(&msgpk, "s2", 1);
		msgpk_write_nil_entry(&msgpk, "s3");
		msgpk_write_str_entry(&msgpk, "s4", "test");
		CPPUNIT_ASSERT(strcmp(_str_val, "\x84\xa2\x73\x31\x01\xa2\x73\x32\xc3\xa2\x73\x33\xc0\xa2\x73\x34\xa4\x74\x65\x73\x74") == 0);
		memset(_str_val, 0, MAX_SIZE);
		msgpk_write_start_map(&msgpk, 15);
		int i;

		for (i = 0; i < 15; i++) {
			msgpk_write_number_entry(&msgpk, "s", 1);
		}

		CPPUNIT_ASSERT(strcmp(_str_val, "\x8f\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01") == 0);
	}

	void testError()
	{
		memset(_str_val, 0, MAX_SIZE);
		_flags = 0;
		_error = 0;
		msgpk_write_number(&msgpk, 128);
		CPPUNIT_ASSERT(_flags & FLAG_ERROR);
		CPPUNIT_ASSERT(_error == ERROR_OUT_OF_RANGE);
		memset(_str_val, 0, MAX_SIZE);
		_flags = 0;
		_error = 0;
		msgpk_write_str(&msgpk, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
		CPPUNIT_ASSERT(_error == ERROR_OUT_OF_RANGE);
	}
};



CPPUNIT_TEST_SUITE_REGISTRATION( WriteTest );
