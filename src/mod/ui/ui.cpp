#include "ui/ui.h"
#include "externals/ItemWork.h"
#include "data/items.h"
#include "externals/PlayerWork.h"
#include "data/species.h"
#include "externals/il2cpp.h"
#include "data/arenas.h"
#include "data/zones.h"
#include "externals/Pml/PmlUse.h"
#include "externals/Pml/PokePara/PokemonParam.h"
#include "data/moves.h"
#include "data/utils.h"
#include "save/save.h"

using namespace ui;

static Window debugLogWindow = Window::single([](Window &_) {
    _.title = "Debug Log";
    auto *view = _.StringView();

    Logger::addListener([view](const char *message) {
        view->append(message);
    });
});

static ElementInspector inspector = ElementInspector::single([](ElementInspector &_) {

});

void ui_inspect(UnityEngine::Transform* transform) {
    inspector.root = transform;
}

static Window inspectorWindow = Window::single([](Window &_) {
    _.title = "Inspector";
    _.flags |= ImGuiWindowFlags_MenuBar;
    _.initialPos = ImVec2(50, 50);
    _.initialSize = ImVec2(600, 650);

    _.MenuBar([](MenuBar &_) {
        _.Menu([](Menu &_) {
            _.label = "Options";
            _.MenuItem([](MenuItem &_) {
                _.label = "Show Children";
                _.checked = &inspector.showChildren;
            });
            _.MenuItem([](MenuItem &_) {
                _.label = "Show Components";
                _.checked = &inspector.showComponents;
            });
        });
    });
    _.addChild(&inspector);
});

static Window mainWindow = Window::single([](Window &_) {
    _.title = "BDSP Toolbox";
    _.toggleable = false;
    _.flags |= ImGuiWindowFlags_MenuBar;
    _.initialPos = ImVec2(50, 50);
    _.initialSize = ImVec2(250, 300);

    _.Text([](Text &_) {
        _.content = "Press ZL+ZR to toggle all menus.\nHold L to walk through walls.";
    });

    _.Spacing();

    _.MenuBar([](MenuBar &_) {
        _.Menu([](Menu &_) {
            _.label = "Tools";
            _.MenuItem([](MenuItem &_) {
                _.label = "Debug Log";
                _.checked = &debugLogWindow.open;
            });
            _.MenuItem([](MenuItem &_) {
                _.label = "Inspector";
                _.checked = &inspectorWindow.open;
            });
        });
    });

    _.CollapsingHeader([](CollapsingHeader &_) {
        _.label = "Item Tool";

        auto* item = _.ComboSimple([](ComboSimple &_) {
            _.label = " Item";
            _.items = ITEMS;
            _.items_count = ITEM_COUNT;
            _.selected = array_index(ITEMS, "Master Ball");
        });
        auto *amount = _.InputInt([](InputInt &_) {
            _.label = "Amount";
            _.min = 1;
            _.max = 100;
            _.value = 1;
        });
        _.Button([item, amount](Button &_) {
            _.label = "Give Item";
            _.onClick = [item, amount]() {
                ItemWork::AddItem(item->selected, amount->value);
            };
        });
    });

    _.CollapsingHeader([](CollapsingHeader &_) {
        _.label = "Pokemon Tool";

        auto* species = _.ComboSimple([](ComboSimple &_) {
            _.label = " Species";
            _.items = SPECIES;
            _.items_count = SPECIES_COUNT;
            _.selected =  array_index(SPECIES, "Bulbasaur");
        });
        auto *level = _.InputInt([](InputInt &_) {
            _.label = "Level";
            _.min = 1;
            _.max = 100;
            _.value = 5;
        });
        auto* customMoves = _.Checkbox([](Checkbox &_) {
            _.label = "Custom moves";
            _.enabled = false;
        });
        auto *move1 = _.ComboSimple([](ComboSimple &_) {
            _.label = " Move 1";
            _.items = MOVES;
            _.items_count = MOVE_COUNT;
            _.selected = array_index(MOVES, "Pound");
        });
        auto *move2 = _.ComboSimple([](ComboSimple &_) {
            _.label = " Move 2";
            _.items = MOVES;
            _.items_count = MOVE_COUNT;
            _.selected = array_index(MOVES, "Karate Chop");
        });
        auto *move3 = _.ComboSimple([](ComboSimple &_) {
            _.label = " Move 3";
            _.items = MOVES;
            _.items_count = MOVE_COUNT;
            _.selected = array_index(MOVES, "Double Slap");
        });
        auto *move4 = _.ComboSimple([](ComboSimple &_) {
            _.label = " Move 4";
            _.items = MOVES;
            _.items_count = MOVE_COUNT;
            _.selected = array_index(MOVES, "Comet Punch");
        });

        _.Button([species, level, customMoves, move1, move2, move3, move4](Button &_) {
            _.label = "Give Pokemon";
            _.onClick = [species, level, customMoves, move1, move2, move3, move4]() {
                auto party = PlayerWork::get_playerParty();

                auto param = Pml::PokePara::PokemonParam::newInstance(species->selected, level->value, 0);
                auto pmlUse = Pml::PmlUse::get_Instance();
                int32_t lang = pmlUse->get_LangId();

                param->fields.m_accessor->SetLangID(lang);
                param->fields.m_accessor->SetOwnedOthersFlag(false);

                if (customMoves->enabled) {
                    auto core = param->cast<Pml::PokePara::CoreParam>();
                    core->SetWaza(0, move1->selected);
                    core->SetWaza(1, move2->selected);
                    core->SetWaza(2, move3->selected);
                    core->SetWaza(3, move4->selected);
                }

                party->ReplaceMember(0, param);
            };
        });
    });

    _.CollapsingHeader([](CollapsingHeader &_) {
        _.label = "Arena Tool";

        static bool enabled = false;
        static int id = array_index(ARENAS, "Grass");
        _.Checkbox([](Checkbox &_) {
            _.label = "Enabled";
            _.enabled = enabled;
            _.onChange = [](bool value) {
                enabled = value;
                setArenaSettings(enabled, id);
            };
        });

        _.ComboSimple([](ComboSimple &_) {
            _.label = " Arena";
            _.items = ARENAS;
            _.items_count = ARENA_COUNT;
            _.selected = id;
            _.onChange = [](int _id) {
                id = _id;
                setArenaSettings(enabled, id);
            };
        });
    });

    _.CollapsingHeader([](CollapsingHeader &_) {
        _.label = "Warp Tool";

        auto *zoneSelector = _.ComboSimple([](ComboSimple &_) {
            _.label = " Map";
            _.items = ZONES;
            _.items_count = ZONE_COUNT;
            _.selected = array_index(ZONES, "Jubilife City");
        });

        auto *warpSelector = _.InputInt([](InputInt &_) {
            _.label = "Warp ID";
            _.min = 0;
            _.max = 100;
            _.value = 0;
        });

        _.Button([zoneSelector, warpSelector](Button &_) {
            _.label = "Warp";
            _.onClick = [zoneSelector, warpSelector]() {
                PlayerWork::SetWarpSorawotobu(zoneSelector->selected, warpSelector->value);
            };
        });
    });

    _.CollapsingHeader([](CollapsingHeader &_) {
        _.label = "Misc Tools";

        _.Button([](Button &_) {
            _.label = "Billionaire!";
            _.onClick = []() {
                PlayerWork::SetMoney(999999);
            };
        });

        _.Button([](Button &_) {
            _.label = "Play last area name";
            _.onClick = []() {
                showAreaName();
            };
        });

        _.Checkbox([](Checkbox &_) {
            _.label = "Fly anywhere";
            _.enabled = false;
            _.onChange = [](bool value) {
                setFlyOverride(value);
            };
        });
    });

    _.CollapsingHeader([](CollapsingHeader& _){
        _.label = "Save Tools";

        _.Button([](Button &_) {
            _.label = "Delete custom save";
            _.onClick = []() {
                if (nn::fs::DeleteFile("SaveData:/Custom.bin").isSuccess()) {
                    Logger::log("Deleted custom save\n");
                } else {
                    Logger::log("Failed to delete custom save\n");
                }
            };
        });

        _.Button([](Button &_) {
            _.label = "Delete custom save backup";
            _.onClick = []() {
                if (nn::fs::DeleteFile("SaveData:/Custom_Backup.bin").isSuccess()) {
                    Logger::log("Deleted custom save backup\n");
                } else {
                    Logger::log("Failed to delete custom save backup\n");
                }
            };
        });

        _.Button([](Button &_) {
            _.label = "Dump custom save to SD";
            _.onClick = []() {
                FsHelper::writeFileToPath(getCustomSaveData(), sizeof(CustomSaveData), "sd:/Custom.bin");
            };
        });

        _.Button([](Button &_) {
            _.label = "Dump custom save backup to SD";
            _.onClick = []() {
                FsHelper::writeFileToPath(getCustomSaveData(), sizeof(CustomSaveData), "sd:/Custom_Backup.bin");
            };
        });
    });
});

void setup_ui() {
    ROOT->addChild(&mainWindow);
    ROOT->addChild(&debugLogWindow);
    ROOT->addChild(&inspectorWindow);
}
