import socket
import random
def Initialize_Socket():
    return socket.socket(socket.AF_INET,socket.SOCK_DGRAM)

def Socket_Send(sock,UDP_IP,UDP_PORT,MESSAGE):
    if random.randint(1,10) < 2:
        print("Packet Dropped: ",MESSAGE)
    else:
        sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))

def Socket_Rcv(sock):
    data, addr = sock.recvfrom(1024)
    print("Received: ", data.decode("utf-8"))
