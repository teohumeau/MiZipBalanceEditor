#include "../mizip_balance_editor.h"

static void
    mizip_balance_editor_scene_confirm_dialog_callback(DialogExResult result, void* context) {
    MiZipBalanceEditorApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

void mizip_balance_editor_get_balances(void* context) {
    MiZipBalanceEditorApp* app = context;

    if(app->is_valid_mizip_data) {
        if(app->mf_classic_data->block[10].data[0] == 0x55) {
            app->previous_credit_pointer = 0x08;
            app->credit_pointer = 0x09;
        } else {
            app->previous_credit_pointer = 0x09;
            app->credit_pointer = 0x08;
        }
        app->previous_balance =
            (app->mf_classic_data->block[app->previous_credit_pointer].data[2] << 8) |
            (app->mf_classic_data->block[app->previous_credit_pointer].data[1]);
        app->current_balance = (app->mf_classic_data->block[app->credit_pointer].data[2] << 8) |
                               (app->mf_classic_data->block[app->credit_pointer].data[1]);
        app->new_balance = app->current_balance;
    }
}

void mizip_balance_editor_show_balances(void* context) {
    MiZipBalanceEditorApp* app = context;

    char str[100];
    snprintf(
        str,
        sizeof(str),
        "Previous balance:  %d.%02d E\nCurrent balance: %d.%02d E\nPress back to write",
        app->previous_balance / 100,
        app->previous_balance % 100,
        app->new_balance / 100,
        app->new_balance % 100);
    dialog_ex_set_text(app->dialog_ex, str, 64, 29, AlignCenter, AlignCenter);
}

void mizip_balance_editor_scene_show_result_on_enter(void* context) {
    furi_assert(context);
    MiZipBalanceEditorApp* app = context;
    dialog_ex_reset(app->dialog_ex);
    if(app->is_valid_mizip_data) {
        //Get and show UID
        char uid[18];
        snprintf(
            uid,
            sizeof(uid),
            "UID: %02X %02X %02X %02X",
            app->mf_classic_data->block[0].data[0],
            app->mf_classic_data->block[0].data[1],
            app->mf_classic_data->block[0].data[2],
            app->mf_classic_data->block[0].data[3]);
        dialog_ex_set_header(app->dialog_ex, uid, 64, 0, AlignCenter, AlignTop);

        //Get balances
        if(!app->is_number_input_active) {
            mizip_balance_editor_get_balances(context);
        }

        //Show balances
        mizip_balance_editor_show_balances(context);

        //Init buttons
        dialog_ex_set_left_button_text(app->dialog_ex, "-");
        dialog_ex_set_right_button_text(app->dialog_ex, "+");
        dialog_ex_set_center_button_text(app->dialog_ex, "Custom value");
        app->is_number_input_active = false;
    } else {
        dialog_ex_set_text(
            app->dialog_ex, "No MiZip data found", 64, 29, AlignCenter, AlignCenter);
        dialog_ex_set_center_button_text(app->dialog_ex, "Retry");
    }
    dialog_ex_set_result_callback(
        app->dialog_ex, mizip_balance_editor_scene_confirm_dialog_callback);
    dialog_ex_set_context(app->dialog_ex, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, MiZipBalanceEditorViewIdShowResult);
}

bool mizip_balance_editor_scene_show_result_on_event(void* context, SceneManagerEvent event) {
    MiZipBalanceEditorApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DialogExResultCenter:
            if(app->is_valid_mizip_data) {
                //Open NumberInput for custom value
                app->is_number_input_active = true;
                scene_manager_next_scene(app->scene_manager, MiZipBalanceEditorViewIdNumberInput);
            } else {
                //Get back if data isn't valid
                scene_manager_previous_scene(app->scene_manager);
            }
            consumed = true;
            break;
        case DialogExResultLeft:
            //Remove 100 cents from balance
            app->new_balance = app->new_balance - 100;
            mizip_balance_editor_show_balances(context);
            consumed = true;
            break;
        case DialogExResultRight:
            //Add 100 cents from balance
            app->new_balance = app->new_balance + 100;
            mizip_balance_editor_show_balances(context);
            consumed = true;
            break;
        default:
            break;
        }
    }
    return consumed;
}

void mizip_balance_editor_scene_show_result_on_exit(void* context) {
    MiZipBalanceEditorApp* app = context;

    if(app->is_number_input_active) {
        return;
    } else {
        mizip_balance_editor_write_new_balance(context);
    }
}
