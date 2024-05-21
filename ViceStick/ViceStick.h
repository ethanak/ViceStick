/*
 * ViceStick.h
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

#ifndef VICESTICK_H
#define VICESTICK_H
#include <Arduino.h>
#include "Bounce2.h"

//#define INVERT_FIRE2

/* pin declarations */

#define PIN_UP 14
#define PIN_DOWN 26
#define PIN_LEFT 25
#define PIN_RITE 27
#define PIN_FIRE 32
#define PIN_FIRE2 33

#define PIN_AUTOFIRE 2
#define PIN_PORT 13
#define PIN_PROGRAM 15

#define PIN_SCL 22
#define PIN_SDA 19

#define LED_AUTOFIRE 16
#define LED_BLE 4



extern Bounce inplines[9];
extern uint8_t MainMode;

enum {
    MODE_NORMAL=0,
    MODE_CURRENT,
    MODE_PROGRAM
};

extern void prgInit();
extern int prgLoop();
extern const uint16_t AutoFirePeriods[];

enum {
    CURP_SWFIRES=0,
    CURP_ROTATE,
    CURP_RSPACE,
    CURP_AFPERIOD,
    CURP_AFMODE,
    CURP_AFON,
    CURP_PORT,
    CURP_4DIR,
    CURP_KEYSET1,
    CURP_KEYSET2,
    CURP_BLENAME,
    CURP_PRESETS,
    CURP_MAX
};

enum {
    PRMD_KEYSETS=0,
    PRMD_PRESETS,
    PRMD_NAMES,
    PRMD_STORE,
    PRMD_CANCEL,
    PRMD_MAX
};

enum {
    CPRE_COPY =0,
    CPRE_SWFIRES,
    CPRE_AFPERIOD,
    CPRE_AFMODE,
    CPRE_AFON,
    CPRE_PORT,
    CPRE_KEYSET1,
    CPRE_KEYSET2,
    CPRE_4DIR,
    CPRE_BLENAME,
    CPRE_NAME,
    CPRE_MAX

};

extern uint8_t curProgMode;
extern uint8_t cur_rot;
extern uint8_t cur_Pset;
extern uint8_t cur_rspace;
extern int progMode;
extern uint8_t prg_Keyset;
extern uint8_t prg_Name;
extern uint8_t prg_Preset;
extern uint8_t prg_Modify;

extern uint8_t prg_KSPos;
extern uint8_t *prg_tmpPtr;
extern uint8_t prg_tmpTable[16];
extern uint8_t prg_Cursor;
extern struct JoyPreset prg_JoyPset;
extern uint8_t prg_PsetPos;
extern uint8_t prg_PsetModName;

extern const uint8_t keypdir[];

extern void initDisplay();
extern void displayCurrentPort();
extern void displayAutoFire();
extern void displayCurProg();
extern void displayDiscarded();
extern void displayStored();
extern void displayReset();
extern void displayCFP();
extern void displayWorkProgram();
extern void displayCharge();
extern void displayDisable();
extern void displayBanner(const char *txt,int nodisplay = 0);

extern void displayLoop();

struct JoyPreset {
    
    char name[4];
    unsigned int port:1;
    unsigned int autofire:1;
    unsigned int autofmode:1;
    unsigned int fourdirs:1;
    unsigned int afspeed:4;
    unsigned int keyset1:3;
    unsigned int keyset2:3;
    unsigned int blename:1;
    unsigned int swapFire:1;
};

extern struct EEPrefs {
    uint16_t magic;
    unsigned int cur_port:1;
    unsigned int cur_autofire:1;
    unsigned int cur_autofmode:1;
    unsigned int cur_fourdirs:1;
    unsigned int cur_afspeed:4;
    unsigned int cur_keyset1:3;
    unsigned int cur_keyset2:3;
    unsigned int cur_blename:1;
    unsigned int cur_swapfire:1;
    char names[2][16];
    uint8_t keysets[6][10];
    uint8_t presets[5][6];
    uint8_t lastPreset;
    uint8_t cur_extras;
    
} CurrentEE, EEcopy;

#define EEBluName CurrentEE.names[CurrentEE.cur_blename]

extern void initEE();
extern void storePort();
extern void storeAfState();
extern void storeAfPeriod();
extern void storeCurrent();
extern void storePrefs();
extern const struct bleKey {
    char name[6];
    uint16_t value;
} bleKeys[];

extern const int bleKeyNum;


extern void initBLE();
extern uint8_t bleNameChanged();
extern int sendBt(uint16_t *ktab);

extern const uint8_t numPadKeys[10];

#define DIR_UP 1
#define DIR_RITE 2
#define DIR_DOWN 4
#define DIR_LEFT 8
#define DIR_FIRE2 16
#define DIR_FIRE 32
#define DIR_PORT 64

#ifdef INVERT_FIRE2
#define FIRE2FELL joy_Fire2->rose()
#define READFIRE2 ((fire2 != 4)?inplines[fire2].read():!inplines[fire2].read())
#define FELLFIRE2 ((fire2 != 4)?inplines[fire2].fell():inplines[fire2].rose())
#define ROSEFIRE2 ((fire2 != 4)?inplines[fire2].rose():inplines[fire2].fell())
#define READFIRE1 ((fire1 != 4)?inplines[fire1].read():!inplines[fire1].read())
#define FELLFIRE1 ((fire1 != 4)?inplines[fire1].fell():inplines[fire1].rose())
#else
#define FIRE2FELL joy_Fire2->fell()
#define READFIRE2 inplines[fire2].read()
#define FELLFIRE2 inplines[fire2].fell()
#define ROSEFIRE2 inplines[fire2].rose()
#define READFIRE1 inplines[fire1].read()
#define FELLFIRE1 inplines[fire1].fell()
#endif

#endif
