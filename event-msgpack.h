#ifndef EVENT_MSGPACK_H
#define EVENT_MSGPACK_H

#define KEY_SIZE 512

#define OBJ_FLAG   1
#define ARRAY_FLAG 2
#define VAL_FLAG   4

#define PFIXINT_MASQ   0x80
#define PFIXINT_VAL    0x00
#define FIXMAP_MASQ    0xF0
#define FIXMAP_VAL     0x80
#define FIXARRAY_MASQ  0xF0
#define FIXARRAY_VAL   0x90
#define FIXSTR_MASQ    0xE0
#define FIXSTR_VAL     0xA0
#define NIL_MASQ       0xFF
#define NIL_VAL        0xC0
#define BOOLEAN_MASQ   0xFE
#define BOOLEAN_VAL    0xC2
#define NFIXINT_MASQ   0xE0
#define NFIXINT_VAL    0xE0

#define _IS(_a, _b, _c) ((_a & _b) == _c)

#define IS_PFIXINT(_a)   _IS(_a, PFIXINT_MASQ , PFIXINT_VAL)
#define IS_FIXMAP(_a)    _IS(_a, FIXMAP_MASQ  , FIXMAP_VAL)
#define IS_FIXARRAY(_a)  _IS(_a, FIXARRAY_MASQ, FIXARRAY_VAL)
#define IS_FIXSTR(_a)    _IS(_a, FIXSTR_MASQ  , FIXSTR_VAL)
#define IS_NIL(_a)       _IS(_a, NIL_MASQ     , NIL_VAL)
#define IS_BOOLEAN(_a)   _IS(_a, BOOLEAN_MASQ , BOOLEAN_VAL)
#define IS_NFIXINT(_a)   _IS(_a, NFIXINT_MASQ , NFIXINT_VAL)

#define ERROR_UNKNOWN 1

struct vtable {
    char (*read_start_map)(int);
    char (*read_stop_map)();

    char (*read_start_array)(int);
    char (*read_stop_array)();

    char (*read_str)(char*);
    char (*read_str_entry)(char*,char*);

    char (*read_nil)();
    char (*read_nil_entry)(char*);

    char (*read_boolean)(char);
    char (*read_boolean_entry)(char*,char);

    char (*read_number)(int);
    char (*read_number_entry)(char*,int);

    void (*read_error)(char);
};

typedef struct msgpk_s {
    struct vtable vt;
    char key[KEY_SIZE];
    uint8_t flags;
    int8_t map_el_cnt;
    int8_t array_el_cnt;
} msgpk_t;


#ifdef __cplusplus
extern "C" {
#endif

void msgpk_init(msgpk_t *msgpk);
void msgpk_do(msgpk_t *msgpk, char *cs);

#ifdef __cplusplus
}
#endif

#endif //EVENT_MSGPACK_H
