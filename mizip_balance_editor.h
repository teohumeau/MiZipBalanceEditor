#pragma once

#include <dialogs/dialogs.h>

#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/number_input.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>

#include <storage/storage.h>

#include <gui/modules/widget.h>
#include <gui/modules/submenu.h>

#include <nfc/nfc.h>
#include <nfc/nfc_device.h>
#include <nfc/protocols/mf_classic/mf_classic.h>

#include <mizip_balance_editor_icons.h>
#include "scenes/mizip_balance_editor_scene.h"

#include "adapted_from_ofw/mizip.h"

#define NFC_APP_FOLDER    ANY_PATH("nfc")
#define NFC_APP_EXTENSION ".nfc"

// Enumeration of the view indexes.
typedef enum {
    MiZipBalanceEditorViewIdMainMenu,
    MiZipBalanceEditorViewIdFileSelect,
    MiZipBalanceEditorViewIdNumberInput,
    MiZipBalanceEditorViewIdShowResult,
} MiZipBalanceEditorViewId;

// Enumeration of submenu items.
typedef enum {
    SubmenuIndexDirectToTag,
    SubmenuIndexEditMiZipFile,
    SubmenuIndexAbout,
} SubmenuIndex;

// Main application structure.
typedef struct {
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    Submenu* submenu;
    DialogEx* dialog_ex;
    NumberInput* number_input;

    Storage* storage;
    DialogsApp* dialogs;

    MfClassicData* mf_classic_data;

    FuriString* filePath;
    bool is_valid_mizip_data;

    uint8_t credit_pointer;
    uint16_t current_balance;
    uint16_t min_value;
    uint16_t max_value;
} MiZipBalanceEditorApp;

void mizip_balance_editor_write_new_balance(void* context, uint16_t new_balance);
