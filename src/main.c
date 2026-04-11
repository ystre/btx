#include "btx/btx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_all(FILE *f, uint8_t **out, size_t *out_len) {
    size_t cap = 4096, len = 0;
    uint8_t *buf = malloc(cap);
    if (!buf) return -1;
    size_t n;
    while ((n = fread(buf + len, 1, cap - len, f)) > 0) {
        len += n;
        if (len == cap) {
            uint8_t *nb = realloc(buf, cap *= 2);
            if (!nb) { free(buf); return -1; }
            buf = nb;
        }
    }
    *out     = buf;
    *out_len = len;
    return 0;
}

static FILE *open_input(const char *path, const char *mode) {
    return strcmp(path, "-") == 0 ? stdin : fopen(path, mode);
}

static int cmd_decode(const char *path) {
    FILE *f = open_input(path, "r");
    if (!f) { perror(path); return 1; }

    uint8_t *raw = NULL; size_t raw_len = 0;
    if (read_all(f, &raw, &raw_len) < 0) {
        fprintf(stderr, "btx: out of memory\n");
        if (f != stdin) fclose(f);
        return 1;
    }
    if (f != stdin) fclose(f);

    uint8_t *out = NULL; size_t out_len = 0;
    btx_result_t r = btx_decode((char *)raw, raw_len, &out, &out_len);
    free(raw);
    if (r != BTX_OK) {
        fprintf(stderr, "btx: %s\n", btx_strerror(r));
        return 1;
    }
    fwrite(out, 1, out_len, stdout);
    btx_free(out);
    return 0;
}

static int cmd_encode(const char *path) {
    FILE *f = open_input(path, "rb");
    if (!f) { perror(path); return 1; }

    uint8_t *raw = NULL; size_t raw_len = 0;
    if (read_all(f, &raw, &raw_len) < 0) {
        fprintf(stderr, "btx: out of memory\n");
        if (f != stdin) fclose(f);
        return 1;
    }
    if (f != stdin) fclose(f);

    char *out = NULL; size_t out_len = 0;
    btx_result_t r = btx_encode(raw, raw_len, &out, &out_len);
    free(raw);
    if (r != BTX_OK) {
        fprintf(stderr, "btx: %s\n", btx_strerror(r));
        return 1;
    }
    fwrite(out, 1, out_len, stdout);
    btx_free(out);
    return 0;
}

static int cmd_validate(const char *path) {
    FILE *f = open_input(path, "r");
    if (!f) { perror(path); return 1; }

    uint8_t *raw = NULL; size_t raw_len = 0;
    if (read_all(f, &raw, &raw_len) < 0) {
        fprintf(stderr, "btx: out of memory\n");
        if (f != stdin) fclose(f);
        return 1;
    }
    if (f != stdin) fclose(f);

    btx_result_t r = btx_validate((char *)raw, raw_len);
    free(raw);
    if (r != BTX_OK) {
        fprintf(stderr, "btx: %s\n", btx_strerror(r));
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc >= 2
        && (strcmp(argv[1], "--version") == 0
         || strcmp(argv[1], "-V") == 0)) {
        printf("btx %s\n", BTX_VERSION);
        return 0;
    }
    if (argc < 2) {
        fprintf(stderr, "Usage: btx [options] <command> [args]\ntry: btx --help\n");
        return 1;
    }
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        printf(
            "Usage: btx [options] <command> [args]\n"
            "\n"
            "commands:\n"
            "  encode <file>     encode binary to BTX text, write to stdout\n"
            "  decode <file>     decode BTX text to binary, write to stdout\n"
            "  validate <file>   validate BTX text; exit 1 on error\n"
            "\n"
            "options:\n"
            "  -h, --help        show this help\n"
            "  -V, --version     print version\n"
            "\n"
            "use - as <file> to read from stdin\n"
        );
        return 0;
    }
    if (strcmp(argv[1], "decode")   == 0
     || strcmp(argv[1], "encode")   == 0
     || strcmp(argv[1], "validate") == 0) {
        if (argc < 3) {
            fprintf(stderr, "btx: '%s' requires a file argument\ntry: btx --help\n", argv[1]);
            return 1;
        }
        if (argc > 3) {
            fprintf(stderr, "btx: unexpected argument '%s'\ntry: btx --help\n", argv[3]);
            return 1;
        }
        if (strcmp(argv[1], "decode")   == 0) return cmd_decode(argv[2]);
        if (strcmp(argv[1], "encode")   == 0) return cmd_encode(argv[2]);
        if (strcmp(argv[1], "validate") == 0) return cmd_validate(argv[2]);
    }
    fprintf(stderr, "btx: unknown command '%s'\ntry: btx --help\n", argv[1]);
    return 1;
}
