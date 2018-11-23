import os

import socket

from ClientWindow import ClientWindow
from ClientPacketHandler import ClientPacketHandler
from ClientACKHandler import ClientACKHandler

class ClientSender(object):
    def __init__(self, senderIP="127.0.0.1", senderPort=8081, sequenceNumberBits=2, windowSize=None, maxSegmentSize=1500, www=os.path.join(os.getcwd(), "data", "sender")):
        self.senderIP = senderIP
        self.senderPort = senderPort
        self.sequenceNumberBits = sequenceNumberBits
        self.windowSize = windowSize
        self.maxSegmentSize = maxSegmentSize
        self.www = www

    def open(self):
        try:
            self.senderSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.senderSocket.bind((self.senderIP, self.senderPort))
            self.senderSocket.setblocking(0)
        except Exception as e:
            print("Creating UDP socket %s:%d for communication with the server failed!" % (self.senderIP, self.senderPort))
            raise Exception

    def send(self, filename, receiverIP="127.0.0.1", receiverPort=8080, totalPackets="ALL", timeout=10):
        print("Transmitting file '%s' to the receiver", filename)
        filename = os.path.join(self.www, filename)
        if not os.path.exists(filename):
            raise Exception("File does not exist!\nFilename: %s" % filename)

        window = ClientWindow(self.sequenceNumberBits, self.windowSize)
        packetHandler = ClientPacketHandler(filename, self.senderSocket, self.senderIP, self.senderPort, receiverIP, receiverPort, window, self.maxSegmentSize, totalPackets, timeout)
        ackHandler = ClientACKHandler(self.senderSocket, self.senderIP, self.senderPort, receiverIP, receiverPort, window)
        packetHandler.start()
        ackHandler.start()
        packetHandler.join()
        ackHandler.join()

    def close(self):
        try:
            if self.senderSocket:
                self.senderSocket.close()
        except Exception as e:
            print("Closing UDP socket %s:%d failed!" % (self.senderIP, self.senderPort))
            raise Exception