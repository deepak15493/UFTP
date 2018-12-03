import time
import math
from collections import OrderedDict
from threading import Lock

LOCK = Lock()

class SenderWindow(object):
    def __init__(self, sequenceNumberBits, windowSize, debug):
        self.expectedAck = 0
        self.nextSequenceNumber = 0
        self.nextPkt = 0
        self.maxSequenceSpace = int(math.pow(2, sequenceNumberBits))
        if windowSize is None:
            self.maxWindowSize = int(math.pow(2, sequenceNumberBits - 1))
        else:
            if windowSize > int(math.pow(2, sequenceNumberBits - 1)):
                print("Invalid window size!!")
            else:
                self.maxWindowSize = windowSize
        self.transmissionWindow = OrderedDict()
        self.isPacketTransmission = True
        self.debug = debug

    def expectedACK(self):
        return self.expectedAck

    def maxSequenceNumber(self):
        return self.maxSequenceSpace

    def empty(self):
        if len(self.transmissionWindow) == 0:
            return True
        return False

    def full(self):
        if len(self.transmissionWindow) >= self.maxWindowSize:
            return True
        return False

    def exist(self, key):
        if key in self.transmissionWindow:
            return True
        return False

    def next(self):
        return self.nextPkt

    def consume(self, key):
        with LOCK:
            self.transmissionWindow[key] = [None, False]

            self.nextSequenceNumber += 1
            if self.nextSequenceNumber >= self.maxSequenceSpace:
                self.nextSequenceNumber %= self.maxSequenceSpace

            self.nextPkt += 1

    def start(self, key):
        with LOCK:
            self.transmissionWindow[key][0] = time.time()

    def restart(self, key):
        with LOCK:
            self.transmissionWindow[key][0] = time.time()

    def stop(self, key):
        if self.exist(key):
            self.transmissionWindow[key][0] = None

        if key == self.expectedAck:
            flag=1
            while(flag==1):
                for k, v in self.transmissionWindow.items():
                    if v[0] == None and v[1] == True:
                        del self.transmissionWindow[k]
                        break
                    else:
                        flag=0
                        break

            if len(self.transmissionWindow) == 0:
                self.expectedAck = self.nextSequenceNumber
            else:
                self.expectedAck = list(self.transmissionWindow.items())[0][0]

    def start_time(self, key):
        return self.transmissionWindow[key][0]

    def unacked(self, key):
        if self.exist(key) and self.transmissionWindow[key][1] == False:
            return True
        return False

    def mark_acked(self, key):
        with LOCK:
            self.transmissionWindow[key][1] = True

    def stop_transmission(self):
        with LOCK:
            self.isPacketTransmission = False

    def transmit(self):
        return self.isPacketTransmission
