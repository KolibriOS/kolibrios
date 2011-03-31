#include "SDL.h"

/* utility functions */
Uint32 pygame_list_to_color(TP,tp_obj clr,SDL_Surface *s) {
    int r,g,b;
    r = tp_get(tp,clr,tp_number(0)).number.val;
    g = tp_get(tp,clr,tp_number(1)).number.val;
    b = tp_get(tp,clr,tp_number(2)).number.val;
    return SDL_MapRGB(s->format,r,g,b);
}

/* surface */

#define PYGAME_TYPE_SURF 0x1001

void pygame_surf_free(TP,tp_obj d) {
    if (d.data.magic != PYGAME_TYPE_SURF) { tp_raise(,"%s","not a surface"); }
    SDL_FreeSurface((SDL_Surface*)d.data.val);
}

SDL_Surface *pygame_obj_to_surf(TP,tp_obj self) {
    tp_obj d = tp_get(tp,self,tp_string("__surf"));
    if (d.data.magic != PYGAME_TYPE_SURF) { tp_raise(0,"%s","not a surface"); }
    return (SDL_Surface*)d.data.val;
}


tp_obj pygame_surface_set_at(TP) {
    tp_obj self = TP_OBJ();
    tp_obj pos = TP_TYPE(TP_LIST);
    tp_obj clr = TP_TYPE(TP_LIST);
    SDL_Rect r;
    r.x = tp_get(tp,pos,tp_number(0)).number.val;
    r.y = tp_get(tp,pos,tp_number(1)).number.val;
    r.w = 1; r.h = 1;
    SDL_Surface *s =pygame_obj_to_surf(tp,self);
    Uint32 c = pygame_list_to_color(tp,clr,s);
    SDL_FillRect(s, &r, c);
    return tp_None;
}


tp_obj pygame_surf_to_obj(TP,SDL_Surface *s) {
    tp_obj self = tp_dict(tp);
    
    tp_obj d = tp_data(tp,PYGAME_TYPE_SURF,s);
    d.data.info->free = pygame_surf_free;

    tp_set(tp,self,tp_string("__surf"),d);
    tp_set(tp,self,tp_string("set_at"),tp_method(tp,self,pygame_surface_set_at));
    return self;
}




/* display module */

tp_obj pygame_display_set_mode(TP) {
    tp_obj sz = TP_TYPE(TP_LIST);
    int w = tp_get(tp,sz,tp_number(0)).number.val;
    int h = tp_get(tp,sz,tp_number(1)).number.val;
    SDL_Surface *s = SDL_SetVideoMode(w, h, 0, 0);
    return pygame_surf_to_obj(tp,s);
}

tp_obj pygame_display_flip(TP) {
    SDL_Flip(SDL_GetVideoSurface());
    return tp_None;
}

SDL_Rect pygame_list_to_rect(TP,tp_obj o) {
    SDL_Rect r;
    r.x = tp_get(tp,o,tp_number(0)).number.val;
    r.y = tp_get(tp,o,tp_number(1)).number.val;
    r.w = tp_get(tp,o,tp_number(2)).number.val;
    r.h = tp_get(tp,o,tp_number(3)).number.val;
    return r;
}

tp_obj pygame_display_update(TP) {
    SDL_Rect r = pygame_list_to_rect(tp,TP_TYPE(TP_LIST));
    SDL_UpdateRects(SDL_GetVideoSurface(), 1, &r);
    return tp_None;
}

/* event module */
tp_obj pygame_event_get(TP) {
    SDL_Event e;
    tp_obj r = tp_list(tp);
    while (SDL_PollEvent(&e)) {
        tp_obj d = tp_dict(tp);
        tp_set(tp,d,tp_string("type"),tp_number(e.type));
        switch (e.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                tp_set(tp,d,tp_string("key"),tp_number(e.key.keysym.sym));
                tp_set(tp,d,tp_string("mod"),tp_number(e.key.keysym.mod));
                break;
            case SDL_MOUSEMOTION:
                tp_set(tp,d,tp_string("pos"),tp_list_n(tp,2,(tp_obj[]){tp_number(e.motion.x),tp_number(e.motion.y)}));
                tp_set(tp,d,tp_string("rel"),tp_list_n(tp,2,(tp_obj[]){tp_number(e.motion.xrel),tp_number(e.motion.yrel)}));
                tp_set(tp,d,tp_string("state"),tp_number(e.motion.state));
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                tp_set(tp,d,tp_string("pos"),tp_list_n(tp,2,(tp_obj[]){tp_number(e.button.x),tp_number(e.button.y)}));
                tp_set(tp,d,tp_string("button"),tp_number(e.button.button));
                break;
        }
        tp_set(tp,r,tp_None,d);
    }
    return r;
}

/* mouse */
tp_obj pygame_mouse_get_pos(TP) {
    int x,y;
    SDL_GetMouseState(&x,&y);
    tp_obj r = tp_list_n(tp,2,(tp_obj[]){tp_number(x),tp_number(y)});
    return r;
}

/* time */
tp_obj pygame_time_get_ticks(TP) {
    return tp_number(SDL_GetTicks());
}
    

/* pygame */
#define PYGAME_LOCALS(a,b) tp_set(tp,m,tp_string(a),tp_number(b));

tp_obj _pygame_init(TP) {
    SDL_Init(SDL_INIT_VIDEO);
    return tp_None;
}


void pygame_init(TP) {
    tp_obj g,m;
    g = tp_dict(tp);
    tp_set(tp,tp->modules,tp_string("pygame"),g);
    tp_set(tp,g,tp_string("init"),tp_fnc(tp,_pygame_init));
    
    /* display */
    m = tp_dict(tp); tp_set(tp,g,tp_string("display"),m);
    tp_set(tp,m,tp_string("set_mode"),tp_fnc(tp,pygame_display_set_mode));
    tp_set(tp,m,tp_string("flip"),tp_fnc(tp,pygame_display_flip));
    tp_set(tp,m,tp_string("update"),tp_fnc(tp,pygame_display_update));
    
    /* event */
    m = tp_dict(tp); tp_set(tp,g,tp_string("event"),m);
    tp_set(tp,m,tp_string("get"),tp_fnc(tp,pygame_event_get));
    
    /* locals */
    m = tp_dict(tp); tp_set(tp,g,tp_string("locals"),m);
    PYGAME_LOCALS("QUIT",SDL_QUIT);
    PYGAME_LOCALS("KEYDOWN",SDL_KEYDOWN);
    PYGAME_LOCALS("KEYUP",SDL_KEYUP);
    PYGAME_LOCALS("MOUSEBUTTONDOWN",SDL_MOUSEBUTTONDOWN);
    PYGAME_LOCALS("MOUSEBUTTONUP",SDL_MOUSEBUTTONUP);
    PYGAME_LOCALS("MOUSEMOTION",SDL_MOUSEMOTION);
    PYGAME_LOCALS("K_UP",SDLK_UP);
    PYGAME_LOCALS("K_DOWN",SDLK_DOWN);
    PYGAME_LOCALS("K_LEFT",SDLK_LEFT);
    PYGAME_LOCALS("K_RIGHT",SDLK_RIGHT);
    PYGAME_LOCALS("K_ESCAPE",SDLK_ESCAPE);
    PYGAME_LOCALS("K_SPACE",SDLK_SPACE);
    PYGAME_LOCALS("K_RETURN",SDLK_RETURN);
    
    /* mouse */
    m = tp_dict(tp); tp_set(tp,g,tp_string("mouse"),m);
    tp_set(tp,m,tp_string("get_pos"),tp_fnc(tp,pygame_mouse_get_pos));
    
    /* time */
    m = tp_dict(tp); tp_set(tp,g,tp_string("time"),m);
    tp_set(tp,m,tp_string("get_ticks"),tp_fnc(tp,pygame_time_get_ticks));
    

}

/**/
