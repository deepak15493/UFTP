import sys
import traceback
import UFTP_Sockets
import sys

addressList = []
#when server starts up: Initialize JSON Tree, variables, and client states

#continuous loop for server parent thread
def UFTP_Server_Parent():
    server_IP = "127.0.0.1"
    server_Port = 5731
    sock = UFTP_Sockets.Initialize_Socket()
    UFTP_Server_Child(sock,server_IP,server_Port)
    #while(True):
        #accept socket conn
        #if new socket : get IP addr and track it
        #create server child thread and pass socket ID
        #threads must be detatched

def UFTP_Server_Child(sock,server_IP,server_Port):
    sock.bind((server_IP,server_Port))
    while(True):
        data,addr = UFTP_Sockets.Socket_Rcv(sock)
        if addr not in addressList:
            #new client
            print("new client at : ", addr)
            addressList.append(addr)
        if data.startswith("DGET",0,4):
            print(data.split("DGET ")[1])
            jsonpayload = string_at(UFTP_DLL.Server_FJB(data.split("DGET ")[1].encode("utf-8"),final_tree)).decode("utf-8")
            print(jsonpayload)
            UFTP_Sockets.Socket_Send(sock,addr[0],addr[1],jsonpayload.encode("utf-8"))            
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

if __name__ == "__main__":
    try:
        final_tree = UFTP_DLL.Server_JTB()
        sys.exit(UFTP_Server_Parent())
    except SystemExit:
        print('Quit the Thread.\n\n')
        sys.exit()
    except KeyboardInterrupt:
        Server_PDT()
        print("Purging JSON Tree and exiting")
        raise
    except Exception as e:
        print ("Error!")
        print(e) 
        print('-' * 60)
        traceback.print_exc(file=sys.stdout)
        print('-' * 60)
