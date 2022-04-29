'''
Erstellt am 26.06.2022, Author Paul Wochner,
questions to: wochner@magnotherm.com or paulfrederikwochner@gmail.com
This Programm should read data from a serial interface using the module pyserial
'''

# import serial
# import io




# ser = serial.Serial('COM3', 115200, timeout=0,
#         parity=serial.PARITY_EVEN, rtscts=1)
# s = ser.read(1000)       # read up to one hundred bytes
# # or as much is in the buffer
# print(s.decode("Ascii"))

# sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser))

# sio.write(unicode("hello\n"))
# sio.flush() # it is buffering. required to get the data out *now*
# hello = sio.readline()
# ser.close()



#das funktioniert, aber es ist nicht das, was ich will, da es readlines enthält.
#aber es ist in Ordnung

import serial
import time
import io
import threading
# import msvcrt # zum beenden mit Drücken eines Buchstaben


# class communicating:

    
def _readline(ser):
    '''
    von hier: https://discuss.dizzycoding.com/pyserial-2-6-specify-end-of-line-in-readline/
    Returns
    -------
    TYPE
        DESCRIPTION.

    '''
    eol = b'\r\n'
    leneol = len(eol)
    line = bytearray()
    while True:
        c = ser.read(1)
        if c:
            line += c
            if line[-leneol:] == eol:
                break
        else:
            break
    return bytes(line)




# start_measurement_time = time.time()

# serialPort = serial.Serial(port="COM3", bytesize=8, stopbits=serial.STOPBITS_ONE,
#                            baudrate=115200,  timeout=2 )#
# serialPort.xonxoff
# time.sleep(1)#give time to set up
# serialPort.write(b'A')
# # serialPort.flush()# it is buffering. required to get the data out *now* was auch immer das bedeutet
# serialString = ""  # Used to hold data coming over UART
# all_serialstrings = serialString
# allserialString = bytes()
# all_bytes = bytes()
# while 1:
# #     if msvcrt.kbhit():
# #         if ord(msvcrt.getch()) == 27:
# #             serialPort.close()
# #             break
#     # Wait until there is data waiting in the serial buffer
#     # if serialPort.in_waiting > 0:

#     #     # Read data out of the buffer until a carraige return / new line is found
#     #     # wichtig hierbei kann es zu EOL Problemen kommen, siehe: https://pyserial.readthedocs.io/en/latest/shortintro.html
#     #     #https://discuss.dizzycoding.com/pyserial-2-6-specify-end-of-line-in-readline/'
#     #     serialString = serialPort.readline()
#     #     allserialString += serialString
#     #     print(serialString)
#     #     all_serialstrings += serialString.decode("Ascii")
    
#     #     # # Print the contents of the serial data
#     #     # try:
#     #     print(serialString.decode("Ascii"))
#     #     # except:
#     #     #     pass
#     c = _readline(serialPort)
#     all_bytes += c
#     print(c.decode("Ascii"))
    
#     # sio = io.TextIOWrapper(io.BufferedRandom(serialPort), encoding='ascii')#,line_buffering=True
#     # print(sio.readline()[:-1])
#     # hello = sio.readline()
#     # sio.write(unicode("ID\r"))
#     # sio.flush()

#     # print ("reading...")

#     # x = sio.readline()
    
#     # print(len(x))
#     # print(x)
#     current_time = time.time()
#     if start_measurement_time+5 < current_time:
#         serialPort.write(b"E\n")
#         serialPort.close()
#         break


# serialPort.write(b"Hi How are you \r\n")




class Miniterm(object):
    """\
    Terminal application. Copy data from serial port to console and vice versa.
    Handle special keys from the console to show menu etc.
    """

    def __init__(self, serial_instance, echo=False, eol='crlf', filters=()):
        self.serial = serial_instance
        self.echo = echo
        self.raw = False
        self.input_encoding = 'UTF-8'
        self.output_encoding = 'UTF-8'
        self.eol = eol
        self.filters = filters
        self.alive = True
        self._reader_alive = None
        self.receiver_thread = None
        self.rx_decoder = None
        self.tx_decoder = None
        self.tx_encoder = None        
    
    def _start_reader(self):
        """Start reader thread"""
        self._reader_alive = True
        # start serial->console thread
        self.receiver_thread = threading.Thread(target=self.reader, name='rx')
        self.receiver_thread.daemon = True
        self.receiver_thread.start()
        
    
    def reader(self):
        """loop and copy serial->console"""
        try:
            data = self.serial.read(self.serial.in_waiting or 1)
            print(data.decode("Ascii"))
        except:
            print("fehler reader")
    
    def writer(self,text):
        """\
        Loop and copy console->serial until self.exit_character character is
        found. When self.menu_character is found, interpret the next key
        locally.
        """
        try:
            while self.alive:
                self.serial.write(bytes(text))
                print('sendet')
                print(text)
                self.alive = False
        except:
            print("fehler")
    
serialPort = serial.Serial(port="COM3", bytesize=8, #stopbits=serial.STOPBITS_ONE,
                            baudrate=115200,  timeout=2 )
serialPort.rts = True
serialPort.dtr = True
Mt = Miniterm(serialPort,echo=False,eol = "\r\n")
Mt._start_reader()
time.sleep(3)
# Mt.reader()
Mt.writer(b'A100')
time.sleep(3)
# Mt.reader()
Mt.alive = True
Mt.writer(b'E')
#Mt.reader()
time.sleep(2)
Mt.reader()
serialPort.close()

