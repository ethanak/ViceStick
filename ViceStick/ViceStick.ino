/*
 * ViceStick.ino
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
#include "Wire.h"

uint8_t MainMode;

void i2cscan() {
  int nDevices = 0;

  Serial.println("Scanning...");

  for (byte address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println("  !");

      ++nDevices;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  } else {
    Serial.println("done\n");
  }
}

uint8_t serialMode = 0;
uint32_t serHeartTimer;

void setup()
{
    Serial.begin(115200);
    Wire.begin(PIN_SDA, PIN_SCL);
    printf("\n\n");
    //delay(4000);i2cscan();
    initEE();
    initDisplay();
    prgInit();
    initBLE();
    printf("Started\n");
    serHeartTimer=millis();
    
}

static const struct rotbit {
    uint8_t dir_UP;
    uint8_t dir_RITE;
    uint8_t dir_DOWN;
    uint8_t dir_LEFT;
} rotbits[4]={
    {DIR_UP,DIR_RITE,DIR_DOWN,DIR_LEFT},
    {DIR_RITE,DIR_DOWN,DIR_LEFT,DIR_UP},
    {DIR_DOWN,DIR_LEFT,DIR_UP,DIR_RITE},
    {DIR_LEFT,DIR_UP,DIR_RITE,DIR_DOWN}};
    



uint16_t keytable[6];

static uint8_t dirs4;
static uint8_t makeKeyTable(uint8_t keys)
{
    int nkeys=0;
    memset(keytable,0,sizeof(keytable));
    if ((keys & (DIR_UP | DIR_DOWN)) == (DIR_UP | DIR_DOWN)) {
        keytable[0] = bleKeyNum; // Run/Stop
        //printf("QS=%03x\n",keytable[0]);delay(100);
        return 1;
    }
    if ((keys & (DIR_LEFT | DIR_RITE)) == (DIR_LEFT | DIR_RITE)) {
        keytable[0] = bleKeyNum+((CurrentEE.cur_extras >> 2) & 3); // Space bar etc
        //printf("QS=%03x\n",keytable[0]);delay(100);
        return 1;
    }
        
        
    
    int kset = (keys & DIR_PORT) ? CurrentEE.cur_keyset2:
                                   CurrentEE.cur_keyset1;
    int Rotation = CurrentEE.cur_extras & 3;
    const uint8_t *keyset = kset?CurrentEE.keysets[kset-1]:numPadKeys;
    if (keys & rotbits[Rotation].dir_UP) {
        if (keys & rotbits[Rotation].dir_RITE) {
            if (CurrentEE.cur_fourdirs) {
                if (dirs4 & rotbits[Rotation].dir_RITE) {
                    keytable[nkeys++] = keyset[1];
                }
                else {
                    keytable[nkeys++] = keyset[0];
                    dirs4 = rotbits[Rotation].dir_UP;
                }
            }

            else if (keyset[4]) {
                keytable[nkeys++] = keyset[4];
            }
            else {
                keytable[nkeys++] = keyset[0];
                keytable[nkeys++] = keyset[1];
            }
        }
        else if (keys & rotbits[Rotation].dir_LEFT) {
           if (CurrentEE.cur_fourdirs) {
                if (dirs4 & rotbits[Rotation].dir_LEFT) {
                    keytable[nkeys++] = keyset[3];
                }
                else {
                    keytable[nkeys++] = keyset[0];
                    dirs4 = rotbits[Rotation].dir_UP;
                }
            }
            else if (keyset[7]) {
                keytable[nkeys++] = keyset[7];
            }
            else {
                keytable[nkeys++] = keyset[0];
                keytable[nkeys++] = keyset[3];
            }
        }
        else {
            keytable[nkeys++] = keyset[0];
            dirs4=rotbits[Rotation].dir_UP;
        }
    }
    else if (keys & rotbits[Rotation].dir_DOWN) {
        if (keys & rotbits[Rotation].dir_RITE) {
           if (CurrentEE.cur_fourdirs) {
                if (dirs4 & rotbits[Rotation].dir_RITE) {
                    keytable[nkeys++] = keyset[1];
                }
                else {
                    keytable[nkeys++] = keyset[2];
                    dirs4 = rotbits[Rotation].dir_DOWN;
                }
            }
            else if (keyset[5]) {
                keytable[nkeys++] = keyset[5];
            }
            else {
                keytable[nkeys++] = keyset[2];
                keytable[nkeys++] = keyset[1];
            }
        }
        else if (keys & rotbits[Rotation].dir_LEFT) {
           if (CurrentEE.cur_fourdirs) {
                if (dirs4 & rotbits[Rotation].dir_LEFT) {
                    keytable[nkeys++] = keyset[3];
                }
                else {
                    keytable[nkeys++] = keyset[2];
                    dirs4 = rotbits[Rotation].dir_DOWN;
                }
            }
            else if (keyset[6]) {
                keytable[nkeys++] = keyset[6];
            }
            else {
                keytable[nkeys++] = keyset[2];
                keytable[nkeys++] = keyset[3];
            }
        }
        else {
            keytable[nkeys++] = keyset[2];
            dirs4 = rotbits[Rotation].dir_DOWN;
        }
    }
    else if (keys & rotbits[Rotation].dir_RITE) {
        keytable[nkeys++] = keyset[1];
        dirs4 = rotbits[Rotation].dir_RITE;
         
    }
    else if (keys & rotbits[Rotation].dir_LEFT) {
        keytable[nkeys++] = keyset[3];
        dirs4 = rotbits[Rotation].dir_LEFT;
    }
         
    if (keys & DIR_FIRE) keytable[nkeys++] = keyset[8];
    if (keys & DIR_FIRE2) keytable[nkeys++] = keyset[9];
    while (nkeys < 6) keytable[nkeys++] = 0;
    return 0;
}

void serprint(uint16_t *ktab)
{
    static uint16_t lasts[]={0,0,0,0,0,0};
    if (!memcmp(ktab,lasts,12)) return;
    memcpy(lasts,ktab,12);
    char buf[16],*c;
    buf[0]='#';
    c=buf+1;
    *c=0;
    int i;
    for (i=0;i<6 && ktab[i];i++) {
        if (i) *c++=',';
        c+=sprintf(c,"%X",ktab[i]);
    }
    Serial.println(buf);
}

static char ibuf[4];
static int ibufpos=0;
int serloop()
{
    if (Serial.available()) {
        int z = Serial.read();
        if (z == '\n' && ibufpos == 3 && !memcmp(ibuf,"SER",3)) {
            ibufpos = 0;
            return 1;
        }
        if (ibufpos >= 3) {
            ibuf[0]=ibuf[1];
            ibuf[1] = ibuf[2];
            ibufpos=2;
        }
        ibuf[ibufpos++] = z;
    }
    return 0;
}

void loop()
{
    uint8_t keys=prgLoop();
    if (serloop()) {
        if (!serialMode) {
            serialMode = 1;
            uint16_t zero[]={0,0,0,0,0,0};
            sendBt(zero);
        }
        serHeartTimer = millis();
    }
    else if (serialMode && millis() - serHeartTimer >= 1000) {
        serialMode = 0;
    }
#if 0
    if (keys & 0x3f) {
        printf("K=%02X\n",keys);
        delay(100);
    }
#endif
    int wkey=makeKeyTable(keys);
    serprint(keytable);
    if (serialMode) {
        digitalWrite(LED_BLE,millis()%1000 < 100);
    }
    else {
        int rc=sendBt(keytable);
        if (rc < 0) {
            digitalWrite(LED_BLE,millis()%1000 > 500);
        }
        else {
            digitalWrite(LED_BLE,1);
            if (rc > 0) delay(10);
        }
    }
    if (wkey) {
        delay(50);
        makeKeyTable(0);
        serprint(keytable);
        int rc=sendBt(keytable);
        if (rc > 0) delay(10);
        while (prgLoop()) delay(10);
    }
        
    displayLoop();
}

