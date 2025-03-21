#include "../mizip_balance_editor.h"

void mizip_balance_editor_app_submenu_callback(void* context, uint32_t index) {
    MiZipBalanceEditorApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void mizip_balance_editor_scene_main_menu_on_enter(void* context) {
    furi_assert(context);
    MiZipBalanceEditorApp* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "MiZip Balance Editor");
    submenu_add_item(
        app->submenu,
        "TODO Direct to tag",
        SubmenuIndexDirectToTag,
        mizip_balance_editor_app_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Edit MiZip file",
        SubmenuIndexEditMiZipFile,
        mizip_balance_editor_app_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "About",
        SubmenuIndexEditMiZipFile,
        mizip_balance_editor_app_submenu_callback,
        app);

    view_dispatcher_switch_to_view(app->view_dispatcher, MiZipBalanceEditorViewIdMainMenu);
}

bool mizip_balance_editor_scene_main_menu_on_event(void* context, SceneManagerEvent event) {
    MiZipBalanceEditorApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SubmenuIndexDirectToTag:
            //TODO
            break;
        case SubmenuIndexEditMiZipFile:
            scene_manager_next_scene(app->scene_manager, MiZipBalanceEditorViewIdFileSelect);
            break;
        case SubmenuIndexAbout:
            //TODO
            break;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        view_dispatcher_stop(app->view_dispatcher);
    }

    return consumed;
}

void mizip_balance_editor_scene_main_menu_on_exit(void* context) {
    UNUSED(context);
}
