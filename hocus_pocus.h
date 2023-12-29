#ifndef HOCUS_POCUS_H_
#define HOCUS_POCUS_H_

/// Hocus Pocus, a magical build system
/// 
/// Use instructions:
/// 1. Create your buildscript (hocus_pocus.c)
/// 2. Define "DO_HOCUS_POCUS"
/// 3. Include "hocus_pocus.h"
/// 4. TODO: update as the buildscript grows
/// 5. Bootstrap by running `cl.exe /out:hocus.exe hocus_pocus.c`
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

// header

#define HP_PASTE_HELPER(lhs, rhs) lhs ## rhs
#define HP_PASTE(lhs, rhs) HP_PASTE_HELPER(lhs, rhs)
#define HP_STATIC_ASSERT(cond, msg) int HP_PASTE(_static_assert_line_, __LINE__) [!!(cond)];

// implementation
#ifdef DO_HOCUS_POCUS
#endif//DO_HOCUS_POCUS


#endif//HOCUS_POCUS_H_