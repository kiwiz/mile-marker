#ifndef select_layer_h
#define select_layer_h

#include <pebble.h>
#include "../resources.h"

#define MAX_CHOICES 3

typedef struct {
    void(* back_click)(ClickRecognizerRef recognizer, void* context);
    void(* select_click)(ClickRecognizerRef recognizer, void* context);
} SelectLayerCallbacks;

typedef struct {
    SelectLayerCallbacks callbacks;
    void* callback_context;
    size_t cursor_offset;
    size_t padding;
    char** choice_data[MAX_CHOICES];
    size_t choice_nums[MAX_CHOICES];
    size_t curr_choice[MAX_CHOICES];
    size_t curr_level;
    size_t levels;
    bool disabled;
} SelectLayerData;

typedef Layer SelectLayer;

void select_layer_add_choices(SelectLayer* select_layer, char** choices, size_t num);
void select_layer_set_disabled(SelectLayer* select_layer, bool disabled);
Layer* select_layer_get_layer(SelectLayer* layer);
void select_layer_load_selections(SelectLayer* select_layer, char* buf);
void select_layer_save_selections(SelectLayer* select_layer, char* buf, size_t len);
void select_layer_set_click_config_onto_window(SelectLayer* select_layer, Window* window);
void select_layer_set_callbacks(SelectLayer* select_layer, void* callback_context, SelectLayerCallbacks select_layer_callbacks);

SelectLayer* select_layer_create(GRect frame);
void select_layer_destroy(SelectLayer* layer);

#endif
