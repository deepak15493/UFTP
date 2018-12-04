import random
import struct
import select
import hashlib
import UFTP_Sockets
from collections import namedtuple
from threading import Thread


class ReceiverPacketHandler(Thread):
    PACKET = namedtuple("Packet", ["SequenceNumber", "Data"])
    ACK = namedtuple("ACK", ["AckNumber"])
    senderAddr = ""

    def __init__(self, fileHandle, receiverSocket, window, timeout, debug,
                 bufferSize=2048):
        Thread.__init__(self)
        self.fileHandle = fileHandle
        self.receiverSocket = receiverSocket
        self.window = window
        self.timeout = timeout
        self.bufferSize = bufferSize
        self.debug = debug

    def run(self):
        chance = 0
        while True:
            ready = select.select([self.receiverSocket], [], [], self.timeout)

            if not ready[0]:
                if not self.window.receipt():
                    #waiting for window to receive something
                    print(".",end='',flush=True)
                    continue
                else:
                    if chance == 2:
                        #timeout occured
                        print("Timeout!!")
                        break
                    else:
                        #wait for timeout
                        chance += 1
                        continue
            else:
                chance = 0
                if not self.window.receipt():
                    self.window.start_receipt()

            try:
                receivedPacket, senderAddress = UFTP_Sockets.Socket_Rcv(self.receiverSocket,self.bufferSize)
                self.senderAddr = senderAddress
            except Exception as e:
                print("Receiving UDP packet failed!: ",str(e))

            receivedPacket = self.parse(receivedPacket)

            if self.window.out_of_order(receivedPacket.SequenceNumber):
                if self.debug: print("Discarding packet with sequence number: {}".format(receivedPacket.SequenceNumber))
                if self.debug: print("Transmitting an acknowledgement with ack number: {}".format(receivedPacket.SequenceNumber))
                self.rdt_send(receivedPacket.SequenceNumber, senderAddress)
                continue

            if self.simulate_packet_loss():
                if self.debug: print("Simulating artificial packet loss!!")
                if self.debug: print("Lost a packet with sequence number: {}".format(receivedPacket.SequenceNumber))
                continue

            if self.window.exist(receivedPacket.SequenceNumber):
                if self.debug: print("Received duplicate packet!!")
                if self.debug: print("Discarding packet with sequence number: {}".format(receivedPacket.SequenceNumber))
                continue
            else:
                if self.debug: print("Received packet with sequence number: {}".format(receivedPacket.SequenceNumber))
                if self.debug: print("Transmitting an acknowledgement with ack number: {}".format(receivedPacket.SequenceNumber))
                self.window.store(receivedPacket)
                self.rdt_send(receivedPacket.SequenceNumber, senderAddress)

            if self.window.expected(receivedPacket.SequenceNumber):
                self.deliver_packets()

    def parse(self, receivedPacket):
        sequenceNumber = struct.unpack('>I',receivedPacket[:4].encode("ascii"))[0]
        data = receivedPacket[4:]
        packet = ReceiverPacketHandler.PACKET(SequenceNumber=sequenceNumber, Data=data)
        return packet

    def corrupt(self, receivedPacket):
        computedChecksum = self.checksum(receivedPacket.Data)
        if computedChecksum != receivedPacket.Checksum:
            return True
        else:
            return False

    def checksum(self, data):
        if (len(data) % 2) != 0:
            data += "0"
        intermedsum = 0
        sum = 0
        for i in range(0, len(data), 2):
            data16 = ord(data[i]) + (ord(data[i + 1]) << 8)
            intermedsum = sum + data16
            sum = (intermedsum & 0xffff) + (intermedsum >> 16)

        return ~sum & 0xffff

    def rdt_send(self, ackNumber, senderAddress):
        ack = ReceiverPacketHandler.ACK(AckNumber=ackNumber)
        rawAck = self.make_pkt(ack)

        try:
            UFTP_Sockets.Socket_Send(self.receiverSocket,senderAddress[0],senderAddress[1],rawAck)
        except Exception as e:
            print("Sending UDP packet to {}:{} failed!".format(senderAddress[0], senderAddress[1]))
            print(str(e))
        
    def get_hashcode(self, data):
        hashcode = hashlib.md5()
        hashcode.update(str(data))
        return hashcode.digest()

    def make_pkt(self, ack):
        ackNumber = struct.pack('>I', ack.AckNumber)
        rawAck = ackNumber.decode("ascii")
        return rawAck

    def simulate_packet_loss(self):
        if random.randint(1, 100) <= 1:
            return True
        else:
            return False

    def deliver_packets(self):
        while True:
            packet = self.window.next()
            if packet:
                if self.debug: print("Delivered packet with sequence number: {}".format(packet.SequenceNumber))
                if self.debug: print("Delivered packet with data: ",packet.Data)
                try:
                    self.fileHandle.write(packet.Data)
                except IOError as e:
                    print("Writing to file handle failed!: ",str(e))
            else:
                break
