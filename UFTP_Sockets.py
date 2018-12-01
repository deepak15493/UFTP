import socket
import random
def Initialize_Socket():
    return socket.socket(socket.AF_INET,socket.SOCK_DGRAM)

def Socket_Send(sock,UDP_IP,UDP_PORT,MESSAGE):
    if random.randint(1,10) < 1:
        print("\nPacket Dropped: ",MESSAGE)
    else:
        sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))

def Socket_Rcv(sock):
    data, addr = sock.recvfrom(16384)
    print("\nSocket Received:", data.decode("utf-8"))
    return(data.decode("utf-8"),addr)

def Socket_PrintIP():
    print("IP: " + socket.gethostbyname(socket.gethostname()))
