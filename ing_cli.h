
#ifndef ING_CLI
#define ING_CLI

#define ING_CORE_IMPL

#include "ing_tok.h"

/* typedef Ing_Vector(Ing_String) Ing_Args; */
typedef Ing_String* Ing_Args;

typedef enum {
    Ing_Expected_String,
    Ing_Expected_Integer,
    Ing_Expected_Real,
    Ing_Expected_List,
} Ing_Expected_Kind;

typedef enum {
    Ing_Expected_None,
    Ing_Expected_One,
    Ing_Expected_Multiple,
} Ing_Expected_Count;

typedef struct Ing_Expected_Value {
    Ing_Expected_Count  count;
    union {
        Ing_Expected_Kind           kind;
        struct Ing_Expected_Value*  subtype;
    } value;
} Ing_Expected_Value;

typedef struct {
    Ing_String_Dyn  name_short;
    Ing_String_Dyn  name_long;
} Ing_Arg_Name;

ing_define Ing_Arg_Name ing_arg_name_make(Ing_String_Dyn name_short, Ing_String_Dyn name_long);

typedef enum {
    Ing_Maybe_Kind_Cmd,
    Ing_Maybe_Kind_Arg,
    Ing_Maybe_Kind_Flag,
} Ing_Maybe_Kind;

typedef struct {
    Ing_Maybe_Kind      kind;
    Ing_Arg_Name        name;
    Ing_Expected_Value  value;
} Ing_Maybe;

ing_define Ing_Maybe ing_maybe_flag(Ing_String name_long, Ing_String name_short);
ing_define Ing_Maybe ing_maybe_arg(Ing_String name_long, Ing_String name_short, name, Ing_Expected_Value value);
ing_define Ing_Maybe ing_maybe_cmd(Ing_Arg_Name name);

typedef struct {
    Ing_Args    args;
    Ing_Usize   pos;
} Ing_Arg_Parser;

ing_define Ing_Arg_Parser ing_arg_parser_make(Ing_String* args);

typedef Ing_Result() Ing_AP_Result;

#define ING_PARSE_IMPL
#ifdef ING_PARSE_IMPL

ing_define Ing_Arg_Name ing_arg_name_make(Ing_String_Dyn name_short, Ing_String_Dyn name_long)
{
    return (Ing_Arg_Name) {
        .name_short = name_short,
        .name_long  = name_long,
    };
}

ing_define Ing_Maybe ing_maybe_flag(Ing_String name_long, Ing_String name_short)
{
    return (Ing_Maybe) {
        .kind   = Ing_Maybe_Kind_Flag,
        .name   = ing_arg_name_make(name_long, name_short),
        .value  = { .count = Ing_Expected_None }
    };
}

ing_define Ing_Maybe ing_maybe_arg(Ing_String name_long, Ing_String name_short, Ing_Expected_Value value)
{
    return (Ing_Maybe) {
        .kind   = Ing_Maybe_Kind_Arg,
        .name   = ing_arg_name_make(name_long, name_short),
        .value  = value,
    };
}

ing_define Ing_Maybe ing_maybe_cmd(Ing_String name_long, Ing_String name_short)
{
    return (Ing_Maybe) {
        .kind   = Ing_Maybe_Kind_Cmd,
        .name   = ing_arg_name_make(name_long, name_short),
        .value  = { .count = Ing_Expected_None }
    };
}

ing_define Ing_Arg_Parser ing_arg_parser_make(Ing_String* args)
{
    return (Ing_Arg_Parser) {
        .args   = args,
        .pos    = 0,
    };
}

#endif
#endif
