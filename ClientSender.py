import os
import socket
from ClientWindow import ClientWindow
from ClientPacketHandler import ClientPacketHandler
from ClientACKHandler import ClientACKHandler

class ClientSender(object):
    def __init__(self, senderIP="127.0.0.1", senderPort=8081, sequenceNumberBits=2, windowSize=None, maxSegmentSize=1500, path=os.path.join(os.getcwd(), "data", "sender")):
        self.senderIP = senderIP
        self.senderPort = senderPort
        self.sequenceNumberBits = sequenceNumberBits
        self.windowSize = windowSize
        self.maxSegmentSize = maxSegmentSize
        self.path = path

    def open(self):
        try:
            self.senderSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.senderSocket.bind((self.senderIP, self.senderPort))
            self.senderSocket.setblocking(0)
        except Exception as e:
            print("Creating UDP socket {}:{} for communication with the server failed!".format(self.senderIP, self.senderPort))
            raise Exception

    def send(self, filename, receiverIP="127.0.0.1", receiverPort=8080, totalPackets="ALL", timeout=10):
        print("Transmitting file '{}' to the receiver".format(filename))
        filename = os.path.join(self.path, filename)
        if not os.path.exists(filename):
            print ("File does not exist!\nFilename: {}".format(filename))

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
            print("Closing UDP socket {}:{} failed!".format(self.senderIP, self.senderPort))
            raise Exception