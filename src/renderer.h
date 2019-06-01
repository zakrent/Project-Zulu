#ifndef Z_RENDERER_H
#define Z_RENDERER_H

void renderer_init(u32 w, u32 h);
void renderer_start_frame();
void renderer_end_frame();
void renderer_set_camera(real32 x, real32 y, real32 h); //w is set automatically based on display ratio
void renderer_camera_track(real32 x, real32 y);
void renderer_draw_rect(real32 x, real32 y, real32 w, real32 h);
void renderer_draw_tex(u8 texType, u32 texX, u32 texY, u32 texW, u32 texH, real32 x, real32 y, real32 w, real32 h);
void renderer_draw_map(Map map);
SDL_Texture* renderer_load_tex(SDL_Surface* s);

#endif
