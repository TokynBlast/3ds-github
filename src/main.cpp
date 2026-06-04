#include <3ds.h>
#include <citro2d.h>
#include "settings.hpp"
#include <3ds/services/ac.h>
#include <vector>
#include "state.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <3ds/services/httpc.h>
#include "cacert.h"
#include "network.hpp"
#include "token.hpp"

// Make a new text object on a specified buffer
#define newText(name, buff, text) \
        C2D_Text name; \
        C2D_TextParse(&name, buff, text); \
        C2D_TextOptimize(&name);

#define newBuff(name, size) \
        C2D_TextBuf name = C2D_TextBufNew(size);

#define GRAY C2D_Color32(0x70, 0x70, 0x70, 0xFF) // RGB HEX: #707070
#define WHITE C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF) // RGB HEX: #FFFFFF
#define BLACK C2D_Color32(0x00, 0x00, 0x00, 0xFF) // RGB HEX: #000000
#define DARK_GRAY C2D_Color32(0x40, 0x40, 0x40, 0xFF) // RGB HEX: #404040
#define LIGHT_GRAY C2D_Color32(0x90, 0X90, 0X90, 0XFF) // RGB HEX: #909090
#define DARK_BLUE C2D_Color32(0x20, 0x20, 0xFF, 0xFF) // RGB HEX: #2020FF

bool hasInternet()
{
    u32 status = 0;
    ACU_GetWifiStatus(&status);
    return status > 0;
}


// Checks whether the user clicked within an area
bool touchedThere(u16 x, u16 y, u16 w, u16 h) {
    touchPosition touched;
    hidTouchRead(&touched);
    u16 touch_x = touched.px,
        touch_y = touched.py;

    // Check if it's within the X and Y axi, and that the touch is happening!
    if (touch_x >= x && touch_x <= x + w)
        if (touch_y >= y && touch_y <= y + h )
            return true;

    return false;
}

int main() {
    gfxInitDefault();

    // Safer initalization; If it doesn't work, exit
    if (!C2D_Init(C2D_DEFAULT_MAX_OBJECTS) || !C3D_Init(C3D_DEFAULT_CMDBUF_SIZE))
    {
        C2D_Fini();
        C3D_Fini();
        gfxExit();
        return 1;
    }

    acInit();
    httpcInit(0);
    u32 certChain = net_create_cert_chain();

    // Init program
    Settings settings;
    CurrentState state;
    state.actions.reserve(20);

    C2D_Prepare();

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bot = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    newBuff(generalTexBuff, 2048);
    // The GitHub display name allows up to 255 chars.
    // However, emojis and other special characters take up two instead,
    // meaning 127 special chars at most, which take up 508 bytes.
    // Finally, we account for '\0' :)
    newBuff(pretty_name_buffer, 509);
    newBuff(user_code_buf, 16); // "XXXX-XXXX\0" + breathing room

    C2D_Text user_code_text;

    newText(title, generalTexBuff, "3DS GitHub");
    newText(offline, generalTexBuff, "You're currently offline.\nPlease connect to the internet\nto access GitHub.");
    newText(login_to_access_acc, generalTexBuff, "Please sign in to access your\nGitHub account.");
    newText(pretty_name, pretty_name_buffer, "");

    newText(device_flow_prompt, generalTexBuff, "Go to github.com/login/device\nand enter this code:\n\n\nA - Accept\bY - Get new code");

    DeviceCodeResp dcr = {};
    char token[64] = {};
    bool dcr_fetched = false;
    bool waiting_for_auth = false;
    u64 last_poll_time = 0;

    // Run till the user exits :)
    while (aptMainLoop())
    {
        hidScanInput();
        u32 keys = hidKeysDown();

        if (keys & KEY_L) {
            settings.cycleCatsLeft();
        } else if (keys & KEY_R) {
            settings.CycleCatsRight();
        } else if (keys & KEY_START) {
            state.actions.push_back(Actions::Reload);
        }

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        // Clear top screen
        C2D_TargetClear(top, C2D_Color32(0x0F, 0x61, 0xA5, 0xFF));

        // Begin drawing to bottom screen
        C2D_SceneBegin(top);

        C2D_DrawText(&title, C2D_WithColor, 10.0f, 10.0f, 0.4f, 2.0f, 2.0f, WHITE);

        if (!hasInternet()) {
            C2D_DrawText(&offline, C2D_WithColor, 10.0f, 55.0f, 0.5f, 0.7f, 0.7f, WHITE);
        }

        // Fill in bottom screen, then start drawing everything that belongs there
        C2D_TargetClear(bot, C2D_Color32(0x0F, 0x61, 0xA5, 0xFF));
        C2D_SceneBegin(bot);

        C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, 50.0f, GSP_SCREEN_HEIGHT_BOTTOM, DARK_GRAY);

        switch (settings.sbar_category) {
            case Settings::SideBarCat::Settings:
                C2D_DrawRectSolid(50.0f, 0.0f, 00.0f, GSP_SCREEN_WIDTH + 40, GSP_SCREEN_HEIGHT_BOTTOM, BLACK);
                break;
            case Settings::SideBarCat::MyAccount:
                if (settings.internal.signed_in) {
                    C2D_DrawText(&pretty_name, C2D_WithColor, 20.0f, 50.0f, 0.0f, 0.6f, 0.6f, WHITE);
                } else {
                    if (!waiting_for_auth) {
                        C2D_DrawText(&login_to_access_acc, C2D_WithColor, 50.0f, 20.0f, 0.0f, 0.6f, 0.7f, WHITE);
                        C2D_DrawRectSolid(100.0f, 55.0f, 0.0f, 90.0f, 30.0f, DARK_BLUE);

                        if (touchedThere(100, 55, 90, 30) && !dcr_fetched) {
                            if (github_request_device_code(certChain, &dcr)) {
                                dcr_fetched = true;
                                waiting_for_auth = true;
                                last_poll_time = osGetTime();

                                // parse the user code into our text object
                                C2D_TextBufClear(user_code_buf);
                                C2D_TextParse(&user_code_text, user_code_buf, dcr.user_code);
                                C2D_TextOptimize(&user_code_text);
                            }
                        }
                    } else {
                        // instruction
                        C2D_DrawText(&device_flow_prompt, C2D_WithColor, 60.0f, 10.0f, 0.0f, 0.55f, 0.55f, WHITE);
                        // big user code
                        C2D_DrawText(&user_code_text, C2D_WithColor, 60.0f, 70.0f, 0.5f, 1.4f, 1.4f, WHITE);

                        // poll every dcr.interval seconds
                        u64 now = osGetTime();
                        if (now - last_poll_time >= (u64)dcr.interval * 1000) {
                            last_poll_time = now;
                            if (github_poll_token(certChain, dcr.device_code, token, sizeof(token))) {
                                save_token(token);
                                state.acc_stat = CurrentState::AccountStatus::SignedIn;
                                settings.internal.signed_in = true;
                                waiting_for_auth = false;
                                dcr_fetched = false;
                            }
                        }
                    }
                }
                break;
            case Settings::SideBarCat::Search:
                C2D_DrawRectSolid(100.0f, 20.0f, 0.0f, 90.0f, 30.0f, DARK_BLUE);
                break;
            case Settings::SideBarCat::Count:
                // Incase we somehow get here...
                settings.sbar_category = Settings::SideBarCat::Search;
                break;
        }

        // Tell the 3DS the frame is ready
        C3D_FrameEnd(0);
    }

    C2D_TextBufDelete(generalTexBuff);
    C2D_Fini();
    C3D_Fini();
    httpcExit();
    acExit();
    gfxExit();
    return 0;
}
