
import time

import random

import struct
from collections import namedtuple

from threading import Thread
from threading import Lock


LOCK = Lock()

class ClientPacketHandler(Thread):

    HEADER_LENGTH = 6
    PACKET = namedtuple("Packet", ["SequenceNumber", "Checksum", "Data"])

    def __init__(self, filename, senderSocket, senderIP, senderPort, receiverIP, receiverPort, window, maxSegmentSize=1500, totalPackets="ALL", timeout=10, threadName="PacketHandler", bufferSize=2048):
        Thread.__init__(self)
        self.filename = filename
        self.senderSocket = senderSocket
        self.senderIP = senderIP
        self.senderPort = senderPort
        self.receiverIP = receiverIP
        self.receiverPort = receiverPort
        self.window = window
        self.maxSegmentSize = maxSegmentSize
        self.maxPayloadSize = maxSegmentSize - ClientPacketHandler.HEADER_LENGTH
        self.totalPackets = totalPackets
        self.timeout = timeout
        self.threadName = threadName
        self.bufferSize = bufferSize

    def run(self):
        print("[%s] Generating packets", self.threadName)
        packets = self.generate_packets()
        print("[%s] Starting packet transmission", self.threadName)
        while (not self.window.empty() or
                self.window.next() < self.totalPackets):
            if self.window.full():
                pass
            elif (not self.window.full() and
                    self.window.next() >= self.totalPackets):
                pass
            else:
                packet = packets[self.window.next()]
                self.window.consume(packet.SequenceNumber)
                threadName = "Packet(" + str(packet.SequenceNumber) + ")"
                singlePacket = SinglePacket(self.senderSocket, self.receiverIP, self.receiverPort, self.window, packet, self.timeout, threadName=threadName)
                singlePacket.start()

        print("[%s] Stopping packet transmission", self.threadName)
        self.window.stop_transmission()

    def generate_packets(self):
        packets = []
        with open(self.filename, "rb") as f:
            i = 0

            while True:
                data = f.read(self.maxPayloadSize)
                if not data:
                    break
                sequenceNumber = i % self.window.maxSequenceNumber()
                pkt = ClientPacketHandler.PACKET(SequenceNumber=sequenceNumber, Checksum=self.checksum(data), Data=data)
                packets.append(pkt)
                i += 1

        if self.totalPackets == "ALL":
            self.totalPackets = len(packets)
        else:
            if int(self.totalPackets) <= len(packets):
                self.totalPackets = int(self.totalPackets)
            else:
                self.totalPackets = len(packets)

        return packets[:self.totalPackets]

    def checksum(self, data):
        if (len(data) % 2) != 0:
            data += "0"
        sum = 0
        for i in range(0, len(data), 2):
            data16 = ord(data[i]) + (ord(data[i+1]) << 8)
            sum = self.carry_around_add(sum, data16)

        return ~sum & 0xffff

    def carry_around_add(self, sum, data16):
        sum = sum + data16
        return (sum & 0xffff) + (sum >> 16)


class SinglePacket(Thread):
    def __init__(self, senderSocket, receiverIP, receiverPort, window, packet, timeout=10, threadName="Packet(?)"):
        Thread.__init__(self)
        self.senderSocket = senderSocket
        self.receiverIP = receiverIP
        self.receiverPort = receiverPort
        self.window = window
        self.packet = packet
        self.timeout = timeout
        self.threadName = threadName

    def run(self):
        print("[%s] Transmitting a packet with sequence number: %d", self.threadName, self.packet.SequenceNumber)
        self.rdt_send(self.packet)
        self.window.start(self.packet.SequenceNumber)

        while self.window.unacked(self.packet.SequenceNumber):
            timeLapsed = (time.time() - self.window.start_time(self.packet.SequenceNumber))

            if timeLapsed > self.timeout:
                print("[%s] Retransmitting a packet with sequence number: %d", self.threadName, self.packet.SequenceNumber)
                self.rdt_send(self.packet)
                self.window.restart(self.packet.SequenceNumber)

        with LOCK:
            self.window.stop(self.packet.SequenceNumber)

    def rdt_send(self, packet):
        if self.simulate_bit_error():
            print("[%s] Simulating artificial bit error!!", self.threadName)
            print("[%s] Injected bit error into a packet with sequence number: %d", self.threadName, packet.SequenceNumber)
            packet = self.alter_bits(packet)

        rawPacket = self.make_pkt(packet)

        self.udt_send(rawPacket)

    def simulate_bit_error(self):
        if random.randint(1, 10) <= 2:
            return True
        else:
            return False

    def alter_bits(self, packet, alterations=5):
        error = random.getrandbits(8)
        alteredData = list(packet.Data)
        for i in range(alterations):
            randomByte = random.randint(0, len(alteredData))
            alteredByte = ord(alteredData[randomByte]) & error
            alteredData[randomByte] = struct.pack("B", alteredByte)
        alteredData = "".join(alteredData)

        alteredPacket = ClientPacketHandler.PACKET(SequenceNumber=packet.SequenceNumber, Checksum=packet.Checksum, Data=alteredData)

        return alteredPacket

    def make_pkt(self, packet):
        sequenceNumber = struct.pack('=I', packet.SequenceNumber)
        checksum = struct.pack('=H', packet.Checksum)
        rawPacket = sequenceNumber + checksum + packet.Data
        return rawPacket

    def udt_send(self, packet):
        try:
            with LOCK:
                self.senderSocket.sendto(packet,
                                         (self.receiverIP, self.receiverPort))
        except Exception as e:
            print("[%s] Could not send UDP packet!", self.threadName)
            print("Sending UDP packet to %s:%d failed!" % (self.receiverIP, self.receiverPort))
            raise Exception