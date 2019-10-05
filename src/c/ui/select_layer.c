#include "select_layer.h"
#include "../util.h"

#define SLD_BOX_SIZE 44
#define SLD_BOX_OFFSET(data, n) (SLD_BOX_SIZE * n + data->padding * (n + 1))

void select_layer_update_proc(Layer* layer, GContext* ctx) {
    SelectLayerData* data = layer_get_data(layer);

    if(!data->disabled) {
        // draw background boxes
        graphics_context_set_fill_color(ctx, GColorDarkGray);
        for(size_t i = 0; i < data->levels; ++i) {
            graphics_fill_rect(ctx, GRect(SLD_BOX_OFFSET(data, i), 0, SLD_BOX_SIZE, SLD_BOX_SIZE), 0, GCornerNone);
        }

        // draw selected box
        graphics_context_set_fill_color(ctx, ACCENT_COLOR);
        graphics_fill_rect(ctx, GRect(data->cursor_offset, 0, SLD_BOX_SIZE, SLD_BOX_SIZE), 0, GCornerNone);
    }

    // draw emoji
    graphics_context_set_text_color(ctx, data->disabled ? GColorBlack:GColorWhite);
    for(size_t i = 0; i < data->levels; ++i) {
        graphics_draw_text(ctx, data->choice_data[i][data->curr_choice[i]], s_emoji_font,
            GRect(SLD_BOX_OFFSET(data, i), 0, SLD_BOX_SIZE, SLD_BOX_SIZE),
        GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    }
}

void select_layer_add_choices(SelectLayer* select_layer, char** choices, size_t num) {
    SelectLayerData* data = layer_get_data(select_layer);
    GRect frame = layer_get_frame(select_layer);

    // check that we haven't hit the limit
    size_t level = data->levels;
    if(level >= MAX_CHOICES) {
        level = MAX_CHOICES - 1;
    }

    data->choice_data[level] = choices;
    data->choice_nums[level] = num;
    ++data->levels;

    // update the cursor padding & offset
    data->cursor_offset = data->padding = (frame.size.w - (SLD_BOX_SIZE * data->levels)) / (data->levels + 1);
}

void select_layer_set_disabled(SelectLayer* select_layer, bool disabled) {
    SelectLayerData* data = layer_get_data(select_layer);
    data->disabled = disabled;
}

char* select_layer_get_choice(SelectLayer* select_layer) {
    return NULL;
}

Layer* select_layer_get_layer(SelectLayer* select_layer) {
    return select_layer;
}

void select_layer_change_selection(SelectLayer* select_layer, int i) {
    SelectLayerData* data = layer_get_data(select_layer);

    size_t num = data->choice_nums[data->curr_level];
    data->curr_choice[data->curr_level] = (data->curr_choice[data->curr_level] + num + i) % num;
    layer_mark_dirty(select_layer);
}

void select_layer_load_selections(SelectLayer* select_layer, char* buf) {
    SelectLayerData* data = layer_get_data(select_layer);
    size_t len = strlen(buf);
    size_t offset = 0;

    for(size_t i = 0; i < data->levels; ++i) {
        size_t choice_len = char_len(buf + offset, len - offset);
        data->curr_choice[i] = 0;

        // check if the current character matches a valid selection
        bool found = false;
        for(size_t j = 0; j < data->choice_nums[i]; ++j) {
            if(strncmp(buf + offset, data->choice_data[i][j], choice_len) == 0) {
                data->curr_choice[i] = j;
                found = true;
                break;
            }
        }

        if(found) {
            offset += choice_len;
        }
    }
}

void select_layer_save_selections(SelectLayer* select_layer, char* buf, size_t len) {
    SelectLayerData* data = layer_get_data(select_layer);
    size_t offset = 0;

    for(size_t i = 0; i < data->levels; ++i) {
        char* choice = data->choice_data[i][data->curr_choice[i]];
        size_t choice_len = strlen(choice);
        if(len <= offset + choice_len) {
            break;
        }

        // write the current character to the output buffer
        strncpy(buf + offset, choice, choice_len + 1);
        offset += choice_len;
    }
}

size_t select_layer_get_cursor_offset(SelectLayer* select_layer) {
    SelectLayerData* data = layer_get_data(select_layer);

    return data->cursor_offset;
}

void select_layer_set_cursor_offset(SelectLayer* select_layer, size_t offset) {
    SelectLayerData* data = layer_get_data(select_layer);

    data->cursor_offset = offset;
    layer_mark_dirty(select_layer);
}

PropertyAnimationImplementation select_layer_cursor_animation_impl = {
    .base = {
        .update=(AnimationUpdateImplementation)property_animation_update_int16,
    },
    .accessors = {
        .setter={.int16=(Int16Setter)select_layer_set_cursor_offset},
        .getter={.int16=(Int16Getter)select_layer_get_cursor_offset},
    },
};

void select_layer_animate_level(SelectLayer* select_layer, size_t i) {
    SelectLayerData* data = layer_get_data(select_layer);

    PropertyAnimation* property_animation = property_animation_create(&select_layer_cursor_animation_impl, select_layer, NULL, NULL);
    Animation* animation = property_animation_get_animation(property_animation);

    // configure animation between levels
    size_t from_value = select_layer_get_cursor_offset(select_layer);
    size_t to_value = SLD_BOX_OFFSET(data, i);
    property_animation_from(property_animation, &from_value, sizeof(from_value), true);
    property_animation_to(property_animation, &to_value, sizeof(to_value), true);
    animation_set_duration(animation, 150);

    animation_schedule(animation);
}

void select_layer_click_handler(ClickRecognizerRef recognizer, void* context) {
    SelectLayerData* data = layer_get_data(context);

    switch(click_recognizer_get_button_id(recognizer)) {
        case BUTTON_ID_UP:
            select_layer_change_selection(context, -1);
            break;
        case BUTTON_ID_DOWN:
            select_layer_change_selection(context, 1);
            break;
        case BUTTON_ID_SELECT:
            if(data->curr_level + 1 >= data->levels) {
                data->callbacks.select_click(recognizer, data->callback_context);
            } else {
                // advance to next level
                ++data->curr_level;
                select_layer_animate_level(context, data->curr_level);
                layer_mark_dirty(context);
            }
            break;
        case BUTTON_ID_BACK:
            if(data->curr_level == 0) {
                data->callbacks.back_click(recognizer, data->callback_context);
            } else {
                // return to previous level
                --data->curr_level;
                select_layer_animate_level(context, data->curr_level);
                layer_mark_dirty(context);
            }
            break;
        default:
            break;
    }
}

void select_layer_click_provider(void* context) {
    int key_repeat_delay = 250;

    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)select_layer_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)select_layer_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler)select_layer_click_handler);
    window_single_click_subscribe(BUTTON_ID_BACK, (ClickHandler)select_layer_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_UP, key_repeat_delay, (ClickHandler)select_layer_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, key_repeat_delay, (ClickHandler)select_layer_click_handler);
}

void select_layer_set_click_config_onto_window(SelectLayer* select_layer, Window* window) {
    SelectLayerData* data = layer_get_data(select_layer);
    if(data->disabled) {
        return;
    }

    window_set_click_config_provider_with_context(window, (ClickConfigProvider)select_layer_click_provider, select_layer);
}

void select_layer_set_callbacks(SelectLayer* select_layer, void* callback_context, SelectLayerCallbacks select_layer_callbacks) {
    SelectLayerData* data = layer_get_data(select_layer);

    data->callback_context = callback_context;
    data->callbacks = select_layer_callbacks;
}

SelectLayer* select_layer_create(GRect frame) {
    Layer* layer = layer_create_with_data(frame, sizeof(SelectLayerData));
    layer_set_update_proc(layer, (LayerUpdateProc)select_layer_update_proc);
    return layer;
}

void select_layer_destroy(SelectLayer* select_layer) {
    SelectLayerData* data = layer_get_data(select_layer);

    animation_unschedule_all();
    layer_destroy(select_layer);
}
