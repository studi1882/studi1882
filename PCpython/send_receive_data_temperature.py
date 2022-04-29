# -*- coding: utf-8 -*-
"""
Created on Wed Apr 27 11:59:04 2022

@author: wochn
"""

import serial
import threading
import time
from serial.tools.miniterm import unichr

class TerminalFrame():
    def __init__(self, *args, **kwds):
        self.serial = serial.Serial(port="COM3", stopbits=serial.STOPBITS_ONE, bytesize=8, 
                                    baudrate=115200)#stopbits=serial.STOPBITS_ONE,
        self.serial.timeout = 0.5   # make sure that the alive event can be checked from time to time
        self.thread = None
        self.thread2 = None
        self.alive = threading.Event()
    
    
    def StartThread(self):
        """Start the receiver thread"""
        self.thread = threading.Thread(target=self.ComPortThread)
        self.thread.setDaemon(1)
        self.alive.set()
        self.thread.start()
        self.serial.rts = True
        self.serial.dtr = True


    def StopThread(self):
        """Stop the receiver thread, wait until it's finished."""
        if self.thread is not None:
            self.alive.clear()          # clear alive event for thread
            self.thread.join()          # wait until thread has finished
            self.thread = None
            # self.thread2.join()
            # self.thread2 = None
            
    def Startthread2(self):
        self.thread2 = threading.Thread(target=self.OnKey)
        self.thread2.setDaemon(1)
        self.thread2.start()
            
    def OnClose(self):
        """Called on application shutdown."""
        self.StopThread()               # stop reader thread
        self.serial.close()             # cleanup
        

    def OnKey(self, text):
        """\
        Key event handler. If the key is in the ASCII range, write it to the
        serial port. Newline handling and local echo is also done here.
        """
        # if code < 256:   # XXX bug in some versions of wx returning only capital letters
        #     code = event.GetKeyCode()
        # code = ord(u"%s"%text)
        # char = unichr(code)
        #hier eine Kontrollschleife einbauen, damit man gerade auch senden kann
        #sonst fehlermeldung anzeigen
        print(text)
        print(type(text))
        self.serial.write(text.encode('UTF-8', 'replace'))         # send the character
        
        
    def ComPortThread(self):
        """\
        Thread that handles the incoming traffic. Does the basic input
        transformation (newlines) and generates an SerialRxEvent
        """
        while self.alive.isSet():
            b = self.serial.read(self.serial.in_waiting or 1)
            print(b)

    def OnRTS(self, event):  # wxGlade: TerminalFrame.<event_handler>
        self.serial.rts = event.IsChecked()

    def OnDTR(self, event):  # wxGlade: TerminalFrame.<event_handler>
        self.serial.dtr = event.IsChecked()

Mt = TerminalFrame(None, -1, "")
Mt.StartThread()
# Mt.Startthread2()
time.sleep(2)# nicht zu kurz warten, es dauert ein bisschen bis die Verbindung hergestellt ist. warten
# hier ist es besser nicht mehr Zeitbasier zu arbeiten sondern bis etwas im seriellen input ist, also:
#serial.in_waiting or 1
#das auch in die Funktion einbauen
Mt.OnKey('A1000')
time.sleep(5)
Mt.OnKey('E')
time.sleep(2)#Zeit geben zum beenden
Mt.OnClose()

# mit dem oben noch einmal einfach programmieren, diese Programme später in einen anderen Ordner
#speichern als csv