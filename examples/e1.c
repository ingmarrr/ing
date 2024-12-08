
#include <stdio.h>
#include <string.h>

#define ING_STRIP_PREFIX

#define ING_CORE_IMPL
#define ING_TOK_IMPL

// #include "../ing_core.h"
#include "../ing_tok.h"

// static Ing_Intern_Table PY_TABLE = {0};
// static Ing_Transition_Matrix PY_TRANSITIONS = {0};

Ing_Define_Config(Py_Config);

Ing_Define_Keyword(If,      "if",   &Py_Config);
Ing_Define_Keyword(Def,     "def",  &Py_Config);
Ing_Define_Keyword(Pass,    "pass", &Py_Config);

Ing_Define_Symbol           (Colon,                 ':',                &Py_Config);
Ing_Define_Symbol           (Left_Paren,            '(',                &Py_Config);
Ing_Define_Symbol           (Right_Paren,           ')',                &Py_Config);
Ing_Define_Compound         (Equal_Equal,           "==",               &Py_Config);
Ing_Define_Compound         (Thin_Arrow_Right,      "->",               &Py_Config);
Ing_Define_Wrap             (Comment,               "#", "\n",          &Py_Config);
Ing_Define_Wrap_Disallowed  (Double_Quote_String,   "\"", "\n" ,"\"",   &Py_Config);
Ing_Define_Wrap_Disallowed  (Single_Quote_String,   "'", "\n" ,"'",     &Py_Config);


i32 main()
{
    #define TEST_FILE "./examples/data/t1.py"

    ING_ASSERT(ing_path_is_file(TEST_FILE), "file does not exist");

    Ing_Allocator alloc = ing_heap_allocator();
    Ing_Read_Result src = ing_file_read(alloc, TEST_FILE);
    ING_ASSERT(src.kind == Ing_Ok, "failed reading file contents.");

    Ing_String_View view = ing_string_dyn_view(&src.val.ok);

    Py_Config.ws_policy = Ing_Ws_Ignore;
    Ing_Tokenizer tzer = ing_tzer_make(view, Py_Config);

    Ing_Vector(Token) tokens = {0};
    ing_vec_init(alloc, tokens);

    loop: for (;;)
    {
        Token token = tzer_next(&tzer);
        ing_logf(Ing_Log_Info,
            "token: (kind = (%u/%s), pos = %d)",
            token.kind, ing_token_name(token), token.pos
        );
        switch (token.kind) {
            case Ing_Token_Kind_Unknown: {
                ing_logf(Ing_Log_Error, "unknown token: %d", token.kind);
            } continue;
            case Ing_Token_Kind_Eof: {
                goto defer;
            } break;
            default: {
                ing_vec_push(&tokens, token);
            } continue;
        }
    }

defer:
    return 0;
}
