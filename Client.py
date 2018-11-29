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
        text = "{name: \"John\", age: 395, city: \"New York\"} //basic docker commands 241  docker version 242  sudo apt install docker.io " \
               "246  sudo docker images 248  sudo docker search images  249  sudo docker search hello-world"

        ##before starting transmission read the string data and write to temp.txt file which will be transferred to reciever
        fileTemp = os.path.join(os.getcwd(), "Data", "Sender")
        file = file = os.path.join(fileTemp, fileName)
        with open(file, "wb") as f:
            f.write(text)

        sender = ClientSender(senderIP, senderPort, sequenceNumberBits, windowSize, maxSegmentSize, path)

        try:
            sender.open()
            sender.send(fileName, receiverIP, receiverPort, totalPackets, timeout)
            sender.close()
        except Exception as e:
            print('Exception Occurred')
            raise Exception
        finally:
            sender.close()
