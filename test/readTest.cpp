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

union type_u {
	char c;
	int  i;
};

struct map_el_s {
	char _key[MAX_SIZE];
	char _str_val[MAX_SIZE];
	union type_u _val;
};

static msgpk_t msgpk;
static char _key[MAX_SIZE];
static char _str_val[MAX_SIZE];
static struct map_el_s _map_entry[MAP_SIZE];
static int  _flags;
static int  _error;
static int  _int_val;
static int  _int_size;
static char _chr_val;
static int it;
	
char read_str(char *val)
{
	_flags |= FLAG_FIXSTR;
	strncpy(_str_val, val, MAX_SIZE);
	return 0;
}

char read_nil()
{
	_flags |= FLAG_NIL;
	return 0;
}

char read_boolean(char val)
{
	_flags |= FLAG_BOOLEAN;
	_chr_val = val;
	return 0;
}

char read_number(int val)
{
	_flags |= FLAG_NUMBER;
	_int_val = val;
	return 0;
}


char read_str_entry(char *key, char *val)
{
	_flags |= FLAG_FIXSTR;
	strncpy(_map_entry[it]._key, key, MAX_SIZE);
	strncpy(_map_entry[it]._str_val, val, MAX_SIZE);
	it++;
	return 0;
}

char read_nil_entry(char *key)
{
	_flags |= FLAG_NIL;
	strncpy(_map_entry[it]._key, key, MAX_SIZE);
	it++;
	return 0;
}

char read_boolean_entry(char *key, char val)
{
	_flags |= FLAG_BOOLEAN;
	strncpy(_map_entry[it]._key, key, MAX_SIZE);
	_map_entry[it]._val.c = val;
	it++;
	return 0;
}

char read_number_entry(char *key, int val)
{
	_flags |= FLAG_NUMBER;
	strncpy(_map_entry[it]._key, key, MAX_SIZE);
	_map_entry[it]._val.i = val;
	it++;
	return 0;
}

char read_start_map(int size)
{
	_flags |= FLAG_START_MAP;
	_int_size = size;
	return 0;
}

char read_stop_map()
{
	_flags |= FLAG_STOP_MAP;
	return 0;
}

char read_start_array(int size)
{
	_flags |= FLAG_START_ARRAY;
	_int_size = size;
	return 0;
}

char read_stop_array()
{
	_flags |= FLAG_STOP_ARRAY;
	return 0;
}

void read_error(char error_code)
{
	_flags |= FLAG_ERROR;
	_error = error_code;
}
} // extern "C"

class ReadTest : public CppUnit::TestFixture  {
CPPUNIT_TEST_SUITE( ReadTest );
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
		memset(_key, 0, MAX_SIZE);
		memset(_str_val, 0, MAX_SIZE);
		memset(_map_entry, 0, sizeof(struct map_el_s) * MAP_SIZE);
		_flags = 0;
		_error = 0;
		_int_val = 0;
		_int_size = 0;
		_chr_val = 0;
		it = 0;
	}

	void tearDown() 
	{
	}

	void testNil()
	{
		msgpk.vt.read_nil = read_nil;
		
		msgpk_read(&msgpk, "\xc0", 1);
		CPPUNIT_ASSERT(_flags & FLAG_NIL);
	}

	void testBoolean()
	{
		msgpk.vt.read_boolean = read_boolean;
		msgpk_read(&msgpk, "\xc2", 1); // False
		CPPUNIT_ASSERT(_flags & FLAG_BOOLEAN);
		CPPUNIT_ASSERT(!_chr_val);
		
		msgpk_read(&msgpk, "\xc3", 1); // True
		CPPUNIT_ASSERT(_chr_val);
	}

	void testFixInt()
	{
		msgpk.vt.read_number = read_number;
		_flags = 0;
		_int_val = 255;
		msgpk_read(&msgpk, "\x01", 1);
		CPPUNIT_ASSERT(_flags & FLAG_NUMBER);
		CPPUNIT_ASSERT(_int_val == 1);
		_flags = 0;
		_int_val = 255;
		msgpk_read(&msgpk, "\x7F", 1);
		CPPUNIT_ASSERT(_int_val == 127);
		_flags = 0;
		_int_val = 255;
		msgpk_read(&msgpk, "\xFF", 1);
		CPPUNIT_ASSERT(_int_val == -1);
		_flags = 0;
		_int_val = 255;
		msgpk_read(&msgpk, "\xE0", 1);
		CPPUNIT_ASSERT(_int_val == -32);
	}

	void testFixStr()
	{
		msgpk.vt.read_str = read_str;
		_flags = 0;
		msgpk_read(&msgpk, "\xa4test", 5);
		CPPUNIT_ASSERT(_flags & FLAG_FIXSTR);
		CPPUNIT_ASSERT(strcmp(_str_val, "test") == 0);
		_flags = 0;
		msgpk_read(&msgpk, "\xbf""aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 32);
		CPPUNIT_ASSERT(strcmp(_str_val, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") == 0);
	}

	void testFixArray()
	{
		msgpk.vt.read_str = read_str;
		msgpk.vt.read_nil = read_nil;
		msgpk.vt.read_boolean = read_boolean;
		msgpk.vt.read_number = read_number;
		msgpk.vt.read_start_array = read_start_array;
		msgpk.vt.read_stop_array = read_stop_array;
		
		_flags = 0;
		_int_size = 255;
		msgpk_read(&msgpk, "\x90", 1);
		CPPUNIT_ASSERT(_flags & FLAG_START_ARRAY);
		CPPUNIT_ASSERT(_flags & FLAG_STOP_ARRAY);
		CPPUNIT_ASSERT(_int_size == 0);
		
		_flags = 0;
		_int_size = 255;
		*_str_val = '\0';
		_int_val = 255;
		_chr_val = 1;
		msgpk_read(&msgpk, "\x94\xa4\x74\x65\x73\x74\x01\xc0\xc2", 9);
		CPPUNIT_ASSERT(_flags & FLAG_START_ARRAY);
		CPPUNIT_ASSERT(_int_size == 4);
		CPPUNIT_ASSERT(strcmp(_str_val, "test") == 0);
		CPPUNIT_ASSERT(_int_val == 1);
		CPPUNIT_ASSERT(_flags & FLAG_NIL);
		CPPUNIT_ASSERT(!_chr_val);
		CPPUNIT_ASSERT(_flags & FLAG_STOP_ARRAY);
		
		_flags = 0;
		_int_size = 255;
		msgpk_read(&msgpk, "\x9F\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01", 16);
		CPPUNIT_ASSERT(_flags & FLAG_START_ARRAY);
		CPPUNIT_ASSERT(_int_size == 15);
		CPPUNIT_ASSERT(_flags & FLAG_STOP_ARRAY);
	}

	void testFixMap()
	{
		msgpk.vt.read_str_entry = read_str_entry;
		msgpk.vt.read_nil_entry = read_nil_entry;
		msgpk.vt.read_boolean_entry = read_boolean_entry;
		msgpk.vt.read_number_entry = read_number_entry;
		msgpk.vt.read_start_map = read_start_map;
		msgpk.vt.read_stop_map = read_stop_map;

		msgpk_read(&msgpk, "\x80", 1);
		CPPUNIT_ASSERT(_flags & FLAG_START_MAP);
		CPPUNIT_ASSERT(_flags & FLAG_STOP_MAP);
		CPPUNIT_ASSERT(_int_size == 0);
		
		_flags = 0;
		_int_size = 255;
		msgpk_read(&msgpk, "\x84\xa2\x73\x31\x01\xa2\x73\x32\xc3\xa2\x73\x33\xc0\xa2\x73\x34\xa4\x74\x65\x73\x74", 21);
		CPPUNIT_ASSERT(_flags & FLAG_START_MAP);
		CPPUNIT_ASSERT(_int_size == 4);
		CPPUNIT_ASSERT(strcmp(_map_entry[0]._str_val, "s1") && _map_entry[0]._val.i == 1);
		CPPUNIT_ASSERT(strcmp(_map_entry[1]._str_val, "s2") && _map_entry[1]._val.c == 1);
		CPPUNIT_ASSERT(strcmp(_map_entry[2]._str_val, "s3") && _flags & FLAG_NIL);
		CPPUNIT_ASSERT(strcmp(_map_entry[3]._str_val, "s4") && strcmp(_map_entry[3]._str_val, "test") == 0);
		CPPUNIT_ASSERT(_flags & FLAG_STOP_MAP);
		
		_flags = 0;
		_int_size = 255;
		msgpk_read(&msgpk, "\x8f\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01", 46);
		CPPUNIT_ASSERT(_flags & FLAG_START_MAP);
		CPPUNIT_ASSERT(_int_size == 15);
		CPPUNIT_ASSERT(_flags & FLAG_STOP_MAP);
	}

	void testError()
	{
		msgpk.vt.read_error = read_error;
		msgpk_read(&msgpk, "\xcc", 1);
		CPPUNIT_ASSERT(_flags & FLAG_ERROR);
		CPPUNIT_ASSERT(_error == ERROR_UNKNOWN);
	}
};



CPPUNIT_TEST_SUITE_REGISTRATION( ReadTest );
