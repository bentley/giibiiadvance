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

#include <stdio.h>
#include <stdlib.h>

#include "../build_options.h"
#include "../general_utils.h"
#include "../debug_utils.h"

#include "gameboy.h"
#include "cpu.h"
#include "debug.h"
#include "memory.h"
#include "interrupts.h"
#include "general.h"
#include "sound.h"
#include "ppu.h"
#include "ppu_gbc.h"
#include "sgb.h"
#include "video.h"
#include "gb_main.h"
#include "serial.h"
#include "dma.h"

//----------------------------------------------------------------

extern _GB_CONTEXT_ GameBoy;

//----------------------------------------------------------------

u32 GB_MemRead8_GBC_BootEnabled(u32 address)
{
    _GB_MEMORY_ * mem = &GameBoy.Memory;

    switch(address >> 12)
    {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3: //16KB ROM Bank 00
            if(address < 0x100) return (u32)GameBoy.Emulator.boot_rom[address];
            if(address >= 0x200 && address < 0x900) return (u32)GameBoy.Emulator.boot_rom[address];
            return mem->ROM_Base[address];
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7: //16KB ROM Bank 01..NN
            return mem->ROM_Curr[address-0x4000];
        case 0x8:
        case 0x9: //8KB Video RAM (VRAM)
#ifdef VRAM_MEM_CHECKING
            if(GameBoy.Emulator.lcd_on && GameBoy.Emulator.ScreenMode == 3) return 0xFF;
#endif
            return mem->VideoRAM_Curr[address-0x8000];
        case 0xA:
        case 0xB: //8KB External RAM
            return GameBoy.Memory.MapperRead(address);
        case 0xC: //4KB Work RAM Bank 0
            return mem->WorkRAM[address-0xC000];
        case 0xD: // 4KB Work RAM Bank 1
            return mem->WorkRAM_Curr[address-0xD000];
        case 0xE: // Echo RAM
            return mem->WorkRAM[address-0xE000];
        case 0xF:
            if(address < 0xFE00) // Echo RAM
            {
                return mem->WorkRAM_Curr[address-0xF000];
            }
            else if(address < 0xFEA0) // Sprite Attribute Table
            {
                GB_PPUUpdateClocksCounterReference(GB_CPUClockCounterGet());
                if(GameBoy.Emulator.lcd_on && GameBoy.Emulator.ScreenMode & 0x02) return 0xFF;
                return mem->ObjAttrMem[address-0xFE00];
            }
            else if(address < 0xFF00) // Not Usable
            {
#ifdef VRAM_MEM_CHECKING
                if(GameBoy.Emulator.lcd_on && GameBoy.Emulator.ScreenMode & 0x02) return 0xFF;
#endif
                if( (GameBoy.Emulator.HardwareType == HW_GBA) || (GameBoy.Emulator.HardwareType == HW_GBA_SP) )
                {
                    int val = address & 0xF0;
                    return val | (val>>4);
                }

                // GBC
                u32 addr = address - 0xFEA0;
                if(addr >= 0x30) addr = 0x20 + (addr & 0x0F); //this is what my gbc says...
                return mem->StrangeRAM[addr];
            }
            else if(address < 0xFF80) // I/O Ports
            {
                return GB_MemReadReg8(address);
            }
            //else // High RAM (and IE)

            return mem->HighRAM[address-0xFF80];
        default:
            Debug_ErrorMsgArg("Read address %04X\nPC: %04X\nROM: %d",
                        address,GameBoy.CPU.R16.PC,GameBoy.Memory.selected_rom);
            return GB_MemReadReg8(address&0xFFFF);
    }

    return 0x00;
}

u32 GB_MemRead8_GBC_BootDisabled(u32 address)
{
    _GB_MEMORY_ * mem = &GameBoy.Memory;

    switch(address >> 12)
    {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3: //16KB ROM Bank 00
            return mem->ROM_Base[address];
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7: //16KB ROM Bank 01..NN
            return mem->ROM_Curr[address-0x4000];
        case 0x8:
        case 0x9: //8KB Video RAM (VRAM)
#ifdef VRAM_MEM_CHECKING
            if(GameBoy.Emulator.lcd_on && GameBoy.Emulator.ScreenMode == 3) return 0xFF;
#endif
            return mem->VideoRAM_Curr[address-0x8000];
        case 0xA:
        case 0xB: //8KB External RAM
            return GameBoy.Memory.MapperRead(address);
        case 0xC: //4KB Work RAM Bank 0

            GB_DMAUpdateClocksCounterReference(GB_CPUClockCounterGet());
            if(GameBoy.Emulator.OAM_DMA_enabled)
                return GameBoy.Emulator.OAM_DMA_last_read_byte;

            return mem->WorkRAM[address-0xC000];
        case 0xD: // 4KB Work RAM Bank 1
            return mem->WorkRAM_Curr[address-0xD000];
        case 0xE: // Echo RAM
            return mem->WorkRAM[address-0xE000];
        case 0xF:
            if(address < 0xFE00) // Echo RAM
            {
                return mem->WorkRAM_Curr[address-0xF000];
            }
            else if(address < 0xFEA0) // Sprite Attribute Table
            {
                GB_PPUUpdateClocksCounterReference(GB_CPUClockCounterGet());
                if(GameBoy.Emulator.lcd_on && GameBoy.Emulator.ScreenMode & 0x02) return 0xFF;
                GB_DMAUpdateClocksCounterReference(GB_CPUClockCounterGet());
                if(GameBoy.Emulator.OAM_DMA_enabled) return 0xFF;

                return mem->ObjAttrMem[address-0xFE00];
            }
            else if(address < 0xFF00) // Not Usable
            {
#ifdef VRAM_MEM_CHECKING
                if(GameBoy.Emulator.lcd_on && GameBoy.Emulator.ScreenMode & 0x02) return 0xFF;
#endif
                if( (GameBoy.Emulator.HardwareType == HW_GBA) || (GameBoy.Emulator.HardwareType == HW_GBA_SP) )
                {
                    int val = address & 0xF0;
                    return val | (val>>4);
                }

                // GBC
                u32 addr = address - 0xFEA0;
                if(addr >= 0x30) addr = 0x20 + (addr & 0x0F); //this is what my gbc says...
                return mem->StrangeRAM[addr];
            }
            else if(address < 0xFF80) // I/O Ports
            {
                return GB_MemReadReg8(address);
            }
            //else // High RAM (and IE)

            return mem->HighRAM[address-0xFF80];
        default:
            Debug_ErrorMsgArg("Read address %04X\nPC: %04X\nROM: %d",
                        address,GameBoy.CPU.R16.PC,GameBoy.Memory.selected_rom);
            return GB_MemReadReg8(address&0xFFFF);
    }

    return 0x00;
}

//----------------------------------------------------------------

void GB_MemWrite8_GBC(u32 address, u32 value)
{
    _GB_MEMORY_ * mem = &GameBoy.Memory;

    switch(address >> 12)
    {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3: //16KB ROM Bank 00
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7: //16KB ROM Bank 01..NN
            GameBoy.Memory.MapperWrite(address, value);
            return;
        case 0x8:
        case 0x9: //8KB Video RAM (VRAM)
#ifdef VRAM_MEM_CHECKING
            if(GameBoy.Emulator.lcd_on && GameBoy.Emulator.ScreenMode == 3) return;
#endif
            mem->VideoRAM_Curr[address-0x8000] = value;
            return;
        case 0xA:
        case 0xB: //8KB External RAM
            GameBoy.Memory.MapperWrite(address, value);
            return;
        case 0xC: //4KB Work RAM Bank 0
            mem->WorkRAM[address-0xC000] = value;
            return;
        case 0xD: // 4KB Work RAM Bank 1
            mem->WorkRAM_Curr[address-0xD000] = value;
            return;
        case 0xE: // Echo RAM
        {
            mem->WorkRAM[address-0xE000] = value;
            GameBoy.Memory.MapperWrite(address-0xE000+0xA000, value);
            return;
        }
        case 0xF:
            if(address < 0xFE00) // Echo RAM
            {
                mem->WorkRAM_Curr[address-0xF000] = value;
                GameBoy.Memory.MapperWrite(address-0xF000+0xB000, value);
                return;
            }
            else if(address < 0xFEA0) // Sprite Attribute Table
            {
#ifdef VRAM_MEM_CHECKING
                if(GameBoy.Emulator.lcd_on && (GameBoy.Emulator.ScreenMode & 0x02)) return;
#endif
                mem->ObjAttrMem[address-0xFE00] = value;
                return;
            }
            else if(address < 0xFF00) // Not Usable
            {
                if( (GameBoy.Emulator.HardwareType == HW_GBA) || (GameBoy.Emulator.HardwareType == HW_GBA_SP) )
                    return;
                //GBC
                u32 addr = address - 0xFEA0;
                if(addr >= 0x30) addr = 0x20 + (addr & 0x0F); //this is what my gbc says...
                mem->StrangeRAM[addr] = value;
                return;
            }
            else if(address < 0xFF80) // I/O Ports
            {
                GB_MemWriteReg8(address,value);
                return;
            }
            //else // High RAM (and IE)

            if(address == IE_REG)
            {
                GB_InterruptsWriteIE(value);
                return;
            }

            mem->HighRAM[address-0xFF80] = value;
            return;
        default:
            Debug_ErrorMsgArg("Wrote address %04x\nPC: %04x\nROM: %d",
                        address,GameBoy.CPU.R16.PC,GameBoy.Memory.selected_rom);
            GB_MemWrite8(address&0xFFFF,value);
            return;
    }

    return;
}

//----------------------------------------------------------------

u32 GB_MemReadReg8_GBC(u32 address)
{
    _GB_MEMORY_ * mem = &GameBoy.Memory;

    switch(address)
    {
        // Serial
        case SB_REG:
            GB_SerialUpdateClocksCounterReference(GB_CPUClockCounterGet());
            return mem->IO_Ports[SB_REG-0xFF00];
        case SC_REG:
            GB_SerialUpdateClocksCounterReference(GB_CPUClockCounterGet());
            return mem->IO_Ports[SC_REG-0xFF00] | ((GameBoy.Emulator.CGBEnabled == 1) ? 0x7C: 0x7E);

        // Timer
        case TMA_REG:
            return mem->IO_Ports[address-0xFF00];
        case TIMA_REG:
            GB_TimersUpdateClocksCounterReference(GB_CPUClockCounterGet());
            return mem->IO_Ports[TIMA_REG-0xFF00];
        case TAC_REG:
            return mem->IO_Ports[TAC_REG-0xFF00] | (0xF8);

        // Divider
        case DIV_REG:
            GB_TimersUpdateClocksCounterReference(GB_CPUClockCounterGet());
            return mem->IO_Ports[DIV_REG-0xFF00];

        // Interrupts
        case IF_REG:
            //GB_UpdateCounterToClocks(GB_CPUClockCounterGet());
            GB_PPUUpdateClocksCounterReference(GB_CPUClockCounterGet());
            GB_TimersUpdateClocksCounterReference(GB_CPUClockCounterGet());
            GB_SerialUpdateClocksCounterReference(GB_CPUClockCounterGet());
            return mem->IO_Ports[IF_REG-0xFF00];

        // DMA
        case DMA_REG: // This is R/W in all GB models
            return mem->IO_Ports[DMA_REG-0xFF00];

        // Video
        case LCDC_REG:
        case SCY_REG:
        case SCX_REG:
        case LYC_REG:
        case BGP_REG:
        case OBP0_REG:
        case OBP1_REG:
        case WY_REG:
        case WX_REG:
            return mem->IO_Ports[address-0xFF00];

        // GBC Registers
        // -------------

        // HDMA
        case HDMA5_REG: // Updated in execution loop. The other HDMAx registers are write only
            if(GameBoy.Emulator.CGBEnabled == 0) return 0xFF;
            return mem->IO_Ports[address-0xFF00];

        // Speed switch
        case KEY1_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return 0xFF;
            return mem->IO_Ports[KEY1_REG-0xFF00] | (0x7E);

        // Video ram bank
        case VBK_REG:
            if(GameBoy.Emulator.CGBEnabled == 0)
                return 0xFE;
            return mem->IO_Ports[VBK_REG-0xFF00];

        // Work ram bank
        case SVBK_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return 0xFF;
            return mem->IO_Ports[SVBK_REG-0xFF00];

        // TODO
        case BCPS_REG:
        case OCPS_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return 0xFF;
            return mem->IO_Ports[address-0xFF00];

        case NR12_REG:
        case NR22_REG:
        case NR42_REG:
        case NR43_REG:
        case NR50_REG:
        case NR51_REG:
            return mem->IO_Ports[address-0xFF00];

        case STAT_REG:
            GB_PPUUpdateClocksCounterReference(GB_CPUClockCounterGet());
            if(GameBoy.Emulator.lcd_on) return mem->IO_Ports[STAT_REG-0xFF00] | (0x80);
            return (mem->IO_Ports[STAT_REG-0xFF00] | 0x80) & 0xFC;

        case P1_REG:
            //GB_SGBUpdate(GB_CPUClockCounterGet()); TODO

            if(GameBoy.Emulator.SGBEnabled == 1)
                return SGB_ReadP1();

            u32 result = 0;

            u32 p1_reg = mem->IO_Ports[P1_REG-0xFF00];
            int Keys = GB_Input_Get(0);
            if((p1_reg & (1<<5)) == 0) //A-B-SEL-STA
            {
                result |= (Keys & KEY_A) ? JOY_A : 0;
                result |= (Keys & KEY_B) ? JOY_B : 0;
                result |= (Keys & KEY_SELECT) ? JOY_SELECT : 0;
                result |= (Keys & KEY_START) ? JOY_START : 0;
            }
            if((p1_reg & (1<<4)) == 0) //PAD
            {
                result |= (Keys & KEY_UP) ? JOY_UP : 0;
                result |= (Keys & KEY_DOWN) ? JOY_DOWN : 0;
                result |= (Keys & KEY_LEFT) ? JOY_LEFT : 0;
                result |= (Keys & KEY_RIGHT) ? JOY_RIGHT : 0;
            }

            result = (~result) & 0x0F;
            result |= p1_reg & 0xF0;
            result |= 0xC0;

            mem->IO_Ports[P1_REG-0xFF00] = result;
            return result;

        case NR10_REG:
            return mem->IO_Ports[NR10_REG-0xFF00] | 0x80;
        case NR11_REG:
        case NR21_REG:
            return mem->IO_Ports[address-0xFF00] | 0x3F;
        case NR13_REG:
        case NR23_REG:
        case NR33_REG:
            return 0xFF;
        case NR14_REG:
        case NR24_REG:
        case NR34_REG:
        case NR44_REG:
            return mem->IO_Ports[address-0xFF00] | 0xBF;
        case NR30_REG:
            return mem->IO_Ports[NR30_REG-0xFF00] | 0x7F;
        case NR32_REG:
            return mem->IO_Ports[NR32_REG-0xFF00] | 0x9F;
        case NR31_REG:
        case NR41_REG:
            return 0xFF;
        case NR52_REG:
            GB_SoundUpdateClocksCounterReference(GB_CPUClockCounterGet());
            return mem->IO_Ports[NR52_REG-0xFF00] | 0x70;

        //Wave pattern for channel 3
        case 0xFF30: case 0xFF31: case 0xFF32: case 0xFF33:
        case 0xFF34: case 0xFF35: case 0xFF36: case 0xFF37:
        case 0xFF38: case 0xFF39: case 0xFF3A: case 0xFF3B:
        case 0xFF3C: case 0xFF3D: case 0xFF3E: case 0xFF3F:
            GB_SoundUpdateClocksCounterReference(GB_CPUClockCounterGet());

            //if(GameBoy.Emulator.CGBEnabled == 1) //gbc enabled or gbc hardware?
            //    return mem->IO_Ports[address-0xFF00];
            //Demotronic says that it can't be read... :S

            //There are some moments when playing that the sound hardware reads from this ram
            //and those registers read the current value being played, but who cares?
            if(mem->IO_Ports[NR52_REG-0xFF00] & (1<<2)) //If playing...
                return 0xFF;
            else return mem->IO_Ports[address-0xFF00];

        case LY_REG:
            if(GameBoy.Emulator.lcd_on)
            {
                GB_PPUUpdateClocksCounterReference(GB_CPUClockCounterGet());
                return mem->IO_Ports[LY_REG-0xFF00];
            }
            else return 0; // verified on hardware (GBC and GBA SP)





        //                   GAMEBOY COLOR REGISTERS

        case RP_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return 0xFF;
            return (mem->IO_Ports[RP_REG-0xFF00] | 0x3C) | 0x02; //0x02 = no receive signal

        case BCPD_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return 0xFF;
            GB_PPUUpdateClocksCounterReference(GB_CPUClockCounterGet());
#ifdef VRAM_MEM_CHECKING
            if(GameBoy.Emulator.lcd_on && GameBoy.Emulator.ScreenMode == 3) return 0xFF;
#endif
            return GameBoy.Emulator.bg_pal[mem->IO_Ports[BCPS_REG-0xFF00] & 0x3F];

        case OCPD_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return 0xFF;
            GB_PPUUpdateClocksCounterReference(GB_CPUClockCounterGet());
#ifdef VRAM_MEM_CHECKING
            if(GameBoy.Emulator.lcd_on && GameBoy.Emulator.ScreenMode == 3) return 0xFF;
#endif
            return GameBoy.Emulator.spr_pal[mem->IO_Ports[OCPS_REG-0xFF00] & 0x3F];



        //Undocumented registers...
        case 0xFF6C:
            if(GameBoy.Emulator.CGBEnabled == 0) return 0xFF;
            return mem->IO_Ports[0xFF6C-0xFF00] | (0xFE);

        case 0xFF76:
        case 0xFF77:
            return 0x00;

        case 0xFF74:
            if(GameBoy.Emulator.CGBEnabled == 0) return 0xFF;
            return mem->IO_Ports[0xFF74-0xFF00];

        case 0xFF75:
            return mem->IO_Ports[0xFF75-0xFF00] | (0x8F);

        default:
            return 0xFF;
    }

    return 0xFF;
}

//----------------------------------------------------------------

void GB_MemWriteReg8_GBC(u32 address, u32 value)
{
    _GB_MEMORY_ * mem = &GameBoy.Memory;

    switch(address)
    {
        // Serial
        case SB_REG:
            GB_SerialWriteSB(GB_CPUClockCounterGet(),value);
            return;
        case SC_REG:
            GB_SerialWriteSC(GB_CPUClockCounterGet(),value);
            return;

        // Timer
        case TIMA_REG:
            GB_TimersWriteTIMA(GB_CPUClockCounterGet(),value);
            return;
        case TMA_REG:
            GB_TimersWriteTMA(GB_CPUClockCounterGet(),value);
            return;
        case TAC_REG:
            GB_TimersWriteTAC(GB_CPUClockCounterGet(),value);
            return;

        // Divider
        case DIV_REG:
            GB_TimersWriteDIV(GB_CPUClockCounterGet(),value);
            return;

        // Interrupts
        case IF_REG:
            GB_InterruptsWriteIF(GB_CPUClockCounterGet(),value);
            return;

        // DMA
        case DMA_REG:
            GB_DMAWriteDMA(GB_CPUClockCounterGet(),value);
            return;

        // Video
        case LY_REG: //Read only
            return;
        case LYC_REG:
            GB_PPUWriteLYC_GBC(GB_CPUClockCounterGet(),value);
            return;
        case LCDC_REG:
            GB_PPUWriteLCDC_GBC(GB_CPUClockCounterGet(),value);
            return;
        case STAT_REG:
            GB_PPUWriteSTAT_GBC(GB_CPUClockCounterGet(),value);
            return;

        // Joypad
        case P1_REG:
            mem->IO_Ports[P1_REG-0xFF00] = value & 0x30;
            GB_CheckJoypadInterrupt(); // This can trigger the joypad interrupt!!
            return;

        // GBC Registers
        // -------------

        // GDMA/HDMA
        case HDMA1_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return;
            GB_DMAWriteHDMA1(value);
            return;
        case HDMA2_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return;
            GB_DMAWriteHDMA2(value);
            return;
        case HDMA3_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return;
            GB_DMAWriteHDMA3(value);
            return;
        case HDMA4_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return;
            GB_DMAWriteHDMA4(value);
            return;
        case HDMA5_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return;
            GB_DMAWriteHDMA5(value);
            return;

        // Speed switch
        case KEY1_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return;
            mem->IO_Ports[KEY1_REG-0xFF00] &= 0xFE;
            mem->IO_Ports[KEY1_REG-0xFF00] |= value&1;
            return;

        // Work ram bank
        case SVBK_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return;
            GB_MemoryWriteSVBK(value);
            return;

        // Video ram bank
        case VBK_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return;
            GB_MemoryWriteVBK(value);
            return;

        //      TODO
        // ---------------

        case SCY_REG:
        case SCX_REG:
        case BGP_REG:
        case OBP0_REG:
        case OBP1_REG:
        case WY_REG:
        case WX_REG:
            GB_PPUUpdateClocksCounterReference(GB_CPUClockCounterGet());
            mem->IO_Ports[address-0xFF00] = value;
            return;

        //                   Sound...
        case NR10_REG: case NR11_REG: case NR12_REG: case NR13_REG: case NR14_REG:
        case NR21_REG: case NR22_REG: case NR23_REG: case NR24_REG:
        case NR30_REG: case NR31_REG: case NR32_REG: case NR33_REG: case NR34_REG:
        case NR41_REG: case NR42_REG: case NR43_REG: case NR44_REG:
        case NR50_REG: case NR51_REG:
            GB_SoundUpdateClocksCounterReference(GB_CPUClockCounterGet());
            mem->IO_Ports[address-0xFF00] = value;
            GB_SoundRegWrite(address, value);
            return;
        case NR52_REG:
            GB_SoundUpdateClocksCounterReference(GB_CPUClockCounterGet());
            mem->IO_Ports[NR52_REG-0xFF00] &= 0x0F; //Status flags
            mem->IO_Ports[NR52_REG-0xFF00] |= (value & 0xF0);
            GB_SoundRegWrite(address, value);
            return;

        case 0xFF30: case 0xFF31: case 0xFF32: case 0xFF33: //wave for channel 3
        case 0xFF34: case 0xFF35: case 0xFF36: case 0xFF37:
        case 0xFF38: case 0xFF39: case 0xFF3A: case 0xFF3B:
        case 0xFF3C: case 0xFF3D: case 0xFF3E: case 0xFF3F:
            GB_SoundUpdateClocksCounterReference(GB_CPUClockCounterGet());
            if((mem->IO_Ports[NR52_REG-0xFF00] & (1<<2)) == 0) //If not playing...
                mem->IO_Ports[address-0xFF00] = value;
            return;

        //                   GAMEBOY COLOR REGISTERS

        //                   PALETTES
        case BCPS_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return;

            mem->IO_Ports[BCPS_REG-0xFF00] = value | (1<<6);
            return;

        case BCPD_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return;
#ifdef VRAM_MEM_CHECKING
            if(GameBoy.Emulator.ScreenMode == 3 && GameBoy.Emulator.lcd_on) return;
#endif
            GameBoy.Emulator.bg_pal[mem->IO_Ports[BCPS_REG-0xFF00] & 0x3F] = value;
            mem->IO_Ports[BCPD_REG-0xFF00] = value;

            if(mem->IO_Ports[BCPS_REG-0xFF00] & (1<<7))
            {
                u32 index = (mem->IO_Ports[BCPS_REG-0xFF00] + 1) & 0x3F;
                index |= (mem->IO_Ports[BCPS_REG-0xFF00]&(1<<7)) | (1<<6);
                mem->IO_Ports[BCPS_REG-0xFF00] = index;
            }
            return;

        case OCPS_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return;

            mem->IO_Ports[OCPS_REG-0xFF00] = value | (1<<6);
            return;

        case OCPD_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return;
#ifdef VRAM_MEM_CHECKING
            if(GameBoy.Emulator.ScreenMode == 3 && GameBoy.Emulator.lcd_on) return;
#endif
            GameBoy.Emulator.spr_pal[mem->IO_Ports[OCPS_REG-0xFF00] & 0x3F] = value;
            mem->IO_Ports[OCPD_REG-0xFF00] = value;

            if(mem->IO_Ports[OCPS_REG-0xFF00] & (1<<7))
            {
                u32 index = (mem->IO_Ports[OCPS_REG-0xFF00] + 1) & 0x3F;
                index |= (mem->IO_Ports[OCPS_REG-0xFF00]&(1<<7)) | (1<<6);
                mem->IO_Ports[OCPS_REG-0xFF00] = index;
            }
            return;

        case RP_REG:
            if(GameBoy.Emulator.CGBEnabled == 0) return;

            mem->IO_Ports[RP_REG-0xFF00] |= 0x3C;
            return;

        //Undocumented registers...

        // CGB ROM: [FF50]=11
        //  GB game:     [FF6C]=FE, [FF4C]=04, [FF6C]=01
        //  GB+GBC game: [FF6C]=FE, [FF4C]=80
        //  GBC game:    [FF6C]=FE, [FF4C]=C0

        //Change to DMG mode is done when disabling boot ROM?

        case 0xFF4C: // change to gb mode ?
            if(GameBoy.Emulator.CGBEnabled == 0) return;
            // 80h/C0h is writen here if gbc features (bit 7 = gbc mode?)
            // 04h is writen if no gbc features (bit 3 = gb mode?)

            //if(value & 0x80) //GBC mode
            //if(value & 0x04) //GB mode
            return;
        case 0xFF6C: // lock/unlock gbc palettes?
            if(GameBoy.Emulator.CGBEnabled == 0) return;

            mem->IO_Ports[0xFF6C-0xFF00] = value | 0xFE;
            return;
        case 0xFF74:
            if(GameBoy.Emulator.CGBEnabled == 0) return;

            mem->IO_Ports[0xFF74-0xFF00] = value;
            return;

        case 0xFF50: //Disable boot rom
            if(GameBoy.Emulator.enable_boot_rom)
            {
                //if(value == 1) // DMG or SGB
                //if(value == 0xFF) // MGB

                GameBoy.Emulator.enable_boot_rom = 0;

                if(GameBoy.Emulator.CGBEnabled && (mem->IO_Ports[0xFF6C-0xFF00] & 1))
                {
                    //if(value == 0x11) // CGB

                    GameBoy.Emulator.CGBEnabled = 0;
                    GameBoy.Emulator.gbc_in_gb_mode = 1;
                    GameBoy.Emulator.DrawScanlineFn = &GBC_GB_ScreenDrawScanline;
                }

                GB_MemUpdateReadWriteFunctionPointers();

                GB_CPUBreakLoop();
            }
            return;

        case 0xFF76:
        case 0xFF77:
            return;
        case 0xFF72:
            mem->IO_Ports[0xFF72-0xFF00] = value;
            return;
        case 0xFF73:
            mem->IO_Ports[0xFF73-0xFF00] = value;
            return;
        case 0xFF75:
            mem->IO_Ports[0xFF75-0xFF00] = value | 0x8F;
            return;

        default:
            return;
    }

    return;
}

//----------------------------------------------------------------
