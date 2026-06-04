#include <3ds.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

// Unique code :)
#define MOVABLE_SED "sdmc:/Nintendo 3DS/private/movable.sed"
#define APP_DIR "sdmc:/meimei-dev/3ds_github/"
#define TOKEN_PATH "sdmc:/meimei-dev/3ds_github/tok.tok"

// derive a simple key from movable.sed
// (the first 0x10 bytes after the header are unique per console)
bool derive_key(u8 key[16]) {
    FILE *f = fopen(MOVABLE_SED, "rb");
    if (!f) return false;
    fseek(f, 0x110, SEEK_SET); // keyY offset in movable.sed
    fread(key, 1, 16, f);
    fclose(f);
    return true;
}

// simple XOR encryption with the key (good enough for token protection)
void xor_crypt(u8 *data, size_t len, const u8 key[16]) {
    for (size_t i = 0; i < len; i++)
        data[i] ^= key[i % 16];
}

void save_token(const char *token) {
    u8 key[16];
    if (!derive_key(key)) return;

    mkdir("sdmc:/meimei-dev", 0777);
    mkdir("sdmc:/meimei-dev/3ds_github", 0777);

    size_t len = strlen(token);
    u8 *buf = static_cast<u8 *>(malloc(len));
    memcpy(buf, token, len);
    xor_crypt(buf, len, key);

    FILE *f = fopen(TOKEN_PATH, "wb");
    fwrite(&len, 1, sizeof(size_t), f); // store length prefix
    fwrite(buf, 1, len, f);
    fclose(f);
    free(buf);
}

bool load_token(char *token, size_t maxlen) {
    u8 key[16];
    if (!derive_key(key)) return false;

    FILE *f = fopen(TOKEN_PATH, "rb");
    if (!f) return false;

    size_t len;
    fread(&len, 1, sizeof(size_t), f);
    if (len >= maxlen) { fclose(f); return false; }

    u8 *buf = static_cast<u8 *>(malloc(len));
    fread(buf, 1, len, f);
    fclose(f);

    xor_crypt(buf, len, key);
    memcpy(token, buf, len);
    token[len] = '\0';
    free(buf);
    return true;
}
