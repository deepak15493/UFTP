import random
import struct
import select
import hashlib
from collections import namedtuple
from threading import Thread

class ClientACKHandler(Thread):
    ACK = namedtuple("ACK", ["AckNumber", "Checksum"])

    def __init__(self, senderSocket, senderIP, senderPort, receiverIP, receiverPort, window, timeout=10, ackLossProbability=0.05, threadName="ACKHandler", bufferSize=2048):
        Thread.__init__(self)
        self.senderSocket = senderSocket
        self.senderIP = senderIP
        self.senderPort = senderPort
        self.receiverIP = receiverIP
        self.receiverPort = receiverPort
        self.window = window
        self.timeout = timeout
        self.ackLossProbability = ackLossProbability
        self.threadName = threadName
        self.bufferSize = bufferSize

    def run(self):
        while self.window.transmit():
            if self.window.empty():
                continue

            ready = select.select([self.senderSocket], [], [], self.timeout)

            if not ready[0]:
                continue


            try:
                receivedAck, receiverAddress = self.senderSocket.recvfrom(self.bufferSize)
            except Exception as e:
                print("Receiving UDP packet failed!")
                raise Exception

            if receiverAddress[0] != self.receiverIP:
                continue

            receivedAck = self.parse(receivedAck)


            if self.corrupt(receivedAck):
                print("[%s] Received corrupt acknowledgement!!", self.threadName)
                print("[%s] Discarding acknowledgement with ack number: %d", self.threadName, receivedAck.AckNumber)
                continue

            if not self.window.exist(receivedAck.AckNumber):
                print("[%s] Received acknowledgement outside transmission window!!", self.threadName)
                print("[%s] Discarding acknowledgement with ack number: %d", self.threadName, receivedAck.AckNumber)
                continue

            if self.simulate_ack_loss():
                print("[%s] Simulating artificial acknowledgement loss!!", self.threadName)
                print("[%s] Lost a acknowledgement with ack number: %d", self.threadName, receivedAck.AckNumber)
                continue

            print("[%s] Received acknowledgement with ack number: %d", self.threadName, receivedAck.AckNumber)
            self.window.mark_acked(receivedAck.AckNumber)

    def parse(self, receivedAck):
        ackNumber = struct.unpack('=I', receivedAck[0:4])[0]
        checksum = struct.unpack('=16s', receivedAck[4:])[0]

        ack = ClientACKHandler.ACK(AckNumber=ackNumber, Checksum=checksum)

        return ack

    def corrupt(self, receivedAck):
        hashcode = hashlib.md5()
        hashcode.update(str(receivedAck.AckNumber))

        if hashcode.digest() != receivedAck.Checksum:
            return True
        else:
            return False

    def simulate_packet_loss(self):
        if random.randint(1, 10) <= 2:
            return True
        else:
            return False