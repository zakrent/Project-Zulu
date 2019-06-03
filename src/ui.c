#include "common.h"
#include "renderer.h"
#include "ui.h"

enum WidgetType{
	WIDGET_INVALID,
	WIDGET_BUTTON,
	MAX_WIDGET_TYPE
};

typedef struct Widget{
	u8 type;
	bool refreshed;
	u32 x;
	u32 y;
	u32 w;
	u32 h;	
} Widget;

Widget widgets[UI_MAX_ID+1];
Widget *hot;
Widget *active;

void ui_init(){
}

void ui_free(){
	for(int i = 0; i <= UI_MAX_ID; i++){
		widgets[i] = (Widget){0};
	}
}

void ui_update(){
	for(int i = 0; i <= UI_MAX_ID; i++){
		Widget *w = widgets+i;
		if(w->type != WIDGET_INVALID && !w->refreshed)
		   w->type = WIDGET_INVALID;	
		w->refreshed = false;
	}
}

void ui_render(){
	for(int i = 0; i <= UI_MAX_ID; i++){
		Widget *w = widgets+i;
		if(w->type != WIDGET_INVALID){
			renderer_draw_rectP(w->x, w->y, w->w, w->h,255,255*(hot == w),0);
		}
	}
}

bool ui_button(u8 id, u32 x, u32 y, u32 w, u32 h){
	Widget *wi = widgets+id;
	wi->type = WIDGET_BUTTON;
	wi->refreshed = true;
	wi->x = x;
	wi->y = y;
	wi->w = w;
	wi->h = h;
	if(active == wi){
		active = 0;
		return true;
	}
	return false;
}

bool ui_handle_event(SDL_Event *event){
	if(event->type == SDL_MOUSEMOTION){
		for(int i = 0; i <= UI_MAX_ID; i++){
			Widget *w = widgets+i;
			u32 x = event->motion.x;
			u32 y = event->motion.y;
			if(w->type != WIDGET_INVALID && x > w->x && x < w->x+w->w && y > w->y && y < w->y + w->h){
				hot = w;
				return true;
			}
		}
		hot = 0;
	}
	else if(event->type == SDL_MOUSEBUTTONUP){
		if(hot != 0){
			active = hot;
			return true;
		}
	}
	return false;
}
