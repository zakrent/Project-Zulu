#ifndef Z_UI_H
#define Z_UI_H

#define UI_MAX_ID 255

void ui_init();
void ui_free();
void ui_update();
void ui_render();
bool ui_handle_event(SDL_Event *event);
bool ui_button(u8 id, u32 x, u32 y, u32 w, u32 h);

#endif
