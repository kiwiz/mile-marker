#include "prompt_window.h"

Window* s_prompt_window;
Layer* s_canvas_layer;
TextLayer* s_message_layer;
GDrawCommandSequence* s_command_seq;
GDrawCommandImage* s_command_img;
char* s_message;
AppTimer* s_timer;
int s_index;
bool s_is_sequence;
bool s_prompt;


void prompt_window_frame_handler(void* context) {
    // draw the next frame
    layer_mark_dirty(s_canvas_layer);

    // continue the sequence
    s_timer = app_timer_register(PDC_WINDOW_DELTA, prompt_window_frame_handler, NULL);
}

void prompt_window_update_image_proc(Layer* layer, GContext* ctx) {
    GRect bounds = layer_get_bounds(layer);
    GSize img_bounds = gdraw_command_image_get_bounds_size(s_command_img);

    // draw the next frame
    gdraw_command_image_draw(ctx, s_command_img, GPoint(
        (bounds.size.w - img_bounds.w) / 2,
        (bounds.size.h - img_bounds.h) / 2
    ));
}

void prompt_window_update_sequence_proc(Layer* layer, GContext* ctx) {
    GRect bounds = layer_get_bounds(layer);
    GSize seq_bounds = gdraw_command_sequence_get_bounds_size(s_command_seq);

    // pop window stack after animation has played + a brief pause
    int num_frames = gdraw_command_sequence_get_num_frames(s_command_seq);
    if(s_index >= num_frames + (200 / PDC_WINDOW_DELTA)) {
        window_stack_remove(s_prompt_window, true);
        return;
    }

    // don't advance past the last frame
    int next_index = s_index;
    if (next_index >= num_frames) {
      next_index = num_frames - 1;
    }
    GDrawCommandFrame* frame = gdraw_command_sequence_get_frame_by_index(s_command_seq, next_index);

    // draw the next frame
    if(frame) {
        gdraw_command_frame_draw(ctx, s_command_seq, frame, GPoint(
            (bounds.size.w - seq_bounds.w) / 2,
            (bounds.size.h - seq_bounds.h) / 2
        ));
    }

    ++s_index;
}

void prompt_window_load(Window* window) {
    Layer* window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // add message layer
    s_message_layer = text_layer_create(GRect(bounds.origin.x, bounds.origin.y + 125, bounds.size.w, 24));
    text_layer_set_font(s_message_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_message_layer, GTextAlignmentCenter);
    text_layer_set_text(s_message_layer, s_message);
    text_layer_set_background_color(s_message_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_message_layer));

    // add canvas layer
    s_canvas_layer = layer_create(bounds);
    layer_set_update_proc(s_canvas_layer, s_is_sequence ? prompt_window_update_sequence_proc:prompt_window_update_image_proc);
    layer_add_child(window_layer, s_canvas_layer);

    // register timer
    if(s_is_sequence) {
        s_timer = app_timer_register(PDC_WINDOW_DELTA, prompt_window_frame_handler, NULL);
    }
}

void prompt_window_unload(Window* window) {
    if(s_is_sequence) {
        app_timer_cancel(s_timer);
    }

    text_layer_destroy(s_message_layer);

    layer_destroy(s_canvas_layer);
}


Window* prompt_window_create(char* message, uint32_t resource, bool prompt, bool is_sequence) {
    if(s_prompt_window != NULL) {
        prompt_window_destroy();
    }

    s_prompt_window = window_create();
    window_set_window_handlers(s_prompt_window, (WindowHandlers) {
        .load = prompt_window_load,
        .unload = prompt_window_unload,
    });
    window_set_background_color(s_prompt_window, ACCENT_COLOR);

    if(is_sequence) {
        s_command_seq = gdraw_command_sequence_create_with_resource(resource);
    } else {
        s_command_img = gdraw_command_image_create_with_resource(resource);
    }
    s_message = message;
    s_prompt = prompt;
    s_is_sequence = is_sequence;
    s_index = 0;

    return s_prompt_window;
}

void prompt_window_destroy(void) {
    gdraw_command_sequence_destroy(s_command_seq);
    s_command_seq = NULL;

    gdraw_command_image_destroy(s_command_img);
    s_command_img = NULL;

    window_destroy(s_prompt_window);
    s_prompt_window = NULL;
}
