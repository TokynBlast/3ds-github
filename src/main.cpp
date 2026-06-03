#include <3ds.h>
#include <citro2d.h>
#include "settings.hpp"

#define newText(name, buff, text) \
        C2D_Text name; \
        C2D_TextParse(&name, buff, text); \
        C2D_TextOptimize(&name);

#define GRAY C2D_Color32(0x70, 0x70, 0x70, 0x7F)
#define WHITE C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)
#define newBuff(name, size) \
        C2D_TextBuf name = C2D_TextBufNew(size);

bool hasInternet()
{
    u32 status = 0;
    ACU_GetWifiStatus(&status);
    return status > 0;
}

int main() {
    Settings settings;

    gfxInitDefault();

    // Safer initalization; If it doesn't work,
    if (!C2D_Init(C2D_DEFAULT_MAX_OBJECTS) || !C3D_Init(C3D_DEFAULT_CMDBUF_SIZE))
    {
      gfxExit();
      return 1;
    }

    C2D_Prepare();

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bot = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    newBuff(generalTexBuff, 2048);
    newBuff(online_or_login_msg, 70);

    newText(title, generalTexBuff, "3DS GitHub");
    newText(online, online_or_login_msg, "");

    // Run till the user exits :)
    while (aptMainLoop())
    {
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        // Clear top screen
        C2D_TargetClear(top, C2D_Color32(0x0F, 0x61, 0xA5, 0xFF));

        // Begin drawing to bottom screen
        C2D_SceneBegin(top);

        C2D_DrawText(&title, C2D_WithColor, 10.0f, 10.0f, 0.4f, 2.0f, 2.0f, WHITE);

        C2D_TextBufClear(online_or_login_msg);

        if (!hasInternet()) {
            C2D_TextParse(&online, online_or_login_msg, "You're currently offline.\nPlease connect to the internet\nto access GitHub.");
        } else {
            if (signed_in)
            {
                C2D_TextParse(&online, online_or_login_msg, "");
            } else {
                C2D_TextParse(&online, online_or_login_msg, "Please sign into GitHub.");
            }
        }

        C2D_TextOptimize(&online);
        C2D_DrawText(&online, C2D_WithColor, 10.0f, 55.0f, 0.5f, 0.7f, 0.7f, WHITE);

        // Fill in bottom screen, then start drawing everything that belongs there
        C2D_TargetClear(bot, C2D_Color32(0x0F, 0x61, 0xA5, 0xFF));
        C2D_SceneBegin(bot);

        C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, 50.0f, GSP_SCREEN_HEIGHT_BOTTOM, GRAY);

        // Tell the 3DS the frame is ready
        C3D_FrameEnd(0);
    }

    C2D_TextBufDelete(generalTexBuff);
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}

#undef newText
