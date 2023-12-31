#define DO_HOCUS_POCUS
#define HOCUS_DEV
#include "hocus.h"


// demo file
int main(int argc, char *argv[]) {
    hocus_pocus(argc, argv);

    hocus_build_params params = {
        .the_binary = "hocus.exe",
    };

    printf("we have rebuild tech!\n");

    // hocus_clean(&params);
    // hocus_build(&params);
    // hocus_link(&params);

    return 0;
}
