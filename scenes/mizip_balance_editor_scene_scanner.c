#include "../mizip_balance_editor_i.h"
#include "adapted_from_ofw/mizip.h"
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

static NfcCommand
    mizip_balance_editor_scene_scanner_poller_callback(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolMfClassic);
    MiZipBalanceEditorApp* app = context;
    NfcCommand command = NfcCommandContinue;

    const MfClassicPollerEvent* mfc_event = event.event_data;
    if(mfc_event->type == MfClassicPollerEventTypeRequestMode) {
        FURI_LOG_I("MiZipBalanceEditor", "Request mode");
        const MfClassicData* mfc_data = nfc_poller_get_data(app->poller);

        if(nfc_device_get_protocol(app->nfc_device) == NfcProtocolInvalid) {
            nfc_device_set_data(app->nfc_device, NfcProtocolMfClassic, mfc_data);
        } else {
            mfc_data = nfc_device_get_data(app->nfc_device, NfcProtocolMfClassic);
        }

        mfc_event->data->poller_mode.mode = MfClassicPollerModeDictAttackStandard;
        mfc_event->data->poller_mode.data = mfc_data;
        app->sectors_total = mf_classic_get_total_sectors_num(mfc_data->type);
        mf_classic_get_read_sectors_and_keys(mfc_data, &app->sectors_read, &app->keys_found);
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestKey) {
        FURI_LOG_I("MiZipBalanceEditor", "Request key");
        const MfClassicData* mfc_data = nfc_poller_get_data(app->poller);

        if(nfc_device_get_protocol(app->nfc_device) == NfcProtocolInvalid) {
            nfc_device_set_data(app->nfc_device, NfcProtocolMfClassic, mfc_data);
        } else {
            mfc_data = nfc_device_get_data(app->nfc_device, NfcProtocolMfClassic);
        }

        MizipCardConfig cfg = {};
        mizip_get_card_config(&cfg, mfc_data->type);

        MfClassicKey key = {};
        key.data[app->current_sector] = cfg.keys[app->current_sector].a;
        mfc_event->data->key_request_data.key = key;
        mfc_event->data->key_request_data.key_provided = true;
    } else if(mfc_event->type == MfClassicPollerEventTypeDataUpdate) {
        FURI_LOG_I("MiZipBalanceEditor", "Data update");
        MfClassicPollerEventDataUpdate* data_update = &mfc_event->data->data_update;
        app->sectors_read = data_update->sectors_read;
        app->current_sector = data_update->current_sector;
    } else if(mfc_event->type == MfClassicPollerEventTypeSuccess) {
        FURI_LOG_I("MiZipBalanceEditor", "Success");
        const MfClassicData* mf_classic_data = nfc_poller_get_data(app->poller);
        nfc_device_set_data(app->nfc_device, NfcProtocolMfClassic, mf_classic_data);
        mf_classic_copy(app->mf_classic_data, mf_classic_data);
        if(mf_classic_is_card_read(mf_classic_data)) {
            FURI_LOG_I("MiZipBalanceEditor", "Card readed");
        }
        memcpy(app->uid, app->mf_classic_data->iso14443_3a_data->uid, UID_LENGTH);
        app->is_valid_mizip_data = mizip_parse(context);

        view_dispatcher_send_custom_event(
            app->view_dispatcher, MiZipBalanceEditorCustomEventPollerSuccess);
        command = NfcCommandStop;
    }
    return command;
}

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

    app->sectors_total = 0;
    app->sectors_read = 0;
    app->current_sector = 0;

    view_dispatcher_switch_to_view(app->view_dispatcher, MiZipBalanceEditorViewIdScanner);
    app->scanner = nfc_scanner_alloc(app->nfc);
    nfc_scanner_start(app->scanner, mizip_balance_editor_scene_scanner_scan_callback, app);
    app->is_scan_active = true;
    FURI_LOG_I("MiZipBalanceEditor", "Enter scanner scene");
}

bool mizip_balance_editor_scene_scanner_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    MiZipBalanceEditorApp* app = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MiZipBalanceEditorCustomEventCardDetected) {
            popup_set_header(app->popup, "Reading...", 65, 40, AlignLeft, AlignTop);
            consumed = true;
        } else if(event.event == MiZipBalanceEditorCustomEventWrongCard) {
            nfc_scanner_stop(app->scanner);
            nfc_scanner_free(app->scanner);
            app->is_scan_active = false;
            popup_set_header(app->popup, "Wrong tag", 65, 40, AlignLeft, AlignTop);
            consumed = true;
        } else if(event.event == MiZipBalanceEditorCustomEventMfClassicCard) {
            nfc_scanner_stop(app->scanner);
            nfc_scanner_free(app->scanner);
            app->is_scan_active = false;
            app->poller = nfc_poller_alloc(app->nfc, NfcProtocolMfClassic);
            nfc_poller_start(
                app->poller, mizip_balance_editor_scene_scanner_poller_callback, context);
            consumed = true;
        } else if(event.event == MiZipBalanceEditorCustomEventPollerSuccess) {
            nfc_poller_stop(app->poller);
            nfc_poller_free(app->poller);
            scene_manager_next_scene(app->scene_manager, MiZipBalanceEditorViewIdShowBalance);
            consumed = true;
        }
    }
    return consumed;
}

void mizip_balance_editor_scene_scanner_on_exit(void* context) {
    furi_assert(context);
    MiZipBalanceEditorApp* app = context;
    if(app->is_scan_active) {
        nfc_scanner_stop(app->scanner);
        nfc_scanner_free(app->scanner);
    }
    popup_reset(app->popup);
}
