import math
from collections import OrderedDict


class ReceiverWindow(object):
    def __init__(self, sequenceNumberBits, windowSize, debug):
        self.expectedPkt = 0
        self.maxSequenceSpace = int(math.pow(2, sequenceNumberBits))
        if windowSize is None:
            self.maxWindowSize = int(math.pow(2, sequenceNumberBits-1))
        else:
            if windowSize > int(math.pow(2, sequenceNumberBits-1)):
                print("Invalid window size!!")
            else:
                self.maxWindowSize = windowSize
        self.lastPkt = self.maxWindowSize - 1
        self.receiptWindow = OrderedDict()
        self.isPacketReceipt = False
        self.debug = debug

    def expectedPacket(self):
        return self.expectedPkt

    def lastPacket(self):
        return self.lastPkt

    def out_of_order(self, key):
        if self.expectedPacket() > self.lastPacket():
            if key < self.expectedPacket() and key > self.lastPacket():
                return True
        else:
            if key < self.expectedPacket() or key > self.lastPacket():
                return True
        return False

    def exist(self, key):
        if key in self.receiptWindow and self.receiptWindow[key] != None:
            return True
        return False

    def store(self, receivedPacket):
        if not self.expected(receivedPacket.SequenceNumber):
            sequenceNumber = self.expectedPkt

            while sequenceNumber != receivedPacket.SequenceNumber:
                if sequenceNumber not in self.receiptWindow:
                    self.receiptWindow[sequenceNumber] = None

                sequenceNumber += 1
                if sequenceNumber >= self.maxSequenceSpace:
                    sequenceNumber %= self.maxSequenceSpace

        self.receiptWindow[receivedPacket.SequenceNumber] = receivedPacket

    def expected(self, sequenceNumber):
        if sequenceNumber == self.expectedPkt:
            return True
        return False

    def next(self):
        packet = None

        if len(self.receiptWindow) > 0:
            nextPkt = list(self.receiptWindow.items())[0]
            if nextPkt[1] != None:
                packet = nextPkt[1]

                del self.receiptWindow[nextPkt[0]]

                self.expectedPkt = nextPkt[0] + 1
                if self.expectedPkt >= self.maxSequenceSpace:
                    self.expectedPkt %= self.maxSequenceSpace

                self.lastPkt = self.expectedPkt + self.maxWindowSize - 1
                if self.lastPkt >= self.maxSequenceSpace:
                    self.lastPkt %= self.maxSequenceSpace

        return packet

    def receipt(self):
        return self.isPacketReceipt

    def start_receipt(self):
        self.isPacketReceipt = True
