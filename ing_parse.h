#include "ing.h"
#define ING_STRIP_PREFIX

#define ING_CORE_IMPL

/* #include "ing_core.h" */
#define ING_TOK_IMPL
#include "ing_tok.h"

#ifndef ING_PARSE
#define ING_PARSE

typedef void (*Ing_Error_Handler)(Ing_Origin origin, Ing_String_Dyn msg);

#define ING_ERROR_HANDLER(name)                         \
    void name(Ing_Origin origin, Ing_String_Dyn msg)    \

ing_define ING_ERROR_HANDLER(ing_default_error_handler);

typedef struct {
    Ing_Tokenizer       tzer;
    Ing_Error_Handler   error;
    Ing_Allocator       alloc;
} Ing_Parser;

typedef struct {

} Ing_Parse_Error;

typedef Ing_Result(Ing_Token, Ing_None_Kind) Ing_Parse_Result;
typedef Ing_View(Ing_Token_Kind) Ing_Token_Kinds;

#define ing_kinds(...) cast(Ing_Token_Kinds)ing_view_make(Ing_Token_Kind, __VA_ARGS__)

ing_define Ing_Parser       ing_parser_make(Ing_Tokenizer tzer);
ing_define Ing_Parse_Result ing_parser_assert(Ing_Parser* self, Ing_Token_Kind kind);
ing_define Ing_Parse_Result ing_parser_oneof(Ing_Parser* self, Ing_Token_Kinds kinds);

// typedef struct {
//     Ing_U8 left_bp;
//     Ing_U8 right_bp;
// } Ing_Binding_Power;

// ing_define Ing_Binding_Power ing_prefix_binding_power(Ing_U8 op);
// ing_define Ing_Binding_Power ing_postfix_binding_power(Ing_U8 op);
// ing_define Ing_Binding_Power ing_infix_binding_power(Ing_U8 op);

#define ING_PARSE_IMPL
#ifdef ING_PARSE_IMPL

ing_define Ing_Parser ing_parser_make(Ing_Tokenizer tzer)
{
    return (Ing_Parser) {
        .tzer   = tzer,
        .error  = ing_default_error_handler,
        .alloc  = tzer.cfg.alloc
    };
}

ing_define Ing_Parse_Result ing_parser_assert(Ing_Parser* self, Ing_Token_Kind kind)
{
    Ing_Token next = ing_tzer_next(&self->tzer);
    if (next.kind != kind)
    {
        Ing_String_Dyn msg = ing_string_dyn_empty(self->alloc);
        ing_string_dyn_push_many(&msg, "invalid token, expected (");
        ing_string_dyn_push_many(&msg, ing_token_kind_name(kind));
        ing_string_dyn_push_many(&msg, "), got (");
        ing_string_dyn_push_many(&msg, ing_token_kind_name(next.kind));
        ing_string_dyn_push_many(&msg, ").");
        Ing_Origin origin = ing_tzer_origin(&self->tzer, next.pos);
        self->error(origin, msg);
        return ing_err(Ing_None);
    }

    return ing_ok(next);
}

ing_define Ing_Parse_Result ing_parser_oneof(Ing_Parser* self, Ing_Token_Kinds kinds)
{
    Ing_Token next = ing_tzer_next(&self->tzer);

    for (Ing_Usize ix = 0; ix < kinds.len; ix++)
    {
        Ing_Token_Kind kind = kinds.data[ix];
        if (next.kind == kind)
        {
            return ing_ok(next);
        }
    }

    Ing_String_Dyn msg = ing_string_dyn_empty(self->alloc);
    ing_string_dyn_push_many(&msg, "invalid token, expected one of (");

    for (Ing_Usize ix = 0; ix < kinds.len; ix++)
    {
        Ing_Token_Kind kind = kinds.data[ix];
        if (ix != 0) ing_string_dyn_push_many(&msg, " |");
        ing_string_dyn_push_many(&msg, ing_token_kind_name(kind));
    }
    ing_string_dyn_push_many(&msg, "), got (");
    ing_string_dyn_push_many(&msg, ing_token_kind_name(next.kind));
    ing_string_dyn_push_many(&msg, ").");
    self->error(ing_tzer_origin(&self->tzer, next.pos), msg);
    return ing_err(Ing_None);
}

// ing_define Ing_Binding_Power ing_infix_binding_power(Ing_U8 op)
// {
//     switch (op) {
//         case '+': case '-': return { left_bp = 1, right_bp = 2 };
//         case '*': case '/': return { left_bp = 3, right_bp = 4 };
//         default: ING_PANIC("bad operator.")
//     };
// }

#endif
#endif
