#include <string.h>
#include <stdint.h>

#include "event-msgpack.h"

#define _CAST_FUNC(_func) \
    if (msgpk->vt._func != NULL) {\
        msgpk->vt._func();\
    }
#define _CAST_KEY_FUNC(_func) \
    if (msgpk->vt._func != NULL) {\
        msgpk->vt._func(msgpk->key);\
    }
#define _CAST_VALUE_FUNC(_func,_a) \
    if (msgpk->vt._func != NULL) {\
        msgpk->vt._func((_a));\
    }
#define _CAST_KEY_VALUE_FUNC(_func,_a) \
    if (msgpk->vt._func != NULL) {\
        msgpk->vt._func(msgpk->key, (_a) );\
    }
#define _ELEMENTS_CHECK \
    if (msgpk->map_el_cnt == 0) {\
        if (msgpk->vt.read_stop_map != NULL) {\
            msgpk->vt.read_stop_map();\
        }\
        msgpk->map_el_cnt = -1;\
    }\
    if (msgpk->array_el_cnt == 0) {\
        if (msgpk->vt.read_stop_array != NULL) {\
            msgpk->vt.read_stop_array();\
        }\
        msgpk->array_el_cnt = -1;\
    }

void msgpk_init(msgpk_t *msgpk)
{
    struct vtable tmp = {
#ifndef NO_READER
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
#endif
#ifndef NO_WRITER
        NULL, NULL,
#endif
    };

    msgpk->vt = tmp;
    memset(msgpk->key, 0, KEY_SIZE);
    msgpk->flags = 0;
    msgpk->map_el_cnt = -1;
    msgpk->array_el_cnt = -1;
}

#ifndef NO_READER
void msgpk_read(msgpk_t *msgpk, char *cs)
{
    unsigned long i;
    char c;

    for (i = 0; i < strlen(cs); i++) {
        c = cs[i];

        _ELEMENTS_CHECK;

        if (IS_PFIXINT(c)) {
            if (msgpk->map_el_cnt > 0) {
                msgpk->map_el_cnt--;
                msgpk->flags &= ~VAL_FLAG;
                _CAST_KEY_VALUE_FUNC(read_number_entry, c);
            } else {
                _CAST_VALUE_FUNC(read_number, c);
            }
        } else if (IS_FIXMAP(c)) {
            unsigned char size;
            size = c & 0x0F;
            msgpk->map_el_cnt = size;
            msgpk->flags |= OBJ_FLAG;
            _CAST_VALUE_FUNC(read_start_map, size);
        } else if (IS_FIXARRAY(c)) {
            unsigned char size;
            size = c & 0x0F;
            msgpk->array_el_cnt = size;
            msgpk->flags |= ARRAY_FLAG;
            _CAST_VALUE_FUNC(read_start_array, size);
        } else if (IS_NIL(c)) {
            if (msgpk->map_el_cnt > 0) {
                msgpk->map_el_cnt--;
                msgpk->flags &= ~VAL_FLAG;
                _CAST_KEY_FUNC(read_nil_entry);
            } else {
                _CAST_FUNC(read_nil);
            }
        } else if (IS_BOOLEAN(c)) {
            if (msgpk->map_el_cnt > 0) {
                msgpk->map_el_cnt--;
                msgpk->flags &= ~VAL_FLAG;
                _CAST_KEY_VALUE_FUNC(read_boolean_entry, c & 0x01);
            } else {
                _CAST_VALUE_FUNC(read_boolean, c & 0x01);
            }
        } else if (IS_FIXSTR(c)) {
            if (msgpk->map_el_cnt > 0) {
                if (!(msgpk->flags & VAL_FLAG)) {
                    unsigned char size;
                    size = c & 0x1F;
                    strncpy(msgpk->key, &cs[i+1], size);
                    msgpk->key[size] = '\0';
                    i += size;
                    msgpk->flags |= VAL_FLAG;
                } else {
                    unsigned char size;
                    char val[KEY_SIZE];
                    size = c & 0x1F;
                    msgpk->map_el_cnt--;
                    strncpy(val, &cs[i+1], size);
                    val[size] = '\0';
                    i += size;
                    if (msgpk->vt.read_str_entry != NULL) {
                        msgpk->vt.read_str_entry(msgpk->key, val);
                    }
                    msgpk->flags &= ~VAL_FLAG;
                }
            } else {
                unsigned char size;
                char val[KEY_SIZE];
                size = c & 0x1F;
                strncpy(val, &cs[i+1], size);
                val[size] = '\0';
                i += size;
                if (msgpk->vt.read_str != NULL) {
                    msgpk->vt.read_str(val);
                }
            }
        } else if (IS_NFIXINT(c)) {
            if (msgpk->map_el_cnt > 0) {
                msgpk->map_el_cnt--;
                msgpk->flags &= ~VAL_FLAG;
                _CAST_KEY_VALUE_FUNC(read_number_entry, -32+(c & 0x1f));
            } else {
                _CAST_VALUE_FUNC(read_number, -32+(c & 0x1f));
            }
        } else {
            _CAST_VALUE_FUNC(read_error, ERROR_UNKNOWN  );
        }

        if (msgpk->array_el_cnt > 0) {
            msgpk->array_el_cnt--;
        }
    }

    _ELEMENTS_CHECK
}
#endif


#ifndef NO_WRITER
char msgpk_write_start_map(msgpk_t *msgpk, int size)
{
    if (size > 15) {
        _CAST_VALUE_FUNC(write_error, ERROR_OUT_OF_RANGE);
        return -1;
    }

    _CAST_VALUE_FUNC(writer, FIXMAP_VAL + size);
    return 0;
}

char msgpk_write_start_array(msgpk_t *msgpk, int size)
{
    if (size > 15) {
        _CAST_VALUE_FUNC(write_error, ERROR_OUT_OF_RANGE);
        return -1;
    }

    _CAST_VALUE_FUNC(writer, FIXARRAY_VAL + size);
    return 0;
}

void _write_fixstr_no_test(msgpk_t *msgpk, char* val, unsigned char size)
{
    unsigned char i;

    _CAST_VALUE_FUNC(writer, FIXSTR_VAL + size);
    for (i=0; i < size; i++) {
        _CAST_VALUE_FUNC(writer, val[i]);
    }
}

char msgpk_write_str(msgpk_t *msgpk, char* val)
{
    unsigned char size;

    size = strlen(val);

    if (size <= 31) {
        _write_fixstr_no_test(msgpk, val, size);
        return 0;
    }

    _CAST_VALUE_FUNC(write_error, ERROR_OUT_OF_RANGE);
    return -1;
}

char msgpk_write_str_entry(msgpk_t *msgpk, char* key, char* val)
{
    unsigned char k_size, v_size;

    k_size = strlen(key);
    v_size = strlen(val);

    if (k_size <= 31 && v_size <= 31) {
        _write_fixstr_no_test(msgpk, key, k_size);
        _write_fixstr_no_test(msgpk, val, v_size);
        return 0;
    }

    _CAST_VALUE_FUNC(write_error, ERROR_OUT_OF_RANGE);
    return -1;
}


char msgpk_write_nil(msgpk_t *msgpk)
{
    _CAST_VALUE_FUNC(writer, NIL_VAL);
    return 0;
}

char msgpk_write_nil_entry(msgpk_t *msgpk, char* key)
{
    unsigned char k_size;

    k_size = strlen(key);

    if (k_size <= 31) {
        _write_fixstr_no_test(msgpk, key, k_size);
        _CAST_VALUE_FUNC(writer, NIL_VAL);
        return 0;
    }

    _CAST_VALUE_FUNC(write_error, ERROR_OUT_OF_RANGE);
    return -1;
}


char msgpk_write_boolean(msgpk_t *msgpk, char val)
{
    if (val == 0) {
        _CAST_VALUE_FUNC(writer, BOOLEAN_VAL);
    } else {
        _CAST_VALUE_FUNC(writer, BOOLEAN_VAL + 1);
    }
    return 0;
}

char msgpk_write_boolean_entry(msgpk_t *msgpk, char* key,char val)
{
    unsigned char k_size;

    k_size = strlen(key);

    if (k_size <= 31) {
        _write_fixstr_no_test(msgpk, key, k_size);
        if (val == 0) {
            _CAST_VALUE_FUNC(writer, BOOLEAN_VAL);
        } else {
            _CAST_VALUE_FUNC(writer, BOOLEAN_VAL + 1);
        }
        return 0;
    }

    _CAST_VALUE_FUNC(write_error, ERROR_OUT_OF_RANGE);
    return -1;
}


char msgpk_write_number(msgpk_t *msgpk, int val)
{
    if (val >= 0 && val <= PFIXINT_MAX) {
        _CAST_VALUE_FUNC(writer, val);
    } else if (val < 0 && val >= NFIXINT_MIN) {
        _CAST_VALUE_FUNC(writer, NFIXINT_VAL + val + 32);
    } else {
        _CAST_VALUE_FUNC(write_error, ERROR_OUT_OF_RANGE);
        return -1;
    }
    return 0;
}

char msgpk_write_number_entry(msgpk_t *msgpk, char* key, int val)
{
    unsigned char k_size;

    k_size = strlen(key);

    if (k_size <= 31) {
        if (val >= 0 && val <= PFIXINT_MAX) {
            _write_fixstr_no_test(msgpk, key, k_size);
            _CAST_VALUE_FUNC(writer, val);
            return 0;
        } else if (val < 0 && val >= NFIXINT_MIN) {
            _write_fixstr_no_test(msgpk, key, k_size);
            _CAST_VALUE_FUNC(writer, NFIXINT_VAL + val + 32);
            return 0;
        }
    }

    _CAST_VALUE_FUNC(write_error, ERROR_OUT_OF_RANGE);
    return -1;
}

#endif
