import sys
import traceback
import UFTP_Sockets
import UFTP_DLL
import time
import UFTP_SR
addressList = []
debug = 0

def UFTP_Server_Parent():
    global debug
    #ask user if debugs should be printed
    if input("Debug? y/n ").lower().startswith("y",0,1):
        debug = 1
    else:
        debug = 0
    #get IP of this machine and set default port #
    server_IP = UFTP_Sockets.Socket_GetIP()
    server_Port = 5731
    #initialize socket and start server
    sock = UFTP_Sockets.Initialize_Socket(debug)
    UFTP_Server_Child(sock,server_IP,server_Port)

def UFTP_Server_Child(sock,server_IP,server_Port):
    #listen for incoming messages from potential clients
    sock.bind((server_IP,server_Port))
    while(True):
        print("Waiting for message from client(s)")
        #wait for client to connect via selective repeat receive code
        #and save their IP and port #
        data, addr = UFTP_SR.SR_Receiver(sock,debug,"Server")
        clientIP = addr[0]
        clientPort = addr[1]
        #if new client, display to screen
        if addr not in addressList:
            print("New Client at :", addr)
            addressList.append(addr)
        #if server received directory get (ls) message
        if data.startswith("DGET",0,4):
            print("got DGET for: " + data.split("DGET ")[1] + " from " + clientIP + ":" + str(clientPort))
            #get JSON representation of working directory
            jsonpayload = UFTP_DLL.Server_StringAt(UFTP_DLL.Server_FJB(data.split("DGET ")[1].encode("utf-8"),final_tree)).decode("utf-8")
            if debug: print(jsonpayload)
            #send payload to client via selective repeat send code
            UFTP_SR.SR_Sender(sock,clientIP,clientPort,jsonpayload,debug,"Server")
        #if server received file get message
        if data.startswith("GET",0,3):
            #get absolute path for file
            fpath = data.split("GET ")[1]
            print("got GET for: " + fpath + " from " + clientIP + ":" + str(clientPort))
            #call file send API
            Send_File(sock,clientIP,clientPort,fpath)

def Send_File(sock,UDP_IP,UDP_PORT,filepath):
    try:
        #copy contents of file into buffer
        with open(filepath, "r") as f:
            pkt = f.read()
            #send file via selective repeat send code
            UFTP_SR.SR_Sender(sock,UDP_IP,UDP_PORT,pkt,debug,"Server")
            print("Sent " + filepath)
    except Exception as e:
        print(e)

if __name__ == "__main__":
    try:
        #start server initialization
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
