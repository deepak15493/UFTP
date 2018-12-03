import time
import random
import struct
import UFTP_Sockets
from collections import namedtuple
from threading import Thread
from threading import Lock

LOCK = Lock()

class SenderPacketHandler(Thread):
    HEADER_LENGTH = 6
    PACKET = namedtuple("Packet", ["SequenceNumber", "Data"])
    def __init__(self, filename, senderSocket, receiverIP, receiverPort, window, maxSegmentSize, totalPackets, timeout, debug, threadName="PacketHandler", bufferSize=2048):
        Thread.__init__(self)
        self.filename = filename
        self.senderSocket = senderSocket
        self.receiverIP = receiverIP
        self.receiverPort = receiverPort
        self.window = window
        self.maxSegmentSize = maxSegmentSize
        self.maxPayloadSize = maxSegmentSize - SenderPacketHandler.HEADER_LENGTH
        self.totalPackets = totalPackets
        self.timeout = timeout
        self.threadName = threadName
        self.bufferSize = bufferSize
        self.debug = debug

    def run(self):
        packets = self.generate_packets()
        print("Starting packet transmission")
        
        while (not self.window.empty() or self.window.next() < self.totalPackets):
            if self.window.full():
                pass
            elif (not self.window.full() and
                    self.window.next() >= self.totalPackets):
                pass
            else:
                packet = packets[self.window.next()]
                self.window.consume(packet.SequenceNumber)
                threadName = "Packet(" + str(packet.SequenceNumber) + ")"
                singlePacket = SinglePacket(self.senderSocket, self.receiverIP, self.receiverPort, self.window, packet, self.timeout, self.debug, threadName=threadName)
                singlePacket.start()

        print("Stopping packet transmission")
        self.window.stop_transmission()

    def generate_packets(self):
        packets = []
        with open(self.filename, "r") as f:
            i = 0

            while True:
                data = f.read(self.maxPayloadSize)

                if not data:
                    break

                sequenceNumber = i % self.window.maxSequenceNumber()
                pkt = SenderPacketHandler.PACKET(SequenceNumber=sequenceNumber, Data=data)
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
        intermedsum = 0

        for i in range(0, len(data), 2):
            data16 = ord(data[i]) + (ord(data[i+1]) << 8)
            intermedsum = sum + data16
            sum = (intermedsum & 0xffff) + (intermedsum >> 16)

        return ~sum & 0xffff

class SinglePacket(Thread):
    def __init__(self, senderSocket, receiverIP, receiverPort, window, packet, timeout, debug, threadName="Packet(?)"):
        Thread.__init__(self)
        self.senderSocket = senderSocket
        self.receiverIP = receiverIP
        self.receiverPort = receiverPort
        self.window = window
        self.packet = packet
        self.timeout = timeout
        self.threadName = threadName
        self.debug = debug

    def run(self):
        if self.debug: print("Transmitting a packet with sequence number: {}".format(self.packet.SequenceNumber))
        self.rdt_send(self.packet)
        self.window.start(self.packet.SequenceNumber)

        while self.window.unacked(self.packet.SequenceNumber):
            timeLapsed = (time.time() - self.window.start_time(self.packet.SequenceNumber))

            if timeLapsed > self.timeout:
                if self.debug: print("Retransmitting a packet with sequence number: {}".format(self.packet.SequenceNumber))
                self.rdt_send(self.packet)
                self.window.restart(self.packet.SequenceNumber)

        with LOCK:
            self.window.stop(self.packet.SequenceNumber)

    def rdt_send(self, packet):
        rawPacket = self.make_pkt(packet)
        
        try:
            with LOCK:
                UFTP_Sockets.Socket_Send(self.senderSocket,self.receiverIP,self.receiverPort,rawPacket)
        except Exception as e:
            print("Sending UDP packet to {}:{} failed!".format(self.receiverIP, self.receiverPort))
            print(str(e))

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
        alteredPacket = SenderPacketHandler.PACKET(SequenceNumber=packet.SequenceNumber, Checksum=packet.Checksum, Data=alteredData)
        return alteredPacket

    def make_pkt(self, packet):
        sequenceNumber = struct.pack('>I',packet.SequenceNumber)
        rawPacket = sequenceNumber.decode("ascii") +  packet.Data
        return rawPacket

