#include <3ds.h>
#include <citro2d.h>
#include "settings.hpp"

#define newText(name, buff, text) \
        C2D_Text name; \
        C2D_TextParse(&name, buff, text); \
        C2D_TextOptimize(&name);

#define GRAY C2D_Color32(0x70, 0x70, 0x70, 0xFF)
#define WHITE C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)
#define BLACK C2D_Color32(0x00, 0x00, 0x00, 0xFF)
#define DARK_GRAY C2D_Color32(0x40, 0x40, 0x40, 0xFF)
#define LIGHT_GRAY C2D_Color32(0x90, 0X90, 0X90, 0XFF)
#define DARK_BLUE C2D_Color32(0X20, 0X20, 0XFF, 0XFF)

#define newBuff(name, size) \
        C2D_TextBuf name = C2D_TextBufNew(size);

bool hasInternet()
{
    acInit();
    u32 status = 0;
    ACU_GetWifiStatus(&status);
    acExit();
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

    newText(title, generalTexBuff, "3DS GitHub");
    newText(online, generalTexBuff, "You're currently offline.\nPlease connect to the internet\nto access GitHub.");

    // Run till the user exits :)
    while (aptMainLoop())
    {
        hidScanInput();
        u32 keys = hidKeysDown();

        if (keys & KEY_B) {
            settings.sbar_category = Settings::SideBarCat::Search;
        } else if (keys & KEY_A) {
            settings.sbar_category = Settings::SideBarCat::Settings;
        } else if (keys & KEY_CPAD_DOWN) {
            settings.sbar_category = Settings::SideBarCat::MyAccount;
        }

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        // Clear top screen
        C2D_TargetClear(top, C2D_Color32(0x0F, 0x61, 0xA5, 0xFF));

        // Begin drawing to bottom screen
        C2D_SceneBegin(top);

        C2D_DrawText(&title, C2D_WithColor, 10.0f, 10.0f, 0.4f, 2.0f, 2.0f, WHITE);

        C2D_TextBufClear(online_or_login_msg);

        if (!hasInternet()) {
            C2D_DrawText(&online, C2D_WithColor, 10.0f, 55.0f, 0.5f, 0.7f, 0.7f, WHITE);
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
                C2D_DrawCircle(0.0f, 0.0f, 0.0f, 5.0f, BLACK, WHITE, LIGHT_GRAY, GRAY);
                break;
            case Settings::SideBarCat::Search:
                break;
        }

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
