#include <string>

class Settings {
  public:
      enum class SideBarCat {
          Settings,
          Search,
          MyAccount,
          // This is used to track the total, and therefore MUST ALWAYS
          // go at the end of the enum, or it WILL break.
          Count,
      };

      enum class Actions {
          LogOut,
          LogIn,
          ChangeAccount,
      };

      struct OpenSettings {
          float font_size = 0.7f;
      };

      struct InternalSettings {
          std::string username = "";
          std::string password = "";
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
      SideBarCat sbar_category = SideBarCat::Settings;

      Settings() : normal(), internal() {}
};
