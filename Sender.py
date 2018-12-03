import os
import socket
from SenderWindow import SenderWindow
from SenderPacketHandler import SenderPacketHandler
from SenderACKHandler import SenderACKHandler

class Sender(object):
    def __init__(self, senderSocket, debug, sequenceNumberBits=2, windowSize=2, maxSegmentSize=1500, path=os.path.join(os.getcwd(), "Data", "Sender")):
        self.sequenceNumberBits = sequenceNumberBits
        self.windowSize = windowSize
        self.maxSegmentSize = maxSegmentSize
        self.path = path
        self.senderSocket = senderSocket
        self.debug = debug

    # def open(self):
    #     try:
    #         self.senderSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    #         self.senderSocket.bind((self.senderIP, self.senderPort))
    #         self.senderSocket.setblocking(0)
    #     except Exception as e:
    #         print("Creating UDP socket {}:{} for communication with the server failed!".format(self.senderIP, self.senderPort))
    #         raise Exception

    def send(self, filename, receiverIP, receiverPort, totalPackets="ALL", timeout=2):
        if self.debug: print("inside sender.send()")
        #print("Transmitting file '{}' to the receiver".format(filename))
        filename = os.path.join(self.path, filename)
        if not os.path.exists(filename):
            print ("File does not exist!\nFilename: {}".format(filename))

        window = SenderWindow(self.sequenceNumberBits, self.windowSize,self.debug)
        packetHandler = SenderPacketHandler(filename, self.senderSocket, receiverIP, receiverPort, window, self.maxSegmentSize, totalPackets, timeout, self.debug)
        ackHandler = SenderACKHandler(self.senderSocket, receiverIP, receiverPort, window, self.debug)
        packetHandler.start()
        ackHandler.start()
        packetHandler.join()
        ackHandler.join()

    # def close(self):
    #     try:
    #         if self.senderSocket:
    #             self.senderSocket.close()
    #     except Exception as e:
    #         print("Closing UDP socket {}:{} failed!".format(self.senderIP, self.senderPort))
    #         raise Exception
