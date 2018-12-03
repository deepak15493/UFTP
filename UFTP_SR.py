import os
import UFTP_Sockets
from Sender import Sender
from Receiver import Receiver

def SR_Sender(sock,receiverIP,receiverPort,payload,debug):
    if debug: print("In SR_Sender")
    fileName = 'temp.txt'
    fileTemp = os.path.join(os.getcwd(),"Data","Sender")
    file = os.path.join(os.getcwd(),fileTemp,fileName)
    with open(file, "w") as f:
        f.write(payload)

    sender = Sender(sock,debug)

    try:
        sender.send(fileName,receiverIP,receiverPort)
    except Exception as e:
        print("Exception Occured: " + str(e))

def SR_Receiver(sock,debug):
    fileName = 'temp.txt'
    fileTemp = os.path.join(os.getcwd(),"Data","Receiver")
    file = os.path.join(os.getcwd(),fileTemp,fileName)
    if debug: print("SR_Receiver filename: ",file)
    buffer = ""
    
    receiver = Receiver(sock,debug)

    try:
        senderAddress = receiver.receive(fileName)
        if debug: print("filename after rcv: ",file)
        with open(file, "r") as f:
            buffer += f.read()
        if debug: print("SR_Receiver buffer: ", buffer)
        if debug: print("SR_Receiver senderAddr: ", senderAddress)
        return buffer,senderAddress
    except Exception as e:
        print("Exception Occured: " + str(e))
