import sys
import traceback
import UFTP_Sockets
import UFTP_DLL
import time
import UFTP_SR
addressList = []
debug = 0
#when server starts up: Initialize JSON Tree, variables, and client states

#continuous loop for server parent thread
def UFTP_Server_Parent():
    global debug
    if input("Debug? y/n ").lower().startswith("y",0,1):
        debug = 1
    else:
        debug = 0
    server_IP = UFTP_Sockets.Socket_GetIP()
    server_Port = 5731
    sock = UFTP_Sockets.Initialize_Socket(debug)
    UFTP_Server_Child(sock,server_IP,server_Port)
    #while(True):
        #accept socket conn
        #if new socket : get IP addr and track it
        #create server child thread and pass socket ID
        #threads must be detatched

def UFTP_Server_Child(sock,server_IP,server_Port):
    sock.bind((server_IP,server_Port))
    while(True):
        print("Waiting for message from client(s)")
        #try:
            #while(True):
                #try:
                    #sock.settimeout(2)
                    #data,addr = UFTP_Sockets.Socket_Rcv(sock)
                    #clientIP = addr[0]
                    #clientPort = addr[1]
                    #call selRep receive
        data, addr = UFTP_SR.SR_Receiver(sock,debug)
        clientIP = addr[0]
        clientPort = addr[1]
        if debug: print("SR code done, got: ",data)
                #except UFTP_Sockets.socket.timeout:
                    #print(".",end='',flush=True)
                    #continue
        if addr not in addressList:
                    #new client
            print("New Client at :", addr)
            addressList.append(addr)
        if data.startswith("DGET",0,4):
            print("got DGET for:",data.split("DGET ")[1])
            jsonpayload = UFTP_DLL.Server_StringAt(UFTP_DLL.Server_FJB(data.split("DGET ")[1].encode("utf-8"),final_tree)).decode("utf-8")
            if debug: print(jsonpayload)
                    #call selRep send
            UFTP_SR.SR_Sender(sock,clientIP,clientPort,jsonpayload,debug)
        if data.startswith("GET",0,3):
            fpath = data.split("GET ")[1]
            print("got GET for:",fpath)
            Send_File(sock,clientIP,clientPort,fpath)
                #UFTP_Sockets.Socket_Send(sock,addr[0],addr[1],"here is your file".encode("utf-8"))
    #except KeyboardInterrupt:
        #raise
        #check for socket input
            #if directory exploration command
                #look up JSON tree
                #determine size of response message
                #send requested JSON tree to socket
            #else if request for file xfer
                #map JSON tree to local directory tree
                #place file into output buffer
                #use socket to send file using custom selective repeat
                #start timer
            #else if ACK for seq #
                #mark packet as received, move window
        #check for timer event
            #if timer expired send latest unACKd packet supported by SR
            #reset timer

def Send_File(sock,UDP_IP,UDP_PORT,filepath):
    try:
        with open(filepath, "r") as f:
            pkt = f.read()
            if debug: print("pkt = " + str(pkt))

            #call selRep send
            UFTP_SR.SR_Sender(sock,UDP_IP,UDP_PORT,pkt,debug)
            #sock.sendto(pkt,(UDP_IP,UDP_PORT))
        # while(pkt):
        #     print("3")
        #     if(sock.sendto(pkt,(UDP_IP,UDP_PORT))):
        #         pkt = f.read(1024)
        #         time.sleep(0.01)
        #         print("In while loop")
            print("Sent " + filepath)
    except Exception as e:
        print(e)

if __name__ == "__main__":
    try:
        final_tree = UFTP_DLL.Server_JTB()
        sys.exit(UFTP_Server_Parent())
    except SystemExit:
        print('Quit the Thread.\n\n')
        UFTP_DLL.Server_PDT()
        print("Purging JSON Tree and exiting")
        sys.exit()
    except KeyboardInterrupt:
        print("^C")
        UFTP_DLL.Server_PDT()
        print("Purging JSON Tree and exiting")
        sys.exit()
    except Exception as e:
        print ("Error!")
        print(e)
        print('-' * 60)
        traceback.print_exc(file=sys.stdout)
        print('-' * 60)
