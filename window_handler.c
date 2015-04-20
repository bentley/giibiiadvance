/*
    GiiBiiAdvance - GBA/GB  emulator
    Copyright (C) 2011-2015 Antonio Ni�o D�az (AntonioND)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//#define OPENGL_BLIT

#include <SDL2/SDL.h>
#ifdef OPENGL_BLIT
#include <SDL2/SDL_opengl.h>
#endif
#include <stdio.h>
#include <string.h>

#include "debug_utils.h"
#include "window_handler.h"

#define MAX_WINDOWS 20

typedef struct {
    //Window data
    SDL_Window * mWindow;
    SDL_Renderer * mRenderer;
    SDL_GLContext GLContext;
    SDL_Texture * mTexture;
    int mWindowID;

    WH_CallbackFn mEventCallback;

    //Window dimensions
    int mWidth;
    int mHeight;
    int mTexWidth;
    int mTexHeight;
    int mTexScale; // if 0 texture will be scaled to window size. If not, centered and scaled to this factor

    //Window focus
    int mMouseFocus;
    int mKeyboardFocus;
    int mShown;
} WindowHandle;

//Windows
static WindowHandle gWindows[MAX_WINDOWS];

static WindowHandle * gMainWindow = NULL; // all unhandled events will be sent to this

static WindowHandle * _wh_get_from_index(int index)
{
    if((index >= MAX_WINDOWS) || (index < 0)) return NULL;
    return &(gWindows[index]);
}

static WindowHandle * _wh_get_from_windowID(int id)
{
    int i;
    for(i = 0; i < MAX_WINDOWS; i++)
        if(gWindows[i].mWindowID == id)
            return &(gWindows[i]);
    return NULL;
}

void WH_Init(void)
{
    memset((void*)gWindows,0,sizeof(gWindows));
}

//-------------------------------------------------------------------------------------

const char icon_data[] = {
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x97,0x46,0x1A,0xFF,0x9B,0x4B,0x20,0xFF,
    0x9F,0x53,0x29,0xFF,0xA6,0x5B,0x33,0xFF,0xAD,0x66,0x3F,0xFF,0xB5,0x71,0x4B,0xFF,
    0xBE,0x7C,0x58,0xFF,0xC6,0x88,0x66,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xEE,0xC1,0xA8,0xFF,0xF2,0xC9,0xB2,0xFF,
    0xF8,0xD0,0xBB,0xFF,0xFA,0xD5,0xC1,0xFF,0xFB,0xD8,0xC4,0xFF,0xFB,0xD7,0xC5,0xFF,
    0xFA,0xD7,0xC4,0xFF,0xF8,0xD4,0xC2,0xFF,0xF4,0xD1,0xBF,0xFF,0xF1,0xCD,0xBB,0xFF,
    0xED,0xC8,0xB6,0xFF,0xE8,0xC3,0xB1,0xFF,0xE3,0xBD,0xAB,0xFF,0xDD,0xB6,0xA3,0xFF,
    0xD7,0xAF,0x9C,0xFF,0xD2,0xA8,0x94,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x9B,0x4B,0x20,0xFF,0xA0,0x53,0x29,0xFF,
    0xA6,0x5C,0x32,0xFF,0xAD,0x66,0x3E,0xFF,0xB5,0x70,0x4B,0xFF,0xBD,0x7C,0x58,0xFF,
    0xC6,0x88,0x67,0xFF,0xCE,0x94,0x74,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xF2,0xC9,0xB2,0xFF,0xF7,0xD0,0xBA,0xFF,
    0xFA,0xD5,0xC0,0xFF,0xFB,0xD8,0xC4,0xFF,0xFA,0xD8,0xC5,0xFF,0xF9,0xD6,0xC4,0xFF,
    0xF7,0xD4,0xC2,0xFF,0xF5,0xD1,0xBF,0xFF,0xF1,0xCD,0xBB,0xFF,0xED,0xC8,0xB6,0xFF,
    0xE9,0xC3,0xB1,0xFF,0xE4,0xBD,0xAB,0xFF,0xDD,0xB6,0xA4,0xFF,0xD7,0xAF,0x9D,0xFF,
    0xD2,0xA8,0x94,0xFF,0xCA,0xA0,0x8C,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xA0,0x52,0x28,0xFF,0xA6,0x5C,0x33,0xFF,
    0xAD,0x65,0x3E,0xFF,0xB5,0x71,0x4B,0xFF,0xBD,0x7C,0x59,0xFF,0xC5,0x89,0x67,0xFF,
    0xCF,0x94,0x75,0xFF,0xD7,0xA0,0x82,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xF7,0xD1,0xBA,0xFF,0xFA,0xD5,0xC1,0xFF,
    0xFB,0xD8,0xC4,0xFF,0xFB,0xD8,0xC5,0xFF,0xFA,0xD7,0xC4,0xFF,0xF7,0xD5,0xC2,0xFF,
    0xF5,0xD1,0xBF,0xFF,0xF1,0xCD,0xBB,0xFF,0xED,0xC9,0xB6,0xFF,0xE9,0xC4,0xB0,0xFF,
    0xE3,0xBD,0xAB,0xFF,0xDE,0xB6,0xA4,0xFF,0xD7,0xAF,0x9C,0xFF,0xD1,0xA8,0x95,0xFF,
    0xCB,0xA0,0x8C,0xFF,0xC4,0x97,0x84,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xA6,0x5B,0x33,0xFF,0xAD,0x65,0x3F,0xFF,
    0xB5,0x70,0x4B,0xFF,0xBD,0x7C,0x59,0xFF,0xC5,0x88,0x66,0xFF,0xCE,0x94,0x75,0xFF,
    0xD7,0xA1,0x83,0xFF,0xDF,0xAD,0x90,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xFA,0xD5,0xC0,0xFF,0xFB,0xD7,0xC4,0xFF,
    0xFB,0xD8,0xC4,0xFF,0xF9,0xD6,0xC4,0xFF,0xF7,0xD5,0xC2,0xFF,0xF5,0xD2,0xBF,0xFF,
    0xF1,0xCD,0xBB,0xFF,0xED,0xC9,0xB6,0xFF,0xE8,0xC3,0xB0,0xFF,0xE4,0xBD,0xAA,0xFF,
    0xDE,0xB7,0xA4,0xFF,0xD7,0xAF,0x9D,0xFF,0xD1,0xA7,0x94,0xFF,0xCB,0xA0,0x8C,0xFF,
    0xC4,0x97,0x84,0xFF,0xBD,0x90,0x7B,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xAD,0x65,0x3E,0xFF,0xB5,0x71,0x4B,0xFF,
    0xBD,0x7C,0x58,0xFF,0xC6,0x88,0x67,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xFB,0xD8,0xC4,0xFF,0xFB,0xD8,0xC5,0xFF,
    0xF9,0xD6,0xC4,0xFF,0xF7,0xD4,0xC2,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xCA,0x9F,0x8D,0xFF,0xC4,0x98,0x83,0xFF,
    0xBD,0x8F,0x7B,0xFF,0xB6,0x87,0x73,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xB5,0x71,0x4B,0xFF,0xBD,0x7C,0x58,0xFF,
    0xC6,0x89,0x66,0xFF,0xCF,0x94,0x75,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xFB,0xD8,0xC4,0xFF,0xF9,0xD6,0xC4,0xFF,
    0xF7,0xD5,0xC2,0xFF,0xF5,0xD2,0xBF,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xC4,0x97,0x84,0xFF,0xBD,0x90,0x7C,0xFF,
    0xB6,0x87,0x73,0xFF,0xB0,0x7E,0x69,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xBE,0x7C,0x58,0xFF,0xC6,0x88,0x67,0xFF,
    0xCE,0x94,0x74,0xFF,0xD7,0xA1,0x83,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xF9,0xD7,0xC3,0xFF,0xF7,0xD5,0xC2,0xFF,
    0xF4,0xD1,0xBF,0xFF,0xF1,0xCE,0xBB,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xBD,0x8F,0x7B,0xFF,0xB6,0x87,0x72,0xFF,
    0xAF,0x7E,0x6A,0xFF,0xA8,0x76,0x61,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xC6,0x89,0x66,0xFF,0xCF,0x94,0x75,0xFF,
    0xD6,0xA1,0x83,0xFF,0xDF,0xAC,0x90,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xF8,0xD4,0xC2,0xFF,0xF4,0xD1,0xBF,0xFF,
    0xF1,0xCE,0xBB,0xFF,0xED,0xC9,0xB6,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xB6,0x87,0x72,0xFF,0xAF,0x7E,0x6A,0xFF,
    0xA8,0x76,0x61,0xFF,0xA2,0x6E,0x58,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xCE,0x95,0x75,0xFF,0xD7,0xA0,0x83,0xFF,
    0xDF,0xAD,0x90,0xFF,0xE6,0xB7,0x9D,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xF5,0xD1,0xBF,0xFF,0xF1,0xCD,0xBB,0xFF,
    0xED,0xC9,0xB6,0xFF,0xE8,0xC3,0xB1,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xAF,0x7E,0x69,0xFF,0xA8,0x75,0x60,0xFF,
    0xA1,0x6E,0x58,0xFF,0x9B,0x65,0x50,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xD7,0xA1,0x83,0xFF,0xDF,0xAC,0x90,0xFF,
    0xE7,0xB7,0x9D,0xFF,0xED,0xC1,0xA9,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xF1,0xCD,0xBB,0xFF,0xED,0xC9,0xB6,0xFF,
    0xE8,0xC3,0xB1,0xFF,0xE3,0xBD,0xAB,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xA8,0x76,0x61,0xFF,0xA1,0x6D,0x58,0xFF,
    0x9A,0x65,0x50,0xFF,0x94,0x5D,0x48,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xDF,0xAD,0x90,0xFF,0xE7,0xB7,0x9D,0xFF,
    0xEE,0xC1,0xA9,0xFF,0xF2,0xCA,0xB2,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xED,0xC8,0xB6,0xFF,0xE8,0xC3,0xB1,0xFF,
    0xE3,0xBD,0xAA,0xFF,0xDE,0xB7,0xA3,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xA1,0x6E,0x58,0xFF,0x9A,0x66,0x4F,0xFF,
    0x95,0x5E,0x48,0xFF,0x8E,0x56,0x40,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xE6,0xB7,0x9D,0xFF,0xED,0xC1,0xA8,0xFF,
    0xF3,0xC9,0xB2,0xFF,0xF7,0xD0,0xBB,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xE8,0xC4,0xB1,0xFF,0xE3,0xBD,0xAA,0xFF,
    0xDE,0xB7,0xA3,0xFF,0xD7,0xAF,0x9C,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x9B,0x66,0x50,0xFF,0x94,0x5E,0x47,0xFF,
    0x8E,0x56,0x3F,0xFF,0x88,0x4F,0x38,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xEE,0xC1,0xA8,0xFF,0xF2,0xC9,0xB2,0xFF,
    0xF7,0xD0,0xBA,0xFF,0xFA,0xD5,0xC1,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xE3,0xBD,0xAB,0xFF,0xDE,0xB6,0xA4,0xFF,
    0xD8,0xAF,0x9D,0xFF,0xD1,0xA8,0x95,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x94,0x5D,0x48,0xFF,0x8D,0x56,0x40,0xFF,
    0x88,0x4F,0x38,0xFF,0x82,0x48,0x31,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xF3,0xC9,0xB2,0xFF,0xF8,0xD0,0xBB,0xFF,
    0xFA,0xD5,0xC1,0xFF,0xFB,0xD8,0xC4,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xDD,0xB6,0xA4,0xFF,0xD7,0xAF,0x9C,0xFF,
    0xD1,0xA7,0x94,0xFF,0xCB,0xA0,0x8D,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x8E,0x56,0x40,0xFF,0x88,0x4F,0x38,0xFF,
    0x82,0x48,0x31,0xFF,0x7D,0x42,0x2A,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xF7,0xD0,0xBA,0xFF,0xFB,0xD5,0xC0,0xFF,
    0xFB,0xD8,0xC4,0xFF,0xFB,0xD8,0xC5,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xD8,0xAF,0x9C,0xFF,0xD1,0xA7,0x94,0xFF,
    0xCB,0xA0,0x8D,0xFF,0xC4,0x98,0x84,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x88,0x4F,0x39,0xFF,0x82,0x48,0x31,0xFF,
    0x7D,0x42,0x2B,0xFF,0x79,0x3D,0x25,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xFA,0xD5,0xC0,0xFF,0xFB,0xD8,0xC4,0xFF,
    0xFB,0xD8,0xC4,0xFF,0xFA,0xD7,0xC4,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xD1,0xA7,0x94,0xFF,0xCB,0xA0,0x8C,0xFF,
    0xC4,0x97,0x84,0xFF,0xBD,0x8F,0x7B,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x82,0x48,0x31,0xFF,0x7D,0x42,0x2B,0xFF,
    0x79,0x3D,0x25,0xFF,0x75,0x38,0x20,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xFB,0xD7,0xC4,0xFF,0xFB,0xD8,0xC5,0xFF,
    0xFA,0xD7,0xC4,0xFF,0xF7,0xD4,0xC2,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xCA,0xA0,0x8C,0xFF,0xC4,0x98,0x84,0xFF,
    0xBD,0x8F,0x7B,0xFF,0xB6,0x87,0x73,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x7D,0x42,0x2B,0xFF,0x79,0x3D,0x26,0xFF,
    0x75,0x38,0x20,0xFF,0x72,0x34,0x1C,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xFB,0xD8,0xC5,0xFF,0xF9,0xD7,0xC4,0xFF,
    0xF8,0xD4,0xC2,0xFF,0xF5,0xD1,0xBF,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xC4,0x97,0x84,0xFF,0xBD,0x8F,0x7B,0xFF,
    0xB6,0x87,0x72,0xFF,0xAF,0x7E,0x69,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x79,0x3D,0x25,0xFF,0x75,0x38,0x20,0xFF,
    0x72,0x34,0x1C,0xFF,0x6F,0x31,0x19,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xF9,0xD7,0xC4,0xFF,0xF7,0xD5,0xC2,0xFF,
    0xF5,0xD1,0xBF,0xFF,0xF1,0xCD,0xBB,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x75,0x38,0x21,0xFF,0x72,0x35,0x1C,0xFF,
    0x6F,0x31,0x19,0xFF,0x6D,0x2F,0x17,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xF7,0xD5,0xC2,0xFF,0xF4,0xD1,0xBF,0xFF,
    0xF1,0xCE,0xBB,0xFF,0xED,0xC8,0xB7,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x72,0x34,0x1D,0xFF,0x6F,0x31,0x1A,0xFF,
    0x6D,0x2F,0x17,0xFF,0x6D,0x2E,0x16,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xF5,0xD1,0xBF,0xFF,0xF1,0xCD,0xBB,0xFF,
    0xED,0xC9,0xB6,0xFF,0xE9,0xC3,0xB1,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x6F,0x31,0x19,0xFF,0x6D,0x2F,0x17,0xFF,
    0x6C,0x2E,0x16,0xFF,0x6C,0x2F,0x18,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xF1,0xCE,0xBB,0xFF,0xED,0xC8,0xB7,0xFF,
    0xE8,0xC4,0xB1,0xFF,0xE3,0xBD,0xAB,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x6E,0x2F,0x17,0xFF,0x6C,0x2E,0x16,0xFF,
    0x6C,0x2F,0x17,0xFF,0x71,0x36,0x1E,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xED,0xC9,0xB7,0xFF,0xE8,0xC3,0xB1,0xFF,
    0xE3,0xBE,0xAB,0xFF,0xDD,0xB6,0xA4,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x6C,0x2E,0x16,0xFF,0x6C,0x2F,0x17,0xFF,
    0x71,0x35,0x1E,0xFF,0x79,0x40,0x2A,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xE8,0xC3,0xB1,0xFF,0xE3,0xBD,0xAB,0xFF,
    0xDE,0xB7,0xA4,0xFF,0xD7,0xAF,0x9D,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x6C,0x2F,0x17,0xFF,0x71,0x35,0x1E,0xFF,
    0x78,0x40,0x2A,0xFF,0x84,0x50,0x3A,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xE3,0xBD,0xAB,0xFF,0xDD,0xB6,0xA4,0xFF,
    0xD8,0xAF,0x9D,0xFF,0xD1,0xA8,0x94,0xFF,0xCB,0x9F,0x8C,0xFF,0xC4,0x97,0x84,0xFF,
    0xBD,0x90,0x7B,0xFF,0xB6,0x87,0x73,0xFF,0xAF,0x7E,0x69,0xFF,0xA8,0x75,0x60,0xFF,
    0xA1,0x6E,0x58,0xFF,0x9A,0x65,0x50,0xFF,0x95,0x5D,0x47,0xFF,0x8E,0x56,0x40,0xFF,
    0x88,0x4F,0x38,0xFF,0x83,0x49,0x31,0xFF,0x7E,0x43,0x2B,0xFF,0x79,0x3D,0x25,0xFF,
    0x75,0x39,0x20,0xFF,0x71,0x34,0x1C,0xFF,0x6F,0x32,0x19,0xFF,0x6D,0x2F,0x17,0xFF,
    0x6C,0x2E,0x16,0xFF,0x6D,0x2F,0x17,0xFF,0x71,0x35,0x1E,0xFF,0x79,0x41,0x29,0xFF,
    0x85,0x50,0x3A,0xFF,0x92,0x62,0x4D,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xDD,0xB7,0xA4,0xFF,0xD7,0xB0,0x9D,0xFF,
    0xD1,0xA7,0x95,0xFF,0xCA,0xA0,0x8D,0xFF,0xC4,0x98,0x84,0xFF,0xBD,0x90,0x7B,0xFF,
    0xB6,0x87,0x73,0xFF,0xAF,0x7F,0x6A,0xFF,0xA8,0x75,0x61,0xFF,0xA2,0x6E,0x58,0xFF,
    0x9B,0x66,0x4F,0xFF,0x94,0x5E,0x47,0xFF,0x8D,0x56,0x3F,0xFF,0x88,0x4F,0x38,0xFF,
    0x83,0x48,0x31,0xFF,0x7D,0x42,0x2B,0xFF,0x79,0x3D,0x26,0xFF,0x75,0x38,0x21,0xFF,
    0x72,0x34,0x1D,0xFF,0x6F,0x31,0x19,0xFF,0x6D,0x2F,0x17,0xFF,0x6C,0x2E,0x16,0xFF,
    0x6D,0x2F,0x17,0xFF,0x71,0x35,0x1E,0xFF,0x79,0x41,0x2A,0xFF,0x84,0x50,0x3A,0xFF,
    0x92,0x62,0x4D,0xFF,0xA1,0x76,0x62,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xD8,0xAF,0x9C,0xFF,0xD1,0xA7,0x95,0xFF,
    0xCB,0xA0,0x8C,0xFF,0xC4,0x97,0x84,0xFF,0xBD,0x8F,0x7B,0xFF,0xB6,0x87,0x73,0xFF,
    0xAF,0x7E,0x6A,0xFF,0xA8,0x76,0x60,0xFF,0xA2,0x6D,0x58,0xFF,0x9B,0x65,0x50,0xFF,
    0x94,0x5D,0x47,0xFF,0x8E,0x56,0x40,0xFF,0x88,0x4F,0x38,0xFF,0x82,0x48,0x31,0xFF,
    0x7E,0x42,0x2B,0xFF,0x79,0x3D,0x25,0xFF,0x75,0x38,0x20,0xFF,0x71,0x34,0x1C,0xFF,
    0x70,0x31,0x19,0xFF,0x6D,0x2F,0x17,0xFF,0x6C,0x2E,0x16,0xFF,0x6D,0x2F,0x17,0xFF,
    0x71,0x35,0x1E,0xFF,0x79,0x40,0x2A,0xFF,0x84,0x4F,0x3A,0xFF,0x92,0x62,0x4D,0xFF,
    0xA2,0x76,0x62,0xFF,0xB8,0x93,0x81,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xD1,0xA8,0x95,0xFF,0xCB,0x9F,0x8D,0xFF,
    0xC4,0x98,0x84,0xFF,0xBD,0x8F,0x7B,0xFF,0xB6,0x87,0x73,0xFF,0xAF,0x7F,0x6A,0xFF,
    0xA8,0x76,0x61,0xFF,0xA1,0x6E,0x58,0xFF,0x9B,0x65,0x4F,0xFF,0x94,0x5D,0x48,0xFF,
    0x8E,0x56,0x3F,0xFF,0x88,0x4F,0x38,0xFF,0x82,0x48,0x32,0xFF,0x7D,0x42,0x2A,0xFF,
    0x79,0x3D,0x25,0xFF,0x75,0x38,0x21,0xFF,0x72,0x34,0x1C,0xFF,0x6F,0x32,0x19,0xFF,
    0x6D,0x2F,0x17,0xFF,0x6C,0x2E,0x16,0xFF,0x6D,0x2F,0x17,0xFF,0x71,0x35,0x1E,0xFF,
    0x79,0x40,0x2A,0xFF,0x85,0x4F,0x3A,0xFF,0x92,0x61,0x4D,0xFF,0xA2,0x76,0x62,0xFF,
    0xB8,0x93,0x81,0xFF,0xCF,0xB1,0xA1,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF
};

static void _wh_set_window_icon(SDL_Window * window)
{
    SDL_Surface * surface = SDL_CreateRGBSurfaceFrom((void*)icon_data, 32,32,
                                       32,32*4,0x000000FF,0x0000FF00,0x00FF0000,0xFF000000);

    SDL_SetWindowIcon(window, surface);

    SDL_FreeSurface(surface);
}

//-------------------------------------------------------------------------------------

int WH_Create(int width, int height, int texw, int texh, int scale) // returns -1 if error
{
    WindowHandle * w;
    int WindowHandleIndex = -1;
    int i;
    for(i = 0; i < MAX_WINDOWS; i++)
    {
        if(gWindows[i].mWindow == NULL)
        {
            w = &(gWindows[i]);
            WindowHandleIndex = i;
            break;
        }
    }
    if(WindowHandleIndex == -1) return -1;

    if(texw == 0) texw = width;
    if(texh == 0) texh = height;

    //Initialize non-existant window
    w->mWindow = NULL;
    w->mRenderer = NULL;
    w->mEventCallback = NULL;
    w->mMouseFocus = 0;
    w->mKeyboardFocus = 0;
    w->mShown = 0;
    w->mWindowID = -1;
    w->mTexScale = scale;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

    //Create window
    w->mWindow = SDL_CreateWindow( "Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if(w->mWindow != NULL)
    {
        _wh_set_window_icon(w->mWindow);

        w->mMouseFocus = 1;
        w->mKeyboardFocus = 1;
        w->mWidth = width;
        w->mHeight = height;
        w->mTexWidth = texw;
        w->mTexHeight = texh;
        w->GLContext = SDL_GL_CreateContext(w->mWindow);

        int oglIdx = -1;
        int nRD = SDL_GetNumRenderDrivers();
        for(i=0; i<nRD; i++)
        {
            SDL_RendererInfo info;
            if(!SDL_GetRenderDriverInfo(i, &info)) if(!strcmp(info.name, "opengl"))
            {
                oglIdx = i;
                break;
            }
        }

        //Create renderer for window
        w->mRenderer = SDL_CreateRenderer( w->mWindow, oglIdx,
                                          SDL_RENDERER_ACCELERATED  /*| SDL_RENDERER_PRESENTVSYNC*/);
        //SDL_SetWindowFullscreen(w->mWindow,SDL_WINDOW_FULLSCREEN);
        if(w->mRenderer == NULL)
        {
            Debug_LogMsgArg("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
            SDL_DestroyWindow(w->mWindow);
            w->mWindow = NULL;
        }
        else
        {
            //SDL_RenderSetLogicalSize(w->mRenderer, w->mWidth, w->mHeight);

            w->mWindowID = SDL_GetWindowID( w->mWindow ); //Grab window identifier
            w->mShown = 1; //Flag as opened

            w->mTexture = SDL_CreateTexture(w->mRenderer,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING,texw,texh);
            if(w->mTexture == NULL)
            {
                Debug_LogMsgArg("Couldn't create texture! SDL Error: %s\n", SDL_GetError());
                SDL_DestroyWindow( w->mWindow ); // this message shows even if everything is correct... weird...
                SDL_GL_DeleteContext(w->GLContext);
                w->mWindow = NULL;
            }
            else
            {
                return WindowHandleIndex;
            }
        }
    }
    else
    {
        Debug_LogMsgArg("Window could not be created! SDL Error: %s\n", SDL_GetError());
    }

    return -1;
}

void WH_SetSize(int index, int width, int height, int texw, int texh, int scale)
{
    WindowHandle * w = _wh_get_from_index(index);

    if(texw == 0) texw = width;
    if(texh == 0) texh = height;

    w->mTexScale = scale;

    if( !( (w->mWidth == width) && (w->mHeight == height) ) )
    {
        SDL_SetWindowSize(w->mWindow,width,height);
        w->mWidth = width;
        w->mHeight = height;
    }

    if( !( (w->mTexWidth == texw) && (w->mTexHeight == texh) ) )
    {
        w->mTexWidth = texw;
        w->mTexHeight = texh;
        SDL_DestroyTexture(w->mTexture);
        w->mTexture = SDL_CreateTexture(w->mRenderer,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING,texw,texh);
        if( w->mTexture == NULL )
        {
            Debug_LogMsgArg( "Couldn't create texture! SDL Error: %s\n", SDL_GetError() );
        }
    }
}

static void _wh_free_from_handle(WindowHandle * w)
{
    if(w == gMainWindow) gMainWindow = NULL;

    if( w->mWindow != NULL )
    {
        SDL_DestroyTexture(w->mTexture);
        SDL_GL_DeleteContext(w->GLContext);
        SDL_DestroyWindow(w->mWindow);
    }

    w->mWindow = NULL;
    w->mWindowID = -1;

    w->mMouseFocus = 0;
    w->mKeyboardFocus = 0;
    w->mShown = 0;
    w->mWidth = 0;
    w->mHeight = 0;
}

int WH_SetEventCallback(int index, WH_CallbackFn fn)
{
    WindowHandle *  w = _wh_get_from_index(index);
    if(w == NULL) return 0;
    w->mEventCallback = fn;
    return 1;
}

int WH_SetEventMainWindow(int index)
{
    WindowHandle *  w = _wh_get_from_index(index);
    if(w == NULL) return 0;
    gMainWindow = w;
    return 1;
}

static int _wh_handle_event(SDL_Event * e) // returns 1 if handled, 0 if not (send it to main window)
{
    if( e->type == SDL_QUIT )
    {
        int i;
        for(i = 0; i < MAX_WINDOWS; i++)
            WH_Close(i);
        return 1;
    }

    //All those events are specific for a window. If not handled, ignore them. Always return 1 from this function
    //Exception : The second switch in this function

    //If an event was detected for this window
    if( e->type == SDL_WINDOWEVENT )
    {
        int windowID = e->window.windowID;
        WindowHandle *  w = _wh_get_from_windowID(windowID);
        if(w == NULL) return 1;

        switch( e->window.event )
        {
            //Window appeared
            case SDL_WINDOWEVENT_SHOWN:
                w->mShown = 1;
                break;

            //Window disappeared
            case SDL_WINDOWEVENT_HIDDEN:
                w->mShown = 0;
                break;

            //Get new dimensions and repaint
            //case SDL_WINDOWEVENT_SIZE_CHANGED:
            //    w->mWidth = e->window.data1;
            //    w->mHeight = e->window.data2;
            //    SDL_RenderPresent( w->mRenderer );
            //    break;

            //Repaint on expose
            case SDL_WINDOWEVENT_EXPOSED:
                SDL_RenderPresent( w->mRenderer );
                break;

            //Mouse enter
            case SDL_WINDOWEVENT_ENTER:
                w->mMouseFocus = 1;
                break;

            //Mouse exit
            case SDL_WINDOWEVENT_LEAVE:
                w->mMouseFocus = 0;
                break;

            //Keyboard focus gained
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                w->mKeyboardFocus = 1;
                break;

            //Keyboard focus lost
            case SDL_WINDOWEVENT_FOCUS_LOST:
                w->mKeyboardFocus = 0;
                break;

            //Hide on close
            case SDL_WINDOWEVENT_CLOSE:
                if(w->mWindow) if(w->mEventCallback) w->mEventCallback(e);
                _wh_free_from_handle(w);
                //SDL_HideWindow( w->mWindow );
                return 1;

            default:
                break;
        }
    }

    int windowID = 0;
    switch(e->type)
    {
        case SDL_WINDOWEVENT:
            windowID = e->window.windowID;
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            windowID = e->key.windowID;
            break;
        case SDL_TEXTEDITING:
            windowID = e->edit.windowID;
            break;
        case SDL_TEXTINPUT:
            windowID = e->text.windowID;
            break;
        case SDL_MOUSEMOTION:
            windowID = e->motion.windowID;
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            windowID = e->button.windowID;
            break;
        case SDL_MOUSEWHEEL:
            windowID = e->wheel.windowID;
            break;
        default:
            windowID = -1;
            return 0; // This event is not window-specific. Send it to the main window handler.
    }

    WindowHandle * w = _wh_get_from_windowID(windowID);
    if(w == NULL) return 1;

    if(w->mWindow) if(w->mEventCallback) w->mEventCallback(e);

    return 1;
}

void WH_HandleEvents(void)
{
    SDL_Event e;

    while(SDL_PollEvent(&e))
    {
        //Handle window events
        if(_wh_handle_event(&e) == 0)
        {
            if(gMainWindow)
                if(gMainWindow->mEventCallback)
                    gMainWindow->mEventCallback(&e);
        }
    }
}

void WH_SetCaption(int index, const char * caption)
{
    WindowHandle *  w = _wh_get_from_index(index);
    if(w == NULL) return;
    SDL_SetWindowTitle( w->mWindow, caption );
}

void WH_Render(int index, const char * buffer)
{
    WindowHandle *  w = _wh_get_from_index(index);
    if(w == NULL) return;
    if(w->mWindow == NULL) return;

    SDL_UpdateTexture(w->mTexture, NULL, (void*)buffer, w->mTexWidth * 3);

#ifdef OPENGL_BLIT
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glClearColor(0,0,0, 0);
    glViewport(0,0, w->mWidth,w->mHeight);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    glOrtho(0.0, w->mWidth, w->mHeight, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);

    glClear(GL_COLOR_BUFFER_BIT); //Clear screen

    SDL_GL_BindTexture(w->mTexture, NULL, NULL); // returns 0 if OK

    if(w->mTexScale)
    {
        glColor3f(1.0,1.0,1.0);
        glBegin( GL_QUADS );
            glTexCoord2i(0,0); // Top-left vertex
            glVertex3f(0,0,0);

            glTexCoord2i(w->mTexWidth,0); // Bottom-left vertex
            glVertex3f(w->mWidth,0,0);

            glTexCoord2i(w->mTexWidth,w->mTexHeight); // Bottom-right vertex
            glVertex3f(w->mWidth,w->mHeight,0);

            glTexCoord2i(0,w->mTexHeight); // Top-right vertex
            glVertex3f(0,w->mHeight,0);
        glEnd();
    }
    else
    {
        //TODO: This
    }

#else

    SDL_RenderClear(w->mRenderer);

    if(w->mTexScale == 0)
    {
        SDL_RenderCopy(w->mRenderer, w->mTexture, NULL, NULL);
    }
    else
    {
        int x_size = w->mTexWidth * w->mTexScale;
        int y_size = w->mTexHeight * w->mTexScale;
        int x_offset = (w->mWidth - x_size) / 2;
        int y_offset = (w->mHeight - y_size) / 2;

        if( (x_offset < 0) || (y_offset < 0) )
        {
            Debug_LogMsgArg("WH_Render(): Invalid scaling. Scaling has been set to default.");
            w->mTexScale = 0;
            return;
        }

        SDL_Rect src;
        src.x = 0;
        src.y = 0;
        src.w = w->mTexWidth;
        src.h = w->mTexHeight;

        SDL_Rect dst;
        dst.x = x_offset;
        dst.y = y_offset;
        dst.w = x_size;
        dst.h = y_size;

        SDL_RenderCopy(w->mRenderer, w->mTexture, &src, &dst);
    }

#endif // OPENGL_BLIT

    SDL_RenderPresent(w->mRenderer);
}

int WH_AreAllWindowsClosed(void)
{
    int i;
    for(i = 0; i < MAX_WINDOWS; i++)
        if(gWindows[i].mWindow)
            return 0;

    return 1;
}

void WH_Close(int index)
{
    WindowHandle * w = _wh_get_from_index(index);
    if(w == NULL) return;
    SDL_Event e;
    e.type = SDL_WINDOWEVENT;
    e.window.event = SDL_WINDOWEVENT_CLOSE;
    e.window.windowID = w->mWindowID;
    SDL_PushEvent(&e);
}

void WH_CloseAll(void)
{
    int i;
    for(i = 0; i < MAX_WINDOWS; i++)
        WH_Close(i);
}

void WH_CloseAllBut(int index)
{
    int i;
    for(i = 0; i < MAX_WINDOWS; i++)
        if(i != index)
            WH_Close(i);
}

void WH_CloseAllButMain(void)
{
    int i;
    for(i = 0; i < MAX_WINDOWS; i++)
        if( (&(gWindows[i])) != gMainWindow)
            WH_Close(i);
}

void WH_Focus(int index)
{
    WindowHandle * w = _wh_get_from_index(index);
    if(w == NULL) return;

    //Restore window if needed
    if( !w->mShown )
    {
        SDL_ShowWindow( w->mWindow );
    }

    //Move window forward
    SDL_RaiseWindow( w->mWindow );
}

int WH_GetWidth(int index)
{
    WindowHandle * w = _wh_get_from_index(index);
    if(w == NULL) return 0;
    return w->mWidth;
}

int WH_GetHeight(int index)
{
    WindowHandle * w = _wh_get_from_index(index);
    if(w == NULL) return 0;
    return w->mHeight;
}

int WH_HasMouseFocus(int index)
{
    WindowHandle * w = _wh_get_from_index(index);
    if(w == NULL) return 0;
    return w->mMouseFocus;
}

int WH_HasKeyboardFocus(int index)
{
    WindowHandle * w = _wh_get_from_index(index);
    if(w == NULL) return 0;
    return w->mKeyboardFocus;
}

int WH_IsShown(int index)
{
    WindowHandle * w = _wh_get_from_index(index);
    if(w == NULL) return 0;
    return w->mShown;
}

