import os

from ClientSender import ClientSender

class Client(object):
        fileName = 'temp.txt'
        senderIP = '127.0.0.1'
        senderPort = 8081
        receiverIP = '127.0.0.1'
        receiverPort = 8080
        sequenceNumberBits = 2
        windowSize = 2
        path = os.path.join(os.getcwd(), 'Data', 'Sender')
        timeout = 10
        maxSegmentSize = 1500
        totalPackets = "ALL"

        sender = ClientSender(senderIP, senderPort, sequenceNumberBits, windowSize, maxSegmentSize, path)

        try:
            sender.open()
            sender.send(fileName, receiverIP, receiverPort, totalPackets, timeout)
            sender.close()
        except Exception as e:
            print('Exception Occurred')
            raise  Exception
        finally:
            sender.close()
