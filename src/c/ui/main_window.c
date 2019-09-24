#include "main_window.h"

#define DRAW_ADD_SIZE 12
#define DRAW_ADD_WIDTH 2

Window* s_main_window;
MenuLayer* s_menu_layer;

uint16_t main_window_get_num_sections(MenuLayer* menu_layer, void* callback_context) {
    return 1;
}

uint16_t main_window_get_num_rows(MenuLayer* menu_layer, uint16_t section_index, void* callback_context) {
    return records_count() + 1;
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

    graphics_context_set_compositing_mode(ctx, GCompOpSet);

    GBitmap *imageToUse = menu_cell_layer_is_highlighted(cell_layer) ? s_add_bitmap_white:s_add_bitmap_black;

    graphics_draw_bitmap_in_rect(ctx, imageToUse, GRect(pos.x, pos.y, bitmap_bounds.size.w, bitmap_bounds.size.h));
}

void main_window_draw_row(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* callback_context) {
    char date_buf[32];

    if(cell_index->row == 0) {
        main_window_add_button_draw(ctx, cell_layer);
    } else {
        record_t record;
        records_load(&record, cell_index->row - 1);

        tm* time_parts = localtime(&record.timestamp);
        strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", time_parts);
        menu_cell_basic_draw(ctx, cell_layer, record.emoji, date_buf, NULL);
    }
}

void main_window_select_click(MenuLayer* menu_layer, MenuIndex* cell_index, void* callback_context) {
    record_t* record = calloc(sizeof(record_t), 1);
    bool new = false;
    if(cell_index->row == 0) {
        record->timestamp = time(NULL);
        record->emoji[0] = 'c';
        new = true;
    } else {
        records_load(record, cell_index->row - 1);
    }
    Window* record_window = record_window_create(record, new);
    window_stack_push(record_window, true);
}

void main_window_load(Window* window) {
    Layer* window_layer = window_get_root_layer(window);
    GRect frame = layer_get_bounds(window_layer);

    s_menu_layer = menu_layer_create(frame);
    menu_layer_set_highlight_colors(s_menu_layer, GColorFolly, GColorWhite);
    menu_layer_set_click_config_onto_window(s_menu_layer, window);
    menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
        .get_num_sections = (MenuLayerGetNumberOfSectionsCallback) main_window_get_num_sections,
        .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) main_window_get_num_rows,
        .draw_row = (MenuLayerDrawRowCallback) main_window_draw_row,
        .select_click = (MenuLayerSelectCallback) main_window_select_click,
    });

    layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

void main_window_unload(Window* window) {
    record_window_destroy();

    menu_layer_destroy(s_menu_layer);
}

Window* main_window_create(void) {
    if(s_main_window == NULL) {
        s_main_window = window_create();
        window_set_window_handlers(s_main_window, (WindowHandlers) {
            .load = main_window_load,
            .unload = main_window_unload,
        });
    }

    return s_main_window;
}

void main_window_destroy(void) {
    window_destroy(s_main_window);
    s_main_window = NULL;
}
