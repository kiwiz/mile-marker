#include "main_window.h"

#define DRAW_ADD_SIZE 12
#define DRAW_ADD_WIDTH 2

Window* s_main_window;
MenuLayer* s_menu_layer;
TextLayer* s_placeholder_layer;

uint16_t main_window_get_num_sections(MenuLayer* menu_layer, void* callback_context) {
    return 1;
}

uint16_t main_window_get_num_rows(MenuLayer* menu_layer, uint16_t section_index, void* callback_context) {
    // return number of records + enough room for the "Delete all" and "+" buttons
    return records_count() + 2;
}

int16_t main_window_get_cell_height(MenuLayer* menu_layer, MenuIndex* cell_index, void* callback_context) {
    bool has_markers = records_count() > 0;

    // don't show the "Delete all" button if there are no markers
    if(!has_markers && cell_index->row == 0) {
        return 0;
    }
    // otherwise, show the "Delete all" and "+" buttons at half height
    if(has_markers && cell_index->row < 2) {
        return MENU_CELL_BASIC_CELL_HEIGHT / 2;
    }
    // otherwise, show the cell at normal height
    return MENU_CELL_BASIC_CELL_HEIGHT;
}

void main_window_selection_will_change(MenuLayer* menu_layer, MenuIndex* new_index, MenuIndex old_index, void* callback_context) {
    // don't allow selection of the "Delete all" button if there are no markers
    if(new_index->row == 0 && records_count() == 0) {
        new_index->row = old_index.row;
    }
}

void main_window_delete_all_button_draw(GContext* ctx, const Layer* cell_layer) {
    GRect bounds = layer_get_bounds(cell_layer);

    // draw text
    graphics_draw_text(ctx, "Delete all", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(
        0, -6, bounds.size.w, 24
    ), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

void main_window_add_button_draw(GContext* ctx, const Layer* cell_layer) {
    GRect bounds = layer_get_bounds(cell_layer);

    graphics_context_set_stroke_color(ctx, menu_cell_layer_is_highlighted(cell_layer) ? GColorWhite:GColorBlack);

    // draw vert line
    graphics_draw_rect(ctx, GRect(
        (bounds.size.w / 2) - (DRAW_ADD_WIDTH / 2),
        (bounds.size.h / 2) - (DRAW_ADD_SIZE / 2),
        DRAW_ADD_WIDTH, DRAW_ADD_SIZE
    ));

    // draw horiz line
    graphics_draw_rect(ctx, GRect(
        (bounds.size.w / 2) - (DRAW_ADD_SIZE / 2),
        (bounds.size.h / 2) - (DRAW_ADD_WIDTH / 2),
        DRAW_ADD_SIZE, DRAW_ADD_WIDTH
    ));
}

void main_window_record_draw(GContext* ctx, const Layer* cell_layer, const MenuIndex* cell_index) {
    GRect bounds = layer_get_bounds(cell_layer);

    char date_buf[16];

    // load in record
    record_t record;
    records_load(&record, records_count() - (cell_index->row - 2) - 1);

    tm* time_parts = localtime(&record.timestamp);

    // format & draw time
    strftime(date_buf, sizeof(date_buf), "%H:%M:%S", time_parts);
    graphics_draw_text(ctx, date_buf, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(
        bounds.origin.x + bounds.size.h + 3, bounds.origin.y - 4, bounds.size.w - bounds.size.h, 24
    ), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

    // format & draw date
    strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", time_parts);
    graphics_draw_text(ctx, date_buf, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(
        bounds.origin.x + bounds.size.h + 3, bounds.origin.y + 20, bounds.size.w - bounds.size.h, 18
    ), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

    // draw emoji
    size_t emoji_len = char_len(record.emoji, sizeof(((record_t*)0)->emoji));
    record.emoji[emoji_len] = '\0';
    graphics_draw_text(ctx, record.emoji, s_emoji_font, GRect(
        bounds.origin.x + 2, bounds.origin.y, bounds.size.h, bounds.size.h
    ), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

void main_window_draw_row(GContext* ctx, const Layer* cell_layer, const MenuIndex* cell_index, void* callback_context) {
    switch(cell_index->row) {
        case 0:
            main_window_delete_all_button_draw(ctx, cell_layer);
            break;
        case 1:
            main_window_add_button_draw(ctx, cell_layer);
            break;
        default:
            main_window_record_draw(ctx, cell_layer, cell_index);
            break;
    }
}

void main_window_select_click(MenuLayer* menu_layer, MenuIndex* cell_index, void* callback_context) {
    Window* window;

    // handle "Delete all" button
    if(cell_index->row == 0) {
        records_clear();
        window = prompt_window_create("Markers Deleted", RESOURCE_ID_DELETED_SEQUENCE, false, true);
        window_stack_push(window, true);
        return;
    }

    // handle "+" button when limit is reached
    if(cell_index->row == 1 && records_count() >= STORAGE_RECORDS_MAX) {
        window = prompt_window_create("Limit Reached", RESOURCE_ID_WARNING_IMAGE, false, false);
        window_stack_push(window, true);
        return;
    }

    record_t* record = record_create();
    size_t index = STORAGE_RECORDS_INDEX_NEW;

    // determine if target exists & fetch a record accordingly
    switch(cell_index->row) {
        default:
            index = records_count() - (cell_index->row - 2) - 1;
            records_load(record, index);
        case 1:
            window = record_window_create(record, index);
            window_stack_push(window, true);
            break;
    }
}

void main_window_load(Window* window) {
    Layer* window_layer = window_get_root_layer(window);
    GRect frame = layer_get_bounds(window_layer);

    // add menu layer
    s_menu_layer = menu_layer_create(frame);
    menu_layer_set_highlight_colors(s_menu_layer, ACCENT_COLOR, GColorWhite);
    menu_layer_set_click_config_onto_window(s_menu_layer, window);
    menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
        .get_num_sections = (MenuLayerGetNumberOfSectionsCallback) main_window_get_num_sections,
        .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) main_window_get_num_rows,
        .draw_row = (MenuLayerDrawRowCallback) main_window_draw_row,
        .select_click = (MenuLayerSelectCallback) main_window_select_click,
        .get_cell_height = (MenuLayerGetCellHeightCallback) main_window_get_cell_height,
        .selection_will_change = (MenuLayerSelectionWillChangeCallback) main_window_selection_will_change,
    });
    layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

    // add placeholder layer
    s_placeholder_layer = text_layer_create(GRect(0, 94, frame.size.w, 18));
    text_layer_set_font(s_placeholder_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_placeholder_layer, GTextAlignmentCenter);
    text_layer_set_text(s_placeholder_layer, "No Markers");
    layer_add_child(window_layer, text_layer_get_layer(s_placeholder_layer));
}

void main_window_unload(Window* window) {
    text_layer_destroy(s_placeholder_layer);

    menu_layer_destroy(s_menu_layer);
}

void main_window_appear(Window* window) {
    MenuIndex cell_index = menu_layer_get_selected_index(s_menu_layer);
    // never select "Delete all" by default
    if(cell_index.row == 0) {
        menu_layer_set_selected_next(s_menu_layer, false, MenuRowAlignNone, false);
    }

    layer_set_hidden(text_layer_get_layer(s_placeholder_layer), records_count() > 0);

    menu_layer_reload_data(s_menu_layer);
}

Window* main_window_create(void) {
    if(s_main_window == NULL) {
        s_main_window = window_create();
        window_set_window_handlers(s_main_window, (WindowHandlers) {
            .load=main_window_load,
            .unload=main_window_unload,
            .appear=main_window_appear,
        });
    }

    return s_main_window;
}

void main_window_destroy(void) {
    window_destroy(s_main_window);
    s_main_window = NULL;
}
