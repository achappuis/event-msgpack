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
    memset(&(msgpk->vt), 0, sizeof(struct vtable));
    memset(msgpk->key, 0, KEY_SIZE);
    msgpk->flags = 0;
    msgpk->map_el_cnt = -1;
    msgpk->array_el_cnt = -1;
}

#ifndef NO_READER
void msgpk_read(msgpk_t *msgpk, const char *cs, unsigned int len)
{
    if (msgpk == NULL)
        return;
    if (cs == NULL)
        return;

    unsigned long i;

    for (i = 0; i < len; i++) {
		char c;
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
                    strncpy(msgpk->key, &cs[i + 1], size);
                    msgpk->key[size] = '\0';
                    i += size;
                    msgpk->flags |= VAL_FLAG;
                } else {
                    unsigned char size;
                    char val[KEY_SIZE];
                    size = c & 0x1F;
                    msgpk->map_el_cnt--;
                    strncpy(val, &cs[i + 1], size);
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
                strncpy(val, &cs[i + 1], size);
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
                _CAST_KEY_VALUE_FUNC(read_number_entry, -32 + (c & 0x1f));
            } else {
                _CAST_VALUE_FUNC(read_number, -32 + (c & 0x1f));
            }
        } else {
            _CAST_VALUE_FUNC(read_error, ERROR_UNKNOWN);
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
    if (msgpk == NULL)
        return -1;

    if (size > 15) {
        _CAST_VALUE_FUNC(write_error, ERROR_OUT_OF_RANGE);
        return -1;
    }

    _CAST_VALUE_FUNC(writer, FIXMAP_VAL + size);
    return 0;
}

char msgpk_write_start_array(msgpk_t *msgpk, int size)
{
    if (msgpk == NULL)
        return -1;

    if (size > 15) {
        _CAST_VALUE_FUNC(write_error, ERROR_OUT_OF_RANGE);
        return -1;
    }

    _CAST_VALUE_FUNC(writer, FIXARRAY_VAL + size);
    return 0;
}

void _write_fixstr_no_test(msgpk_t *msgpk, const char *val, unsigned char size)
{
    if (msgpk == NULL)
        return;
    if (val == NULL) {
        _CAST_VALUE_FUNC(write_error, ERROR_BAD_PARAMETER);
        return ;
    }

    unsigned char i;
    _CAST_VALUE_FUNC(writer, FIXSTR_VAL + size);

    for (i = 0; i < size; i++) {
        _CAST_VALUE_FUNC(writer, val[i]);
    }
}

char msgpk_write_str(msgpk_t *msgpk, const char *val)
{
    if (msgpk == NULL)
        return -1;
    if (val == NULL) {
        _CAST_VALUE_FUNC(write_error, ERROR_BAD_PARAMETER);
        return -1;
    }

    unsigned char size;
    size = strlen(val);

    if (size <= 31) {
        _write_fixstr_no_test(msgpk, val, size);
        return 0;
    }

    _CAST_VALUE_FUNC(write_error, ERROR_OUT_OF_RANGE);
    return -1;
}

char msgpk_write_str_entry(msgpk_t *msgpk, const char *key, const char *val)
{
    if (msgpk == NULL)
        return -1;
    if (key == NULL) {
        _CAST_VALUE_FUNC(write_error, ERROR_BAD_PARAMETER);
        return -1;
    }
    if (val == NULL) {
        _CAST_VALUE_FUNC(write_error, ERROR_BAD_PARAMETER);
        return -1;
    }

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
    if (msgpk == NULL)
        return -1;

    _CAST_VALUE_FUNC(writer, NIL_VAL);
    return 0;
}

char msgpk_write_nil_entry(msgpk_t *msgpk, const char *key)
{
    if (msgpk == NULL)
        return -1;
    if (key == NULL) {
        _CAST_VALUE_FUNC(write_error, ERROR_BAD_PARAMETER);
        return -1;
    }

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
    if (msgpk == NULL)
        return -1;

    if (val == 0) {
        _CAST_VALUE_FUNC(writer, BOOLEAN_VAL);
    } else {
        _CAST_VALUE_FUNC(writer, BOOLEAN_VAL + 1);
    }

    return 0;
}

char msgpk_write_boolean_entry(msgpk_t *msgpk, const char *key, char val)
{
    if (msgpk == NULL)
        return -1;
    if (key == NULL) {
        _CAST_VALUE_FUNC(write_error, ERROR_BAD_PARAMETER);
        return -1;
    }

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

char msgpk_write_number(msgpk_t *msgpk, long int val)
{
    if (msgpk == NULL)
        return -1;

    if (val < 0) {
        /* Negative number */
        if (val >= MSGPACK_NFIXINT_MIN) {
            _CAST_VALUE_FUNC(writer, NFIXINT_VAL + val + 32);
        } else if (val >= MSGPACK_INT8_MIN) {
            _CAST_VALUE_FUNC(writer, INT8_VAL);
            _CAST_VALUE_FUNC(writer, (int8_t)val);
        } else if (val >= MSGPACK_INT16_MIN) {
            _CAST_VALUE_FUNC(writer, INT16_VAL);
            _CAST_VALUE_FUNC(writer, (int8_t)((val >> 8) & 0xFF));
            _CAST_VALUE_FUNC(writer, (int8_t)(val & 0xFF));
        } else if (val >= MSGPACK_INT32_MIN) {
            _CAST_VALUE_FUNC(writer, INT32_VAL);
            _CAST_VALUE_FUNC(writer, (int8_t)((val >> 24) & 0xFF));
            _CAST_VALUE_FUNC(writer, (int8_t)((val >> 16) & 0xFF));
            _CAST_VALUE_FUNC(writer, (int8_t)((val >> 8) & 0xFF));
            _CAST_VALUE_FUNC(writer, (int8_t)(val & 0xFF));
        } else {
            _CAST_VALUE_FUNC(write_error, ERROR_OUT_OF_RANGE);
            return -1;
        }
    } else {
        /* Positive number */
        if ((unsigned)val <= MSGPACK_PFIXINT_MAX) {
            _CAST_VALUE_FUNC(writer, val);
        } else if ((unsigned)val <= MSGPACK_UINT8_MAX) {
            _CAST_VALUE_FUNC(writer, UINT8_VAL);
            _CAST_VALUE_FUNC(writer, (uint8_t)val);
        } else if ((unsigned)val <= MSGPACK_UINT16_MAX) {
            _CAST_VALUE_FUNC(writer, UINT16_VAL);
            _CAST_VALUE_FUNC(writer, (uint8_t)((val >> 8) & 0xFF));
            _CAST_VALUE_FUNC(writer, (uint8_t)val);
        } else if ((unsigned)val <= MSGPACK_UINT32_MAX) {
            _CAST_VALUE_FUNC(writer, UINT32_VAL);
            _CAST_VALUE_FUNC(writer, (uint8_t)((val >> 24) & 0xFF));
            _CAST_VALUE_FUNC(writer, (uint8_t)((val >> 16) & 0xFF));
            _CAST_VALUE_FUNC(writer, (uint8_t)((val >> 8) & 0xFF));
            _CAST_VALUE_FUNC(writer, (uint8_t)val);
        } else {
            _CAST_VALUE_FUNC(write_error, ERROR_OUT_OF_RANGE);
            return -1;
        }
    }

    return 0;
}

char msgpk_write_number_entry(msgpk_t *msgpk, const char *key, long int val)
{
    if (msgpk == NULL)
        return -1;
    if (key == NULL) {
        _CAST_VALUE_FUNC(write_error, ERROR_BAD_PARAMETER);
        return -1;
    }

    unsigned char k_size;
    k_size = strlen(key);

    if (k_size <= 31) {
        _write_fixstr_no_test(msgpk, key, k_size);
        msgpk_write_number(msgpk, val);
    }

    _CAST_VALUE_FUNC(write_error, ERROR_OUT_OF_RANGE);
    return -1;
}

#endif
