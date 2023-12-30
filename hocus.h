#ifndef HOCUS_POCUS_H_
#define HOCUS_POCUS_H_

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
// custom defined ALLOCATOR&free - supported
#elif
#error "Bad custom allocator - Define all of: hp_alloc, hp_realloc, hp_free"
#endif//allocator checks

int hocus_pocus(char argc, char* argv[]);

#ifdef DO_HOCUS_POCUS
// IMPLEMENTATION

#ifndef _WIN32
#error "Only windows is currently supported!"
#else
#include <stdio.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int compare_last_edit(char const* lhs, char const* rhs) {
    HANDLE hfile;
    hfile = CreateFileA(lhs, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    FILETIME lhs_file_time = {0};
    GetFileTime(hfile, NULL, NULL, &lhs_file_time);
    CloseHandle(hfile);

    hfile = CreateFileA(rhs, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    FILETIME rhs_file_time = {0};
    GetFileTime(hfile, NULL, NULL, &rhs_file_time);
    CloseHandle(hfile);

    return CompareFileTime(&lhs_file_time, &rhs_file_time);
}

int move(char const* src, char const* dst) {
    char buf[256];

    snprintf(buf, sizeof(buf), "move /y %s %s >NUL 2>&1", src, dst);
    return system(buf);
}

// int delete(char const* tgt) {
//     char buf[256];

//     snprintf(buf, sizeof(buf), "del %s", tgt);
//     return system(buf);
// }

int build(char const* src, char const* exe) {
    char buf[256];

    snprintf(buf, sizeof(buf), "cl.exe /Fe:%s %s >NUL 2>&1", exe, src);
    return system(buf);
}

int run(char argc, char* argv[], char const* exe) {
    int command_len = strlen(exe) + 2;
    for (int i = 1; i < argc; ++i) {
        command_len += strlen(argv[i]);
    }

    char* command = malloc(command_len);
    strcpy(command, exe);
    strcat(command, " ");

    for (int i = 1; i < argc; ++i) {
        strcat(command, argv[i]);
    }

    int ret = system(command);
    free(command);
    return ret;
}

int hocus_pocus(char argc, char* argv[]) {
    char const* src = "hocus.c";
    char const* lib = "hocus.h";
    char const* exe = "hocus.exe";
    char const* old_exe = "hocus.old.exe";

    if (compare_last_edit(src, exe) < 0
        && compare_last_edit(lib, exe) < 0) return 0;
    move(exe, old_exe);
    build(src, exe);
    int err = run(argc, argv, exe);

    exit(err);
}
#endif//_WIN32
#endif//DO_HOCUS_POCUS
#endif//HOCUS_POCUS_H_
