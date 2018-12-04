import os
import socket
from SenderWindow import SenderWindow
from SenderPacketHandler import SenderPacketHandler
from SenderACKHandler import SenderACKHandler

class Sender(object):
    def __init__(self, senderSocket, debug, parent, sequenceNumberBits=2, windowSize=2, maxSegmentSize=1500):
        self.sequenceNumberBits = sequenceNumberBits
        self.windowSize = windowSize
        self.maxSegmentSize = maxSegmentSize
        self.path = os.path.join(os.getcwd(), parent, "Data", "Sender")
        self.senderSocket = senderSocket
        self.debug = debug

    def send(self, filename, receiverIP, receiverPort, totalPackets="ALL", timeout=2):
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
