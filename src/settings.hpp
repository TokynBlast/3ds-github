#include <string>

class Settings {
  public:
      enum class SideBarCat {
          // The users account
          MyAccount,
          Search,
          Settings,
          // ## DO NOT USE
          // This is used internally with the cycling functions.
          // It it not meant for anywhere else.
          Count,
      };

      struct OpenSettings
      {
          float font_size = 0.7f;
      };

      // Settings that should be hidden, or the user should have no reason to modify directly
      struct InternalSettings {
          std::string user_id = "", password = "";
          bool signed_in = false;
      };

      // Move the current setting left :)
      void cycleCatsLeft() {
          int current = static_cast<int>(sbar_category);

          // Increment then wrap around
          int next = (current + 1) % static_cast<int>(SideBarCat::Count);

          sbar_category = static_cast<SideBarCat>(next);
      }

      void CycleCatsRight() {
          int current = static_cast<int>(sbar_category);
          int count = static_cast<int>(SideBarCat::Count);

          int next = (current + count - 1) % count;

          sbar_category = static_cast<SideBarCat>(next);
      }

      OpenSettings normal;
      InternalSettings internal;
      SideBarCat sbar_category = SideBarCat::MyAccount;

      Settings() : normal(), internal() {}
};
