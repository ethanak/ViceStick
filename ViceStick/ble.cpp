/*
 * ble.cpp
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
#include <BleKeyboard.h>


const struct bleKey bleKeys[]={
    {"=None=",0},         // 0
    {"F1",KEY_F1},      // 1
    {"F2",KEY_F2},      // 2
    {"F3",KEY_F3},      // 3
    {"F4",KEY_F4},      // 4
    {"F5",KEY_F5},      // 5
    {"F6",KEY_F6},      // 6
    {"F7",KEY_F7},      // 7
    {"F8",KEY_F8},      // 8
    {"F9",KEY_F9},      // 9
    {"F10",KEY_F10},    // 10
    {"F11",KEY_F11},    // 11
    {"F12",KEY_F12},    // 12
    {"F13",KEY_F13},    // 13
    {"F14",KEY_F14},    // 14
    {"F15",KEY_F15},    // 15
    {"F16",KEY_F16},    // 16
    {"F17",KEY_F17},    // 17
    {"F18",KEY_F18},    // 18
    {"F19",KEY_F19},    // 19
    {"F20",KEY_F20},    // 20
    {"F21",KEY_F21},    // 21
    {"F22",KEY_F22},    // 22
    {"F23",KEY_F23},    // 23
    {"F24",KEY_F24},    // 24
    {"NUM0",KEY_NUM_0}, // 25
    {"NUM1",KEY_NUM_1}, // 26
    {"NUM2",KEY_NUM_2}, // 27
    {"NUM3",KEY_NUM_3}, // 28
    {"NUM4",KEY_NUM_4}, // 29
    {"NUM5",KEY_NUM_5}, // 30
    {"NUM6",KEY_NUM_6}, // 31
    {"NUM7",KEY_NUM_7}, // 32
    {"NUM8",KEY_NUM_8}, // 33
    {"NUM9",KEY_NUM_9}, // 34
    {"SLASH",KEY_NUM_SLASH}, // 35
    {"STAR",KEY_NUM_ASTERISK}, // 36
    {"PLUS",KEY_NUM_PLUS}, // 37
    {"MINUS",KEY_NUM_MINUS}, // 38
    {"ENTER",KEY_NUM_ENTER}, // 39
    {"PERIOD",KEY_NUM_PERIOD}, // 40
    
    {"Esc",KEY_ESC}, // Run/Stop
    {"Space",0x2c}}; // Space bar

const int bleKeyNum =(sizeof(bleKeys) / sizeof(bleKeys[0]))-2;

BleKeyboard bleKeyboard;
static char blename[16];
static uint8_t lastKeys[6];
void initBLE()
{
    strcpy(blename,EEBluName);
    bleKeyboard.setName(blename);
    bleKeyboard.begin();
    memset(lastKeys,0,6);
}

uint8_t bleNameChanged()
{
    return strcmp(EEBluName, blename) ? 1 : 0;
}

void setBleName()
{
    char *c=EEBluName;
    if (strcmp(c,blename)) {
        ESP.restart();
        strcpy(blename,c);
        bleKeyboard.end();
        bleKeyboard.setName(blename);
        bleKeyboard.begin();
    }
}

static void addKey (KeyReport *kr, uint8_t key)
{
    int i;
    if (key >= 136) key -= 136;
    else if (key >= 128) {
        kr->modifiers |= 1 << (key & 7);
        return;
    }
    for (i=0;i<6;i++) if (!kr->keys[i]) {
        kr->keys[i] = key;
        break;
    }
}

int sendBt(uint8_t *ktab)
{
    if(!bleKeyboard.isConnected()) return -1;
    if (!memcmp(ktab,lastKeys,6)) return 0;
    memcpy(lastKeys,ktab,6);
    KeyReport kr;
    memset((void *)&kr,0,sizeof(kr));

    int i;
    for (i=0;ktab[i];i++) {
        addKey(&kr, bleKeys[ktab[i]].value);
    }
    bleKeyboard.sendReport(&kr);
    return 1;
}
