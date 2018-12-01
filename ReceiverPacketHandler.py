import random
import struct
import select
import hashlib
from collections import namedtuple
from threading import Thread


class ReceiverPacketHandler(Thread):
    PACKET = namedtuple("Packet", ["SequenceNumber", "Checksum", "Data"])
    ACK = namedtuple("ACK", ["AckNumber", "Checksum"])

    def __init__(self, fileHandle, receiverSocket, senderIP, senderPort, receiverIP, receiverPort, window, timeout=10,
                 bufferSize=2048):
        Thread.__init__(self)
        self.fileHandle = fileHandle
        self.receiverSocket = receiverSocket
        self.senderIP = senderIP
        self.senderPort = senderPort
        self.receiverIP = receiverIP
        self.receiverPort = receiverPort
        self.window = window
        self.timeout = timeout
        self.bufferSize = bufferSize

    def run(self):
        chance = 0
        while True:
            ready = select.select([self.receiverSocket], [], [], self.timeout)
            if not ready[0]:
                if not self.window.receipt():
                    continue
                else:
                    if chance == 5:
                        print("Timeout!!")
                        break
                    else:
                        chance += 1
                        continue
            else:
                chance = 0
                if not self.window.receipt():
                    self.window.start_receipt()
            try:
                receivedPacket, _ = self.receiverSocket.recvfrom(self.bufferSize)
            except Exception as e:
                print("Receiving UDP packet failed!")
            receivedPacket = self.parse(receivedPacket)
            if self.corrupt(receivedPacket):
                print("Discarding packet with sequence number: {}".format(receivedPacket.SequenceNumber))
                continue

            if self.window.out_of_order(receivedPacket.SequenceNumber):
                print("Discarding packet with sequence number: {}".format(receivedPacket.SequenceNumber))
                print("Transmitting an acknowledgement with ack number: {}".format(receivedPacket.SequenceNumber))
                self.rdt_send(receivedPacket.SequenceNumber)
                continue

            if self.simulate_packet_loss():
                print("Simulating artificial packet loss!!")
                print("Lost a packet with sequence number: {}".format(receivedPacket.SequenceNumber))
                continue

            if self.window.exist(receivedPacket.SequenceNumber):
                print("Received duplicate packet!!")
                print("Discarding packet with sequence number: {}".format(receivedPacket.SequenceNumber))
                continue
            else:
                print("Received packet with sequence number: {}".format(receivedPacket.SequenceNumber))
                print("Transmitting an acknowledgement with ack number: {}".format(receivedPacket.SequenceNumber))
                self.window.store(receivedPacket)
                self.rdt_send(receivedPacket.SequenceNumber)

            if self.window.expected(receivedPacket.SequenceNumber):
                self.deliver_packets()

    def parse(self, receivedPacket):
        header = receivedPacket[0:6]
        data = receivedPacket[6:]
        sequenceNumber = struct.unpack('=I', header[0:4])[0]
        checksum = struct.unpack('=H', header[4:])[0]
        packet = ReceiverPacketHandler.PACKET(SequenceNumber=sequenceNumber, Checksum=checksum, Data=data)
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
        intermedsum=0
        sum = 0
        for i in range(0, len(data), 2):
            data16 = ord(data[i]) + (ord(data[i + 1]) << 8)
            intermedsum = sum + data16
            sum = (intermedsum & 0xffff) + (intermedsum >> 16)

        return ~sum & 0xffff

    def rdt_send(self, ackNumber):
        ack = ReceiverPacketHandler.ACK(AckNumber=ackNumber, Checksum=self.get_hashcode(ackNumber))
        rawAck = self.make_pkt(ack)

        try:
            self.receiverSocket.sendto(rawAck, (self.senderIP, self.senderPort))
        except Exception as e:
            print("Sending UDP packet to {}:{} failed!".format(self.senderIP, self.senderPort))

    def get_hashcode(self, data):
        hashcode = hashlib.md5()
        hashcode.update(str(data))
        return hashcode.digest()

    def make_pkt(self, ack):
        ackNumber = struct.pack('=I', ack.AckNumber)
        checksum = struct.pack('=16s', ack.Checksum)
        rawAck = ackNumber + checksum
        return rawAck

    def simulate_packet_loss(self):
        if random.randint(1, 10) <= 2:
            return True
        else:
            return False

    def deliver_packets(self):
        while True:
            packet = self.window.next()
            if packet:
                print("Delivered packet with sequence number: {}".format(packet.SequenceNumber))
                try:
                    self.fileHandle.write(packet.Data)
                except IOError as e:
                    print("Writing to file handle failed!")
            else:
                break