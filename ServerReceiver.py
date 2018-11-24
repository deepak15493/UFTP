import os
import socket
from ServerWindow import ServerWindow
from ServerPacketHandler import ServerPacketHandler

class ServerReceiver(object):
    def __init__(self, receiverIP, receiverPort, sequenceNumberBits=2, windowSize=None, path=os.path.join(os.getcwd(), "data", "receiver")):
        self.receiverIP = receiverIP
        self.receiverPort = receiverPort
        self.sequenceNumberBits = sequenceNumberBits
        self.windowSize = windowSize
        self.path = path
        self.fileHandle = None

    def open(self):
        try:
            self.receiverSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.receiverSocket.bind((self.receiverIP, self.receiverPort))
            self.receiverSocket.setblocking(0)
        except Exception as e:
            print("Creating UDP socket {}:{} for communication with the client failed!".format(self.receiverIP, self.receiverPort))

    def receive(self, filename, senderIP="127.0.0.1", senderPort=8081, timeout=10):
        filename = os.path.join(self.path, filename)
        try:
            self.fileHandle = open(filename, "wb")
        except IOError as e:
            print("Creating a file handle failed! Filename: {}".format(filename))

        window = ServerWindow(self.sequenceNumberBits, self.windowSize)

        packetHandler = ServerPacketHandler(self.fileHandle, self.receiverSocket, senderIP, senderPort, self.receiverIP, self.receiverPort, window, timeout)

        packetHandler.start()

        packetHandler.join()

    def close(self):
        try:
            if self.fileHandle:
                self.fileHandle.close()
        except IOError as e:
            print("Closing a file handle failed!")
        try:
            if self.receiverSocket:
                self.receiverSocket.close()
        except Exception as e:
            print("Closing UDP socket {}:{} failed!".format(self.receiverIP, self.receiverPort))