import sys
import traceback
import UFTP_Sockets
import threading
import CLI
from ctypes import *

libUFTP = CDLL("./ClientJSONStuff.dll")
#get socket info from user input
def UFTPC_Get_Socket():
    server_IP = input("Server IP Address: ")
    ##check to verify correct IP format
    server_Port = int(input("Server UDP Port: "))
    ##check to verify correct port format
    return (UFTP_Sockets.Initialize_Socket(),server_IP,server_Port)

#when socket has been established, use CLI to send command
def UFTPC_CLI():
    #get user input("Command: ")
    #command = input("Command: ")
    #CLI_Class = CLI.CLI()
    while(True):
        return_code = CLI_Class.cmdloop()
        #return serialized valid CLI command to send over socket
        if return_code == 0:
            #success
            pass
        if return_code == 1:
            #error, directory DNE
            break
        if return_code == 2:
            #
            pass
        #return command.encode("utf-8")
    
#continuous loop for server parent thread
def UFTPC_Send(socket_info):
    UFTPC_CLI()
    #while(True):
        #command = UFTPC_CLI()
        #UFTP_Sockets.Socket_Send(socket_info[0],socket_info[1],socket_info[2],command)

def UFTPC_Init_Tree(socket_info,rqpath):
    rqpath1 = "DGET " + rqpath + "/"
    print(rqpath1)
    socket_info[0].bind(('',0))
    client_send_port = socket_info[0].getsockname()[1]
    UFTP_Sockets.Socket_Send(socket_info[0],socket_info[1],socket_info[2],rqpath1.encode("utf-8"))
    data,addr = UFTP_Sockets.Socket_Rcv(socket_info[0])
    print("Received : " + data + " from " + addr[0] + ":" + str(addr[1]))
    print(libUFTP.JSONTreeInterpret(data.encode("utf-8")))
    
def UFTPC_Receive():
    while(True):
        #wait for server response on socket
        pass

def UFTP_Client():
    #when client starts up: request server IP/UDP Port and establish socket
    socket_info = UFTPC_Get_Socket()
    print(socket_info[0])
    rqpath = CLI_Class.initTree()
    rqpathptr = string_at(rqpath).decode("utf-8")
    UFTPC_Init_Tree(socket_info,rqpathptr)
    #make threads for send/rcv?
    UFTPC_Send(socket_info)
    #UFTPC_Receive()
    
if __name__ == "__main__":
    try:
        CLI_Class = CLI.CLI()
        client_send_port = 0
        sys.exit(UFTP_Client())
    except SystemExit:
        print('Quit the Thread.\n\n')
        sys.exit() 
    except Exception as e:
        print ("Error!")
        print(e) 
        print('-' * 60)
        traceback.print_exc(file=sys.stdout)
        print('-' * 60)
