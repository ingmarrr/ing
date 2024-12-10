#define ING_STRIP_PREFIX

#define ING_CORE_IMPL

#include "ing_core.h"

#ifndef ING_TOK
#define ING_TOK


/// CLS     TOKEN_KIND
/// 000 |   0_0000_0000_0000
#define ING_TOKEN_KIND_CLASS_SHIFT 13
#define ING_MAX_NUM_TOKEN_KINDS         1 << (ING_TOKEN_KIND_CLASS_SHIFT - 1)
#define ING_INTERN_TABLE_SIZE           4096
#define ING_INTERN_TABLE_LOAD_FACTOR    0.75

typedef Ing_U32 (*Ing_Hash_Func)(const char*);

#define ING_FNV_PRIME 16777619
#define ING_FNV_BASIS 2166136261
ing_define Ing_U32 ing_fnv_1a(const Ing_String_View str)
{
    Ing_U32 hash = ING_FNV_BASIS;
    for (Ing_Usize ix = 0; ix < str.len; ix++)
    {
        hash ^= ING_FNV_PRIME;
        hash *= cast(Ing_U8)str.data[ix];
    }
    return hash;
}

#define ING_HASH_FUNC           ing_fnv_1a
#define ING_LOAD_FACTOR         0.75

typedef struct {
    Ing_Umbra_String    str;
    Ing_Usize           token_kind;
} Ing_Intern_Entry;

typedef Ing_Vector(Ing_Intern_Entry) Ing_Intern_Table;
typedef Ing_Result(Ing_Usize, Ing_None_Kind) Ing_Lookup_Result;
ING_STATIC_ASSERT_SIZE(Ing_Lookup_Result, 16); // Maybe reduce Ing_Result(..) to Ing_U32 to have size = 8
typedef struct {
    Ing_Usize   cap;
    Ing_Usize   count;
    Ing_F64     load_factor;
    Ing_Usize   collisions;
} Ing_Intern_Stats;

ing_define void                 ing_intern_resize(Ing_Intern_Table* table, Ing_Usize new_cap);
ing_define Ing_Usize            ing_intern_probe_slot(Ing_U32 hash, Ing_Usize index, Ing_Usize cap);
ing_define void                 ing_intern_put(Ing_Intern_Table* table, const Ing_String_View str, Ing_Usize token_kind);
ing_define Ing_Lookup_Result    ing_intern_lookup(Ing_Intern_Table* table, const Ing_String_View str);

ing_define Ing_Intern_Stats     ing_intern_stats(Ing_Intern_Table* table);
ing_define Ing_String_Vector    ing_interned_strings(Ing_Intern_Table* table);

typedef Ing_U16 Ing_Token_Kind;

typedef struct {
    Ing_B8          has_next;
    Ing_Token_Kind  token_kind;
    Ing_U16         next;
} Ing_Transition;
ING_STATIC_ASSERT_SIZE(Ing_Transition, 6);

typedef Ing_Vector(Ing_Transition)          Ing_Transition_Vector;
typedef Ing_Vector(Ing_Transition_Vector)   Ing_Transition_Matrix;
typedef Ing_Result(Ing_Token_Kind, Ing_None_Kind)    Ing_Transition_Result;

ing_define Ing_Transition ing_transition_make_accepting(Ing_Token_Kind token_kind);
ing_define Ing_Transition ing_transition_make_forwarding(Ing_U16 next);

static Ing_String               ING_TOKEN_KIND_NAMES[ING_MAX_NUM_TOKEN_KINDS] = {0};
// static Ing_Transition           ING_SYMBOLS[256] = {0};

#define Ing_Define_Config(__name)                                                               \
    static Ing_Tokenizer_Config __name = {0};                                                   \
    __attribute__((constructor))                                                                \
    static void Ing_Init_Config_##__name(void) {                                                \
        (__name).alloc      = ing_heap_allocator();                                             \
        Ing_Transition_Matrix __transitions = {0};                                              \
        ing_vec_init((__name).alloc, __transitions);                                            \
        (__name).transitions = (__transitions);                                                 \
        ing_vec_init((__name).alloc, (__name).transitions);                                     \
    }

#define Ing_Define_Keyword(__name, __value, __cfg)                                              \
    enum { Ing_Token_Kind_##__name = __COUNTER__ };                                             \
    enum { Ing_##__name = Ing_Token_Kind_##__name };                                            \
    __attribute__((constructor))                                                                \
    static void Ing_Register_Keyword_##__name(void) {                                           \
        static char str_##__name[] = #__name;                                                   \
        ING_TOKEN_KIND_NAMES[Ing_Token_Kind_##__name] = str_##__name;                           \
        Ing_String_View view = ing_string_view_from(                                            \
            cast(Ing_String)__value,                                                            \
            ing_strlen(cast(Ing_String)__value));                                               \
        ing_intern_put(&(__cfg)->itable, view, Ing_Token_Kind_##__name);                        \
    }

#define Ing_Define_Symbol(__name, __value, __cfg)                                               \
    enum { Ing_Token_Kind_##__name = __COUNTER__ };                                             \
    enum { Ing_##__name = Ing_Token_Kind_##__name };                                            \
    __attribute__((constructor))                                                                \
    static void Ing_Register_Symbol_##__name(void) {                                            \
        ing_register_symbol(                                                                    \
            Ing_Token_Kind_##__name,                                                            \
            cast(const Ing_String)#__name,                                                      \
            cast(const Ing_U8)__value,                                                          \
            __cfg                                                                               \
        );                                                                                      \
    }

#define Ing_Define_Compound(__name, __values, __cfg)                                            \
    enum { Ing_Token_Kind_##__name = __COUNTER__ };                                             \
    enum { Ing_##__name = Ing_Token_Kind_##__name };                                            \
    __attribute__((constructor))                                                                \
    static void Ing_Register_Compound_##__name(void) {                                          \
        ing_register_compound(                                                                  \
            Ing_Token_Kind_##__name,                                                            \
            cast(const Ing_String)#__name,                                                      \
            cast(const Ing_String)__values,                                                     \
            __cfg                                                                               \
        );                                                                                      \
    }

#define Ing_Define_Wrap_Disallowed(__name, __initiator, __disallowed, __terminator, __cfg)      \
    enum { Ing_Token_Kind_##__name = __COUNTER__ };                                             \
    enum { Ing_##__name = Ing_Token_Kind_##__name };                                            \
    __attribute__((constructor))                                                                \
    static void Ing_Register_Wrap_Disallowed_##__name(void) {                                   \
        ing_register_wrap_disallowed(                                                           \
            Ing_Token_Kind_##__name,                                                            \
            cast(const Ing_String)#__name,                                                      \
            cast(const Ing_String)__initiator,                                                  \
            cast(const Ing_String)__disallowed,                                                 \
            cast(const Ing_String)__terminator,                                                 \
            __cfg                                                                               \
        );                                                                                      \
    }

#define Ing_Define_Wrap(__name, __initiator, __terminator, __cfg)                               \
    enum { Ing_Token_Kind_##__name = __COUNTER__ };                                             \
    enum { Ing_##__name = Ing_Token_Kind_##__name };                                            \
    __attribute((constructor))                                                                  \
    static void Ing_Register_Wrap_##__name(void) {                                              \
        ing_register_wrap_any(                                                                  \
            Ing_Token_Kind_##__name,                                                            \
            cast(const Ing_String)#__name,                                                      \
            cast(const Ing_String)__initiator,                                                  \
            cast(const Ing_String)__terminator,                                                 \
            __cfg                                                                               \
        );                                                                                      \
    }

#define Ing_Define_Token_Kind(__name)                                                           \
    enum { Ing_Token_Kind_##__name = __COUNTER__ };                                             \
    enum { Ing_##__name = Ing_Token_Kind_##__name };                                            \
    __attribute__((constructor))                                                                \
    static void Ing_Register_Token_##__name(void) {                                             \
        static char str_##__name[] = #__name;                                                   \
        ING_TOKEN_KIND_NAMES[Ing_Token_Kind_##__name] = str_##__name;                           \
    }


ing_define Ing_String ing_token_kind_name(Ing_Usize kind)
{
    return ING_TOKEN_KIND_NAMES[kind];
}

Ing_Define_Token_Kind(Unknown)
Ing_Define_Token_Kind(Invalid)
Ing_Define_Token_Kind(Any)
Ing_Define_Token_Kind(Ident)
// Ing_Define_Token_Kind(String)
Ing_Define_Token_Kind(Int)
Ing_Define_Token_Kind(Real)

Ing_Define_Token_Kind(Space)
Ing_Define_Token_Kind(Tab)
Ing_Define_Token_Kind(Newline)
Ing_Define_Token_Kind(Eof)

typedef struct {
    Ing_Token_Kind  kind;
    u32             pos;
} Ing_Token;

typedef struct {
    u32 start;
    u32 end;
} Ing_Span;

typedef struct {
    u32 line;
    u32 col;
} Ing_Loc;

typedef struct {
    Ing_Span        span;
    Ing_Loc         loc;
    Ing_String_View file;
} Ing_Origin;

typedef enum {
    Ing_Comment_None                        = 0,
    Ing_Comment_Double_Slash                = 1 << 0,   // //
    Ing_Comment_Triple_Slash                = 1 << 1,   // ///
    Ing_Comment_Double_Slash_Bang           = 1 << 2,   // //!
    Ing_Comment_Pound                       = 1 << 3,   // #
    Ing_Comment_Double_Minus                = 1 << 4,   // --
    Ing_Comment_Double_Semi                 = 1 << 5,   // ;;
} Ing_Comment_Policy;

typedef enum {
    Ing_Ws_Ignore           = 0,
    Ing_Ws_Collapse         = 1 << 0,
    Ing_Ws_Track_Indent     = 1 << 1,
    Ing_Ws_Tabs_As_Spaces   = 1 << 2,
    Ing_Ws_Spaces_As_Tabs   = 1 << 3,
    Ing_Ws_Newline_Token    = 1 << 4,
} Ing_Whitespace_Policy;

typedef enum {
    Ing_Allow_Alpha_Num_Ident,
    Ing_Allow_Underscore_Ident,
    Ing_Allow_Utf8_Idents,
    Ing_Allow_Utf8_Symbols,
} Ing_Allow_Token_Policy;

typedef enum {
    Ing_Policy_State_Not_Set,
    Ing_Policy_State_Allowed,
    Ing_Policy_State_Not_Allowed,
} Ing_Policy_State;

typedef Ing_Token_Kind (*Ing_Keyword_Handler)(const Ing_String ident);

typedef struct {
    Ing_Allocator           alloc;
    Ing_Transition          symbols[256];
    Ing_Transition_Matrix   transitions;
    Ing_Intern_Table        itable;

    /// TODO(ingmar): Maybe intern the token kinds into the config
    Ing_String              token_kinds_names[ING_MAX_NUM_TOKEN_KINDS];
    usize                   token_kind_iota;

    usize   ws_policy;
} Ing_Tokenizer_Config;

typedef struct {
    usize                   pos;
    usize                   line;
    usize                   lastnewline;
    Ing_String_View         src;
    Ing_Tokenizer_Config    cfg;
    Ing_String_View         file;
} Ing_Tokenizer;

ing_define Ing_Tokenizer    ing_tzer_make(Ing_String_View src, Ing_Tokenizer_Config cfg);
ing_define Ing_Token        ing_tzer_next(Ing_Tokenizer* self);
ing_define void             ing_tzer_checkout(Ing_Tokenizer* self, u32 pos);
ing_define Ing_Span         ing_tzer_span(Ing_Tokenizer* self, u32 pos);
ing_define Ing_Loc          ing_tzer_loc(Ing_Tokenizer* self, u32 pos);
ing_define Ing_Origin       ing_tzer_origin(Ing_Tokenizer* self, u32 pos);

ing_define u8               ing_tzer_peek(const Ing_Tokenizer* self);
ing_define u8               ing_tzer_take(Ing_Tokenizer* self);
ing_define Ing_String_View  ing_tzer_take_slice(Ing_Tokenizer* self, Ing_Usize len);

ing_define Ing_Token        ing_token_make(Ing_Token_Kind kind, u32 pos);
ing_define Ing_String       ing_token_name(Ing_Token token);

ing_define void ing_register_symbol(
    const Ing_Token_Kind    token_kind,
    const Ing_String        name,
    const Ing_U8            symbol,
    Ing_Tokenizer_Config*   cfg
);
ing_define void ing_ensure_state_transition(
    const Ing_String        values,
    const Ing_Usize         values_len,
    Ing_Transition_Vector*  state,
    Ing_Transition*         tran,
    Ing_Tokenizer_Config*   cfg
);
ing_define void ing_register_compound(
    const Ing_Token_Kind    token_kind,
    const Ing_String        name,
    const Ing_String        values,
    Ing_Tokenizer_Config*   cfg
);
ing_define void ing_register_wrap_any(
    const Ing_Token_Kind    token_kind,
    const Ing_String        name,
    const Ing_String        initiator,
    const Ing_String        terminator,
    Ing_Tokenizer_Config*   cfg
);
ing_define void ing_register_wrap_disallowed(
    const Ing_Token_Kind    token_kind,
    const Ing_String        name,
    const Ing_String        initiator,
    const Ing_String        disallowed,
    const Ing_String        terminator,
    Ing_Tokenizer_Config*   cfg
);

#ifndef ING_TOK_STRIP_PREFIX_GUARD_
#define ING_TOK_STRIP_PREFIX_GUARD_

    #ifdef ING_STRIP_PREFIX

        typedef Ing_Origin            Location;
        typedef Ing_Token               Token;

        typedef Ing_Tokenizer_Config    Tokenizer_Config;

        typedef Ing_Tokenizer           Tokenizer;
        #define tzer_make ing_tzer_make
        #define tzer_next ing_tzer_next

        typedef usize                   Token_Kind;

        typedef Ing_Comment_Policy      Comment_Policy;

        typedef Ing_Whitespace_Policy   Whitespace_Policy;


    #endif

#endif
#endif

#define ING_TOK_IMPL
#ifdef ING_TOK_IMPL

#define ING_ASCII_IMPLEMENTATION

ing_define void ing_register_symbol(
    const Ing_Token_Kind    token_kind,
    const Ing_String        name,
    const Ing_U8            symbol,
    Ing_Tokenizer_Config*   cfg
) {
    ING_TOKEN_KIND_NAMES[token_kind] = name;
    Ing_Transition* tran = &cfg->symbols[symbol];
    ING_ASSERT(
        tran->token_kind == Ing_Token_Kind_Unknown,
        "symbol '%c' (%s) is already defined.",
        symbol, name);
    *tran = ing_transition_make_accepting(token_kind);
}

/// Ensures there is a transition sequence with valid states
/// from a starting character ⍺ of a compound literal (⍺βγ) to β.
/// What is done with the last state γ is purposefully left
/// up to the caller, as depending on the purpose it might
/// serve a different purpose.
ing_define void ing_ensure_state_transition(
    const Ing_String        values,
    const Ing_Usize         values_len,
    Ing_Transition_Vector*  state,
    Ing_Transition*         tran,
    Ing_Tokenizer_Config*   cfg
) {
    // Ensure the initial character ⍺ has a valid transition
    // into the state transition matrix
    if (!(tran->has_next)) {
        Ing_U16 next = cast(Ing_U16)cfg->transitions.len;
        Ing_Transition_Vector new_state = {0};
        ing_vec_reserve(cfg->alloc, new_state, 256);
        ing_vec_push(&cfg->transitions, new_state);
        tran->has_next  = 1;
        tran->next      = next;
    }

    *state = cfg->transitions.data[tran->next];
    for (Ing_Usize ix = 1; ix < values_len - 1; ix++) {
        tran = &state->data[cast(Ing_U8)values[ix]];
        if (tran->has_next) {
            state = &cfg->transitions.data[tran->next];
            continue;
        }
        Ing_Usize new_state_id = cfg->transitions.len;
        Ing_Transition_Vector new_state = {0};
        ing_vec_reserve(cfg->alloc, new_state, 256);
        ing_vec_push(&cfg->transitions, new_state);
        *tran = ing_transition_make_forwarding(cast(Ing_U16)new_state_id);
        *state = cfg->transitions.data[tran->next];
    }
}

ing_define void ing_register_compound(
    const Ing_Token_Kind    token_kind,
    const Ing_String        name,
    const Ing_String        values,
    Ing_Tokenizer_Config*   cfg
) {
    ING_TOKEN_KIND_NAMES[token_kind] = name;
    Ing_Usize values_len = ing_strlen(values);
    ING_ASSERT(values_len > 0, "compound literal must cannot be empty");
    if (values_len == 1)
    {
        Ing_Transition* tran = &(cfg)->symbols[cast(Ing_U8)(values)[0]];
        ING_ASSERT(tran->token_kind == Ing_Token_Kind_Unknown,
            "token '%c' (%s) already defined",
            (values[0]), ing_token_kind_name(tran->token_kind));
        *tran = ing_transition_make_accepting(token_kind);
        return;
    }
    Ing_Transition* tran = &((cfg)->symbols[cast(Ing_U8)values[0]]);
    Ing_Transition_Vector state = {0};
    ing_ensure_state_transition(values, values_len, &state, tran, cfg);
    Ing_U8 last_char = cast(Ing_U8)values[values_len-1];
    ING_ASSERT(last_char != '\0', "must be a valid character");
    Ing_Transition* last = &state.data[last_char];
    ING_ASSERT(last->token_kind == Ing_Token_Kind_Unknown,
        "compound: '%s' (%s) already defined",
        values, ing_token_kind_name(last->token_kind)
    );
    last->token_kind = token_kind;
}

ing_define void ing_register_wrap_any(
    const Ing_Token_Kind    token_kind,
    const Ing_String        name,
    const Ing_String        initiator,
    const Ing_String        terminator,
    Ing_Tokenizer_Config*   cfg
) {
    ing_register_wrap_disallowed(token_kind, name, initiator, "", terminator, cfg);
}

ing_define void ing_register_wrap_disallowed(
    const Ing_Token_Kind    token_kind,
    const Ing_String        name,
    const Ing_String        initiator,
    const Ing_String        disallowed,
    const Ing_String        terminator,
    Ing_Tokenizer_Config*   cfg
) {
    ING_TOKEN_KIND_NAMES[token_kind] = name;
    Ing_Usize initiator_len     = ing_strlen(initiator);
    Ing_Usize terminator_len    = ing_strlen(terminator);
    Ing_Usize disallowed_len    = ing_strlen(disallowed);
    ING_ASSERT(initiator_len > 0, "initiator sequence cannot be empty");
    ING_ASSERT(terminator_len > 0, "terminator sequence cannot be empty");

    Ing_Transition* initiator_tran = &cfg->symbols[cast(Ing_U8)initiator[0]];
    Ing_Usize intermediate_state_id = cfg->transitions.len;
    Ing_Transition_Vector intermediate_state = {0};
    ing_vec_reserve(cfg->alloc, intermediate_state, 256);
    ing_vec_push(&cfg->transitions, intermediate_state);
    Ing_Usize terminating_state_id  = cfg->transitions.len;
    Ing_Transition_Vector terminating_state = {0};
    ing_vec_reserve(cfg->alloc, terminating_state, 256);

    if (initiator_len == 1) {
        ING_ASSERT(initiator_tran->token_kind == Ing_Token_Kind_Unknown,
            "token '%c' (%s) already defined",
            initiator[0], ing_token_kind_name(initiator_tran->token_kind));
        *initiator_tran = ing_transition_make_forwarding(cast(Ing_U16)intermediate_state_id);
    } else {
        Ing_Transition_Vector last_initiating_state = {0};
        ing_ensure_state_transition(initiator, initiator_len, &last_initiating_state, initiator_tran, cfg);
        Ing_U8 last_initiator_char = cast(Ing_U8)initiator[initiator_len];
        initiator_tran = &last_initiating_state.data[last_initiator_char];
        *initiator_tran = ing_transition_make_forwarding(cast(Ing_U16)intermediate_state_id);
    }

    for (Ing_Usize ix = 0; ix < 256; ix++)
    {
        Ing_Transition* itran = &intermediate_state.data[ix];
        itran->has_next = 1;
        itran->next     = cast(Ing_U16)intermediate_state_id;
    }
    // TODO(ingmar): ING_ASSERT(ing_vec_intersection(disallowed, terminating).len == 0);
    for (Ing_Usize dix = 0; dix < disallowed_len; dix++)
    {
        intermediate_state.data[cast(Ing_U8)disallowed[dix]] = ing_transition_make_accepting(Ing_Token_Kind_Invalid);
    }

    Ing_Transition terminator_tran = ing_transition_make_forwarding(cast(Ing_U16)terminating_state_id);
    if (terminator_len == 1) {
        terminator_tran = ing_transition_make_accepting(token_kind);
    } else {
        for (Ing_Usize tix = 1; tix < terminator_len; tix++)
        {
            terminator_tran = terminating_state.data[cast(Ing_U8)terminator[tix]];
            if (terminator_tran.has_next && terminator_tran.next != intermediate_state_id) {
                terminating_state = cfg->transitions.data[terminator_tran.next];
                continue;
            }

            Ing_Usize new_state_id = cfg->transitions.len;
            Ing_Transition_Vector new_state = {0};
            ing_vec_reserve(cfg->alloc, new_state, 256);
            for (Ing_Usize ix = 0; ix < 256; ix++)
            {
                Ing_Transition partial_terminator_tran = new_state.data[ix];
                partial_terminator_tran.has_next = 1;
                partial_terminator_tran.next     = cast(Ing_U16)intermediate_state_id;
            }
            ing_vec_push(&cfg->transitions, new_state);
            terminator_tran = ing_transition_make_forwarding(cast(Ing_U16)new_state_id);
            terminating_state = cfg->transitions.data[terminator_tran.next];
        }
    }
    Ing_U8 last_terminator_char = cast(Ing_U8)terminator[terminator_len-1];
    Ing_Transition* last = &terminating_state.data[last_terminator_char];
    *last = ing_transition_make_accepting(token_kind);

    intermediate_state.data[cast(Ing_U8)terminator[0]] = terminator_tran;
}

ing_define void ing_intern_resize(Ing_Intern_Table* table, Ing_Usize new_cap)
{
    ING_ASSERT(table != NULL, "received null ptr to table");
    Ing_Intern_Entry* new_entries = ing_alloc(table->alloc, new_cap);
    ING_ASSERT(new_entries != NULL, "failed allocation of entries");

    for (Ing_Usize ix = 0; ix < table->cap; ix++)
    {
        Ing_Intern_Entry* entry = &table->data[ix];
        if (ing_umbra_is_empty(entry->str)) continue;

        Ing_String_View str = ing_string_view_from(ing_umbra_raw_ptr(&entry->str), ing_umbra_len(entry->str));
        Ing_U32 hash = ING_HASH_FUNC(str);
        for (Ing_Usize jx = 0;;jx++)
        {
            Ing_Usize slot = ing_intern_probe_slot(hash, jx, new_cap);
            if (ing_umbra_is_empty(table->data[slot].str))
            {
                table->data[slot] = *entry;
                break;
            }
        }
    }

    ing_free(table->alloc, table->data);
    table->data = new_entries;
    table->cap  = new_cap;
}

ing_define Ing_Usize ing_intern_probe_slot(Ing_U32 hash, Ing_Usize index, Ing_Usize cap)
{
    return (hash + index) & (cap - 1);
}

ing_define Ing_Lookup_Result ing_intern_lookup(Ing_Intern_Table* table, const Ing_String_View str)
{
    ING_ASSERT(table != NULL, "received null ptr to table");
    Ing_U32 hash = ING_HASH_FUNC(str);
    Ing_Umbra_String ustr = ing_umbra_make_persistent(str.data, str.len);

    Ing_Usize slot  = hash & (table->cap - 1);
    Ing_Usize index = 0;

    while (index < table->cap)
    {
        Ing_Intern_Entry* entry = &table->data[slot];

        if (ing_umbra_is_empty(entry->str))
        {
            break;
        }

        if (ing_umbra_is_eq(entry->str, ustr))
        {
            return (Ing_Lookup_Result) ing_ok(entry->token_kind);
        }

        index++;
        slot = (hash + index) & (table->cap - 1);
    }

    return (Ing_Lookup_Result) ing_err(Ing_None);
}

ing_define void ing_intern_put(Ing_Intern_Table* table, const Ing_String_View str, Ing_Usize token_kind)
{
    ING_ASSERT(table != NULL, "received null ptr to table");
    if (table->data == NULL)
    {
        ing_vec_init(ing_heap_allocator(), *table);
    }
    ING_ASSERT(table->data != NULL, "failed allocating table data");
    ING_ASSERT(table->cap == ING_INIT_CAP, "expected to be ING_INIT_CAP");

    if (table->len >= table->cap * ING_INTERN_TABLE_LOAD_FACTOR)
    {
        ing_intern_resize(table, table->cap * 2);
    }

    Ing_U32 hash = ING_HASH_FUNC(str);
    Ing_Umbra_String ustr = ing_umbra_make_persistent(str.data, str.len);

    Ing_Usize slot  = hash & (table->cap - 1);
    Ing_Usize index = 0;

    while (index < table->cap)
    {
        Ing_Intern_Entry* entry = &table->data[slot];

        if (ing_umbra_is_empty(entry->str))
        {
            entry->str = ustr;
            entry->token_kind = token_kind;
            table->len++;
            return;
        }

        if (ing_umbra_is_eq(entry->str, ustr)) return;

        index++;
        slot = (hash + index) & (table->cap - 1);
    }

    ING_ASSERT(false, "failed finding slot in table");
}

ing_define Ing_Intern_Stats ing_intern_stats(Ing_Intern_Table* table)
{
    Ing_Intern_Stats stats = {
        .cap   = table->cap,
        .count = table->len,
        .load_factor = cast(Ing_F64)table->cap / table->len,
        .collisions  = 0,
    };

    for (Ing_Usize ix = 0; ix < table->cap; ix++)
    {
        Ing_Intern_Entry* entry = &table->data[ix];
        if (!ing_umbra_is_empty(entry->str))
        {
            Ing_U32 len     = ing_umbra_len(entry->str);
            Ing_String cstr = ing_umbra_to_cstring(table->alloc, &entry->str);
            Ing_U32 hash = ING_HASH_FUNC(ing_string_view_from(cstr, len));
            Ing_Usize ideal = hash & (table->cap - 1);
            if (ideal != ix) stats.collisions++;
        }
    }

    return stats;
}

ing_define Ing_String_Vector ing_interned_strings(Ing_Intern_Table* table)
{
    Ing_String_Vector out = {0};
    ing_vec_init(table->alloc, out);

    for (Ing_Usize ix = 0; ix < table->cap; ix++)
    {
        Ing_Intern_Entry entry = table->data[ix];
        if (!ing_umbra_is_empty(entry.str))
        {
            Ing_String_Dyn interned = ing_string_dyn_empty(table->alloc);
            ing_string_dyn_push_many(&interned, ing_token_kind_name(entry.token_kind));
            ing_string_dyn_push(&interned, '(');
            Ing_String lexeme[1024];
            Ing_String lptr = cast(Ing_String)lexeme;
            ING_ASSERT(ing_umbra_to_cstring_buf(&entry.str, lptr, 1024), "lexeme too long (> 1024)");
            ing_string_dyn_push_many(&interned, lptr);
            ing_string_dyn_push(&interned, ')');
            ing_vec_push(&out, interned);
        }
    }

    return out;
}

ing_define Ing_Transition ing_transition_make_accepting(Ing_Token_Kind token_kind)
{
    return (Ing_Transition) {
        .has_next   = 0,
        .token_kind = token_kind,
        .next       = 0,
    };
}

ing_define Ing_Transition ing_transition_make_forwarding(Ing_U16 next)
{
    return (Ing_Transition) {
        .has_next   = 1,
        .token_kind = Ing_Token_Kind_Unknown,
        .next       = next,
    };
}

// ing_define Ing_Transition_Vector ing_transitions(Ing_Transition_Matrix* transitions, Ing_Token_Kind token_kind)
// {
//     return transitions->data[token_kind];
// }

// ing_define Ing_B8 ing_has_transitions(Ing_Transition_Matrix* transitions, Ing_Token_Kind token_kind)
// {
//     return transitions->data[token_kind].len > 0;
// }

// ing_define Ing_Transition_Result ing_next_transition(Ing_Transition_Matrix* transitions, Ing_Token_Kind token_kind, Ing_U8 next)
// {
//     if (!ing_has_transitions(transitions, token_kind)) return (Ing_Transition_Result) ing_err({});
//     Ing_Transition_Vector possibilities = ing_transitions(transitions, token_kind);

//     for (Ing_Usize ix = 0; ix < possibilities.len; ix++)
//     {
//         Ing_Transition transition = possibilities.data[ix];
//         if (transition.match == next)
//         {
//             possibilities.data[ix] = possibilities.data[0];
//             possibilities.data[0]  = transition;
//             return (Ing_Transition_Result) ing_ok(transition.token_kind);
//         }
//     }

//     return (Ing_Transition_Result) ing_err({});
// }

Ing_Tokenizer ing_tzer_make(Ing_String_View src, Ing_Tokenizer_Config cfg)
{
    return (Ing_Tokenizer) {
        .pos    = 0,
        .src    = src,
        .cfg    = cfg,
    };
}

ing_define u8 ing_tzer_peek(const Ing_Tokenizer *self)
{
    if (self->pos >= self->src.len) return '0';
    return self->src.data[self->pos];
}

ing_define u8 ing_tzer_take(Ing_Tokenizer *self)
{
    if (self->pos >= self->src.len) return '0';
    return self->src.data[self->pos++];
}

ing_define Ing_String_View ing_tzer_take_slice(Ing_Tokenizer* self, Ing_Usize len)
{
    ING_ASSERT(self->pos + len <= self->src.len, "too little characters left");
    self->pos += len;
    return ing_string_view_slice(&self->src, self->pos, self->pos + len);
}

ing_define Ing_Token ing_token_make(Ing_Token_Kind kind, u32 pos)
{
    return (Ing_Token) {
        .kind   = kind,
        .pos    = pos,
    };
}

ing_define Ing_String ing_token_name(Ing_Token token)
{
    return ing_token_kind_name(token.kind);
}

static const Ing_Usize ING_ALPHA_BASE_LOWER     = 0x4141414141414141ULL;    // A
static const Ing_Usize ING_ALPHA_BASE_UPPER     = 0x5a5a5a5a5a5a5a5aULL;    // Z
static const Ing_Usize ING_TO_ALPHA_UPPER       = 0x2020202020202020ULL;    // 00100000, remove byte that differentiaces LOWER from UPPER
static const Ing_Usize ING_DIGIT_BASE_LOWER     = 0x3030303030303030ULL;    // 0
static const Ing_Usize ING_DIGIT_BASE_UPPER     = 0x4040404040404040ULL;    // 9
static const Ing_Usize ING_OVERFLOW             = 0x8080808080808080ULL;    // Most significant byte (7)
static const Ing_Usize ING_UNDERSCORE_L             = 0x5e5e5e5e5e5e5e5eULL;
static const Ing_Usize ING_UNDERSCORE_U             = 0x6060606060606060ULL;

ing_define Ing_Usize ing_seek_non_alpha_linear(const Ing_String_View str, Ing_Usize start)
{
    Ing_U8* bytes = cast(Ing_U8*)str.data + start;
    for (Ing_Usize byteix = 0; byteix < str.len; byteix++)
    {
        if (!ing_is_alpha(bytes[byteix])) return byteix;
    }

    return str.len;
}

ing_define Ing_Usize ing_seek_non_alpha(const Ing_String_View str, Ing_Usize start)
{
    if (ING_UNEXPECTED(str.len - start < 8))
    {
        return ing_seek_non_alpha_linear(str, start);
    }

    Ing_U8* bytes = cast(Ing_U8*)str.data + start;

    Ing_Usize first     = *cast(Ing_Usize*)bytes;

    Ing_Usize f_uppercase   = first & ~(ING_TO_ALPHA_UPPER);
    Ing_Usize f_lowerbounds = (f_uppercase - ING_ALPHA_BASE_LOWER) & ING_OVERFLOW;
    Ing_Usize f_upperbounds = (ING_ALPHA_BASE_UPPER - f_uppercase) & ING_OVERFLOW;

    Ing_Usize f_final = f_lowerbounds | f_upperbounds;
    if (f_final)
    {
        return (__builtin_ctzll(f_final) >> 3);
    }

    if (ING_UNEXPECTED(str.len - start < 16))
    {
        return ing_seek_non_alpha_linear(str, start + 8);
    }

    Ing_Usize second = *cast(Ing_Usize*)bytes+8;

    Ing_Usize s_uppercase   = second & ~(ING_TO_ALPHA_UPPER);
    Ing_Usize s_lowerbounds = (s_uppercase - ING_ALPHA_BASE_LOWER) & ING_OVERFLOW;
    Ing_Usize s_upperbounds = (ING_ALPHA_BASE_UPPER - s_uppercase) & ING_OVERFLOW;

    Ing_Usize s_final = s_lowerbounds | s_upperbounds;
    if (s_final)
    {
        return (__builtin_ctzll(s_final) >> 3) + 8;
    }

    return ing_seek_non_alpha_linear(str, start + 16);
}

ing_define Ing_Usize ing_seek_non_digit_linear(const Ing_String_View str, Ing_Usize start)
{
    Ing_U8* bytes = cast(Ing_U8*)str.data + start;
    for (Ing_Usize byteix = 0; byteix < str.len; byteix++)
    {
        if (!ing_is_dec(bytes[byteix])) return byteix;
    }

    return str.len;
}

ing_define Ing_Usize ing_seek_non_digit(const Ing_String_View str, Ing_Usize start)
{
    if (ING_UNEXPECTED(str.len - start < 8))
    {
        return ing_seek_non_digit_linear(str, start);
    }

    Ing_U8* bytes = cast(Ing_U8*)str.data + start;

    Ing_Usize first = *cast(Ing_Usize*)bytes;

    Ing_Usize f_lowerbounds = (first - ING_DIGIT_BASE_LOWER) & ING_OVERFLOW;
    Ing_Usize f_upperbounds = (ING_DIGIT_BASE_UPPER - first) & ING_OVERFLOW;

    Ing_Usize f_final = f_lowerbounds | f_upperbounds;
    if (f_final)
    {
        return (__builtin_ctzll(f_final) >> 3);
    }

    if (ING_UNEXPECTED(str.len - start < 16))
    {
        return ing_seek_non_digit_linear(str, start+8);
    }

    Ing_Usize second = *cast(Ing_Usize*)bytes + 8;

    Ing_Usize s_lowerbounds = (second - ING_DIGIT_BASE_LOWER) & ING_OVERFLOW;
    Ing_Usize s_upperbounds = (ING_DIGIT_BASE_UPPER - second) & ING_OVERFLOW;

    Ing_Usize s_final = s_lowerbounds | s_upperbounds;
    if (s_final)
    {
        return (__builtin_ctzll(s_final) >> 3) + 8;
    }

    return ing_seek_non_digit_linear(str, start+16);
}

ing_define Ing_Usize ing_seek_non_alphanum_linear(const Ing_String_View str, Ing_Usize start)
{
    Ing_U8* bytes = cast(Ing_U8*)str.data + start;
    for (Ing_Usize byteix = 0; byteix < str.len; byteix++)
    {
        Ing_U8 byte = bytes[byteix];
        if (!ing_is_alpha_num(byte)) return byteix;
    }
    return str.len;
}

ing_define Ing_Usize ing_seek_non_alphanum(const Ing_String_View str, Ing_Usize start)
{
    if (ING_UNEXPECTED(str.len - start < 8))
    {
        return ing_seek_non_alphanum_linear(str, start);
    }

    Ing_U8* bytes = cast(Ing_U8*)str.data + start;

    Ing_Usize first = *cast(Ing_Usize*)bytes;

    Ing_Usize f_uppercase   = first & ~(ING_TO_ALPHA_UPPER);
    Ing_Usize f_alpha_lower = (f_uppercase - ING_ALPHA_BASE_LOWER) & ING_OVERFLOW;
    Ing_Usize f_alpha_upper = (ING_ALPHA_BASE_UPPER - f_uppercase) & ING_OVERFLOW;
    Ing_Usize f_digit_lower = (first - ING_DIGIT_BASE_LOWER) & ING_OVERFLOW;
    Ing_Usize f_digit_upper = (ING_DIGIT_BASE_UPPER - first) & ING_OVERFLOW;
    Ing_Usize f_underscore  = ((first - ING_UNDERSCORE_L) | (ING_UNDERSCORE_U - first)) & ING_OVERFLOW;

    Ing_Usize f_final = (f_alpha_lower | f_alpha_upper) & (f_digit_lower | f_digit_upper) & f_underscore;
    if (f_final)
    {
        return (__builtin_ctzll(f_final) >> 3);
    }

    if (ING_UNEXPECTED(str.len - start < 16))
    {
        return ing_seek_non_alphanum_linear(str, start+8);
    }

    Ing_Usize second = *cast(Ing_Usize*)bytes + 8;

    Ing_Usize s_uppercase   = second & ~(ING_TO_ALPHA_UPPER);
    Ing_Usize s_alpha_lower = (s_uppercase - ING_ALPHA_BASE_LOWER) & ING_OVERFLOW;
    Ing_Usize s_alpha_upper = (ING_ALPHA_BASE_UPPER - s_uppercase) & ING_OVERFLOW;
    Ing_Usize s_digit_lower = (second - ING_DIGIT_BASE_LOWER) & ING_OVERFLOW;
    Ing_Usize s_digit_upper = (ING_DIGIT_BASE_UPPER - second) & ING_OVERFLOW;
    Ing_Usize s_underscore  = ((second - ING_UNDERSCORE_L) | (ING_UNDERSCORE_U - second)) & ING_OVERFLOW;

    Ing_Usize s_final = (s_alpha_lower | s_alpha_upper) & (s_digit_lower | s_digit_upper) & s_underscore;
    if (s_final)
    {
        return (__builtin_ctzll(s_final) >> 3) + 8;
    }

    return ing_seek_non_alphanum_linear(str, start+16);
}

ing_define Ing_Usize ing_seek_non_ident_linear(const Ing_String_View str, Ing_Usize start)
{
    Ing_U8* bytes = cast(Ing_U8*)str.data + start;
    for (Ing_Usize byteix = 0; byteix < str.len; byteix++)
    {
        Ing_U8 byte = bytes[byteix];
        if (!ing_is_alpha_num(byte) && byte != '_') return byteix;
    }
    return str.len;
}

ing_define Ing_Usize ing_seek_non_ident(const Ing_String_View str, Ing_Usize start)
{
    if (ING_UNEXPECTED(str.len - start < 8))
    {
        return ing_seek_non_ident_linear(str, start);
    }

    Ing_U8* bytes = cast(Ing_U8*)str.data + start;

    Ing_Usize first = *cast(Ing_Usize*)bytes;

    Ing_Usize f_uppercase   = first & ~(ING_TO_ALPHA_UPPER);
    Ing_Usize f_alpha_lower = (f_uppercase - ING_ALPHA_BASE_LOWER) & ING_OVERFLOW;
    Ing_Usize f_alpha_upper = (ING_ALPHA_BASE_UPPER - f_uppercase) & ING_OVERFLOW;
    Ing_Usize f_digit_lower = (first - ING_DIGIT_BASE_LOWER) & ING_OVERFLOW;
    Ing_Usize f_digit_upper = (ING_DIGIT_BASE_UPPER - first) & ING_OVERFLOW;
    Ing_Usize f_underscore  = ((first - ING_UNDERSCORE_L) | (ING_UNDERSCORE_U - first)) & ING_OVERFLOW;

    Ing_Usize f_final = (f_alpha_lower | f_alpha_upper) & (f_digit_lower | f_digit_upper) & f_underscore;
    if (f_final)
    {
        return (__builtin_ctzll(f_final) >> 3);
    }

    if (ING_UNEXPECTED(str.len - start < 16))
    {
        return ing_seek_non_ident_linear(str, start+8);
    }

    Ing_Usize second = *cast(Ing_Usize*)bytes + 8;

    Ing_Usize s_uppercase   = second & ~(ING_TO_ALPHA_UPPER);
    Ing_Usize s_alpha_lower = (s_uppercase - ING_ALPHA_BASE_LOWER) & ING_OVERFLOW;
    Ing_Usize s_alpha_upper = (ING_ALPHA_BASE_UPPER - s_uppercase) & ING_OVERFLOW;
    Ing_Usize s_digit_lower = (second - ING_DIGIT_BASE_LOWER) & ING_OVERFLOW;
    Ing_Usize s_digit_upper = (ING_DIGIT_BASE_UPPER - second) & ING_OVERFLOW;
    Ing_Usize s_underscore  = ((second - ING_UNDERSCORE_L) | (ING_UNDERSCORE_U - second)) & ING_OVERFLOW;

    Ing_Usize s_final = (s_alpha_lower | s_alpha_upper) & (s_digit_lower | s_digit_upper) & s_underscore;
    if (s_final)
    {
        return (__builtin_ctzll(s_final) >> 3) + 8;
    }

    return ing_seek_non_ident_linear(str, start+16);
}

ing_define Ing_Token ing_tzer_next(Ing_Tokenizer* self)
{
    Ing_Token_Kind kind = Ing_Token_Kind_Unknown;
    u32 start      = self->pos;
    u8  token           = ing_tzer_peek(self);

    if (ing_is_ws(token)) {
        if (ing_is_vert_ws(token))
        {
            self->line++;
            self->lastnewline = self->pos;
        }

        if ((self->cfg.ws_policy | Ing_Ws_Ignore) == 0)
        {
            ing_tzer_take(self);
            while (ing_is_ws(ing_tzer_peek(self))) ing_tzer_take(self);
        }

        if (self->cfg.ws_policy & Ing_Ws_Newline_Token && ing_is_vert_ws(token))
        {
            ing_tzer_take(self);
            return ing_token_make(Ing_Token_Kind_Newline, start);
        }

        if (self->cfg.ws_policy & Ing_Ws_Collapse)
        {
            return ing_token_make(Ing_Token_Kind_Space, start);
        }
    }

    start = self->pos;
    token = ing_tzer_peek(self);

    if (ing_is_alpha(token) || token == '_')
    {
        Ing_Usize end = ing_seek_non_ident(self->src, self->pos);
        // Ing_Usize end = ing_seek_non_ident(self->src, self->pos);
        // ing_info("end: %lu", end);
        self->pos += end;
        Ing_String_View ident = ing_string_view_slice(&self->src, start, start+end);
        Ing_Lookup_Result res = ing_intern_lookup(&self->cfg.itable, ident);

        if (res.kind == Ing_Err)
        {
            ing_intern_put(&self->cfg.itable, ident, Ing_Token_Kind_Ident);
            return ing_token_make(Ing_Token_Kind_Ident, start);
        }
        else
        {
            return ing_token_make(res.val.ok, start);
        }
    }

    Ing_Transition tran = self->cfg.symbols[token];

    switch (ing_tzer_take(self)) {
        case '0': {
            kind = Ing_Token_Kind_Eof;
        } break;
        default : {
            if (!tran.has_next) return ing_token_make(tran.token_kind, start);
            Ing_Usize checkpoint = start;

            while (tran.has_next)
            {
                if (tran.token_kind != Ing_Token_Kind_Unknown)
                {
                    checkpoint = self->pos;
                    kind = tran.token_kind;
                }
                Ing_Transition_Vector state = self->cfg.transitions.data[tran.next];
                tran = state.data[ing_tzer_take(self)];
            }

            if (tran.token_kind == Ing_Token_Kind_Unknown)
            {
                self->pos = checkpoint;
                return ing_token_make(kind, start);
            }

            return ing_token_make(tran.token_kind, start);
        }
    }

    return (Ing_Token) {
        .kind = kind,
        .pos  = start,
    };
}

ing_define void ing_tzer_checkout(Ing_Tokenizer *self, u32 pos)
{
    ING_ASSERT(pos > 0 && pos < self->src.len, "position out of bounds");
    self->pos = pos;
}

ing_define Ing_Span ing_tzer_span(Ing_Tokenizer* self, u32 pos)
{
    u32 checkpoint = self->pos;
    ing_tzer_checkout(self, pos);
    ING_UNUSED(ing_tzer_next(self));
    u32 end     = self->pos;
    ing_tzer_checkout(self, checkpoint);
    return (Ing_Span) { .start = pos, .end = end };
}

ing_define Ing_Loc ing_tzer_loc(Ing_Tokenizer* self, u32 pos)
{
    return (Ing_Loc) {
        .line = cast(u32)self->line,
        .col = pos - cast(u32)self->lastnewline
    };
    // u32 checkpoint = self->pos;
    // ing_tzer_checkout(self, 0);
    // u32 line = 0;
    // u32 col  = 0;
    // for (u32 ix = 0; ix < pos; ix++)
    // {
    //     if (self->src.data[ix] == '\n')
    //     {
    //         line++;
    //         col = 0;
    //     }
    //     else col++;
    // }

    // ing_tzer_checkout(self, checkpoint);
    // return (Ing_Loc) { .line = line, .col = col };
}


ing_define Ing_Origin ing_tzer_origin(Ing_Tokenizer* self, u32 pos)
{
    Ing_Span span   = ing_tzer_span(self, pos);
    Ing_Loc loc     = ing_tzer_loc(self, pos);

    return (Ing_Origin) {
        .span   = span,
        .loc    = loc,
        .file   = self->file,
    };
}

#endif
