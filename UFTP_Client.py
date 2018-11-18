import sys
import traceback
import UFTP_Sockets
import threading
import CLI

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
    CLI_Class = CLI.CLI()
    #do error checking for valid command syntax
    #return serialized valid CLI command to send over socket
    return command.encode("utf-8")
    
#continuous loop for server parent thread
def UFTPC_Send(socket_info):
    while(True):
        command = UFTPC_CLI()
        UFTP_Sockets.Socket_Send(socket_info[0],socket_info[1],socket_info[2],command)
        
def UFTPC_Receive():
    while(True):
        #wait for server response on socket
        pass

def UFTP_Client():
    #when client starts up: request server IP/UDP Port and establish socket
    socket_info = UFTPC_Get_Socket()
    #make threads for send/rcv?
    UFTPC_Send(socket_info)
    #UFTPC_Receive()
    
if __name__ == "__main__":
    try:
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
