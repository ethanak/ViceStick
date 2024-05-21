/*
 * program.cpp
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

Bounce inplines[9]={
    Bounce(),Bounce(),Bounce(),Bounce(),Bounce(),
    Bounce(),Bounce(),Bounce(),Bounce()};

uint8_t intpin[]={
    PIN_UP, PIN_RITE, PIN_DOWN, PIN_LEFT, PIN_FIRE2, PIN_FIRE,
    PIN_AUTOFIRE, PIN_PROGRAM, PIN_PORT

};

Bounce *key_Autofire = &inplines[6];  
Bounce *key_Program = &inplines[7];
Bounce *key_Port = &inplines[8];

Bounce *joy_Up = &inplines[0];
Bounce *joy_Rite = &inplines[1];
Bounce *joy_Down = &inplines[2];
Bounce *joy_Left = &inplines[3];
Bounce *joy_Fire2 = &inplines[4];
Bounce *joy_Fire = &inplines[5];

const uint8_t numPadKeys[10]={33,31,27,29,0,0,0,0,25,0};
uint8_t Port = 0;
static uint8_t lockProgram=0;
static uint8_t lockAF=0;
static uint8_t lockPt=0;
static uint8_t rsLock=0;
static uint32_t rsTimer;
static uint32_t autoFireStart;
const uint16_t AutoFirePeriods[]={
    100,150,200,250,300,400,500,650,800,1000};
struct EEPrefs EEcopy;
int progMode;
uint8_t prg_Keyset;
uint8_t prg_Name;
uint8_t prg_Preset;
uint8_t prg_Modify;

uint8_t prg_KSPos;
uint8_t *prg_tmpPtr;
uint8_t prg_tmpTable[16];
uint8_t prg_Cursor;

uint8_t waitUpKeys;

struct JoyPreset prg_JoyPset;

void prgInit()
{
    int i;
    for (i=0;i<9;i++) {
        inplines[i].attach(intpin[i],
#ifdef INVERT_FIRE2
    ((i==4) ? INPUT_PULLDOWN : INPUT_PULLUP)
#else
        INPUT_PULLUP
#endif
        );
        inplines[i].interval(20);
    }
    for (i=0;i<9;i++) inplines[i].update();
    pinMode(LED_AUTOFIRE,OUTPUT);
    pinMode(LED_BLE,OUTPUT);
    digitalWrite(LED_AUTOFIRE,0);
    digitalWrite(LED_BLE,0);
    waitUpKeys=0;
}

void waitUps()
{
    int i;
    for (;;) {
        for (i=0;i<9;i++) {
            inplines[i].update();
            if (!inplines[i].read()) break;
        }
       
        if (i >= 9) {
            waitUpKeys = lockProgram = lockAF = lockPt = 0;
            return;
        }
        delay(10);
    }
}


uint8_t curProgMode;
uint8_t cur_rot;
uint8_t cur_Pset;
uint8_t cur_rspace;
uint32_t curProgTimer;
void startProgCurrent()
{
    curProgMode=0;
    memcpy((uint8_t *)&prg_JoyPset + 2, (uint8_t *)&CurrentEE + 2,2);
    cur_Pset = CurrentEE.lastPreset;
    cur_rot = CurrentEE.cur_extras & 3;
    cur_rspace = (CurrentEE.cur_extras >>2) & 3;
    MainMode = MODE_CURRENT;
    displayCurProg();
}

static void copyFromPreset()
{
    memcpy(&prg_JoyPset,CurrentEE.presets[cur_Pset],6);
    displayCFP();
}

void progCurrent()
{
    if (millis() - curProgTimer > 10000 || FIRE2FELL) {
        waitUpKeys=1;
        displayDiscarded();
        MainMode = 0;
    }
    if (key_Autofire->fell() || joy_Up->fell()) {
        curProgMode= (curProgMode + 1) % CURP_MAX;
        displayCurProg();
        return;
    }
    if (joy_Down->fell()) {
        curProgMode= (curProgMode + CURP_MAX-1) % CURP_MAX;
        displayCurProg();
        return;
    }

    
    if (joy_Fire->fell() && curProgMode == CURP_PRESETS) {
        copyFromPreset();
        return;
    }

    if (joy_Left->fell() || joy_Rite->fell()) {
        switch (curProgMode) {
            case CURP_PRESETS:
            cur_Pset = (cur_Pset+(joy_Left->fell()?4:1)) % 5;
            break;
            case CURP_KEYSET1:
            prg_JoyPset.keyset1 = (prg_JoyPset.keyset1+(joy_Left->fell()?6:1)) % 7;
            break;
            case CURP_KEYSET2:
            prg_JoyPset.keyset2 = (prg_JoyPset.keyset2+(joy_Left->fell()?6:1)) % 7;
            break;
            case CURP_AFPERIOD:
            prg_JoyPset.afspeed = (prg_JoyPset.afspeed + (joy_Left->fell()?9:1)) % 10;
            break;
            case CURP_4DIR:
            prg_JoyPset.fourdirs = !prg_JoyPset.fourdirs ;
            break;
            case CURP_ROTATE:
            cur_rot = (cur_rot+(joy_Left->fell()?3:1)) & 3;
            break;

            case CURP_RSPACE:
            cur_rspace = (cur_rspace + (joy_Left->fell()?3:1)) & 3;
            break;
    

            case CURP_PORT:
            prg_JoyPset.port = !prg_JoyPset.port;
            break;


            case CURP_AFON:
            prg_JoyPset.autofire = !prg_JoyPset.autofire;
            break;

            case CURP_SWFIRES:
            prg_JoyPset.swapFire = !prg_JoyPset.swapFire;
            break;
            
            case CURP_AFMODE:
            prg_JoyPset.autofmode = !prg_JoyPset.autofmode;
            break;

            case CURP_BLENAME:
            prg_JoyPset.blename = !prg_JoyPset.blename;
            break;


            default:
            return;
        }
        displayCurProg();
        return;
    }
        
    if (key_Program->rose()) {
        waitUpKeys = 1;
        memcpy((uint8_t *)&CurrentEE+2,(uint8_t *)&prg_JoyPset+4,2);
        CurrentEE.lastPreset = cur_Pset;
        CurrentEE.cur_extras = (CurrentEE.cur_extras & 0xf0) |
            cur_rot |
            ((cur_rspace & 3) << 2);
        storeCurrent();    
        MainMode = 0;
        if (bleNameChanged()) {
            displayReset();
            ESP.restart();
        }
        displayStored();
        return;
    }
}


void startProgProgram()
{
    
    memcpy(&EEcopy,&CurrentEE,sizeof(struct EEPrefs));
    MainMode = MODE_PROGRAM;
    progMode = 0;
    prg_Keyset = 0;
    prg_Name = 0;
    prg_Preset = 0;
    prg_Modify = 0;
    
    displayWorkProgram();
    
}

static uint32_t joyTimer;
static uint16_t joyRptTime;

static void nextChar()
{
    char c=prg_tmpTable[prg_Cursor];
    if (c == 'z') c='A';
    else if (c >= 'a' && c < 'z') c++;
    else if (c=='Z') c=0;
    else if (c >='A' && c < 'Z') c++;
    else if (c == 0) c='0';
    else if (c >= '0' && c <'9') c++;
    else if (c == '9') c='-';
    else if (c == '-') c='_';
    else if (c == '_') c='a';
    else c=0;
    prg_tmpTable[prg_Cursor] = c;
}

static void prevChar()
{
    char c=prg_tmpTable[prg_Cursor];
    if (c=='A') c='z';
    else if (c >'A' && c <='Z') c--;
    else if (c > 'a' && c <='z') c--;
    else if (c == 'a') c='_';
    else if (c == '_') c='-';
    else if (c == '-') c='9';
    else if (c>'0' && c <='9') c--;
    else if (c == 0) c='Z';
    else c=0;
    prg_tmpTable[prg_Cursor] = c;
}


static uint8_t editText(int maxlen)
{
    static uint32_t editTimer;
    static uint16_t editDelay;
    if (joy_Rite->fell() && prg_Cursor<maxlen-1) {
        prg_Cursor++;
        return 1;
    }
    if (joy_Left->fell() && prg_Cursor>0) {
        prg_Cursor--;
        return 1;
    }
    if (joy_Up->fell()) {
        nextChar();
        editTimer = millis();
        editDelay=500;
        return 1;
    }
    if (joy_Down->fell()) {
        prevChar();
        editTimer = millis();
        editDelay=500;
        return 1;
    }
    if (!joy_Up->read() && millis() - editTimer >= editDelay) {
        nextChar();
        if (editDelay == 500) editDelay=350;
        else if (editDelay > 200) editDelay -=50;
        return 1;
    }
    if (!joy_Down->read() && millis() - editTimer >= editDelay) {
        prevChar();
        if (editDelay == 500) editDelay=350;
        else if (editDelay > 200) editDelay -=50;
        return 1;
    }
    return 0;
        
}

static uint8_t spaceName(int maxlen)
{
    int i,j;
    for (j=maxlen-1;j>0;j--) if (prg_tmpTable[j]) break;
    for (i=0;i<j;i++) if (!prg_tmpTable[i]) prg_tmpTable[i]=' ';
    return prg_tmpTable[0] != 0;
}

static void deSpaceName(int maxlen)
{
    int i;
    for (i=0;i<maxlen;i++) if (prg_tmpTable[i] == ' ') prg_tmpTable[i]=0;
}


static uint8_t reprogramName()
{
    if (FIRE2FELL) {
        prg_Modify=0;
        waitUpKeys = 1;
        displayDiscarded();
        delay(1000);
        return 1;
    }
    if (joy_Fire->fell()) {
        prg_Modify = 0;
        waitUpKeys=1;
        spaceName(15);
        memcpy(EEcopy.names[prg_Name],prg_tmpTable,16);
        displayBanner("OK");
        delay(1000);
        return 1;
    }
    return editText(15);
}

uint8_t prg_PsetPos;
uint8_t prg_PsetModName;
static void startEditPreName()
{
    memcpy(prg_tmpTable,&prg_JoyPset,4);
    deSpaceName(4);
    prg_PsetModName=1;
    prg_Cursor=0;
}

static uint8_t editPreName()
{
    if (FIRE2FELL) {
        prg_PsetModName = 0;
        displayBanner("CANCELED");
        delay(1000);
        return 1;
    }
    if (joy_Fire->fell()) {
        prg_PsetModName = 0;
        spaceName(4);
        memcpy(&prg_JoyPset,prg_tmpTable,4);
        return 1;
    }
    return editText(4);
            
    return 0;
}

static uint8_t reprogramPreset()
{
    if (prg_PsetModName) return editPreName();

    if (FIRE2FELL) {
        prg_Modify=0;
        displayDiscarded();
        delay(1000);
        return 1;
    }
    if (joy_Fire->fell()) {
        prg_Modify = 0;
        if (prg_PsetPos == CPRE_COPY) {
            memcpy(EEcopy.presets[prg_Preset]+4,((uint8_t *)&EEcopy)+2,2);
            EEcopy.lastPreset = prg_Preset;
            displayBanner("COPIED");
        }
        else {
            memcpy(EEcopy.presets[prg_Preset],&prg_JoyPset,6);
            displayBanner("OK");
        }
        delay(1000);
        return 1;
    }
    if (joy_Up->fell()) {
        prg_PsetPos = (prg_PsetPos + 1) % CPRE_MAX;
        return 1;
    }
    if (joy_Down->fell()) {
        prg_PsetPos = (prg_PsetPos + CPRE_MAX-1) % CPRE_MAX;
        return 1;
    }
    if (joy_Left->fell() || joy_Rite->fell()) {
        switch(prg_PsetPos) {
            case CPRE_AFON:
            prg_JoyPset.autofire = !prg_JoyPset.autofire;
            break;

            case CPRE_AFMODE:
            prg_JoyPset.autofmode = (prg_JoyPset.autofmode+1) % 2;
            break;

            case CPRE_AFPERIOD:
            prg_JoyPset.afspeed = (prg_JoyPset.afspeed +
                (joy_Left->fell() ? 9 : 1)) % 10;
            break;

            case CPRE_4DIR:
            prg_JoyPset.fourdirs = 1 - prg_JoyPset.fourdirs;
            break;
            
            case CPRE_BLENAME:
            prg_JoyPset.blename = !prg_JoyPset.blename;
            break;

            case CPRE_KEYSET1:
            prg_JoyPset.keyset1 = (prg_JoyPset.keyset1 +
                (joy_Left->fell() ? 5 : 1)) % 6;
            break;

            case CPRE_KEYSET2:
            prg_JoyPset.keyset2 = (prg_JoyPset.keyset2 +
                (joy_Left->fell() ? 5 : 1)) % 6;
            break;

            case CPRE_NAME:
            startEditPreName();
            break;

            case CPRE_PORT:
            prg_JoyPset.port = !prg_JoyPset.port;
            break;

            case CPRE_SWFIRES:
            prg_JoyPset.swapFire = !prg_JoyPset.swapFire;
            break;

            default:
            return 0;
        }
        return 1;
    }
            
    return 0;
    
}

static uint8_t reprogramKeyset()
{
    int n=keypdir[prg_KSPos];
    if (FIRE2FELL) {
        prg_Modify=0;
        displayDiscarded();
        delay(1000);
        return 1;
    }
    if (joy_Up->fell()) {
        prg_tmpTable[n] = (prg_tmpTable[n]+1) % bleKeyNum;
        joyTimer = millis();
        joyRptTime = 500;
        return 1;
    }
    if (!joy_Up->read()) {
        if (millis() - joyTimer > joyRptTime) {
            joyTimer = millis();
            joyRptTime = 200;
            prg_tmpTable[n] = (prg_tmpTable[n]+1) % bleKeyNum;
            return 1;
        }
        return 0;
    }
            
    if (joy_Down->fell()) {
        prg_tmpTable[n] = (prg_tmpTable[n]+bleKeyNum-1) % bleKeyNum;
        joyTimer = millis();
        joyRptTime = 500;
        return 1;
    }
    if (!joy_Down->read()) {
        if (millis() - joyTimer > joyRptTime) {
            joyTimer = millis();
            joyRptTime = 200;
            prg_tmpTable[n] = (prg_tmpTable[n]+bleKeyNum-1) % bleKeyNum;
            return 1;
        }
        return 0;
    }
    if (joy_Rite->fell()) {
        prg_KSPos = (prg_KSPos + 1) %10;
        return 1;
    }
    if (joy_Left->fell()) {
        prg_KSPos = (prg_KSPos + 9) %10;
        return 1;
    }
    if (FIRE2FELL) {
        prg_Modify=0;
        displayDiscarded();
        delay(1000);
        waitUps();
        return 1;
    }
    if (joy_Fire->fell()) {
        prg_Modify = 0;
        spaceName(15);
        memcpy(EEcopy.keysets[prg_KSPos],prg_tmpTable,10);
        displayBanner("OK");
        delay(1000);
        waitUps();
        return 1;

    }
    return 0;
        
}

static void storeAll(uint8_t store)
{
    MainMode = 0;
    if (!store) {
        displayBanner("CANCELED");
    }
    else {
        memcpy((uint8_t *)&CurrentEE,(uint8_t *)&EEcopy,sizeof(struct EEPrefs));
        storePrefs();
        displayBanner("STORED");
    }
    delay(1000);
    waitUps();
}


void progProgram()
{
    if (millis() - curProgTimer > 10000) {
        displayBanner("CANCELED");
        MainMode = 0;
    }
    if (!prg_Modify) {
        if (key_Program->rose()) {
            if (lockProgram) {
                lockProgram = 0;
            }
        }
        if (key_Autofire->rose()) {
            if (lockAF) lockAF=0;
            else {
                progMode = (progMode + 1) % PRMD_MAX;
                displayWorkProgram();
                return;
            }
        };
        if (key_Port->rose()) {
            if (lockPt) lockPt=0;
            else {
                switch(progMode) {
                    case PRMD_KEYSETS:
                    prg_Keyset = (prg_Keyset+1) % 6;
                    break;

                    case PRMD_NAMES:
                    prg_Name = 1 - prg_Name;
                    break;

                    case PRMD_PRESETS:
                    prg_Preset = (prg_Preset + 1) % 5;
                    break;
                };
                displayWorkProgram();
                return;
            }
        };
        if ((!key_Port->read() && !lockPt && key_Port->duration() > 500) ||
            joy_Fire->fell()) {

            lockPt = 1;
            switch(progMode) {
                case PRMD_KEYSETS:
                prg_Modify = 1;
                prg_KSPos = 0;
                prg_tmpPtr = EEcopy.keysets[prg_Keyset];
                memcpy(prg_tmpTable,prg_tmpPtr,10);
                break;

                case PRMD_NAMES:
                prg_Cursor = 0;
                prg_Modify = 1;
                memset(prg_tmpTable,0,16);
                strcpy((char *)prg_tmpTable,(char *)EEcopy.names[prg_Name]);
                deSpaceName(15);
                break;

                case PRMD_PRESETS:
                prg_PsetPos = 0;
                prg_Modify = 1;
                prg_PsetModName = 0;
                memcpy(&prg_JoyPset,EEcopy.presets[prg_Preset],6);
                break;
                

                case PRMD_CANCEL:
                case PRMD_STORE:
                storeAll(progMode == PRMD_STORE);
                return;
            }
            displayWorkProgram();
            return;
        }
        if (joy_Left->fell()) {
            switch(progMode) {
                case PRMD_KEYSETS:
                prg_Keyset = (prg_Keyset+5) % 6;
                break;

                case PRMD_NAMES:
                prg_Name = 1 - prg_Name;
                break;

                case PRMD_PRESETS:
                prg_Preset = (prg_Preset + 4) % 5;
                break;
            };
            displayWorkProgram();
            return;
        }

        if (joy_Rite->fell()) {
            switch(progMode) {
                case PRMD_KEYSETS:
                prg_Keyset = (prg_Keyset+1) % 6;
                break;

                case PRMD_NAMES:
                prg_Name = 1 - prg_Name;
                break;

                case PRMD_PRESETS:
                prg_Preset = (prg_Preset + 1) % 5;
                break;
            };
            displayWorkProgram();
            return;
        }
        
        if (joy_Up->fell()) {
            progMode = (progMode + 1) % PRMD_MAX;
            displayWorkProgram();
            return;
        }
        if (joy_Down->fell()) {
            progMode = (progMode + PRMD_MAX -1) % PRMD_MAX;
            displayWorkProgram();
            return;
        }
        if (FIRE2FELL) {
            displayBanner("CANCELED");
            MainMode = 0;
            return;
        }
        
            
                
    }
    else {
        int rd=0;
        switch(progMode) {
            case PRMD_KEYSETS:
            rd=reprogramKeyset();
            break;
            case PRMD_NAMES:
            rd=reprogramName();
            break;
            case PRMD_PRESETS:
            rd=reprogramPreset();
            break;
        }
        if (rd) displayWorkProgram();
    }

}

void dsleep()
{
    displayCharge();
    delay(2000);
    displayDisable();
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_32,0);
    esp_deep_sleep_start();
    
}

uint8_t hasFire2()
{
    int kset = Port ? CurrentEE.cur_keyset2:
                                   CurrentEE.cur_keyset1;
    return (kset?CurrentEE.keysets[kset-1]:numPadKeys)[9];
    
}

int prgLoop()
{
    int i;
    static uint32_t afstart=0;
    static uint16_t afdelay=0;
    static uint8_t afnum;
    for (i=0;i<9;i++) inplines[i].update();
    if (MainMode == MODE_CURRENT) {
        progCurrent();
        return 0;
    }
    if (MainMode == MODE_PROGRAM) {
        progProgram();
        return 0;
    }

    if (waitUpKeys) {
        for (i=0;i<9;i++) if (!inplines[i].read()) return 0;
        waitUpKeys = lockProgram = lockAF = lockPt = 0;
    }


    if(key_Program->rose()) {
        if (!lockProgram) {
            startProgCurrent();
            return 0;
        }
        lockProgram = 0;
        return 0;
    }
    if (!key_Program->read()) {
        if (key_Program->duration() >= 500) {
            lockProgram = 1;
            startProgProgram();
            return 0;
        }
    }
    if (key_Autofire->rose()) {
        if (lockAF) {
            if (afnum != CurrentEE.cur_afspeed) storeCurrent();
            lockAF=0;
        }
        else {
            CurrentEE.cur_autofire = ! CurrentEE.cur_autofire;
            autoFireStart = millis();
            storeCurrent();
            displayAutoFire();
        }
    }
    if (!key_Autofire->read()) {
        if (key_Autofire->duration() >= 500) {            
            if (!lockAF) {
                if (!key_Port->read()) {
                    dsleep();
                    lockAF=1;
                    lockPt=1;
                }
                else {
                    lockAF=1;
                    afstart = millis();
                    afdelay=500;
                    afnum=CurrentEE.cur_afspeed;
                    displayAutoFire();
                }
            }
            else if (millis() - afstart > afdelay) {
                afstart=millis();
                if (afdelay == 500) afdelay=450;
                else afdelay=200;
                CurrentEE.cur_afspeed = (CurrentEE.cur_afspeed + 1) % 10;
                displayAutoFire();
            }
        }
    }
    if (key_Port->fell()) {
        displayCurrentPort();
    }
    if (!key_Port->read()) {
        if (key_Port->duration() >= 500) {
            if (!lockPt) {
                if (!key_Autofire->read()) {
                    dsleep();
                    lockAF=1;
                    lockPt=1;
                }
                else {
                    CurrentEE.cur_port = !CurrentEE.cur_port;
                    lockPt=1;
                    storeCurrent();
                    displayCurrentPort();
                }
            }
        }
    }
    if (key_Port->rose()) {
        if (lockPt) lockPt=0;
    }
    uint8_t fire1,fire2;
    if (CurrentEE.cur_swapfire) {
        fire1 = 4;fire2 = 5;
    }
    else {
        fire1 = 5;fire2 = 4;
    }
    if (!hasFire2() && (CurrentEE.cur_extras & 12)) {
        if (FELLFIRE2) {
            rsTimer = millis();
            rsLock = 1;
        }
        else if (!READFIRE2) {
            if (millis() - rsTimer > 500) {
                rsLock=2;
                return DIR_LEFT | DIR_RITE;
            }
        }
        else if (ROSEFIRE2) {
            if (rsLock == 1) {
                rsLock = 0;
                return DIR_UP | DIR_DOWN;
            }
            else rsLock=0;
                
        }
    }
    if (rsLock) return 0;
    uint8_t firefell = FELLFIRE1;
    uint8_t firedown = !READFIRE1;
    if (!hasFire2()) { // both fire buttons
        if (!firefell) firefell=FELLFIRE2;
        if (!firedown) firedown = !READFIRE2;
    }
    if (firefell && CurrentEE.cur_autofire && !CurrentEE.cur_autofmode) autoFireStart = millis();
    uint8_t rc=0;
    uint16_t aper=AutoFirePeriods[CurrentEE.cur_afspeed];
    uint8_t af=(millis() - autoFireStart) % aper < aper/2;
    if (CurrentEE.cur_autofire) {
        digitalWrite(LED_AUTOFIRE,af);
    }
    else {
        digitalWrite(LED_AUTOFIRE,1);
    }
    for (i=0;i<4;i++) if (!inplines[i].read()) rc |= 1<<i;
    if (hasFire2() && !READFIRE2) rc |= 1 <<4; 
    if (!CurrentEE.cur_autofire) {
        if (firedown) rc |= 1<<5;
    }
    else if (af && (CurrentEE.cur_autofmode || firedown)) rc |= 1<<5;    
    if (CurrentEE.cur_port) rc |= 1<<6;
    return rc;
    
}

