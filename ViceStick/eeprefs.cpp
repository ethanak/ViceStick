/*
 * eeprefs.cpp
 * 
 * Copyright 2024 Bohdan R. Rau <ethanak@polip.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include "ViceStick.h"
#include <I2C_eeprom.h>

#define EE_MAGIC 0xfe0b

struct EEPrefs CurrentEE;

static const struct EEPrefs InitialEE={
        EE_MAGIC,1,0,0,0,4,0,1,0,0,
        {
            "BlueShot-2",
            "ViceStick-1"
        },
        {
            {33,31,27,29,0,0,0,0,25,0},
            {35,38,36,37,0,0,0,0,11,0},
            {13,14,15,16,17,18,19,20,21,22},
            {0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0}
        },
        {
            {'P','s','t','1',0x41,0x08},
            {'P','s','t','2',0x33,0x08},
            {'P','s','t','3',0x41,0x08},
            {'P','s','t','4',0x33,0x08},
            {'P','s','t','5',0x41,0x08},
        },
        0,0
};


static I2C_eeprom ee(0x50, I2C_DEVICESIZE_24LC02,&Wire);

void initEE()
{
    ee.begin();
    int t=ee.isConnected();
    ee.readBlock(0,(uint8_t *)&CurrentEE,sizeof(struct EEPrefs));
    if (CurrentEE.magic != EE_MAGIC) {
        printf("Bad magic\n");
        memcpy(&CurrentEE,&InitialEE,sizeof(struct EEPrefs));
        ee.writeBlock(0,(uint8_t *)&CurrentEE,sizeof(struct EEPrefs));
    }
    else printf("Good magic\n");
}

#define storeCDS ee.writeBlock(2,((uint8_t *)&CurrentEE)+2,2)


void storeCurrent()
{
    storeCDS;
    ee.writeBlock(offsetof(struct EEPrefs, lastPreset), (uint8_t *)&CurrentEE.lastPreset,2);
}


void storePrefs()
{
    ee.writeBlock(0,(uint8_t *)&CurrentEE,sizeof(struct EEPrefs));
}
