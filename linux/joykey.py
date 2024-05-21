#!/usr/bin/env python3

import serial, serial.tools.list_ports,sys,re,time
from evdev import UInput, ecodes as e

class serstick(object):
    keymap=[0, e.KEY_F1, e.KEY_F2, e.KEY_F3, e.KEY_F4, e.KEY_F5,
    e.KEY_F6, e.KEY_F7, e.KEY_F8, e.KEY_F9, e.KEY_F10, e.KEY_F11,
    e.KEY_F12, e.KEY_F13, e.KEY_F14, e.KEY_F15, e.KEY_F16, e.KEY_F17,
    e.KEY_F18, e.KEY_F19, e.KEY_F20, e.KEY_F21, e.KEY_F22, e.KEY_F23,
    e.KEY_F24, e.KEY_KP0, e.KEY_KP1, e.KEY_KP2, e.KEY_KP3, e.KEY_KP4,
    e.KEY_KP5, e.KEY_KP6, e.KEY_KP7, e.KEY_KP8, e.KEY_KP9,
    e.KEY_KPSLASH, e.KEY_KPASTERISK, e.KEY_KPPLUS, e.KEY_KPMINUS,
    e.KEY_KPENTER, e.KEY_KPDOT, e.KEY_ESC, e.KEY_SPACE,e.KEY_ENTER,
    (e.KEY_LEFTALT,e.KEY_P)]
    
    def __init__(self):
        #print("%x" % len(self.__class__.keymap))
        self.args=sys.argv[1:]
        self.silent = False
        name = None
        if len(self.args) > 0 and self.args[0] == '-s':
            self.silent = True
            self.args.pop(0)
        if len(self.args) == 0:
            self.help()
        name = self.args.pop(0).strip()
        if len(self.args) > 0:
            self.help()
        r=re.match(r'^[0-9a-f]{4}:[0-9a-f]{4}$',name,re.I)
        if r:
            name = self.getPort(name)
            if name is None:
                self.help('Port not found')
        else:
            r=re.match(r'^/?(?:dev/)?(?:tty?)(.*$)',name)
            if not r:
                self.help('Bad arg')
            name='/dev/tty'+r.group(1)
        self.serdev = name
        self.port = None
        self.keyset=set()
        self.ui = None
        
    def help(self,s=None):
        if s:
            print ("ERROR",s)
        print ("Usage: %s [-s] [ttyName|uid]" % sys.argv[0])
        exit(0)
        
    def getPort(self,usbid):
        usbid=usbid.upper()
        for port in serial.tools.list_ports.comports():
            if port.subsystem not in ('usb','usb-serial'):
                continue
            uid='%04X:%04X' % (port.vid,port.pid)
            if uid == usbid:
                return port.device
        return None

    def start(self):
        if not self.port:
            self.port=serial.Serial(self.serdev,115200,timeout=0.1)
        if not self.ui:
            self.ui=UInput()

    def restart(self):
        time.sleep(1)
        if self.port:
            self.port.close()
            self.port=None
        try:
            self.start()
        except:
            pass
            
            
    def run(self):
        while True:
            try:
                if not self.port:
                    self.start()
            except KeyboardInterrupt:
                break
            except:
                self.restart()
                continue
                
            try:
                s=self.port.read(1)
                self.port.write(b"SER\n")
            except KeyboardInterrupt:
                break
            except:
                self.restart()
                continue
            if s == b'':
                continue
            if s == b'\r':
                continue
            if s != b'#':
                print (chr(ord(s)),end='')
                continue
            sc=[]
            while True:
                s=self.port.read(1)
                if s == b'\r':
                    continue
                if s == b'\n':
                    break
                sc.append(s)
            if len(sc) == 0:
                keys=[]
            else:
                sc=b''.join(sc)
                try:
                    keys=list(self.__class__.keymap[int(x,16)] for x in sc.split(b','))
                except:
                    continue
            #(keys)
            nset=set(keys)

            for a in list(self.keyset):
                if a not in nset:
                    #print('Del ',a)
                    if isinstance(a,tuple):
                        self.ui.write(e.EV_KEY,a[1],0)
                        self.ui.write(e.EV_KEY,a[0],0)
                    else:
                        self.ui.write(e.EV_KEY,a,0)
                    self.keyset.remove(a)
            for a in nset:
                if a not in self.keyset:
                    #print('Add ',a)
                    if isinstance(a,tuple):
                        self.ui.write(e.EV_KEY,a[0],1)
                        self.ui.write(e.EV_KEY,a[1],1)
                    else:
                        self.ui.write(e.EV_KEY,a,1)
                    self.keyset.add(a)
            self.ui.syn()
            #print(self.keyset)
                    
                

    def mapkeys(self,keys):
        for n,a in enumerate(keys):
            keys[n] = self.__class__.keymap[a]
            
            

s=serstick()
try:
    s.run()
except KeyboardInterrupt:
    if s.ui:
        s.ui.close()
except:
    if s.ui:
        s.ui.close()
    import traceback
    traceback.print_exc()
    
