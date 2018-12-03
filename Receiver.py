import os
import socket
from ReceiverWindow import ReceiverWindow
from ReceiverPacketHandler import ReceiverPacketHandler

class Receiver(object):
    def __init__(self, receiverSocket, debug, parent, sequenceNumberBits=2, windowSize=2):
        self.sequenceNumberBits = sequenceNumberBits
        self.windowSize = windowSize
        self.path = os.path.join(os.getcwd(), parent, "Data", "Receiver")
        self.fileHandle = None
        self.receiverSocket = receiverSocket
        self.debug = debug

    def receive(self, filename, timeout=2):
        filename = os.path.join(self.path, filename)

        try:
            self.fileHandle = open(filename, "w")
        except IOError as e:
            print("Creating a file handle failed! Filename: {}".format(filename))

        window = ReceiverWindow(self.sequenceNumberBits, self.windowSize, self.debug)
        packetHandler = ReceiverPacketHandler(self.fileHandle, self.receiverSocket, window, timeout, self.debug)
        packetHandler.start()
        packetHandler.join()
        
        try:
            if self.fileHandle:
                self.fileHandle.close()
        except IOError as e:
            print("Closing a file handle failed!")

        return packetHandler.senderAddr
