import socket
import random
import time
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

def Socket_GetIP():
    IP = socket.gethostbyname(socket.gethostname())
    print("IP: " + IP)
    return IP

def Send_File(sock,UDP_IP,UDP_PORT,filepath):
    try:
        f = open(filepath.encode("utf-8"), "rb")
        pkt = f.read()
        print("pkt = " + str(pkt))
        sock.sendto(pkt,(UDP_IP,UDP_PORT))
        # while(pkt):
        #     print("3")
        #     if(sock.sendto(pkt,(UDP_IP,UDP_PORT))):
        #         pkt = f.read(1024)
        #         time.sleep(0.01)
        #         print("In while loop")
        f.close()
        print("Sent " + filepath)
    except Exception as e:
        print(e)
