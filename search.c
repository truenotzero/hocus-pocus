
// TODO: integrate this into hocus.h

int _hp_expand_path(char const *path, char *buf, int buf_size) {
    return GetFullPathNameA(path, buf_size, buf, NULL);
}

typedef int(*_hp_on_src_cb_t)(char const *base, char const* src, void *user_data);

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

int _hp_on_src_compile(char const *base, char const *src, char const *target) {
    char dest[256];
    printf("src: %s\\%s\n", base, src);

    // splice from first \ to the end
    char const *nobase = strchr(base, '\\');
    if (nobase) {
        snprintf(dest, sizeof(dest), "%s\\%s\\%s", target, nobase+1, src);
    } else {
        snprintf(dest, sizeof(dest), "%s\\%s", target, src);
    }
    int len = strlen(dest);
    dest[len - 1] = 'o';

    // buf now contains the actual dest
    // check if recompile is needed
    #if 0
    if (_hp_check_last_edit(src, dest) < 0) {
        return _hp_compile(src);
    }
    #endif
    return 0;
}
