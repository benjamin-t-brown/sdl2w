#pragma once

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;
struct Mix_Chunk;
#ifdef __EMSCRIPTEN__
typedef struct _TTF_Font TTF_Font;
typedef struct _Mix_Music Mix_Music;
#elif defined(__APPLE__)
typedef struct TTF_Font TTF_Font;
typedef struct Mix_Music Mix_Music;
#elif defined(_WIN32) || defined(__MINGW32__)
typedef struct TTF_Font TTF_Font;
typedef struct Mix_Music Mix_Music;
#else
typedef struct TTF_Font TTF_Font;
typedef struct Mix_Music Mix_Music;
#endif
typedef struct _SDL_Joystick SDL_Joystick;
union SDL_Event;
