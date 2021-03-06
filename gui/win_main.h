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

#ifndef __WIN_MAIN__
#define __WIN_MAIN__

int Win_MainCreate(char * rom_path); // returns 1 if error
void Win_MainRender(void);
int Win_MainRunningGBA(void);
int Win_MainRunningGB(void);
void Win_MainLoopHandle(void);

void Win_MainChangeZoom(int newzoom);
void Win_MainSetFrameskip(int frameskip); // in win_main.c

void Win_MainShowMessage(int type, const char * text); // 0 = error, 1 = debug, 2 = console, 3 = sys info

#endif // __WIN_MAIN__
