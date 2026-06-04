#include <vector>
#include "actions.hpp"

class CurrentState {
    public:
        enum AccountStatus {
            // Signing into an account
            SignedIn,
            // Not signed into an account
            SignedOut,
            // Currently signing in
            SigningInto,
        } acc_stat = SignedOut;
        std::vector<Actions> actions;
};
