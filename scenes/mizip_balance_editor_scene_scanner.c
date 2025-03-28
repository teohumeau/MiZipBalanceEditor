#include "../mizip_balance_editor.h"

void mizip_balance_editor_scene_scanner_scan_callback(NfcScannerEvent event, void* context) {
    furi_assert(context);
    MiZipBalanceEditorApp* app = context;
    if(event.type == NfcScannerEventTypeDetected) {
        view_dispatcher_send_custom_event(
            app->view_dispatcher, MiZipBalanceEditorCustomEventCardDetected);
        if(*event.data.protocols == NfcProtocolMfClassic) {
            view_dispatcher_send_custom_event(
                app->view_dispatcher, MiZipBalanceEditorCustomEventMfClassicCard);
        } else {
            view_dispatcher_send_custom_event(
                app->view_dispatcher, MiZipBalanceEditorCustomEventWrongCard);
        }
    }
}

void mizip_balance_editor_scene_scanner_on_enter(void* context) {
    furi_assert(context);
    MiZipBalanceEditorApp* app = context;
    popup_set_header(
        app->popup, "Apply MiZip\n  tag to the\n      back", 65, 30, AlignLeft, AlignTop);
    popup_set_icon(app->popup, 0, 3, &I_RFIDDolphinReceive_97x61);

    view_dispatcher_switch_to_view(app->view_dispatcher, MiZipBalanceEditorViewIdScanner);

    app->scanner = nfc_scanner_alloc(app->nfc);
    nfc_scanner_start(app->scanner, mizip_balance_editor_scene_scanner_scan_callback, app);
}

bool mizip_balance_editor_scene_scanner_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    MiZipBalanceEditorApp* app = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MiZipBalanceEditorCustomEventCardDetected) {
            popup_set_header(app->popup, "Reading...", 70, 30, AlignLeft, AlignTop);
            consumed = true;
        } else if(event.event == MiZipBalanceEditorCustomEventWrongCard) {
            nfc_scanner_stop(app->scanner);
            nfc_scanner_free(app->scanner);

            popup_set_header(app->popup, "Wrong tag", 70, 30, AlignLeft, AlignTop);
            consumed = true;
        } else if(event.event == MiZipBalanceEditorCustomEventMfClassicCard) {
            nfc_scanner_stop(app->scanner);
            nfc_scanner_free(app->scanner);

            app->poller = nfc_poller_alloc(app->nfc, NfcProtocolMfClassic);
            mf_classic_copy(app->mf_classic_data, nfc_poller_get_data(app->poller));
            app->is_valid_mizip_data = mizip_verify(context);
            if(app->is_valid_mizip_data) {
                popup_set_header(app->popup, "MiZip tag found", 70, 30, AlignLeft, AlignTop);
            } else {
                popup_set_header(app->popup, "Not a MiZip tag", 70, 30, AlignLeft, AlignTop);
            }
            consumed = true;
        }
    }
    return consumed;
}

void mizip_balance_editor_scene_scanner_on_exit(void* context) {
    UNUSED(context);
}
