#pragma once
#include <3ds.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cacert.h"

#define GITHUB_API   "https://api.github.com"
#define GITHUB_AUTH  "https://github.com"
#define CLIENT_ID    "Ov23likpciTh8y1ArBrK"

// call once after httpcInit()
inline u32 net_create_cert_chain() {
    u32 chain;
    httpcCreateRootCertChain(&chain);
    for (int i = 0; i < cacert_count; i++) {
        u32 handle;
        httpcRootCertChainAddCert(chain, cacerts[i].data, cacerts[i].size, &handle);
    }
    return chain;
}

// download full response into a caller-provided buffer, returns bytes read or -1
inline int net_download(httpcContext *ctx, char *buf, size_t maxlen) {
    u32 status = 0;
    httpcGetResponseStatusCode(ctx, &status);
    if (status != 200) return -1;

    memset(buf, 0, maxlen);
    size_t total = 0;
    Result ret;
    do {
        u32 chunk = (maxlen - total) > 1024 ? 1024 : (maxlen - total);
        ret = httpcReceiveData(ctx, (u8*)(buf + total), chunk);
        u32 got = 0;
        httpcGetDownloadSizeState(ctx, &got, nullptr);
        total = got;
    } while (ret == (Result)HTTPC_RESULTCODE_DOWNLOADPENDING && total < maxlen - 1);
    return (int)total;
}

// tiny JSON field extractor — finds "key":"value" or "key":value
// writes value into out, returns true on success
inline bool json_get(const char *json, const char *key, char *out, size_t outlen) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":", key);
    const char *p = strstr(json, search);
    if (!p) return false;
    p += strlen(search);
    while (*p == ' ') p++;
    bool quoted = (*p == '"');
    if (quoted) p++;
    size_t i = 0;
    while (*p && i < outlen - 1) {
        if (quoted  && *p == '"')  break;
        if (!quoted && (*p == ',' || *p == '}')) break;
        out[i++] = *p++;
    }
    out[i] = '\0';
    return i > 0;
}

// --- GitHub device flow ---

struct DeviceCodeResp {
    char device_code[64];
    char user_code[16]; // Code to enter (e.g., 1234-ABC)
    char verification_uri[64];
    int  interval; // polling interval in seconds
};

inline bool github_request_device_code(u32 certChain, DeviceCodeResp *out) {
    httpcContext ctx;
    httpcOpenContext(&ctx, HTTPC_METHOD_POST,
        GITHUB_AUTH "/login/device/code", 0);
    httpcSelectRootCertChain(&ctx, certChain);
    httpcAddRequestHeaderField(&ctx, "Accept",     "application/json");
    httpcAddRequestHeaderField(&ctx, "User-Agent", "3DS-GitHub/1.0");
    httpcAddPostDataAscii(&ctx, "client_id", CLIENT_ID);
    httpcAddPostDataAscii(&ctx, "scope",     "repo read:user");
    httpcBeginRequest(&ctx);

    char buf[1024];
    int n = net_download(&ctx, buf, sizeof(buf));
    httpcCloseContext(&ctx);
    if (n < 0) return false;

    json_get(buf, "device_code",       out->device_code,       sizeof(out->device_code));
    json_get(buf, "user_code",         out->user_code,         sizeof(out->user_code));
    json_get(buf, "verification_uri",  out->verification_uri,  sizeof(out->verification_uri));
    char interval_str[8] = "5";
    json_get(buf, "interval", interval_str, sizeof(interval_str));
    out->interval = atoi(interval_str);
    return out->device_code[0] != '\0';
}

// returns true + fills token on success, false if still pending
inline bool github_poll_token(u32 certChain, const char *device_code, char *token_out, size_t token_maxlen) {
    httpcContext ctx;
    httpcOpenContext(&ctx, HTTPC_METHOD_POST,
        GITHUB_AUTH "/login/oauth/access_token", 0);
    httpcSelectRootCertChain(&ctx, certChain);
    httpcAddRequestHeaderField(&ctx, "Accept",     "application/json");
    httpcAddRequestHeaderField(&ctx, "User-Agent", "3DS-GitHub/1.0");
    httpcAddPostDataAscii(&ctx, "client_id",   CLIENT_ID);
    httpcAddPostDataAscii(&ctx, "device_code", device_code);
    httpcAddPostDataAscii(&ctx, "grant_type",  "urn:ietf:params:oauth:grant-type:device_code");
    httpcBeginRequest(&ctx);

    char buf[512];
    int n = net_download(&ctx, buf, sizeof(buf));
    httpcCloseContext(&ctx);
    if (n < 0) return false;

    // check for error field first (authorization_pending, slow_down, expired_token)
    char err[32] = {};
    if (json_get(buf, "error", err, sizeof(err)))
        return false;  // still waiting or failed

    return json_get(buf, "access_token", token_out, token_maxlen);
}

// fetch the authenticated user's login name, returns true on success
inline bool github_get_username(u32 certChain, const char *token, char *name_out, size_t maxlen) {
    httpcContext ctx;
    httpcOpenContext(&ctx, HTTPC_METHOD_GET, GITHUB_API "/user", 0);
    httpcSelectRootCertChain(&ctx, certChain);
    char auth[128];
    snprintf(auth, sizeof(auth), "token %s", token);
    httpcAddRequestHeaderField(&ctx, "Authorization", auth);
    httpcAddRequestHeaderField(&ctx, "User-Agent",    "3DS-GitHub/1.0");
    httpcAddRequestHeaderField(&ctx, "Accept",        "application/vnd.github+json");
    httpcBeginRequest(&ctx);

    char buf[2048];
    int n = net_download(&ctx, buf, sizeof(buf));
    httpcCloseContext(&ctx);
    if (n < 0) return false;

    return json_get(buf, "login", name_out, maxlen);
}
