# Hocus Pocus, a magical build system
Building C is hard  
But C is a simple language  
  
So building C shouldn't be hard...  

## Introducing Hocus Pocus, a tiny C build system
Setup is as easy as:
1. Create a project
2. Put your sources in `./src`
3. Get `./hocus.h`
4. Create `./hocus.c` (example below)
5. `>cl hocus.c && hocus`
6. Edit your project / buildscript
7. `>hocus` (no need to recompile the build system!)

## Buildscript example
```c
#define DO_HOCUS_POCUS
#include "hocus.h"

int main(int argc, char *argv[]) {
    // this tiny bit of magic makes the buildscript rebuild itself
    // every time you edit this
    // so, bootstrap once and enjoy!
    hocus_pocus(argc, argv);

    hocus_build_params params = {
        // by default, no need to specify:
        // .source_dir = "src",
        // .target_dir = "target",
        // .output_type = 'o',
        .the_binary = "my_program.exe",
    };

    hocus_clean(&params); // to clean the build
    hocus_build(&params); // to build sources, won't rebuild anything unless sources were actually modified!
    hocus_link(&params); // to link sources into the executable, won't link anything unless any sources were recompiled!

    return 0;
}

```

## Features
### This project isn't finished yet!
[x] Project bootstrap
[x] Buildscript rebuilding
[x] Source auto-recompilation
[x] Application auto-linking
[ ] Automatic include directories
[ ] (Shared) library support
[ ] Recompile sources on header file change

## License
MIT License
