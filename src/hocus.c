#ifdef DO_HOCUS_POCUS
// IMPLEMENTATION

#ifndef _WIN32
#error "Only windows is currently supported!"
#else
#include <stdio.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

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
    int ret = system(buf);
    va_end(args);

    return ret;
}

int _hp_move(char const* src, char const* dst) {
    return _hp_cmd("move /y %s %s >NUL 2>&1", src, dst);
}

int _hp_delete(char const* tgt) {
    return _hp_cmd("del /q %s", tgt);
}

int _hp_build(char const* src, char const* exe) {
    return _hp_cmd("cl.exe /Fe:%s %s >NUL 2>&1", exe, src);
}

int _hp_mkdir(char const* dir) {
    return _hp_cmd("mkdir %s >NUL 2>&1");
}

int _hp_run(char argc, char* argv[], char const* exe) {
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

    if (_hp_compare_last_edit(src, exe) < 0
        /*&& _hp_compare_last_edit(lib, exe) < 0*/) return 0;
    
    _hp_move(exe, old_exe);
    if(_hp_build(src, exe) != 0) {
        printf("Build failed!\n");
        _hp_move(old_exe, exe);
        exit(1);
    }
    int err = _hp_run(argc, argv, exe);

    exit(err);
}

int _hp_expand_path(char const *path, char *buf, int buf_size) {
    return GetFullPathNameA(path, buf_size, buf, NULL);
}

int _hp_iterate_dir(char const *base, char const* ending, void *user_data, _hp_on_src_cb_t on_src) {
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
            char *buf = malloc(sizeof(*buf) * len);
            snprintf(buf, len, "%s\\%s", base, data.cFileName);
            _hp_iterate_dir(buf, ending, user_data, on_src);
            free(buf);
        }

        // check that the file ends in the desired ending
        int str_len = strlen(data.cFileName);
        int ending_len = strlen(ending);
        if (strcmp(&data.cFileName[str_len - ending_len], ending) == 0) {
            int on_src_result = on_src(base, data.cFileName, user_data);
            if (on_src_result != 0) return on_src_result;
        }
    } while (FindNextFileA(search, &data));
    FindClose(search);
    return 0;
}

struct _hp_on_src_params {
    char output_type;
    char const *target_dir;
    char const *the_binary;
    int do_relink;
};

int _hp_on_src(char const *base, char const *src, struct _hp_on_src_params *params) {
    char dest[256];
    printf("src: %s\\%s\n", base, src);

    // splice from first \ to the end
    char const *nobase = strchr(base, '\\');
    if (nobase) {
        snprintf(dest, sizeof(dest), "%s\\%s\\%s", params->target_dir, nobase+1, src);
    } else {
        snprintf(dest, sizeof(dest), "%s\\%s", params->target_dir, src);
    }
    int len = strlen(dest);
    dest[len - 1] = 'o';

    // buf now contains the actual dest
    // check if recompile is needed
    if (_hp_compare_last_edit(src, dest) < 0) {
        int ret = 0;// TODO: _hp_compile(src);
// #       error "TODO: _hp_compile(src);"
        if (ret != 0) { return ret; }
        params->do_relink = 1; // since we just updated an object we must relink
    }

    // set relink if the source's matching object file was updated by an external tool
    if (!params->do_relink
        && _hp_compare_last_edit(dest, params->the_binary) < 0) {
            params->do_relink = 1;
        }
    
// #   error "TODO: push_objectfile(&params->objects);"
    return 0;
}
#endif//_WIN32
#endif//DO_HOCUS_POCUS
