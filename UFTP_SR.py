import os
import UFTP_Sockets
from Sender import Sender
from Receiver import Receiver
#selective repeat API

def SR_Sender(sock,receiverIP,receiverPort,payload,debug,parent):
    #create filepath to parent's (Server/Client) sending temp file
    fileName = 'temp.txt'
    fileTemp = os.path.join(os.getcwd(),parent,"Data","Sender")
    file = os.path.join(os.getcwd(),fileTemp,fileName)

    #write payload to temp file that will be read on a per packet basis
    with open(file, "w") as f:
        f.write(payload)

    #initialize sender object
    sender = Sender(sock,debug,parent)

    #send payload contained in temp file via selective repeat
    try:
        sender.send(fileName,receiverIP,receiverPort)
    except Exception as e:
        print("Exception Occured: " + str(e))
    #delete temp file
    os.remove(file)

def SR_Receiver(sock,debug,parent):
    #create filepath to parent's (Server/Client) receiving temp file
    fileName = 'temp.txt'
    fileTemp = os.path.join(os.getcwd(),parent,"Data","Receiver")
    file = os.path.join(os.getcwd(),fileTemp,fileName)
    buffer = ""
    #initialize receiver object
    receiver = Receiver(sock,debug,parent)

    try:
        #receive payload from selective repeat
        senderAddress = receiver.receive(fileName)
        #read payload from temp file
        with open(file, "r") as f:
            buffer += f.read()

        #delete temp file
        os.remove(file)
        return buffer,senderAddress
    except Exception as e:
        print("Exception Occured: " + str(e))
