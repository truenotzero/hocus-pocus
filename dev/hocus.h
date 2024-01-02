#ifndef HOCUS_POCUS_H_
#define HOCUS_POCUS_H_ "0.1.0"

/// Hocus Pocus, a magical build system
/// 
/// Use instructions:
/// 1. Create your buildscript (hocus.c)
/// 2. Define "DO_HOCUS_POCUS"
/// 3. Include "hocus.h"
/// 4. TODO: update as the buildscript grows
/// 5. Open the native build tools command line
/// 6. Bootstrap by running `cl.exe hocus.c`
/// 7. Run `hocus.exe`, which will automatically rebuild itself and your project!
///
/// Project structure
/// . 
/// |- include
/// |- lib
/// |- src
/// |- target # TODO: expand upon structure
///
/// Features
/// * Automatically detects environment (os + compiler)
/// * Rebuilds source (.c) file when edited
/// * Relinks when a source is changed
/// * Programatic buildscript, all in C
///
/// Upcoming features
/// * Linux support

// HEADER

#define HP_PASTE_HELPER(lhs, rhs) lhs ## rhs
#define HP_PASTE(lhs, rhs) HP_PASTE_HELPER(lhs, rhs)
#define HP_STATIC_ASSERT(cond, msg) int HP_PASTE(_static_assert_line_, __LINE__) [!!(cond)];

// Custom allocator support
#if !(defined(hp_alloc) || defined(hp_realloc) || defined(hp_free))
#include <stdlib.h>
#define hp_alloc malloc
#define hp_realloc realloc
#define hp_free free
#elif defined(hp_alloc) && defined(hp_realloc) && defined(hp_free)
// custom defined alloc&free - supported
#elif
#error "Bad custom allocator - Define all of: hp_alloc, hp_realloc, hp_free"
#endif//allocator checks

typedef char const *_hp_cstr;
typedef struct {
    _hp_cstr *elems;
    int size;
    int capacity;
} _hp_sb;

typedef struct _hp_on_src_params {
    char output_type;
    char const *target_dir;
    char const *source_dir;
    char const *the_binary;
    char const **libs;

    int _do_relink;
    _hp_sb _objects;
} hocus_build_params;

int hocus_pocus(char argc, char* argv[]);
int hocus_clean(hocus_build_params *params);
int hocus_build(hocus_build_params *params);
int hocus_link(hocus_build_params *params);

typedef int(*_hp_on_src_cb_t)(char const *base, char const* src, void *user_data);
#endif//HOCUS_POCUS_H_
