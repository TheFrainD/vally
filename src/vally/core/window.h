#ifndef VALLY_CORE_WINDOW_H_
#define VALLY_CORE_WINDOW_H_

#include <vally/config.h>

b8 window_create(i32 width, i32 height, const char* title);

b8 window_poll_events();

void window_swap_buffers();

void window_terminate();

void window_set_key_callback(void *callback);

void window_set_mouse_button_callback(void *callback);

void window_set_cursor_position_callback(void *callback);

i32 window_get_width();

i32 window_get_height();

#endif // !VALLY_CORE_WINDOW_H_