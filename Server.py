import os
from ServerReceiver import ServerReceiver

class Server(object):
        fileName = 'temp.txt'
        senderIP = '127.0.0.1'
        senderPort = 8081
        receiverIP = '127.0.0.1'
        receiverPort = 8080
        sequenceNumberBits = 2
        windowSize = 2
        path = os.path.join(os.getcwd(), 'Data', 'Receiver')
        timeout = 10

        server = ServerReceiver(receiverIP, receiverPort, sequenceNumberBits, windowSize, path)

        try:
            server.open()
            server.receive(fileName,senderIP,senderPort, timeout)
            server.close()
        except Exception as e:
            print('Exception Occurred')
        finally:
            server.close()
