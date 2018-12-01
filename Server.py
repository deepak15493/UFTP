import os
from Receiver import Receiver

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
        fileTemp = os.path.join(os.getcwd(), "Data", "Receiver")
        file = file = os.path.join(fileTemp, fileName)
        buffer = ""

        server = Receiver(receiverIP, receiverPort, sequenceNumberBits, windowSize, path)

        try:
            server.open()
            server.receive(fileName, senderIP, senderPort, timeout)
            server.close()
            ####after completion of transfer data is read from transferred file and converted into string
            with open(file, "rb") as f:
                buffer = f.read()
            print(buffer)
        except Exception as e:
            print('Exception Occurred')
        finally:
            server.close()
