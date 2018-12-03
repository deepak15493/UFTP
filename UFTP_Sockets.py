import socket
import random
import time

debug = 0
def Initialize_Socket(debugp):
    global debug
    debug = debugp
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sock.setblocking(0)
    return sock

def Socket_Send(sock,UDP_IP,UDP_PORT,MESSAGE):
    if debug: print("\nSocket Sending:", MESSAGE)
    sock.sendto(MESSAGE.encode("utf-8"), (UDP_IP, UDP_PORT))

def Socket_Rcv(sock,bufferSize):
    data, addr = sock.recvfrom(bufferSize)
    data = data.decode("utf-8")
    if debug: print("\nSocket Received:", data)
    return(data,addr)

def Socket_GetIP():
    IP = socket.gethostbyname(socket.gethostname())
    print("IP: " + IP)
    return IP

