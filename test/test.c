#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "minunit.h"
#include "../event-msgpack.h"

#define MAX_SIZE 512

int tests_run = 0;
int tests_failed = 0;

int foo = 7;
int bar = 4;

#define FLAG_FIXSTR       1
#define FLAG_NIL          2
#define FLAG_BOOLEAN      4
#define FLAG_NUMBER       8
#define FLAG_START_MAP    16
#define FLAG_STOP_MAP     32
#define FLAG_START_ARRAY  64
#define FLAG_STOP_ARRAY   128
#define FLAG_ERROR        256

union type_u {
    char c;
    int  i;
};

struct map_el_s {
    char _key[MAX_SIZE];
    char _str_val[MAX_SIZE];
    union type_u _val;
};

int  _flags = 0;
int  _error = 0;

char _key[MAX_SIZE];
char _str_val[MAX_SIZE];
char _chr_val;
int  _int_val;
int  _int_size;

struct map_el_s _map_entry[32];
int it = 0;

char read_str(char *val) {
    _flags |= FLAG_FIXSTR;
    strncpy(_str_val, val, MAX_SIZE);
    return 0;
}

char read_nil() {
    _flags |= FLAG_NIL;
    return 0;
}

char read_boolean(char val) {
    _flags |= FLAG_BOOLEAN;
    _chr_val = val;
    return 0;
}

char read_number(int val) {
    _flags |= FLAG_NUMBER;
    _int_val = val;
    return 0;
}


char read_str_entry(char *key, char *val) {
    _flags |= FLAG_FIXSTR;
    strncpy(_map_entry[it]._key, key, MAX_SIZE);
    strncpy(_map_entry[it]._str_val, val, MAX_SIZE);
    it++;
    return 0;
}

char read_nil_entry(char *key) {
    _flags |= FLAG_NIL;
    strncpy(_map_entry[it]._key, key, MAX_SIZE);
    it++;
    return 0;
}

char read_boolean_entry(char *key, char val) {
    _flags |= FLAG_BOOLEAN;
    strncpy(_map_entry[it]._key, key, MAX_SIZE);
    _map_entry[it]._val.c = val;
    it++;
    return 0;
}

char read_number_entry(char *key, int val) {
    _flags |= FLAG_NUMBER;
    strncpy(_map_entry[it]._key, key, MAX_SIZE);
    _map_entry[it]._val.i = val;
    it++;
    return 0;
}

char read_start_map(int size) {
    _flags |= FLAG_START_MAP;
    _int_size = size;
    return 0;
}

char read_stop_map() {
    _flags |= FLAG_STOP_MAP;
    return 0;
}

char read_start_array(int size) {
    _flags |= FLAG_START_ARRAY;
    _int_size = size;
    return 0;
}

char read_stop_array() {
    _flags |= FLAG_STOP_ARRAY;
    return 0;
}

void read_error(char error_code) {
    _flags |= FLAG_ERROR;
    _error = error_code;
}

static char * test_fixstr() {
    msgpk_t msgpk;

    msgpk_init(&msgpk);
    msgpk.vt.read_str = read_str;

    _flags = 0;
    msgpk_read(&msgpk, "\xa4test");
    mu_assert("fixstr    1/3", "error, cb not called", _flags & FLAG_FIXSTR);
    mu_assert("fixstr    2/3", "error, wrong value", strcmp(_str_val, "test") == 0);

    _flags = 0;
    msgpk_read(&msgpk, "\xbf""aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    mu_assert("fixstr    3/3", "error, wrong value", strcmp(_str_val, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") == 0);

    printf("\n");
    return 0;
}

static char * test_nil() {
    msgpk_t msgpk;

    msgpk_init(&msgpk);
    msgpk.vt.read_nil = read_nil;

    _flags = 0;
    msgpk_read(&msgpk, "\xc0");
    mu_assert("nil       1/1", "error, cb not called", _flags & FLAG_NIL);

    printf("\n");
    return 0;
}

static char * test_boolean() {
    msgpk_t msgpk;

    msgpk_init(&msgpk);
    msgpk.vt.read_boolean = read_boolean;

    _flags = 0;
    msgpk_read(&msgpk, "\xc2"); // False
    mu_assert("boolean   1/3", "error, cb not called", _flags & FLAG_BOOLEAN);
    mu_assert("boolean   2/3", "error, wrong value", !_chr_val);

    msgpk_read(&msgpk, "\xc3"); // True
    mu_assert("boolean   3/3", "error, wrong value", _chr_val);

    printf("\n");
    return 0;
}

static char * test_fixint() {
    msgpk_t msgpk;

    msgpk_init(&msgpk);
    msgpk.vt.read_number = read_number;

    _flags = 0;_int_val = 255;
    msgpk_read(&msgpk, "\x01");
    mu_assert("fixint    1/5", "error, cb not called", _flags & FLAG_NUMBER);
    mu_assert("fixint    2/5", "error, wrong value (0x01)", _int_val == 1);

    _flags = 0;_int_val = 255;
    msgpk_read(&msgpk, "\x7F");
    mu_assert("fixint    3/5", "error, wrong value (0x7F)", _int_val == 127);

    _flags = 0;_int_val = 255;
    msgpk_read(&msgpk, "\xFF");
    mu_assert("fixint    4/5", "error, wrong value (0xFF)", _int_val == -1);

    _flags = 0;_int_val = 255;
    msgpk_read(&msgpk, "\xE0");
    mu_assert("fixint    5/5", "error, wrong value (0xE0)", _int_val == -32);

    printf("\n");
    return 0;
}

static char * test_fixarray() {
    msgpk_t msgpk;

    msgpk_init(&msgpk);
    msgpk.vt.read_str= read_str;
    msgpk.vt.read_nil = read_nil;
    msgpk.vt.read_boolean = read_boolean;
    msgpk.vt.read_number = read_number;
    msgpk.vt.read_start_array = read_start_array;
    msgpk.vt.read_stop_array = read_stop_array;

    _flags = 0;
    _int_size = 255;
    msgpk_read(&msgpk, "\x90");
    mu_assert("fixarray  1/13", "error, start cb not called", _flags & FLAG_START_ARRAY);
    mu_assert("fixarray  2/13", "error, stop cb not called", _flags & FLAG_STOP_ARRAY);
    mu_assert("fixarray  3/13", "error, wrong size (0)", _int_size == 0);

    _flags = 0;
    _int_size = 255;
    *_str_val = '\0';
    _int_val = 255;
    _chr_val = 1;
    msgpk_read(&msgpk, "\x94\xa4\x74\x65\x73\x74\x01\xc0\xc2");
    mu_assert("fixarray  4/13", "error, start cb not called", _flags & FLAG_START_ARRAY);
    mu_assert("fixarray  5/13", "error, wrong size (4)", _int_size == 4);

    mu_assert("fixarray  6/13", "error, wrong value (test)", strcmp(_str_val, "test") == 0);
    mu_assert("fixarray  7/13", "error, wrong value (1)", _int_val == 1);
    mu_assert("fixarray  8/13", "error, cb not called", _flags & FLAG_NIL);
    mu_assert("fixarray  9/13", "error, wrong value", !_chr_val);

    mu_assert("fixarray 10/13", "error, stop cb not called", _flags & FLAG_STOP_ARRAY);

    _flags = 0;
    _int_size = 255;
    msgpk_read(&msgpk, "\x9F\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01");
    mu_assert("fixarray 11/13", "error, start cb not called", _flags & FLAG_START_ARRAY);
    mu_assert("fixarray 12/13", "error, wrong size (15)", _int_size == 15);
    mu_assert("fixarray 13/13", "error, stop cb not called", _flags & FLAG_STOP_ARRAY);

    printf("\n");
    return 0;
}

static char * test_fixmap() {
    msgpk_t msgpk;

    msgpk_init(&msgpk);
    msgpk.vt.read_str_entry = read_str_entry;
    msgpk.vt.read_nil_entry = read_nil_entry;
    msgpk.vt.read_boolean_entry = read_boolean_entry;
    msgpk.vt.read_number_entry = read_number_entry;
    msgpk.vt.read_start_map = read_start_map;
    msgpk.vt.read_stop_map = read_stop_map;

    _flags = 0;
    msgpk_read(&msgpk, "\x80");
    mu_assert("fixmap    1/13", "error, start cb not called", _flags & FLAG_START_MAP);
    mu_assert("fixmap    2/13", "error, stop cb not called", _flags & FLAG_STOP_MAP);
    mu_assert("fixmap    3/13", "error, wrong size (0)", _int_size == 0);

    _flags = 0;
    _int_size = 255;
    msgpk_read(&msgpk, "\x84\xa2\x73\x31\x01\xa2\x73\x32\xc3\xa2\x73\x33\xc0\xa2\x73\x34\xa4\x74\x65\x73\x74");
    mu_assert("fixmap    4/13", "error, start cb not called", _flags & FLAG_START_MAP);
    mu_assert("fixmap    5/13", "error, wrong size (4)", _int_size == 4);

    mu_assert("fixmap    6/13", "error, wrong value (1)",    strcmp(_map_entry[0]._str_val, "s1") && _map_entry[0]._val.i == 1);
    mu_assert("fixmap    7/13", "error, wrong value (true)", strcmp(_map_entry[1]._str_val, "s2") && _map_entry[1]._val.c == 1);
    mu_assert("fixmap    8/13", "error, cb not called",      strcmp(_map_entry[2]._str_val, "s3") && _flags & FLAG_NIL);
    mu_assert("fixmap    9/13", "error, wrong value (test)", strcmp(_map_entry[3]._str_val, "s4") && strcmp(_map_entry[3]._str_val, "test") == 0);

    mu_assert("fixmap   10/13", "error, stop cb not called", _flags & FLAG_STOP_MAP);

    _flags = 0;
    _int_size = 255;
    msgpk_read(&msgpk, "\x8f\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01\xa1\x73\x01");
    mu_assert("fixmap   11/13", "error, start cb not called", _flags & FLAG_START_MAP);
    mu_assert("fixmap   12/13", "error, wrong size (15)", _int_size == 15);
    mu_assert("fixmap   13/13", "error, stop cb not called", _flags & FLAG_STOP_MAP);

    printf("\n");
    return 0;
}

static char * test_error() {
    msgpk_t msgpk;

    msgpk_init(&msgpk);
    msgpk.vt.read_error = read_error;

    _flags = 0;
    _error = 0;
    msgpk_read(&msgpk, "\xcc");
    mu_assert("error     1/2", "error, cb not called", _flags & FLAG_ERROR);
    mu_assert("error     2/2", "error, wrong error code", _error == ERROR_UNKNOWN);

    printf("\n");
    return 0;
}

static char * all_tests() {
    mu_run_test(test_nil);
    mu_run_test(test_boolean);
    mu_run_test(test_fixstr);
    mu_run_test(test_fixint);
    mu_run_test(test_fixarray);
    mu_run_test(test_fixmap);
    mu_run_test(test_error);
    return 0;
}

int main(int argc, char **argv) {
    char *result = all_tests();
    if (result != 0) {
        printf("\t%s\n", result);
        printf("\nPassed %d/%d\n", tests_run-tests_failed, tests_run);
    }
    else {
        printf("\nALL TESTS PASSED\n");
        printf("Groups run: %d\n", tests_run);
    }

    return result != 0;
}
