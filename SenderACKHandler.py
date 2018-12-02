import random
import struct
import select
import hashlib
import UFTP_Sockets
from collections import namedtuple

from threading import Thread


class SenderACKHandler(Thread):
    #ACK = namedtuple("ACK", ["AckNumber", "Checksum"])
    ACK = namedtuple("ACK", ["AckNumber"])

    def __init__(self, senderSocket, receiverIP, receiverPort, window, timeout=10, ackLossProbability=0.05, threadName="ACKHandler", bufferSize=2048):
        Thread.__init__(self)
        self.senderSocket = senderSocket
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
                receivedAck, receiverAddress = UFTP_Sockets.Socket_Rcv(self.senderSocket,self.bufferSize)
                #receivedAck, receiverAddress = self.senderSocket.recvfrom(self.bufferSize)
            except Exception as e:
                print("Receiving UDP packet failed!: " + str(e))

            if receiverAddress[0] != self.receiverIP:
                continue

            receivedAck = self.parse(receivedAck)


            #if self.corrupt(receivedAck):
                #print("Received corrupt acknowledgement!!")
                #print("Discarding acknowledgement with ack number: {}".format(receivedAck.AckNumber))
                #continue

            if not self.window.exist(receivedAck.AckNumber):
                print("Received acknowledgement outside transmission window!!")
                print("Discarding acknowledgement with ack number: {}".format(receivedAck.AckNumber))
                continue

            if self.simulate_ack_loss():
                print("Simulating artificial acknowledgement loss!!")
                print("Lost a acknowledgement with ack number: {}".format(receivedAck.AckNumber))
                continue

            print("Received acknowledgement with ack number: {}".format(receivedAck.AckNumber))
            self.window.mark_acked(receivedAck.AckNumber)

    def parse(self, receivedAck):
        ackNumber = struct.unpack('>I', receivedAck[:4].encode("ascii"))[0]
        #checksum = struct.unpack('=16s', receivedAck[4:])[0]
        ack = SenderACKHandler.ACK(AckNumber=ackNumber)#, Checksum=checksum)
        return ack

    def corrupt(self, receivedAck):
        hashcode = hashlib.md5()
        hashcode.update(str(receivedAck.AckNumber))
        if hashcode.digest() != receivedAck.Checksum:
            return True
        else:
            return False

    def simulate_ack_loss(self):
        if random.randint(1, 10) <= 2:
            return True
        else:
            return False
