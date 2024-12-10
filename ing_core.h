
#ifndef ING_CORE
#define ING_CORE

#include "ing.h"

#include <malloc/_malloc.h>
#include <stdarg.h>
#include <errno.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef ING_CORE_IMPL

#define ING_DIGITS_IMPL
#define ING_IO_IMPL
#define ING_MEM_IMPL
#define ING_ALLOCATORS_IMPL
#define ING_DS_IMPL
#define ING_ASCII_IMPL

#endif

#ifndef ING_DIGITS
#define ING_DIGITS

#define ing_max(__left, __right) ((__left > __right) ? (__left) : (__right));
#define ing_min(__left, __right) ((__left < __right) ? (__left) : (__right));

#define VA_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define ing_maxn(...) ing_maxn_impl(VA_NARGS(__VA_ARGS__), __VA_ARGS__)

ing_define Ing_Usize ing_maxn_impl(Ing_Usize count, ...) {
    va_list args;
    va_start(args, count);

    Ing_Usize max = va_arg(args, Ing_Usize);
    for (Ing_Usize i = 1; i < count; i++) {
        Ing_Usize value = va_arg(args, Ing_Usize);
        if (value > max) {
            max = value;
        }
    }

    va_end(args);
    return max;
}

ing_define void ing_print_binary64(Ing_Usize n);
ing_define void ing_print_binary32(Ing_U32 n);
ing_define void ing_print_binary64(Ing_Usize n);

typedef Ing_Maybe(Ing_Usize) Ing_Add_Result;

ing_define Ing_Add_Result ing_save_add(Ing_Usize a, Ing_Usize b);

#define ING_DIGITS_IMPL
#ifdef ING_DIGITS_IMPL

ing_define void ing_print_binary(Ing_U8 n) {
    for (Ing_Usize i = 7; i >= 0; i--) {
        printf("%d", (n >> i) & 1);
    }
    printf("\n");
}

ing_define void ing_print_binary32(Ing_U32 n) {
    for (Ing_Usize i = 31; i >= 0; i--) {
        printf("%d", (n >> i) & 1);
    }
    printf("\n");
}

ing_define void ing_print_binary64(Ing_Usize n) {
    for (int i = 63; i >= 0; i--) {
        printf("%lu", (n >> i) & 1);
        if (i % 8 == 0) printf(" ");
    }
    printf("\n");
}

ing_define Ing_Add_Result ing_save_add(Ing_Usize a, Ing_Usize b)
{
    Ing_Usize out = a + b;
    return (Ing_Add_Result) {
        .value  = out,
        .err    = out < a,
    };
}

#endif // ING_DIGITIS_IMPL
#endif // !ING_DIGITS

#ifndef ING_IO
#define ING_IO

#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

typedef enum {
    Ing_Log_Error,
    Ing_Log_Warn,
    Ing_Log_Info,
    Ing_Log_Debug,
    Ing_Log_None,
} Ing_Log_Level;

ing_define void ing_log(Ing_Log_Level level, const Ing_String fmt, ...);
static const Ing_Log_Level ing_min_log_level = Ing_Log_Info;
/* extern Ing_Log_Level ing_min_log_level; */

#endif // !ING_IO

#ifndef ING_MEM
#define ING_MEM

ing_define Ing_Any_Ptr  ing_valloc(Ing_Usize size);
ing_define void         ing_vfree(Ing_Any_Ptr ptr, Ing_Usize size);
ing_define Ing_Any_Ptr  ing_aalloc(Ing_Usize size, Ing_Usize alignment);

typedef Ing_Maybe(Ing_Any_Ptr) Ing_Aloc_Result;

typedef enum {
    ING_ALLOC_FAILED = 1,
} Ing_Alloc_Error;

ing_define Ing_Aloc_Result  ing_try_valloc(Ing_Usize size);
ing_define Ing_Aloc_Result  ing_try_aalloc(Ing_Usize size, Ing_Usize alignment);

ing_define Ing_Any_Ptr     ing_memcopy       (Ing_Any_Ptr dst, const Ing_Any_Ptr src, Ing_Usize len);
ing_define Ing_Any_Ptr     ing_memmove       (Ing_Any_Ptr dst, const Ing_Any_Ptr src, Ing_Usize len);
ing_define Ing_Any_Ptr     ing_memset        (Ing_Any_Ptr dst, Ing_Isize val, Ing_Usize num);
ing_define Ing_Cmp_Result  ing_memcmp        (Ing_Any_Ptr left, Ing_Any_Ptr right, Ing_Usize len);

#ifdef ING_MEM_IMPL

#ifdef ING_IS_WINDOWS

/// use (VirtualAlloc) <- MS MemoryApi: https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/
ing_define Ing_Any_Ptr ing_valloc(Ing_Usize size)
{
    ING_NOT_SUPPORTED("windows(ing_valloc) not yet supported");
}

/// use (VirtualFree) <- MS MemoryApi: https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/
ing_define void ing_vfree(Ing_Usize size)
{
    ING_NOT_SUPPORTED("windows(ing_vfree) not yet supported");
}

/// use (_aligned_malloc) <- MS: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/aligned-malloc?view=msvc-170
ing_define void ing_aalloc(Ing_Usize size)
{
    ING_NOT_SUPPORTED("windows(ing_vfree) not yet supported");
}

#else // !ING_IS_WINDOWS

#include <sys/mman.h>
#include <stdlib.h>

ing_define Ing_Any_Ptr  ing_valloc(Ing_Usize size)
{
    Ing_Any_Ptr out = mmap(
        NULL, size,
        PROT_READ   | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1, 0
    );

    return (out == MAP_FAILED) ? NULL : out;
}

ing_define void ing_vfree(Ing_Any_Ptr ptr, Ing_Usize size)
{
    munmap(ptr, size);
}

ing_define Ing_Any_Ptr ing_aalloc(Ing_Usize size, Ing_Usize alignment)
{
    // return aligned_alloc(alignment, size);
    Ing_Any_Ptr out = NULL;
    Ing_I32 res = posix_memalign(&out, alignment, size);
    ING_ASSERT(res == 0, "failed memalign allocation");
    return out;
}

#endif // !ING_IS_WINDOWS

ing_define Ing_Aloc_Result ing_try_valloc(Ing_Usize size)
{
    Ing_Aloc_Result out = {0};
    Ing_Any_Ptr ptr = ing_valloc(size);
    if (ptr == NULL) out.err = ING_ALLOC_FAILED;
    return out;
}

ing_define Ing_Aloc_Result ing_try_aalloc(Ing_Usize size, Ing_Usize alignment)
{
    Ing_Aloc_Result out = {0};
    Ing_Any_Ptr ptr = ing_aalloc(size, alignment);
    if (ptr == NULL) out.err = ING_ALLOC_FAILED;
    return out;
}


ing_define Ing_Any_Ptr ing_memcopy(Ing_Any_Ptr dst, const Ing_Any_Ptr src, Ing_Usize len)
{
    Ing_U8*         dst_ = cast(Ing_U8*)dst;
    const Ing_U8*   src_ = cast(const Ing_U8*)src;

    if (dst_ == NULL) return NULL;

    // 1. Small Array Optimisation
    if (len < 32)
    {
        while (len--) *(dst_++) = *(src_++);
        return dst_;
    }

    // 2. Handle Misaligned Data
    // the '&' gives us the missing bytes to next alignment
    Ing_Usize dst_align = cast(Ing_Usize)dst_ & (alignof(max_align_t) - 1);
    if (dst_align)
    {
        Ing_Usize next_align = alignof(max_align_t) - dst_align;
        len -= next_align;
        while (next_align--) *(dst_++) = *(src_++);
    }

    if (len >= sizeof(Ing_Usize))
    {
        // Convert Dst & Src to word-size arrays
        Ing_Usize* dstw        = cast(Ing_Usize*)dst_;
        const Ing_Usize* srcw  = cast(const Ing_Usize*)src_;

        // 256 bytes -> 32 words
        // 32 words >> 3 -> 4 chunks of 8
        Ing_Usize words  = len / sizeof(Ing_Usize);
        Ing_Usize unroll = words >> 3;

        while (unroll--)
        {
            dstw[0] = srcw[0];
            dstw[1] = srcw[1];
            dstw[2] = srcw[2];
            dstw[3] = srcw[3];
            dstw[4] = srcw[4];
            dstw[5] = srcw[5];
            dstw[6] = srcw[6];
            dstw[7] = srcw[7];

            dstw += 8;
            srcw += 8;
        }

        // Handle Non-8 chunk words
        words &= 7;
        while (words--) *(dstw++) = *(srcw++);

        dst_ = cast(Ing_U8*)dstw;
        src_ = cast(const Ing_U8*)srcw;

        // Number of Non-Word bytes left to copy
        len &= (sizeof(Ing_Usize) - 1);
    }

    // Copy rest of the bytes
    while (len--) *(dst_++) = *(src_++);
    return dst_;
}

ing_define Ing_Any_Ptr ing_memmove(Ing_Any_Ptr dst, const Ing_Any_Ptr src, Ing_Usize len)
{
    Ing_U8*         dst_ = cast(Ing_U8*)dst;
    const Ing_U8*   src_ = cast(const Ing_U8*)src;

    if (dst_ == NULL) return NULL;
    if (dst_ == src_ || len == 0) return dst_;

    if (dst_+len <= dst_ || src_+len <= dst_) return ing_memcopy(dst, src, len);

    if (dst_ < src_)
    {
        while (len--) *(dst_++) = *(src_++);
    }
    else if (dst_ > src_)
    {
        dst_ += len;
        src_ += len;
        while (len--) *(--dst_) = *(--src_);
    }

    return dst_;
}

ing_define Ing_Any_Ptr ing_memset(Ing_Any_Ptr dst, Ing_Isize val, Ing_Usize num)
{
    return memset(dst, val, num);
}

ing_define Ing_Cmp_Result ing_memcmp(Ing_Any_Ptr left, Ing_Any_Ptr right, Ing_Usize len)
{
    Ing_String l = cast(Ing_String)left;
    Ing_String r = cast(Ing_String)right;

    for (Ing_Usize ix = 0; ix < len; ix++)
    {
        Ing_U8 li = l[ix];
        Ing_U8 ri = r[ix];

        if (li == ri) continue;

        if (li > ri) return Ing_Cmp_Greater_Than;
        return Ing_Cmp_Less_Than;
    }

    return Ing_Cmp_Equals;
}

#endif // ING_MEM_IMPL
#endif // !ING_MEM

#ifndef ING_ALLOCATORS
#define ING_ALLOCATORS

#define ING_DEFAULT_ALIGNMENT       (sizeof(void*))
#define ING_DEFAULT_MEMBLOCK_SIZE   1024

typedef enum Ing_Mem_Op {
    ING_MEM_OP_MAKE,
    ING_MEM_OP_RESIZE,
    ING_MEM_OP_DROP,
    ING_MEM_OP_DROP_ALL,
} Ing_Mem_Op;

#define ING_ALOC_HANDLER(name)                          \
    Ing_Any_Ptr name(                                   \
        Ing_Any_Ptr     self,                           \
        Ing_Mem_Op      op,                             \
        Ing_Usize       new_size,                       \
        Ing_Any_Ptr     old_mem,                        \
        Ing_Usize       old_size,                       \
        Ing_Usize       alignment                       \
    )                                                   \

typedef ING_ALOC_HANDLER((*Ing_Aloc_Handler));

typedef struct Ing_Allocator {
    Ing_Any_Ptr      data;
    Ing_Aloc_Handler func;
} Ing_Allocator;

ING_STATIC_ASSERT_SIZE(Ing_Any_Ptr, 8);
ING_STATIC_ASSERT_SIZE(Ing_Allocator, 16);

ing_define Ing_Any_Ptr  ing_alloc_align   (Ing_Allocator aloc, Ing_Usize size, Ing_Usize alignment);
ing_define Ing_Any_Ptr  ing_alloc         (Ing_Allocator aloc, Ing_Usize size);
ing_define Ing_Any_Ptr  ing_realloc_align (Ing_Allocator aloc, Ing_Any_Ptr old_ptr, Ing_Usize old_size, Ing_Usize new_size, Ing_Usize alignment);
ing_define Ing_Any_Ptr  ing_realloc       (Ing_Allocator aloc, Ing_Any_Ptr old_ptr, Ing_Usize old_size, Ing_Usize new_size);
ing_define void         ing_free          (Ing_Allocator aloc, Ing_Any_Ptr ptr);
ing_define void         ing_free_all      (Ing_Allocator aloc);

typedef struct Ing_Mem_Block {
    struct Ing_Mem_Block*  prev;
    Ing_Usize   pos;
    Ing_Usize   cap;
    Ing_U8*     data;
} Ing_Mem_Block;

typedef struct Ing_Arena {
    Ing_Mem_Block* curr;
    Ing_Mem_Block* tail;
} Ing_Arena;

ing_define Ing_Arena*   ing_arena_make(Ing_Usize cap);
ing_define void         ing_arena_drop(Ing_Arena* self);
ing_define Ing_Any_Ptr  ing_arena_alloc(Ing_Arena* self, Ing_Usize size, Ing_Usize alignment);
ing_define void         ing_arena_clear(Ing_Arena* self);

typedef struct {
    Ing_Mem_Block*  block;
    Ing_Usize   pos;
} Ing_Arena_Mark;

ing_define Ing_Arena_Mark   ing_arena_checkpoint(Ing_Arena* self);
ing_define void             ing_arena_checkout(Ing_Arena* self, Ing_Arena_Mark mark);

ing_define Ing_Allocator    ing_heap_aloc(void);
ing_define ING_ALOC_HANDLER(ing_heap_aloc_handler);

ing_define Ing_Allocator    ing_arena_aloc(Ing_Arena* arena);
ing_define ING_ALOC_HANDLER(ing_arena_aloc_handler);

#ifdef ING_ALLOCATORS_IMPL

ing_define Ing_Allocator ing_heap_aloc(void)
{
    return (Ing_Allocator) {
        .data = NULL,
        .func = ing_heap_aloc_handler,
    };
}

ing_define ING_ALOC_HANDLER(ing_heap_aloc_handler)
{
    ING_UNUSED(self);
    ING_ASSERT((alignment & (alignment - 1)) == 0, "alignment must be a power of 2");
    switch (op) {
        case ING_MEM_OP_MAKE: {
            ING_ASSERT(new_size > 0, "allocation of 0 bytes is not allowed");
            Ing_Any_Ptr ptr = ing_aalloc(new_size, alignment);

            ING_ASSERT(ptr != NULL, "failed heap allocation");
            return ptr;
        } break;
        case ING_MEM_OP_RESIZE: {
            ING_ASSERT(new_size > 0, "allocation of 0 bytes is not allowed");
            if (old_mem == NULL)
            {
                Ing_Any_Ptr ptr = ing_aalloc(new_size, alignment);
                ING_ASSERT(ptr != NULL, "failed heap allocation");
                return ptr;
            }

            if (new_size <= old_size) break;
            Ing_Any_Ptr ptr = ing_aalloc(new_size, alignment);
            ING_ASSERT(ptr != NULL, "failed heap allocation");
            ing_memmove(ptr, old_mem, old_size);
            free(old_mem);
            return ptr;
        } break;
        case ING_MEM_OP_DROP: {
            free(old_mem);
        } break;
        case ING_MEM_OP_DROP_ALL: break;
        default: ING_UNREACHABLE("invalid operation");
    }

    return NULL;
}

ing_define Ing_Mem_Block* ing_block_make(Ing_Usize cap)
{
    Ing_U8* raw = cast(Ing_U8*)ing_aalloc(sizeof(Ing_Mem_Block) + cap, ING_DEFAULT_ALIGNMENT);
    Ing_Mem_Block* block = cast(Ing_Mem_Block*)raw;
    block->prev   = NULL;
    block->pos    = 0;
    block->cap    = cap;
    block->data   = raw + sizeof(Ing_Mem_Block);
    return block;
}

#define next_multiple_of(__num, __align) ((__num) + (__align) - ((__num) % (__align)))
#define aligned(__num, __align) (((__num) % (__align)) == 0) ? (__num) : (next_multiple_of(__num, (__align)))

ing_define Ing_Arena* ing_arena_make(Ing_Usize cap)
{
    Ing_Usize aligned_size = ing_max(aligned(cap, ING_DEFAULT_ALIGNMENT), ING_DEFAULT_MEMBLOCK_SIZE);
    Ing_Usize base = sizeof(Ing_Arena);
    Ing_Arena* ptr = cast(Ing_Arena*)ing_valloc(base + aligned_size);

    Ing_Mem_Block* new_block = ing_block_make(aligned_size);
    ptr->curr = new_block;
    ptr->tail = new_block;

    return ptr;
}

ing_define void ing_arena_drop(Ing_Arena *self)
{
    for (Ing_Mem_Block* block = self->tail; block != NULL; block = block->prev)
    {
        Ing_Usize aligned_size = ing_max(aligned(block->cap, ING_DEFAULT_ALIGNMENT), ING_DEFAULT_MEMBLOCK_SIZE);
        Ing_Usize base = sizeof(Ing_Arena);
        ing_vfree(block, base + aligned_size);
    }
    free(self);
}

ing_define Ing_Any_Ptr ing_arena_alloc(Ing_Arena* self, Ing_Usize size, Ing_Usize alignment)
{
    Ing_Usize size_aligned = aligned(size, alignment);

    /// Uninitialised Arena
    if (self->curr == NULL)
    {
        ING_ASSERT(self->tail == NULL, "curr and tail must either both be null or both have a value.");
        Ing_Usize cap = ing_max(size_aligned, ING_DEFAULT_MEMBLOCK_SIZE);
        Ing_Mem_Block* new_block = ing_block_make(cap);
        self->curr = new_block;
        self->tail = new_block;
    }

    while (self->curr->pos + size_aligned > self->curr->cap && self->curr->prev != NULL)
    {
        self->curr = self->curr->prev;
    }

    Ing_Usize next_aligned = aligned(self->curr->pos, alignment);
    if (next_aligned + size_aligned > self->curr->cap)
    {
        Ing_Usize cap = ing_max(size_aligned, ING_DEFAULT_MEMBLOCK_SIZE);
        Ing_Mem_Block* new_block = ing_block_make(cap);
        new_block->prev = self->tail;
        self->curr = new_block;
        self->tail = new_block;
        next_aligned = 0;
    }

    Ing_Any_Ptr out = &self->curr->data[next_aligned];
    self->curr->pos = size_aligned + next_aligned;
    return out;
}

ing_define void ing_arena_clear(Ing_Arena* self)
{
    for (Ing_Mem_Block* block = self->tail; block != NULL; block = block->prev)
    {
        block->pos = 0;
        self->curr = block;
    }
}

ing_define Ing_Arena_Mark ing_arena_checkpoint(Ing_Arena* self)
{
    /// aKsHuaLlY we can have one to an uninitialised one. Fix this
    if (self->curr == NULL)
    {
        ING_ASSERT(self->tail != NULL, "curr and tail must both either have a value or be uninitialised.");
        return (Ing_Arena_Mark) {
            .block  = self->curr,
            .pos    = 0,
        };
    }

    return (Ing_Arena_Mark) {
        .block  = self->curr,
        .pos    = self->curr->pos,
    };
}

ing_define void ing_arena_checkout(Ing_Arena* self, Ing_Arena_Mark mark)
{
    if (mark.block == NULL)
    {
        ing_arena_clear(self);
        return;
    }

    mark.block->pos = mark.pos;

    /// Clearning out all temporary allocations that could have happened.
    /// If during the temporary session, more blocks needed to be allocated,
    /// reset their position to 0.
    for (Ing_Mem_Block* block = self->tail; block != mark.block; block = block->prev)
    {
        block->pos = 0;
    }

    self->curr = mark.block;
}

ing_define Ing_Allocator ing_arena_aloc(Ing_Arena* arena)
{
    return (Ing_Allocator) {
        .data   = arena,
        .func   = ing_arena_aloc_handler,
    };
}

ing_define ING_ALOC_HANDLER(ing_arena_aloc_handler)
{
    ING_ASSERT(alignment % 2 == 0, "alignment must be a power of 2");
    Ing_Arena* arena = cast(Ing_Arena*)self;
    switch (op) {
        case ING_MEM_OP_MAKE: {
            ING_ASSERT(new_size > 0, "allocation of 0 bytes is not allowed");
            Ing_Any_Ptr ptr = ing_arena_alloc(arena, new_size, alignment);
            ING_ASSERT(ptr != NULL, "failed heap allocation");
            return ptr;
        } break;
        case ING_MEM_OP_RESIZE: {
            ING_ASSERT(new_size > 0, "allocation of 0 bytes is not allowed");
            if (old_mem == NULL)
            {
                Ing_Any_Ptr ptr = ing_arena_alloc(arena, new_size, alignment);
                ING_ASSERT(ptr != NULL, "failed heap allocation");
                return ptr;
            }

            if (new_size <= old_size) break;
            Ing_Any_Ptr ptr = ing_arena_alloc(arena, new_size, alignment);
            ING_ASSERT(ptr != NULL, "failed heap allocation");
            ing_memmove(ptr, old_mem, old_size);
            return ptr;
        } break;
        case ING_MEM_OP_DROP_ALL: {
            ing_arena_clear(arena);
        } break;
        case ING_MEM_OP_DROP: break;
        default: ING_UNREACHABLE("invalid operation");
    }

    return NULL;
}

// typedef struct {
//     Ing_Any_Ptr     ok;
//     Ing_Alloc_Error err;
// } Ing_Alloc_Result;

// ing_define Ing_Any_Ptr     ing_alloc_align   (Ing_Allocator aloc, Ing_Usize size, Ing_Usize alignment);
// ing_define Ing_Any_Ptr     ing_alloc         (Ing_Allocator aloc, Ing_Usize size);
// ing_define Ing_Any_Ptr     ing_realloc_align (Ing_Allocator aloc, Ing_Any_Ptr old_ptr, Ing_Usize old_size, Ing_Usize new_size, Ing_Usize alignment);
// ing_define Ing_Any_Ptr     ing_realloc       (Ing_Allocator aloc, Ing_Any_Ptr old_ptr, Ing_Usize old_size, Ing_Usize new_size);
// ing_define void            ing_free          (Ing_Allocator aloc, Ing_Any_Ptr ptr);
// ing_define void            ing_free_all      (Ing_Allocator aloc);

/// TODO
// ing_define Ing_Alloc_Result ing_try_alloc_align   (Ing_Allocator aloc, Ing_Usize size, Ing_Usize alignment);
// ing_define Ing_Alloc_Result ing_try_alloc         (Ing_Allocator aloc, Ing_Usize size);
// ing_define Ing_Alloc_Result ing_try_realloc_align (Ing_Allocator aloc, Ing_Any_Ptr old_ptr, Ing_Usize old_size, Ing_Usize new_size, Ing_Usize alignment);
// ing_define Ing_Alloc_Result ing_try_realloc       (Ing_Allocator aloc, Ing_Any_Ptr old_ptr, Ing_Usize old_size, Ing_Usize new_size);

ing_define Ing_Any_Ptr ing_alloc_align(
    Ing_Allocator aloc,
    Ing_Usize size,
    Ing_Usize alignment)
{
    ING_ASSERT(aloc.func != NULL, "allocator not initalised.");
    Ing_Any_Ptr out = (aloc.func(aloc.data, ING_MEM_OP_MAKE, size, NULL, 0, alignment));
    ING_ASSERT(out != NULL, "failed allocation (aligned).");
    return out;
}

ing_define Ing_Any_Ptr ing_alloc(
    Ing_Allocator aloc,
    Ing_Usize size)
{
    ING_ASSERT(aloc.func != NULL, "allocator not initialised");
    Ing_Any_Ptr out = (aloc.func(aloc.data, ING_MEM_OP_MAKE, size, NULL, 0, ING_DEFAULT_ALIGNMENT));
    ING_ASSERT(out != NULL, "failed allocation");
    return out;
}

ing_define Ing_Any_Ptr ing_realloc_align(
    Ing_Allocator aloc,
    Ing_Any_Ptr old_ptr,
    Ing_Usize old_size,
    Ing_Usize new_size,
    Ing_Usize alignment)
{
    ING_ASSERT(aloc.func != NULL, "allocator not initialised");
    ING_ASSERT(old_ptr != NULL, "received null ptr in resize (aligned)");
    Ing_Any_Ptr out = aloc.func(aloc.data, ING_MEM_OP_RESIZE, new_size, old_ptr, old_size, alignment);
    ING_ASSERT(out != NULL, "failed resizing (aligned)");
    return out;
}

ing_define Ing_Any_Ptr ing_realloc(
    Ing_Allocator aloc,
    Ing_Any_Ptr old_ptr,
    Ing_Usize old_size,
    Ing_Usize new_size)
{
    ING_ASSERT(aloc.func != NULL, "allocator not initialised");
    ING_ASSERT(old_ptr != NULL, "received null ptr in resize");
    Ing_Any_Ptr out = ing_realloc_align(aloc, old_ptr, old_size, new_size, ING_DEFAULT_ALIGNMENT);
    ING_ASSERT(out != NULL, "failed resizing");
    return out;
}

ing_define void ing_free(Ing_Allocator aloc, Ing_Any_Ptr ptr)
{
    ING_ASSERT(aloc.func != NULL, "allocator not initialised");
    if (ptr == NULL) return;
    aloc.func(aloc.data, ING_MEM_OP_DROP, 0, 0, 0, ING_DEFAULT_ALIGNMENT);
}

ing_define void ing_free_all(Ing_Allocator aloc)
{
    ING_ASSERT(aloc.func != NULL, "allocator not initialised");
    aloc.func(aloc.data, ING_MEM_OP_DROP_ALL, 0, 0, 0, ING_DEFAULT_ALIGNMENT);
}

#endif // ING_ALLOCATORS_IMPL
#endif // !ING_ALLOCATORS

/// Data Structures
#ifndef ING_DS
#define ING_DS

#define Ing_View(type)              \
    struct {                        \
        Ing_Usize   len;            \
        type*       data;           \
    }                               \

#define ing_view_make(__type, ...) Ing_View(__type)                     \
    (Ing_View(__type)) {                                                \
        .len    = sizeof(cast(__type[]){ __VA_ARGS })/sizeof(__type),   \
        .data   = (__type[]) { __VA_ARGS__ },                           \
    }                                                                   \

#define Ing_Vector(type)            \
    struct {                        \
        Ing_Allocator   aloc;      \
        Ing_Usize       len;        \
        Ing_Usize       cap;        \
        type*           data;       \
    }                               \

typedef Ing_View(Ing_Any_Ptr) Ing_Any_Ptr_View;
typedef Ing_Vector(Ing_Any_Ptr) Ing_Any_Ptr_Vector;

#define ING_INIT_CAP 256

#define ing_vec_init(__al, __vec)                                                                           \
    (__vec).aloc   = (__al);                                                                               \
    (__vec).cap     = ING_INIT_CAP;                                                                         \
    (__vec).len     = 0;                                                                                    \
    (__vec).data    = (__typeof__(((__vec).data)))ing_alloc((__al), ING_INIT_CAP*(sizeof(*(__vec).data)));    \
    ING_ASSERT((__vec).data != NULL, "failed allocation during vector initialisation.")                     \

#define ing_vec_reserve(__al, __vec, __size)                                                                \
    (__vec).aloc   = (__al);                                                                               \
    (__vec).cap     = (__size);                                                                             \
    (__vec).len     = 0;                                                                                    \
    (__vec).data    = (__typeof__((__vec.data)))ing_alloc((__al), (__size)*(sizeof(*(__vec).data)));        \
    ING_ASSERT((__vec).data != NULL, "failed allocation during vector initialisation.");                    \

#define ing_vec_push(__vec, __el)                                                                 \
    do {                                                                                            \
        if ((__vec)->len >= (__vec)->cap) {                                                         \
            Ing_Usize old_size = (__vec)->cap * sizeof(*(__vec)->data);                             \
            (__vec)->cap = (__vec)->cap == 0 ? ING_INIT_CAP : (__vec)->cap * 2;                     \
            Ing_Usize new_size = (__vec)->cap * sizeof(*(__vec)->data);                             \
            ING_ASSERT((__vec)->data != NULL, "uninitialised __vector pointer.");                   \
            Ing_Any_Ptr __new_data = ing_realloc((__vec)->aloc, (__vec)->data, old_size, new_size); \
            ING_ASSERT(__new_data != NULL, "failed reallocation of __vector data");                 \
            (__vec)->data = (__typeof__(((__vec)->data)))__new_data;                                                             \
        }                                                                                           \
        (__vec)->data[(__vec)->len++] = __el;                                                       \
    } while(0);                                                                                     \

#define ing_vec_push_many(__vec, __els, __len)                                                      \
    do {                                                                                            \
        ING_ASSERT((__els) != NULL, "trying to push NULL data");                                    \
        if ((__vec)->len + (__len) >= (__vec)->cap)                                                 \
        {                                                                                           \
            Ing_Usize old_size = (__vec)->cap * sizeof(*(__vec)->data);                             \
            if ((__vec)->cap == 0) (__vec)->cap = ING_INIT_CAP;                                     \
            while ((__vec)->len + __len >= (__vec)->cap) (__vec)->cap *= 2;                         \
            Ing_Usize new_size = (__vec)->cap * sizeof(*(__vec)->data);                             \
            Ing_Any_Ptr __new_data = ing_realloc((__vec)->aloc, (__vec)->data, old_size, new_size); \
            ING_ASSERT(__new_data != NULL, "[push-many]: failed reallocation of vector data");      \
            (__vec)->data  = (__typeof__(((__vec)->data)))__new_data;                                                            \
        }                                                                                           \
        ing_memcopy((__vec)->data + (__vec)->len, (__els), (__len) * sizeof(*(__vec)->data));       \
        (__vec)->len += (__len);                                                                    \
    } while(0)                                                                                      \


#define ing_vec_first(__vec)    \
    (__vec).data[0]             \

#define ing_vec_last(__vec)     \
    (__vec).data[(__vec).len-1] \

#endif // !ING_DS

#ifndef ING_ASCII
#define ING_ASCII

typedef Ing_Vector(Ing_U8)          Ing_String_Dyn;
typedef Ing_Vector(Ing_String_Dyn)  Ing_String_Vector;
typedef Ing_View(Ing_U8)            Ing_String_View;

ING_STATIC_ASSERT_SIZE(Ing_String_Dyn, 40);

ing_define Ing_String_Dyn       ing_string_dyn_empty(Ing_Allocator aloc);
ing_define Ing_String_Dyn       ing_string_dyn_reserve(Ing_Allocator aloc, Ing_Usize cap);
ing_define Ing_String_Dyn       ing_string_dyn_make(Ing_Allocator aloc, const Ing_String str);
ing_define Ing_String_Dyn       ing_string_dyn_clone(Ing_String_Dyn* self);
ing_define void                 ing_string_dyn_free(Ing_String_Dyn self);
ing_define void                 ing_string_dyn_concat(Ing_String_Dyn* self, Ing_String_Dyn* other);
ing_define Ing_String_View      ing_string_dyn_view(Ing_String_Dyn* self);

ing_define Ing_String           ing_string_dyn_to_cstring(Ing_String_Dyn* self);

ing_define Ing_String_View      ing_string_view_make(const Ing_String str);
ing_define Ing_String_View      ing_string_view_from(const Ing_String str, Ing_Usize len);
ing_define Ing_String_View      ing_string_view_clone(Ing_Allocator aloc, const Ing_String_View* self);
ing_define Ing_String_Dyn       ing_string_view_clone_dyn(Ing_Allocator aloc, const Ing_String_View* self);
ing_define Ing_String_View      ing_string_view_slice(Ing_String_View* self, Ing_Usize start, Ing_Usize end);

ing_define Ing_B8               ing_is_alpha(Ing_U8 ch);
ing_define Ing_B8               ing_is_alpha_lower(Ing_U8 ch);
ing_define Ing_B8               ing_is_alpha_upper(Ing_U8 ch);
ing_define Ing_B8               ing_is_alpha_num(Ing_U8 ch);
ing_define Ing_B8               ing_is_dec(Ing_U8 ch);
ing_define Ing_B8               ing_is_hex(Ing_U8 ch);
ing_define Ing_B8               ing_is_oct(Ing_U8 ch);
ing_define Ing_B8               ing_is_bin(Ing_U8 ch);
ing_define Ing_B8               ing_is_ws(Ing_U8 ch);
ing_define Ing_B8               ing_is_hori_ws(Ing_U8 ch);
ing_define Ing_B8               ing_is_vert_ws(Ing_U8 ch);

ing_define Ing_U8               ing_char_to_lower(Ing_U8 ch);
ing_define Ing_U8               ing_char_to_upper(Ing_U8 ch);
ing_define Ing_Usize            ing_dec_to_usize(Ing_U8 ch);
ing_define Ing_Usize            ing_hex_to_usize(Ing_U8 ch);
ing_define Ing_Usize            ing_oct_to_usize(Ing_U8 ch);
ing_define Ing_Usize            ing_bin_to_usize(Ing_U8 ch);

typedef Ing_Result(Ing_Usize, Ing_None_Kind) Ing_Seek_Result;

ing_define Ing_Usize            ing_strlen(const Ing_String str);
ing_define Ing_Usize            ing_strnlen(const Ing_String str, Ing_Usize max);
ing_define Ing_Cmp_Result       ing_strcmp(const Ing_String left, const Ing_String right);
ing_define Ing_Cmp_Result       ing_strncmp(const Ing_String left, const Ing_String right, Ing_Usize len);
ing_define Ing_String           ing_strcpy(Ing_String dst, const Ing_String src);
ing_define Ing_String           ing_strncpy(Ing_String dst, const Ing_String src, Ing_Usize len);
ing_define Ing_Seek_Result      ing_strseek(const Ing_String str, Ing_U8 ch, Ing_Usize start, Ing_Usize end);
ing_define Ing_String_Vector    ing_strsplit(Ing_Allocator aloc, const Ing_String str, Ing_U8 ch);

#ifdef ING_ASCII_IMPL

ing_define Ing_B8 ing_is_alpha(Ing_U8 ch)
{
    return ing_is_alpha_lower(ch) || ing_is_alpha_upper(ch);
}

ing_define Ing_B8 ing_is_alpha_lower(Ing_U8 ch)
{
    return ch >= 'a' && ch <= 'z';
}

ing_define Ing_B8 ing_is_alpha_upper(Ing_U8 ch)
{
    return ch >= 'A' && ch <= 'Z';
}

ing_define Ing_B8 ing_is_alpha_num(Ing_U8 ch)
{
    return ing_is_alpha(ch) || ing_is_dec(ch);
}

ing_define Ing_B8 ing_is_dec(Ing_U8 ch)
{
    return ch >= '0' && ch <= '9';
}

ing_define Ing_B8 ing_is_hex(Ing_U8 ch)
{
    return ing_is_dec(ch) || (ch >= 'a'  && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

ing_define Ing_B8 ing_is_oct(Ing_U8 ch)
{
    return ch >= '0' && ch <= '7';
}

ing_define Ing_B8 ing_is_bin(Ing_U8 ch)
{
    return ch == '0' || ch == '1';
}

ing_define Ing_B8 ing_is_ws(Ing_U8 ch)
{
    return ing_is_vert_ws(ch) || ing_is_hori_ws(ch);
}

ing_define Ing_B8 ing_is_hori_ws(Ing_U8 ch)
{
    return ch == ' ' || ch == '\t';
}

ing_define Ing_B8 ing_is_vert_ws(Ing_U8 ch)
{
    return ch == '\n';
}

// ---------------------------------
// BEGIN: GENERAL C-STRING HELPERS
// ---------------------------------

/// FreeBSD 6.2 implementation
/// TOOD(ingmar): Optimise
ing_define Ing_Usize ing_strlen(const Ing_String str)
{
    Ing_String s;
    for (s = str; *s; ++s) {}
    return (s-str);
}

/// TOOD(ingmar): Optimise
ing_define Ing_Seek_Result ing_strseek(const Ing_String str, Ing_U8 ch, Ing_Usize start, Ing_Usize end)
{
    ING_ASSERT(end >= start, "end cannot be smaller than start.");
    ING_ASSERT(end <= ing_strlen(str), "end is out of bounds"); // TODO(ingmar): maybe remove this one (perf)

    for (Ing_Usize pos = start; pos < end; pos++)
    {
        if (str[pos] == ch)
        {
            fflush(stdout);
            return cast(Ing_Seek_Result) ing_ok(pos);
        }
    }

    return cast(Ing_Seek_Result) ing_err(Ing_None);
}

ing_define Ing_String_Vector ing_strsplit(Ing_Allocator aloc, const Ing_String str, Ing_U8 ch)
{
    ING_ASSERT(str != NULL, "passed null string");
    Ing_String_Vector split = {0};
    ing_vec_init(aloc, split);
    Ing_Usize len   = ing_strlen(str);
    if (len == 0) return split;

    for (Ing_Usize ix = 0; ix < len;)
    {
        Ing_Seek_Result next = ing_strseek(str, ch, ix, len);

        if (next.kind == Ing_Ok && next.val.ok == 0)
        {
            ix++;
            continue;
        }

        Ing_Usize slice_len = next.kind == Ing_Err ? len - ix : next.val.ok - ix;

        Ing_String_View slice = ing_string_view_from(cast(const Ing_String)str + ix, slice_len);
        Ing_String_Dyn clone = ing_string_view_clone_dyn(aloc, &slice);
        ING_ASSERT(clone.data != NULL, "failed clone");
        ing_vec_push(&split, clone);

        if (next.kind == Ing_Err) break;
        ix = next.val.ok + 1;
    }

    return split;
}

#endif // ING_ASCII_IMPL
#endif // !ING_ASCII

// ---------------------------------
// END: GENERAL C-STRING HELPERS
// ---------------------------------

// ---------------------------------
// BEGIN: GERMANY STYLE STRINGS
// ---------------------------------

#define STORAGE_MASK    (3ULL << 62)
#define PTR_MASK        (~STORAGE_MASK)

/// Reference:
/// - https://cedardb.com/blog/german_strings/
/// - https://cedardb.com/blog/strings_deep_dive/
typedef struct { Ing_U64 data[2]; } Ing_Umbra_String;
typedef Ing_U64 Ing_Umbra_Class;

#define Ing_Umbra_Class_Persistent  (0ULL << 62)
#define Ing_Umbra_Class_Temporary   (1ULL << 62)
#define Ing_Umbra_Class_Transient   (2ULL << 62)

/* typedef enum { */
/*     Ing_Umbra_Class_Persistent  = 0ULL << 62, */
/*     Ing_Umbra_Class_Temporary   = 1ULL << 62, */
/*     Ing_Umbra_Class_Transient   = 2ULL << 62, */
/* } Ing_Umbra_Class; */

// #define ING_UMBRA_PTR(ptr) ((Ing_U64)ptr & PTR_MASK)
// #define ING_UMBRA_TAG(ptr) (Ing_Umbra_Class)((Ing_U64)ptr & STORAGE_MASK)
// #define ING_UMBRA_ADD_TAG(ptr, tag) (((Ing_U64)ptr & PTR_MASK) | (Ing_U64)tag))

ing_define Ing_Umbra_String ing_umbra_make(Ing_Allocator aloc, const Ing_String str, Ing_Umbra_Class cls, Ing_U32 len);
ing_define Ing_Umbra_String ing_umbra_make_short(const Ing_String str, Ing_U32 len);
ing_define Ing_Umbra_String ing_umbra_make_persistent(const Ing_String str, Ing_U32 len);
ing_define Ing_Umbra_String ing_umbra_make_temporary(Ing_Allocator aloc, const Ing_String str, Ing_U32 len);
ing_define Ing_Umbra_String ing_umbra_make_transient(const Ing_String str, Ing_U32 len);
ing_define Ing_U64          ing_umbra_pack_first_word(const Ing_String str, Ing_U32 len);
ing_define Ing_U64          ing_umbra_pack_second_word_short(const Ing_String str, Ing_U32 len);
ing_define Ing_U64          ing_umbra_pack_second_word_long(const Ing_String str, Ing_Umbra_Class cls);
ing_define Ing_B8           ing_umbra_is_eq(Ing_Umbra_String left, Ing_Umbra_String right);
ing_define Ing_Cmp_Result   ing_umbra_cmp(Ing_Umbra_String left, Ing_Umbra_String right);
ing_define Ing_U32          ing_umbra_len(Ing_Umbra_String str);
ing_define Ing_B8           ing_umbra_is_empty(Ing_Umbra_String str);
ing_define Ing_String       ing_umbra_raw_ptr(Ing_Umbra_String* str);
ing_define Ing_String       ing_umbra_to_cstring(Ing_Allocator aloc, Ing_Umbra_String* str);
ing_define Ing_B8           ing_umbra_to_cstring_buf(Ing_Umbra_String* str, Ing_String buf, Ing_Usize size);

// ---------------------------------
// END: GERMANY STYLE STRINGS
// ---------------------------------

#ifdef ING_DS_IMPL

// ---------------------------------
// BEGIN: GERMAN STYLE/UMBRA STRINGS
// ---------------------------------

    ing_define Ing_U64 ing_umbra_pack_first_word(const Ing_String str, Ing_U32 len)
{
    Ing_U64 out = len;
    if (str)
    {
        Ing_U64 prefix_bits = 0;
        ing_memcopy(&prefix_bits, str, len);
        out |= (prefix_bits << 32);
    }

    return out;
}

ing_define Ing_U64 ing_umbra_pack_second_word_short(const Ing_String str, Ing_U32 len)
{
    ING_ASSERT(str != NULL, "received null ptr to str");
    Ing_U64 remainder = 0;
    if (len > 4)
    {
        Ing_U32 remaining = len - 4;
        ING_ASSERT(remaining <= 8, "a short string cannot be longer than 12 characters");
        ing_memcopy(&remainder, str + 4, remaining);
    }
    return remainder;
}

ing_define Ing_U64 ing_umbra_pack_second_word_long(const Ing_String str, Ing_Umbra_Class cls)
{
    return ((Ing_U64)cls << 62) | ((Ing_U64)str & PTR_MASK);
}

ing_define Ing_Umbra_String ing_umbra_make(Ing_Allocator aloc, const Ing_String str, Ing_Umbra_Class cls, Ing_U32 len)
{
    if (len <= 12) return ing_umbra_make_short(str, len);

    switch (cls) {
        case Ing_Umbra_Class_Persistent: return ing_umbra_make_persistent(str, len);
        case Ing_Umbra_Class_Temporary: return ing_umbra_make_temporary(aloc, str, len);
        case Ing_Umbra_Class_Transient: return ing_umbra_make_transient(str, len);
        default: ING_PANIC("invalid umbra class");
    }
}

ing_define Ing_Umbra_String ing_umbra_make_short(const Ing_String str, Ing_U32 len)
{
    ING_ASSERT(len <= 12, "short strings must be <= 12 characters");
    Ing_Umbra_String out = {0};
    out.data[0] = ing_umbra_pack_first_word(str, len);
    out.data[1] = ing_umbra_pack_second_word_short(str, len);
    return out;
}

ing_define Ing_Umbra_String ing_umbra_make_temporary(Ing_Allocator aloc, const Ing_String str, Ing_U32 len)
{
    ING_ASSERT(len > 12, "all short strings are persistent");

    Ing_Umbra_String out = {0};

    out.data[0] = ing_umbra_pack_first_word(str, len);

    Ing_String ptr = (Ing_String)ing_alloc(aloc, len);
    ing_memcopy(&ptr, str, len);
    out.data[1] = ing_umbra_pack_second_word_long(ptr, Ing_Umbra_Class_Temporary);

    return out;
}

ing_define Ing_Umbra_String ing_umbra_make_persistent(const Ing_String str, Ing_U32 len)
{
    Ing_Umbra_String out = {0};
    out.data[0] = ing_umbra_pack_first_word(str, len);

    if (len <= 12) out.data[1] = ing_umbra_pack_second_word_short(str, len);
    else out.data[1] = ing_umbra_pack_second_word_long(str, Ing_Umbra_Class_Persistent);
    return out;
}

ing_define Ing_Umbra_String ing_umbra_make_transient(const Ing_String str, Ing_U32 len)
{
    ING_ASSERT(len > 12, "all short strings are persistent");

    Ing_Umbra_String out = {0};
    out.data[0] = ing_umbra_pack_first_word(str, len);
    out.data[1] = ing_umbra_pack_second_word_long(str, Ing_Umbra_Class_Transient);
    return out;
}

/// Optimized for comparisons that are expected to fail
/// E.q.: In database queries, the comparison basically always fails
ing_define Ing_B8 ing_umbra_is_eq(Ing_Umbra_String left, Ing_Umbra_String right)
{
    // EXPLAIN: check if len & prefix (32bit + 32bit) are equal
    // EXPLAIN: if either is not, we can skip all the expensive other checks
    if (left.data[0] != right.data[0]) return false;

    // EXPLAIN: get most significant bytes of first 64bit ptr
    Ing_U32 len = (Ing_U32)left.data[0];

    // EXPLAIN: len and prefix are the same, so now we need to check the rest
    if (len <= 4) return true;
    if (len <= 12) return left.data[1] == right.data[1];

    return ing_memcmp(cast(Ing_U8*)left.data, cast(Ing_U8*)right.data, len);
}

ing_define Ing_Cmp_Result ing_umbra_cmp(Ing_Umbra_String left, Ing_Umbra_String right)
{
    // EXPLAIN: get most significant bytes of first 64bit ptr
    Ing_U32 llen = cast(Ing_U32)left.data[0];
    // EXPLAIN: if len <= 12, we get the least significant 32bits (+4 * 8bits)
    // EXPLAIN: otherwise we cast it as a Ing_String (char*),
    // EXPLAIN: which means (i think) the tag is ignored.
    Ing_String lPtr = llen <= 12 ? cast(Ing_String)&left.data[0] + 4 : cast(Ing_String)left.data[1];
    Ing_U32 rlen = cast(Ing_U32)right.data[0];
    Ing_String rPtr = rlen <= 12 ? cast(Ing_String)&right.data[0] + 4 : cast(Ing_String)right.data[1];

    return ing_memcmp(lPtr, rPtr, llen);
}

ing_define Ing_U32 ing_umbra_len(Ing_Umbra_String str)
{
    return cast(Ing_U32)str.data[0];
}

ing_define Ing_B8 ing_umbra_is_empty(Ing_Umbra_String str)
{
    return ing_umbra_len(str) == 0;
}

ing_define Ing_String ing_umbra_raw_ptr(Ing_Umbra_String* str)
{
    Ing_Usize len = ing_umbra_len(*str);
    if (len <= 12) return cast(Ing_String)&str->data[0]+4;
    else return cast(Ing_String)&str->data[1];
}

ing_define Ing_B8 ing_umbra_to_cstring_buf(Ing_Umbra_String* str, Ing_String buf, Ing_Usize size)
{
    Ing_U32 len = ing_umbra_len(*str);

    if (len <= 12) ing_memcopy(buf, cast(Ing_String)&str->data[0]+4, size);
    else ing_memcopy(buf, cast(Ing_String)&str->data[1], size);

    buf[len] = '\0';
    return len <= size;
}

ing_define Ing_String ing_umbra_to_cstring(Ing_Allocator aloc, Ing_Umbra_String* str)
{
    Ing_String out = {0};
    Ing_U32 len = ing_umbra_len(*str);

    if (len == 0) return cast(Ing_String)"\0";
    if (len < 12)
   {
       out = cast(Ing_String)&str->data[0] + 4;
       out[len] = '\0';
       return out;
   }

   out = cast(Ing_String)ing_alloc(aloc, len+1);

   if (len == 12)
   {
       memcpy(out, cast(Ing_String)&str->data[0] + 4, len);
       out[len] = '\0';
       return out;
   }

    // if (len <= 12) return cast(Ing_String)&str->data[0] + 4;

    memcpy(out, cast(Ing_String)str->data[1], len);
    out[len] = '\0';
    return out;
}

// ---------------------------------
// END: GERMAN STYLE/UMBRA STRINGS
// ---------------------------------

// ---------------------------------
// BEGIN: STRING DYN
// ---------------------------------
ing_define Ing_String_Dyn ing_string_dyn_empty(Ing_Allocator aloc)
{
    Ing_String_Dyn out = {0};
    ing_vec_init(aloc, out);
    return out;
}

ing_define Ing_String_Dyn ing_string_dyn_make(Ing_Allocator aloc, const Ing_String str)
{
    Ing_String_Dyn out = {0};
    out.aloc = aloc;

    if (!str)
    {
        out.cap  = ING_INIT_CAP;
        out.len  = 0;
        out.data = cast(Ing_U8*)ing_alloc(aloc, ING_INIT_CAP);
        ING_ASSERT(out.data != NULL, "failed allocation of dynamic string buffer");
        return out;
    }

    out.len  = ing_strlen(str);
    out.cap  = out.len + (out.len >> 1) + ING_INIT_CAP;
    out.data = cast(Ing_U8*)ing_alloc(aloc, out.cap);

    ING_DEBUG_ASSERT(out.data, "failed data allocation for string builder");
    ing_memcopy(out.data, cast(Ing_Any_Ptr)str, out.len);

    return out;
}

ing_define Ing_String_Dyn ing_string_dyn_clone(Ing_String_Dyn* self)
{
    ING_ASSERT(self != NULL, "passed null self");
    Ing_String_Dyn out = {0};
    out.aloc = self->aloc;

    if (!self || !self->data) return out;

    out.cap  = self->cap;
    out.len  = self->len;
    out.data = cast(Ing_U8*)ing_alloc(self->aloc, out.cap);

    ING_DEBUG_ASSERT(out.data, "failed data allocation for string builder");
    ing_memcopy(out.data, self->data, out.len);

    return out;
}

ing_define void ing_string_dyn_free(Ing_String_Dyn self)
{
    ing_free(self.aloc, self.data);
}

#define ing_string_dyn_push(self, ch) ing_vec_push(self, ch)
#define ing_string_dyn_push_many(self, str) ing_vec_push_many(self, cast(Ing_String)str, ing_strlen(cast(Ing_String)str))

ing_define void ing_string_dyn_concat(Ing_String_Dyn* self, Ing_String_Dyn* other)
{
    ING_ASSERT(self != NULL, "passed null self");
    ING_ASSERT(other != NULL, "passed null other");
    Ing_U8* bytes   = other->data;
    Ing_Usize  l  = other->len;
    ing_vec_push_many(self, bytes, l);
}

ing_define Ing_String_View ing_string_dyn_view(Ing_String_Dyn* self)
{
    return (Ing_String_View) {
        .len  = self->len,
        .data = cast(Ing_U8*)self->data,
    };
}

ing_define Ing_String ing_string_dyn_to_cstring(Ing_String_Dyn* self)
{
    Ing_String out = cast(Ing_String)ing_alloc(self->aloc, self->len+1);
    ing_memcopy(out, self->data, self->len);
    out[self->len] = '\0';
    return out;
}

// ---------------------------------
// END: STRING DYN
// ---------------------------------

// ---------------------------------
// BEGIN: STRING VIEW
// ---------------------------------
ing_define Ing_String_View ing_string_view_make(const Ing_String str)
{
    return (Ing_String_View) {
        .len    = ing_strlen(str),
        .data   = cast(Ing_U8*)str,
    };
}

ing_define Ing_String_View ing_string_view_from(const Ing_String str, Ing_Usize len)
{
    return (Ing_String_View) {
        .len    = len,
        .data   = cast(Ing_U8*)str,
    };
}

ing_define Ing_String_View ing_string_view_clone(Ing_Allocator aloc, const Ing_String_View* self)
{
    Ing_String_View out = {0};
    out.data = cast(Ing_U8*)ing_alloc(aloc, self->len);
    out.len  = self->len;
    ING_ASSERT(out.data, "failed allocation for string view clone");
    ing_memcopy(out.data, self->data, self->len);
    return out;
}

ing_define Ing_String_Dyn ing_string_view_clone_dyn(Ing_Allocator aloc, const Ing_String_View* self)
{
    Ing_String_Dyn out = {0};
    out.aloc= aloc;
    out.len  = self->len;
    out.cap  = self->len * 2;
    out.data = cast(Ing_U8*)ing_alloc(aloc, out.cap);
    ing_memcopy(out.data, self->data, self->len);
    ING_ASSERT(out.data != NULL, "failed allocation for string view clone");
    return out;
}

ing_define Ing_String_View ing_string_view_slice(Ing_String_View* self, Ing_Usize start, Ing_Usize end)
{
    ING_DEBUG_ASSERT(start >= 0, "start cannot be negative");
    ING_DEBUG_ASSERT(start <= end, "end cannot be smaller than start");
    ING_DEBUG_ASSERT(start < self->len, "start cannot be bigger than the string view's length");
    ING_DEBUG_ASSERT(end < self->len, "end cannot be bigger than the string view's length");

    Ing_U8* ptr = self->data + start;

    return (Ing_String_View) {
        .len  = end - start,
        .data = ptr,
    };
}

// ---------------------------------
// END: STRING VIEW
// ---------------------------------


#endif // ING_DS_IMPL

#ifndef ING_FS
#define ING_FS

typedef Ing_Isize Ing_File_Descriptor;

typedef enum {
    Ing_Standard_File_In,
    Ing_Standard_File_Out,
    Ing_Standard_File_Err,

    Ing_Standard_File_Count,
} Ing_Standard_File;

typedef struct {
    Ing_File_Descriptor     fd;
    const Ing_String        path;
} Ing_File;

typedef enum {
    Ing_File_Does_Not_Exist,
    Ing_File_Failed_To_Open,
    Ing_Seek_End_Failed,
    Ing_Seek_Set_Failed,
    Ing_Tell_Size_Failed,
    Ing_Read_Failed,
} Ing_IO_Error_Kind;

typedef struct  {
    Ing_IO_Error_Kind   kind;
    const Ing_String    msg;
} Ing_IO_Error;

ing_define Ing_IO_Error ing_io_error_make(Ing_IO_Error_Kind kind, const Ing_String msg);

typedef Ing_Vector(Ing_String_Dyn) Ing_File_Lines;
typedef Ing_Result(Ing_String_Dyn, Ing_IO_Error) Ing_Read_Result;
typedef Ing_Result(Ing_File_Lines, Ing_IO_Error) Ing_Readlines_Result;

ing_define Ing_File                 ing_file_std(Ing_Standard_File std);
ing_define Ing_Read_Result          ing_file_read(Ing_Allocator aloc, const Ing_String path);
ing_define Ing_Readlines_Result     ing_file_read_lines(const Ing_String path);

// ---------------------------------
// BEGIN: OS PATH
// ---------------------------------
#define ING_MAX_PATH_LEN 4096
#define ING_PATH_SEP_WIN    '\\'
#define ING_PATH_SEP_UNIX   '/'

typedef const Ing_String Ing_Path;

#ifndef ING_PATH_SEP
    #if IS_WINDOWS
        #define ING_PATH_SEP ING_PATH_SEP_WIN
    #else
        #define ING_PATH_SEP ING_PATH_SEP_UNIX
    #endif
#endif

typedef enum {
    Ing_Path_Is_File,
    Ing_Path_Is_Dir,
    Ing_Path_Does_Not_Exist,
    Ing_Path_Access_Denied,
    Ing_Path_Part_Not_Dir,
    Ing_Path_Error,
} Ing_Path_Status;

ing_define Ing_Path_Status      ing_path_check(Ing_Path path);
ing_define Ing_B32              ing_path_exists(Ing_Path path);
ing_define Ing_B32              ing_path_is_file(Ing_Path path);
ing_define Ing_B32              ing_path_is_dir(Ing_Path path);
ing_define Ing_B32              ing_path_is_abs(Ing_Path path);
ing_define Ing_B32              ing_path_is_rel(Ing_Path path);
ing_define Ing_String           ing_path_full_name(Ing_Allocator aloc, Ing_Path path);
ing_define Ing_String           ing_path_dir_name(Ing_Path path);
ing_define Ing_String           ing_path_norm(Ing_Allocator aloc, Ing_Path path);
ing_define Ing_String_Vector    ing_path_segments(Ing_Allocator aloc, Ing_Path path);
ing_define Ing_String_Dyn       ing_path_with_parents(Ing_Allocator aloc, Ing_Path path, Ing_Usize depth);

#ifndef ING_IO_STRIP_PREFIX_GUARD_
#define ING_IO_STRIP_PREFIX_GUARD_
    #ifdef ING_STRIP_PREFIX
        typedef Ing_Log_Level Log_Level;
        #define LOG_ERROR   Ing_Log_Error
        #define LOG_WARN    Ing_Log_Warn
        #define LOG_INFO    Ing_Log_Info
        #define LOG_DEBUG   Ing_Log_Debug
        #define LOG_NONE    Ing_Log_None
        #define log(level, fmt, ...) ing_log(level, fmt, __VA_ARGS__);

        typedef Ing_Usize               File_Descriptor;
        typedef Ing_Standard_File       Standard_File;
        typedef Ing_File                File;
        typedef Ing_IO_Error_Kind       IO_Error_Kind;
        typedef Ing_IO_Error            IO_Error;

        #define io_ok(val) ing_io_ok(val)
        #define io_err(val) ing_io_err(val)

        typedef Ing_Read_Result         Read_Result;
        typedef Ing_File_Lines          File_Lines;
        typedef Ing_Readlines_Result    Readlines_Result;

        #define file_std(std) ing_file_std(std)
        #define file_read(aloc, path) ing_file_read(aloc, path)
        #define file_read_liens(path) ing_file_read_lines(path)

        typedef Ing_Path_Status Path_Status;
        #define path_check(path) ing_path_check(path)
        #define path_exists(path) ing_path_exists(path)
        #define path_is_file(path) ing_path_is_file(path)
        #define path_is_dir(path) ing_path_is_dir(path)
        #define path_is_abs(path) ing_path_is_abs(path)
        #define path_is_rel(path) ing_path_is_rel(path)
        #define path_full_name(aloc, path) ing_path_full_name(aloc, path)
        #define path_dir_name(path) ing_path_dir_name(path)
        #define path_norm(aloc, path) ing_path_norm(aloc, path)
        #define path_segments(aloc, path) ing_path_segments(aloc, path)
        #define path_with_parents(aloc, path, depth) ing_path_with_parents(aloc, path, depth);

    #endif
#endif

#endif // !ING_FS

// ---------------------------------
// END: OS PATH
// ---------------------------------
#ifdef ING_IO_IMPL

#define ing_info(fmt, ...)                          \
    ing_logf(Ing_Log_Info, fmt, ##__VA_ARGS__)      \

#define ing_error(fmt, ...)                         \
    ing_logf(Ing_Log_Error, fmt, ##__VA_ARGS__)     \

#define ing_warn(fmt, ...)                          \
    ing_logf(Ing_Log_Warn, fmt, ##__VA_ARGS__)      \

#define ing_dbg(fmt, ...)                           \
    ing_logf(Ing_Log_Debug, fmt, ##__VA_ARGS__)     \

// TODO: replace heap allocator with temp arena.
#define ing_logf(level, fmt, ...)                                                                                       \
    do {                                                                                                                \
        if (level < ing_min_log_level) break;                                                                           \
        FILE* fd = stdout;                                                                                              \
        Ing_Allocator aloc = ing_heap_aloc();                                                                     \
        Ing_String_Dyn relative = ing_path_with_parents(aloc, cast(Ing_String)__FILE__, 1);                            \
        switch (level) {                                                                                                \
            case Ing_Log_Error  : fprintf(fd, "%20.*s:%-6d: [ERROR] ", (int)relative.len, relative.data, __LINE__); break;  \
            case Ing_Log_Warn   : fprintf(fd, "%20.*s:%-6d: [WARN] " , (int)relative.len, relative.data, __LINE__);  break; \
            case Ing_Log_Info   : fprintf(fd, "%20.*s:%-6d: [INFO] " , (int)relative.len, relative.data, __LINE__);  break; \
            case Ing_Log_Debug  : fprintf(fd, "%20.*s:%-6d: [DEBUG] ", (int)relative.len, relative.data, __LINE__); break;  \
            default: ING_UNREACHABLE("invalid log level");                                                              \
        }                                                                                                               \
        printf(fmt, ##__VA_ARGS__);                                                                                       \
        printf("\n");                                                                                                   \
        ing_string_dyn_free(relative);                                                                                  \
    } while(0)                                                                                                          \

// TODO: replace heap allocator with temp arena
#define ing_log(level, msg)                                                                                             \
    do {                                                                                                                \
        if (level < ing_min_log_level) break;                                                                           \
        FILE* fd = stdout;                                                                                              \
        Ing_Allocator aloc = ing_heap_aloc();                                                                     \
        Ing_String_Dyn relative = ing_path_with_parents(aloc, __FILE__, 1);                                            \
        switch (level) {                                                                                                \
            case Ing_Log_Error  : fprintf(fd, "%20.*s:%-6d: [ERROR] ", (int)relative.len, relative.data, __LINE__); break;  \
            case Ing_Log_Warn   : fprintf(fd, "%20.*s:%-6d: [WARN] " , (int)relative.len, relative.data, __LINE__); break;  \
            case Ing_Log_Info   : fprintf(fd, "%20.*s:%-6d: [INFO] " , (int)relative.len, relative.data, __LINE__); break;  \
            case Ing_Log_Debug  : fprintf(fd, "%20.*s:%-6d: [DEBUG] ", (int)relative.len, relative.data, __LINE__); break;  \
            default: ING_UNREACHABLE("invalid log level");                                                              \
        }                                                                                                               \
        printf(msg);                                                                                                    \
        printf("\n");                                                                                                   \
        ing_string_dyn_free(relative);                                                                                  \
    } while(0)                                                                                                          \


ing_define Ing_IO_Error ing_io_error_make(Ing_IO_Error_Kind kind, const Ing_String msg)
{
    return (Ing_IO_Error) {
        .kind   = kind,
        .msg    = msg,
    };
}

ing_define Ing_Read_Result ing_file_read(Ing_Allocator aloc, const Ing_String path)
{
    if (!ing_path_is_file(path)) return cast(Ing_Read_Result) ing_err(
        ing_io_error_make(
            Ing_File_Does_Not_Exist,
            cast(Ing_String)"provided path is not a valid file."
        )
    );

    FILE* file = fopen(path, "rb");
    if (file == NULL) return cast(Ing_Read_Result) ing_err(
        ing_io_error_make(
            Ing_File_Failed_To_Open,
            cast(Ing_String)"file was found, but was unable to open it."
        )
    );

    if (fseek(file, 0, SEEK_END) < 0) return cast(Ing_Read_Result) ing_err(
        ing_io_error_make(
            Ing_Seek_End_Failed,
            cast(Ing_String)"failed to find end of file."
        )
    );

    Ing_Usize size = ftell(file);
    if (size < 0) return cast(Ing_Read_Result) ing_err(
        ing_io_error_make(
            Ing_Tell_Size_Failed,
            cast(Ing_String)"failed to find size of file."
        )
    );

    if (fseek(file, 0, SEEK_SET) < 0) return cast(Ing_Read_Result) ing_err(
        ing_io_error_make(
            Ing_Seek_Set_Failed,
            cast(Ing_String)"failed to find start of file."
        )
    );

    Ing_String_Dyn out = {0};
    ing_vec_reserve(aloc, out, size+1);

    if (fread(out.data, size, 1, file) < 0) return cast(Ing_Read_Result) ing_err(
        ing_io_error_make(
            Ing_Read_Failed,
            cast(Ing_String)"failed reading file contents into output string buffer."
        )
    );

    out.len = size+1;
    out.data[out.len] = 0;

    fclose(file);
    return cast(Ing_Read_Result) ing_ok(out);
}

ing_define Ing_Path_Status ing_path_check(Ing_Path path)
{
    ING_ASSERT(path != NULL, "path is null");
    struct stat statp;

    if (stat(path, &statp) == -1)
    {
        switch (errno)
        {
            case ENOENT:  return Ing_Path_Does_Not_Exist;
            case EACCES:  return Ing_Path_Access_Denied;
            case ENOTDIR: return Ing_Path_Part_Not_Dir;
            default:      return Ing_Path_Error;
        }
    }

    if (access(path, F_OK) == -1) return Ing_Path_Access_Denied;

    if (S_ISDIR(statp.st_mode))      return Ing_Path_Is_Dir;
    else if (S_ISREG(statp.st_mode)) return Ing_Path_Is_File;

    // Does not handle symlinks/devices etc.
    return Ing_Path_Error;
}

ing_define Ing_B32 ing_path_exists(Ing_Path path)
{
    ING_ASSERT(path != NULL, "path is null");
    Ing_Path_Status status = ing_path_check(path);
    return status == Ing_Path_Is_Dir || status == Ing_Path_Is_File;
}

ing_define Ing_B32 ing_path_is_file(Ing_Path path)
{
    ING_ASSERT(path != NULL, "path is null");
    return access(path, F_OK) != -1;
}

ing_define Ing_B32 ing_path_is_dir(Ing_Path path)
{
    ING_ASSERT(path != NULL, "path is null");
    return ing_path_check(path) == Ing_Path_Is_Dir;
}

ing_define Ing_B32 ing_path_is_abs(Ing_Path path)
{
    ING_ASSERT(path != NULL, "path is null");
    Ing_B32 out = false;

    #ifdef ING_IS_WINDOWS
        ING_NOT_IMPLEMENTED("checking if path is absolute for windows");
    #else
        out = ing_strlen(path) > 0 && path[0] == ING_PATH_SEP;
    #endif

    return out;
}

ing_define Ing_B32 ing_path_is_rel(Ing_Path path)
{
    return !ing_path_is_abs(path);
}

ing_define Ing_String ing_path_full_name(Ing_Allocator aloc, Ing_Path path)
{
    ING_UNUSED(aloc);
    ING_ASSERT(ing_path_exists(path), "path does not exist");

    #ifdef ING_IS_WINDOWS
        ING_NOT_IMPLEMENTED("full path is not implemented for windows");
    #else
        Ing_String fullpath = {0};

        Ing_String err = realpath(path, fullpath);
        ING_ASSERT(err != NULL, "path does not exist");

        Ing_Usize  len = ing_strlen(fullpath);
        Ing_String out = cast(Ing_String)ing_alloc(aloc, sizeof(Ing_U8) * (len + 1));
        ing_memmove(out, fullpath, len);
        out[len] = '\0';
        free(err);

        return out;
    #endif
}

ing_define Ing_String ing_path_dir_name(Ing_Path path)
{
    ING_UNUSED(path);
    ING_NOT_IMPLEMENTED("path dir name");
}

ing_define Ing_String ing_path_norm(Ing_Allocator aloc, Ing_Path path)
{
    ING_ASSERT(path != NULL, "passed null as input");

    Ing_String resolved = realpath(path, NULL);
    if (!resolved) {
        Ing_Usize  len          = ing_strlen(path);
        Ing_String normalized   = cast(Ing_String)ing_alloc(aloc, len + 1);
        ING_ASSERT(normalized != NULL, "failed allocation of norm backing");

        Ing_String src = path;
        Ing_String dst = normalized;

        // Handle absolute vs relative path
        if (*src == '/') *dst++ = *src++;

        while (*src) {
            // Handle "."
            if (src[0] == '.' && (src[1] == '/' || src[1] == '\0')) {
                src += src[1] ? 2 : 1;
                continue;
            }

            // Handle ".."
            if (src[0] == '.' && src[1] == '.' && (src[2] == '/' || src[2] == '\0')) {
                // Go back one directory level in dst
                if (dst > normalized) {
                    dst--;
                    while (dst > normalized && *(dst - 1) != '/') {
                        dst--;
                    }
                }
                src += src[2] ? 3 : 2;
                continue;
            }

            // Copy until next '/' or end
            while (*src && *src != '/') *dst++ = *src++;

            // Handle multiple consecutive slashes
            if (*src == '/') {
                *dst++ = *src++;
                while (*src == '/') src++;
            }
        }

        // Remove trailing slash if not root
        if (dst > normalized + 1 && *(dst - 1) == '/') dst--;

        *dst = '\0';

        // If the result is empty, return "."
        if (normalized[0] == '\0') {
            free(normalized);
            normalized = strdup(".");
        }

        return normalized;
    }

    return resolved;
}

ing_define Ing_String_Vector ing_path_segments(Ing_Allocator aloc, Ing_Path path)
{
    Ing_String normalised = ing_path_norm(aloc, path);
    Ing_String_Vector out = ing_strsplit(aloc, cast(const Ing_String)normalised, '/');
    ing_free(aloc, normalised);
    return out;
}

ing_define Ing_String_Dyn ing_path_with_parents(Ing_Allocator aloc, Ing_Path path, Ing_Usize depth)
{
    ING_ASSERT(path != NULL, "provided null path");

    Ing_String_Vector segments = ing_path_segments(aloc, path);
    Ing_String_Dyn out = ing_string_dyn_empty(aloc);

    if (depth == 0) return ing_vec_last(segments);

    for (Ing_Usize ix = depth; ix > 0; ix--)
    {
        Ing_String_Dyn segment = segments.data[segments.len - ix - 1];
        ing_string_dyn_concat(&out, &segment);
        ing_string_dyn_push(&out, '/');
    }

    Ing_String_Dyn last = ing_vec_last(segments);
    ing_string_dyn_concat(&out, &last);
    return out;
}

#endif // ING_IO_IMPL
#endif // ING_CORE
