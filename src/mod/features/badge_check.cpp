#include "exlaunch.hpp"
#include "externals/PlayerWork.h"
#include "externals/FlagWork_Enums.h"
#include "externals/Dpr/Battle/Logic/MainModule.h"

HOOK_DEFINE_REPLACE(BadgeWork_IsGet) {
    static bool Callback(int32_t id) {
        if (id >= 8)
        {
            return false;
        }

        int32_t systemFlag;
        switch (id)
        {
            case 3:
                systemFlag = BADGE_ID_C05;
                break;
            case 4:
                systemFlag = BADGE_ID_C06;
                break;
            default:
                systemFlag = id + 0x7c;
                break;
        }

        return PlayerWork::GetSystemFlag(systemFlag);
    }
};

HOOK_DEFINE_REPLACE(ObedienceMax) {
    static uint8_t Callback(Dpr::Battle::Logic::MainModule *__this) {
        return 100;
    }
};

void exl_badge_check_main() {
    BadgeWork_IsGet::InstallAtOffset(0x01d603e0);
    ObedienceMax::InstallAtOffset(0x020349e0);
}
