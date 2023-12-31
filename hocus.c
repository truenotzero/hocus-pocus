#define DO_HOCUS_POCUS
#include "src/hocus.h"
#include "src/hocus.c"

typedef char *hp_cstr;

typedef struct {
    hp_cstr *items;
    int size;
    int capacity;
} hp_sb;


#define HP_SB_DEFAULT_CAPACITY 1
int hp_sb_at_least(hp_sb *self, int desired_capacity) {
    int start_capacity = self->capacity;
    if (self->capacity == 0) { self->capacity = HP_SB_DEFAULT_CAPACITY; }

    while (self->capacity < desired_capacity) { self->capacity *= 2; }
    if (self->capacity != start_capacity) {
        self->items = hp_realloc(self->items, sizeof(*self->items) * self->capacity);
    }

    return self->items ? 0 : 1;
}

int hp_sb_push(hp_sb *self, hp_cstr item) {
    if (hp_sb_at_least(self, self->size + 1) != 0) return 1;
    self->items[self->size] = item;
    self->size += 1;
    return 0;
}

// demo file
int main(int argc, char *argv[]) {
    // hocus_pocus(argc, argv);

    // struct _hp_on_src_params params = {
    //     .output_type = 'o',
    //     .target_dir = "target",
    //     .the_binary = "hocus.exe",
    // };

    // _hp_iterate_dir("src", ".c", &params, _hp_on_src);

    return 0;
}
