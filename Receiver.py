import os
import socket
from ReceiverWindow import ReceiverWindow
from ReceiverPacketHandler import ReceiverPacketHandler

class Receiver(object):
    def __init__(self, receiverSocket, debug, sequenceNumberBits=2, windowSize=2, path=os.path.join(os.getcwd(), "Data", "Receiver")):
        # self.receiverIP = receiverIP
        # self.receiverPort = receiverPort
        self.sequenceNumberBits = sequenceNumberBits
        self.windowSize = windowSize
        self.path = path
        self.fileHandle = None
        self.receiverSocket = receiverSocket
        self.debug = debug

    # def open(self):
    #     try:
    #         self.receiverSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    #         self.receiverSocket.bind((self.receiverIP, self.receiverPort))
    #         self.receiverSocket.setblocking(0)
    #     except Exception as e:
    #         print("Creating UDP socket {}:{} for communication with the client failed!".format(self.receiverIP, self.receiverPort))

    def receive(self, filename, timeout=2):
        filename = os.path.join(self.path, filename)
        if self.debug: print("Receiver.receive() filename: ",filename)
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
    
    # def close(self):
    #     try:
    #         if self.fileHandle:
    #             self.fileHandle.close()
    #     except IOError as e:
    #         print("Closing a file handle failed!")
    #     try:
    #         if self.receiverSocket:
    #             self.receiverSocket.close()
    #     except Exception as e:
    #         print("Closing UDP socket {}:{} failed!".format(self.receiverIP, self.receiverPort))
