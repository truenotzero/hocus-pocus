#ifdef DO_HOCUS_POCUS
// IMPLEMENTATION

#define HP_SB_DEFAULT_CAPACITY 1
int _hp_sb_at_least(_hp_sb *self, int desired_capacity) {
    int start_capacity = self->capacity;
    if (self->capacity == 0) { self->capacity = HP_SB_DEFAULT_CAPACITY; }

    while (self->capacity < desired_capacity) { self->capacity *= 2; }
    if (self->capacity != start_capacity) {
        self->elems = hp_realloc((void*) self->elems, sizeof(*self->elems) * self->capacity);
    }

    return self->elems ? 0 : 1;
}

int _hp_sb_push(_hp_sb *self, _hp_cstr item) {
    if (_hp_sb_at_least(self, self->size + 1) != 0) return 1;
    self->elems[self->size] = item;
    self->size += 1;
    return 0;
}

int _hp_sb_free(_hp_sb *self) {
    free((void*) self->elems);
    self->elems = 0;
    self->capacity = 0;
    return 0;
}

#ifndef _WIN32
#error "Only windows is currently supported!"
#else
#include <stdio.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int _hp_file_exists(char const *name) {
    HANDLE file = CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        printf("File not found: %s\n", name);
        return 1;
    }
    CloseHandle(file);
        printf("File was found: %s\n", name);
    return 0;
}

int _hp_compare_last_edit(char const* lhs, char const* rhs) {
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

int _hp_cmd(char const* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    printf("$ %s\n", buf);
    int ret = system(buf);
    va_end(args);

    return ret;
}

int _hp_move(char const *src, char const *dst) {
    return _hp_cmd("move /y %s %s >NUL 2>&1", src, dst);
}

int _hp_delete(char const *tgt) {
    return _hp_cmd("del /q %s", tgt);
}

int _hp_build(char const* src, char const *exe) {
    return _hp_cmd("cl.exe /Fe:%s %s >NUL 2>&1", exe, src);
}

int _hp_mkdir(char const *dir) {
    printf("Creating dir: %s\n", dir);
    // creation succeeded or already exists
    if (CreateDirectory(dir, NULL) != 0
        || GetLastError() == ERROR_ALREADY_EXISTS) {
            return 0;
        } else {
            return 1;
        }
}

int _hp_compile_file(char const *src, char const *obj, char const *inc) {
    return _hp_cmd("cl.exe /c /Fo:%s /I %s %s >NUL 2>&1", obj, inc, src);
}

int _hp_link(_hp_sb *objects, char const *exe) {
    int all_objects_len = 1;
    for (int i = 0; i < objects->size; ++i) {
        all_objects_len += strlen(objects->elems[i]);
    }

    char *all_objects = hp_alloc(sizeof(*all_objects) * all_objects_len);
    *all_objects = 0;

    for (int i = 0; i < objects->size; ++i) {
        strcat(all_objects, objects->elems[i]);
    }

    char buf[1024];
    snprintf(buf, sizeof(buf), "cl.exe /Fe:%s %s >NUL 2>&1", exe, all_objects);
    hp_free(all_objects);

    return _hp_cmd(buf);
}

int _hp_run(char argc, char* argv[], char const* exe) {
    int command_len = strlen(exe) + 2;
    for (int i = 1; i < argc; ++i) {
        command_len += strlen(argv[i]);
    }

    char* command = hp_alloc(command_len);
    strcpy(command, exe);
    strcat(command, " ");

    for (int i = 1; i < argc; ++i) {
        strcat(command, argv[i]);
    }

    int ret = system(command);
    hp_free(command);
    return ret;
}

int _hp_expand_path(char const *path, char *buf, int buf_size) {
    return GetFullPathNameA(path, buf_size, buf, NULL);
}

int _hp_iterate_dir(char const *base, char const* ending, hocus_build_params *params, _hp_on_src_cb_t on_src) {
    printf("Searching %s\n", base);
    char buf[256];
    _hp_expand_path(base, buf, sizeof(buf));
    strncat(buf, "\\*", sizeof(buf));
    WIN32_FIND_DATAA data = {0};
    HANDLE search = FindFirstFileA(buf, &data);

    if (search == INVALID_HANDLE_VALUE) return 1;
    do {
        // check if it's a dir that isn't . or ..
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
            && strchr(data.cFileName, '.') != data.cFileName) {
            int const len = strlen(base) + 1 + strlen(data.cFileName) + 1;
            char *buf = hp_alloc(sizeof(*buf) * len);
            snprintf(buf, len, "%s\\%s", base, data.cFileName);
            _hp_iterate_dir(buf, ending, params, on_src);
            hp_free(buf);
        }

        // check that the file ends in the desired ending
        int str_len = strlen(data.cFileName);
        int ending_len = strlen(ending);
        if (strcmp(&data.cFileName[str_len - ending_len], ending) == 0) {
            int on_src_result = on_src(base, data.cFileName, params);
            if (on_src_result != 0) return on_src_result;
        }
    } while (FindNextFileA(search, &data));
    FindClose(search);
    return 0;
}

int _hp_on_src_recompile_and_mark_link(char const *base, char const *src, struct _hp_on_src_params *params) {
    char path_src[256];
    snprintf(path_src, sizeof(path_src), "%s\\%s", base, src);

    char path_dst[256];
    // splice from first \ to the end
    char const *nobase = strchr(base, '\\');
    if (nobase) {
        snprintf(path_dst, sizeof(path_dst), "%s\\%s\\%s", params->target_dir, nobase+1, src);
    } else {
        snprintf(path_dst, sizeof(path_dst), "%s\\%s", params->target_dir, src);
    }

    int len = strlen(path_dst);
    path_dst[len - 1] = 'o';

    // buf now contains the actual dest
    // check if recompile is needed
    printf("Checking recompile for: %s\n", path_src);
    if (_hp_file_exists(path_dst) != 0
        || _hp_compare_last_edit(path_src, path_dst) > 0) {
        printf("Recompiling: %s\n", path_src);
        int ret = _hp_compile_file(path_src, path_dst, params->include_dir);
        if (ret != 0) { return ret; }
        params->_do_relink = 1; // since we just updated an object we must relink
    }

    char the_binary[256];
    snprintf(the_binary, sizeof(the_binary), "%s\\%s", params->target_dir, params->the_binary);

    // set relink if the source's matching object file was updated by an external tool
    if (!params->_do_relink
        && _hp_compare_last_edit(path_dst, the_binary) > 0) {
            printf("Found object: %s newer than %s\n", path_dst, the_binary);
            params->_do_relink = 1;
        }
    
    int dest_len = strlen(path_dst) + 1;
    char *dyn_dest = hp_alloc(sizeof(*dyn_dest) * dest_len);
    strcpy(dyn_dest, path_dst);
    strcat(dyn_dest, " ");
    _hp_sb_push(&params->_objects, dyn_dest);
    return 0;
}
#endif//_WIN32

int hocus_pocus(char argc, char* argv[]) {
    char const* src = "hocus.c";
    char const* lib = "hocus.h";
    char const* exe = "hocus.exe";
    char const* old_exe = "hocus.old.exe";

#   ifdef HOCUS_DEV
    // if in dev environment
    // development is done on src/hocus.c and src/hocus.h
    // so check if any of them are newer than hocus.h
    // and if so, regenerate it
    // (which will automatically trigger recompilation)

    if (_hp_compare_last_edit("dev\\hocus.c", lib) > 0
        || _hp_compare_last_edit("dev\\hocus.h", lib) > 0) {
        printf("Regenerating %s...\n", lib);
        _hp_cmd("echo /*! Auto generated header from dev dir !*/ >%s", lib);
        _hp_cmd("echo: >>%s", lib);
        _hp_cmd("type dev\\hocus.h >>%s", lib);
        _hp_cmd("type dev\\hocus.c >>%s", lib);
    }
#   endif//HOCUS_DEV

    if (_hp_compare_last_edit(src, exe) < 0
        && _hp_compare_last_edit(lib, exe) < 0) { return 0; }
    
    _hp_move(exe, old_exe);
    if(_hp_build(src, exe) != 0) {
        printf("Build failed!\n");
        _hp_move(old_exe, exe);
        exit(1);
    }
    int err = _hp_run(argc, argv, exe);

    exit(err);
}

int hocus_clean(hocus_build_params *params) {
    return _hp_delete(params->target_dir);
}

int hocus_build(hocus_build_params *params) {
    // set defaults
    if (!params->target_dir) { params->target_dir = "target"; }
    if (!params->source_dir) { params->source_dir = "src"; }
    if (!params->include_dir) { params->include_dir = "include"; }
    if (!params->output_type) { params->output_type = 'o'; }

    _hp_mkdir(params->target_dir);
    return _hp_iterate_dir(params->source_dir, ".c", params, _hp_on_src_recompile_and_mark_link);
}

int hocus_link(hocus_build_params *params) {
    if (!params->_do_relink) {
        printf("Relink not requested\n");
        return 0;
    }

    printf("Relink requested\n");
    char the_binary[256];
    snprintf(the_binary, sizeof(the_binary), "%s\\%s", params->target_dir, params->the_binary);

    int dynamic_size = params->_objects.size;

    if (params->libs) {
        for (char const **it = params->libs; *it; ++it) {
            printf("Adding lib: %s\n", *it);
            _hp_sb_push(&params->_objects, *it);
            _hp_sb_push(&params->_objects, " ");
        }
    }

    int ret = _hp_link(&params->_objects, the_binary);
    for (int i = 0; i < dynamic_size; ++i) {
        hp_free((void*) params->_objects.elems[i]);
    }
    _hp_sb_free(&params->_objects);

    return ret;
}

#endif//DO_HOCUS_POCUS
