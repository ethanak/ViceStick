/*
 * display.cpp
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
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C // do sprawdzenia przez i2cscanner

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static uint32_t displayTimer;
static uint8_t displayStarted;
extern uint32_t curProgTimer;

static void startDisplay()
{
    display.display();
    displayTimer=millis();
    displayStarted=1;
}

void displayDisable()
{
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    delay(100);
}
void initDisplay()
{
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.ssd1306_command(SSD1306_DISPLAYON);
    display.cp437();
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(64-9*6, 0);
    display.print("ViceStick");
    char *blename=EEBluName;
    display.setCursor(64-6*strlen(blename), 16);
    display.print(blename);
    startDisplay();

}

void displayCurrentPort()
{
    display.clearDisplay();
    display.setTextSize(3,4);
    display.setCursor(10,0);
    display.printf("PORT %d",CurrentEE.cur_port+1);
    startDisplay();
}

static const char * const afmodes[2]={"Press","Still"};
static const char * const fbswaps[2]={"straight","swapped"};
static const char *const rotext[4]={"Plain","Left","Back","Right"};
static const char *const rsptext[4]={"Off","Space","Enter","Alt+P"};
void displayAutoFire()
{
    display.clearDisplay();
    display.setTextSize(2,2);
    display.setCursor(64-8*6,0);
    display.printf("AutoFire");
    display.setCursor(128-3*12,16);
    display.printf("%s", CurrentEE.cur_autofire ? "ON" : "OFF");
    display.setTextSize(1,2);
    display.setCursor(0,16);
    display.printf("%s %4d ms",afmodes[CurrentEE.cur_autofmode],
    AutoFirePeriods[CurrentEE.cur_afspeed]); ;
    startDisplay();
}

void displayCFP()
{
    display.clearDisplay();
    display.setTextSize(2,2);
    display.setCursor(64-7*6,0);
    display.printf("Current");
    display.setTextSize(1,2);
    display.setCursor(64-16 * 3,16);
    display.printf("Copied from %-4.4s",
        CurrentEE.presets[cur_Pset]);
    display.display();
    curProgTimer = millis();
}

    
void displayCurProg()
{
    display.clearDisplay();
    display.setTextSize(2,2);
    display.setCursor(64-7*6,0);
    display.printf("Current");
    switch(curProgMode) {
        case CURP_PRESETS:
        display.setTextSize(1,2);
        display.setCursor(64-14*3,16);
        display.printf("Copy from %-4.4s", CurrentEE.presets[cur_Pset]);
        break;

        case CURP_ROTATE:
        display.setTextSize(2,2);
        display.setCursor(4,16);
        display.printf("Rot: %s",rotext[cur_rot]);
        break;

        case CURP_RSPACE:
        display.setTextSize(1,2);
        display.setCursor(64-16*3,16);
        display.printf("Fire2 R/S: %s",rsptext[cur_rspace]);
        break;
        
        case CURP_PORT:
        display.setTextSize(2,2);
        display.setCursor(64-6*6,16);
        display.printf("PORT %d", prg_JoyPset.port+1);
        break;

        
        case CURP_AFON:
        display.setCursor(0,16);
        display.setTextSize(1,2);
        display.print("AutoFire");
        display.setTextSize(2,2);
        display.setCursor(128-60,16);
        display.print(prg_JoyPset.autofire? "ON": "OFF");
        break;
        
        case CURP_AFPERIOD:
        display.setCursor(10,16);
        display.setTextSize(1,2);
        display.printf("AutoFire %4d msec",AutoFirePeriods[prg_JoyPset.afspeed]);
        break;

        case CURP_4DIR:
        display.setCursor(63-7*5,16);
        display.setTextSize(2);
        display.printf("DIRS: %d",prg_JoyPset.fourdirs?4:8);
        break;
        
        
        
        case CURP_AFMODE:
        display.setCursor(0,16);
        display.setTextSize(1,2);
        display.print("AutoFire");
        display.setTextSize(2,2);
        display.setCursor(128-60,16);
        display.print(afmodes[prg_JoyPset.autofmode]);
        break;

        case CURP_KEYSET1:
        if (!prg_JoyPset.keyset1) {
            display.setCursor(4,16);
            display.print("KeySet1 NP");
        }
        else {
            display.setCursor(64-6*9,16);
            display.printf("KeySet1 %d",prg_JoyPset.keyset1);
        }
        break;

        case CURP_KEYSET2:
        if (!prg_JoyPset.keyset2) {
            display.setCursor(4,16);
            display.print("KeySet2 NP");
        }
        else {
            display.setCursor(64-6*9,16);
            display.printf("KeySet2 %d",prg_JoyPset.keyset2);
        }
        break;

        case CURP_SWFIRES:
        display.setCursor(12,16); // plain, swap
        display.setTextSize(1,2);
        display.printf("Buttons: %s",fbswaps[prg_JoyPset.swapFire]);
        break;

        case CURP_BLENAME:
        display.setTextSize(2,2);
        display.setCursor(0,16);
        display.print("BT");
        display.setTextSize(1,2);
        display.setCursor(3*12,16);
        display.print(CurrentEE.names[prg_JoyPset.blename]);
        break;
        
    }
    display.display();
    curProgTimer = millis();
}


void displayBanner(const char *txt,int nodisplay)
{
    int l=strlen(txt) * 6;
    int w,h;
    if (l <=32) {
        w=4;
        h=4;
    }
    else if (l <= 42) {
        w=3;
        h=3;
    }
    else if (l <= 64) {
        w=2;
        h=3;
    }
    else {
        w=1;
        h=2;
    }
    display.clearDisplay();
    display.setTextSize(w,h);
    display.setCursor(64-(l*w)/2,16-4*h);
    display.print(txt);
    if (!nodisplay) startDisplay();
}

void displayDiscarded()
{
    displayBanner("CANCELED");
    
}

void displayStored()
{
    displayBanner("STORED");
}

void displayCharge()
{
    displayBanner("CHARGE");
    startDisplay();
}

void displayReset()
{
    displayBanner("RESETTING");
}

static void displayProgMode()
{
    display.clearDisplay();
    display.setTextSize(2);
    char *cn;
    switch(progMode) {
        case PRMD_STORE:
        displayBanner("STORE",1);
        break;
        case PRMD_CANCEL:
        displayBanner("CANCEL",1);
        break;
        case PRMD_KEYSETS:
        display.setCursor(64-6*6,0);
        display.print("KEYSET");
        display.setCursor(64-8*6,16);
        display.printf("Modify %d",prg_Keyset+1);
        break;

        case PRMD_NAMES:
        display.setCursor(64-8*6,0);
        display.print("BLE NAME");
        display.setTextSize(1,2);
        cn=EEcopy.names[prg_Name];
        display.setCursor(64-3*strlen(cn),16);
        display.print(cn);
        break;

        case PRMD_PRESETS:
        display.setCursor(64-6*6,0);
        display.print("PRESET");
        display.setCursor(0,16);
        display.print("Modify");
        display.setCursor(0 + 13 * 6,16);
        display.printf("%d",prg_Preset+1);
        display.setTextSize(1,2);
        display.setCursor(128-24,16);
        display.printf("%-4.4s",EEcopy.presets[prg_Preset]);

        break;
    }
    display.display();
}
static const char * const keyDirs[]={
    "Up","Down","Left","Right","Up-R","Down-R","Up-L","Down-L","Fire","Fire2"};
const uint8_t keypdir[]={
    0,2,3,1,4,5,7,6,8,9};

static void printName(const char *t,int ile)
{
    int i;
    for (i=0;i<ile;i++) {
        char c=t[i]?t[i]:' ';
        display.print(c);
    }
}


void displayModName()
{
    display.clearDisplay();
    display.setTextSize(2,2);
    display.setCursor(0,8);
    display.printf("%d", prg_Name+1);
    display.setCursor(20,8);
    display.setTextSize(1,2);
    printName((const char *)prg_tmpTable,15);
    display.setCursor(17+6*prg_Cursor,24);
    display.setTextSize(2,2);
    display.print('^');
    display.display();
}
    
void displayModKeyset()
{
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(64-6*8,0);
    display.printf("KEYSET %d",prg_Keyset+1);
    display.setCursor(0,16);
    int n=keypdir[prg_KSPos];
    display.print(keyDirs[prg_KSPos]);
    display.setTextSize(1,2);
    display.setCursor(82,16);
    display.print(bleKeys[prg_tmpTable[n]].name);
    display.display();
}

void displayPrgPreset()
{
    display.clearDisplay();
    if (prg_PsetModName) {
        display.setTextSize(2,2);
        display.setCursor(0,8);
        display.printf("PSET %d",prg_Preset+1);
        display.setCursor(128-48,8);
        printName((const char *)prg_tmpTable,4);
        display.setCursor(128-48+12*prg_Cursor,24);
        display.setTextSize(2,1);
        display.print('^');
        display.display();
        return;
        
    }
    display.setTextSize(2);
    display.setCursor(64-6*8,0);
    display.printf("PRESET %d",prg_Preset+1);
    switch(prg_PsetPos) {
        case CPRE_COPY:
        display.setTextSize(1,2);
        display.setCursor(64-3*17,16);
        display.print("Copy from current");
        break;

        case CPRE_AFMODE:
        display.setCursor(0,16);
        display.setTextSize(1,2);
        display.print("AutoFire");
        display.setTextSize(2,2);
        display.setCursor(128-60,16);
        display.print(afmodes[prg_JoyPset.autofmode]);
        break;

        case CPRE_PORT:
        display.setTextSize(2,2);
        display.setCursor(64-6*6,16);
        display.printf("PORT %d", prg_JoyPset.port+1);
        break;

        case CPRE_4DIR:
        display.setCursor(63-7*5,16);
        display.setTextSize(2);
        display.printf("DIRS: %d",prg_JoyPset.fourdirs?4:8);
        break;
        
        case CPRE_AFON:
        display.setCursor(0,16);
        display.setTextSize(1,2);
        display.print("AutoFire");
        display.setTextSize(2,2);
        display.setCursor(128-60,16);
        display.print(prg_JoyPset.autofire? "ON": "OFF");
        break;
        
        case CPRE_AFPERIOD:
        display.setCursor(10,16);
        display.setTextSize(1,2);
        display.printf("AutoFire %4d msec",AutoFirePeriods[prg_JoyPset.afspeed]);
        break;
        
        case CPRE_KEYSET1:
        if (!prg_JoyPset.keyset1) {
            display.setCursor(4,16);
            display.print("KeySet1 NP");
        }
        else {
            display.setCursor(64-6*9,16);
            display.printf("KeySet1 %d",prg_JoyPset.keyset1);
        }
        break;

        case CPRE_KEYSET2:
        if (!prg_JoyPset.keyset2) {
            display.setCursor(4,16);
            display.print("KeySet2 NP");
        }
        else {
            display.setCursor(64-6*9,16);
            display.printf("KeySet2 %d",prg_JoyPset.keyset2);
        }
        break;

        case CPRE_SWFIRES:
        display.setCursor(12,16); // plain, swap
        display.setTextSize(1,2);
        display.printf("Buttons: %s",fbswaps[prg_JoyPset.swapFire]);
        break;

        case CPRE_BLENAME:
        display.setTextSize(2,2);
        display.setCursor(0,16);
        display.print("BT");
        display.setTextSize(1,2);
        display.setCursor(3*12,16);
        display.print(CurrentEE.names[prg_JoyPset.blename]);
        break;

        case CPRE_NAME:
        display.setTextSize(2,2);
        display.setCursor(64-9*6,16);
        display.printf("Name %-4.4s",prg_JoyPset.name);
        break;
        
        
    }
    display.display();
    
}


void displayWorkProgram()
{
    curProgTimer = millis();
    if (!prg_Modify) {
        displayProgMode();
    }
    else switch(progMode) {
        case PRMD_KEYSETS:
        displayModKeyset();
        break;
        case PRMD_NAMES:
        displayModName();
        break;
        case PRMD_PRESETS:
        displayPrgPreset();
        break;
    }
}

void displayLoop()
{
    if (!MainMode && displayStarted && millis() - displayTimer > 2500) {
        displayStarted = 0;
        display.clearDisplay();
        display.display();
    }
}
