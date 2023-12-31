
// TODO: integrate this into hocus.h

int _hp_expand_path(char const *path, char *buf, int buf_size) {
    return GetFullPathNameA(path, buf_size, buf, NULL);
}

typedef int(*_hp_on_src_cb_t)(char const *base, char const* src, void *user_data);

int _hp_iterate_dir(char const *base, void *user_data, _hp_on_src_cb_t on_src) {
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
            _hp_iterate_dir(buf, user_data, on_src);
            free(buf);
        }

        // check that the file ends in .c
        int str_len = strlen(data.cFileName);
        if (strcmp(&data.cFileName[str_len - 2], ".c") == 0) {
            int on_src_result = on_src(base, data.cFileName, user_data);
            if (on_src_result != 0) return on_src_result;
        }
    } while (FindNextFileA(search, &data));
    FindClose(search);
    return 0;
}
