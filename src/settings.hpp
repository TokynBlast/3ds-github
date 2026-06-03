#include <string>

class Settings {
  public:
      enum class SideBarCat {
          Settings,
          Search,
          MyAccount,
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

      OpenSettings normal;
      InternalSettings internal;
      SideBarCat sbar_category = SideBarCat::Settings;

      Settings() : normal(), internal() {}
};
