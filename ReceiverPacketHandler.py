import random
import struct
import select
import hashlib
import UFTP_Sockets
from collections import namedtuple
from threading import Thread


class ReceiverPacketHandler(Thread):
    #PACKET = namedtuple("Packet", ["SequenceNumber", "Checksum", "Data"])
    PACKET = namedtuple("Packet", ["SequenceNumber", "Data"])
    #ACK = namedtuple("ACK", ["AckNumber", "Checksum"])
    ACK = namedtuple("ACK", ["AckNumber"])
    senderAddr = ""

    def __init__(self, fileHandle, receiverSocket, window, timeout, debug,
                 bufferSize=2048):
        Thread.__init__(self)
        self.fileHandle = fileHandle
        self.receiverSocket = receiverSocket
        # self.senderIP = senderIP
        # self.senderPort = senderPort
        # self.receiverIP = receiverIP
        # self.receiverPort = receiverPort
        self.window = window
        self.timeout = timeout
        self.bufferSize = bufferSize
        self.debug = debug

    def run(self):
        if self.debug: print("ReceiverPacketHandler is running###")
        chance = 0
        while True:
            ready = select.select([self.receiverSocket], [], [], self.timeout)
            if not ready[0]:
                if not self.window.receipt():
                    #waiting for window to receive something
                    print(".",end='',flush=True)
                    continue
                else:
                    if chance == 5:
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
                if self.debug: print("calling UFTP_Sockets.Socket_Rcv()")
                receivedPacket, senderAddress = UFTP_Sockets.Socket_Rcv(self.receiverSocket,self.bufferSize)
                self.senderAddr = senderAddress
                if self.debug: print("receivedPacket: ",receivedPacket)
                #receivedPacket, senderAddress = self.receiverSocket.recvfrom(self.bufferSize)
            except Exception as e:
                print("Receiving UDP packet failed!: ",str(e))
            receivedPacket = self.parse(receivedPacket)
            #if self.corrupt(receivedPacket):
                #print("Discarding packet with sequence number: {}".format(receivedPacket.SequenceNumber))
                #continue

            if self.window.out_of_order(receivedPacket.SequenceNumber):
                #print("Discarding packet with sequence number: {}".format(receivedPacket.SequenceNumber))
                print("Transmitting an acknowledgement with ack number: {}".format(receivedPacket.SequenceNumber))
                self.rdt_send(receivedPacket.SequenceNumber, senderAddress)
                continue

            if self.simulate_packet_loss():
                print("Simulating artificial packet loss!!")
                print("Lost a packet with sequence number: {}".format(receivedPacket.SequenceNumber))
                continue

            if self.window.exist(receivedPacket.SequenceNumber):
                print("Received duplicate packet!!")
                #print("Discarding packet with sequence number: {}".format(receivedPacket.SequenceNumber))
                continue
            else:
                print("Received packet with sequence number: {}".format(receivedPacket.SequenceNumber))
                print("Transmitting an acknowledgement with ack number: {}".format(receivedPacket.SequenceNumber))
                self.window.store(receivedPacket)
                self.rdt_send(receivedPacket.SequenceNumber, senderAddress)

            if self.window.expected(receivedPacket.SequenceNumber):
                self.deliver_packets()

    def parse(self, receivedPacket):
        if self.debug: print("ReceivedPacket: ",receivedPacket)
        sequenceNumber = struct.unpack('>I',receivedPacket[:4].encode("ascii"))[0]
        data = receivedPacket[4:]
        if self.debug: print("headerType: ", type(sequenceNumber))
        if self.debug: print("dataType: ", type(data))
        if self.debug: print("sequenceNumber: " + str(sequenceNumber) + ", data: " + data)
        #sequenceNumber = header[0:4]#struct.unpack('=I', header[0:4])[0]
        #checksum = header[4:]#struct.unpack('=H', header[4:])[0]
        #packet = ReceiverPacketHandler.PACKET(SequenceNumber=sequenceNumber, Checksum=checksum, Data=data)
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
        ack = ReceiverPacketHandler.ACK(AckNumber=ackNumber)#, Checksum=self.get_hashcode(ackNumber))
        if self.debug: print("ACK: ",ack)
        rawAck = self.make_pkt(ack)

        try:
            UFTP_Sockets.Socket_Send(self.receiverSocket,senderAddress[0],senderAddress[1],rawAck)
            #self.receiverSocket.sendto(rawAck, (senderAddress[0], senderAddress[1]))
        except Exception as e:
            print("Sending UDP packet to {}:{} failed!".format(senderAddress[0], senderAddress[1]))
            print(str(e))
        
    def get_hashcode(self, data):
        hashcode = hashlib.md5()
        hashcode.update(str(data))
        return hashcode.digest()

    def make_pkt(self, ack):
        ackNumber = struct.pack('>I', ack.AckNumber)
        #checksum = struct.pack('=16s', ack.Checksum)
        rawAck = ackNumber.decode("ascii")# + checksum
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
                if self.debug: print("inside deliver_packets()\ndata: ",packet.Data)
                print("Delivered packet with sequence number: {}".format(packet.SequenceNumber))
                try:
                    self.fileHandle.write(packet.Data.encode("utf-8"))
                except IOError as e:
                    print("Writing to file handle failed!: ",str(e))
            else:
                break
