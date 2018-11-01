import sys
import traceback

#get socket info from user input
def UFTPC_Get_Socket():
    #server_IP = input("Server IP Address: ")
    #server_Port = input("Server UDP Port: ")
    #return initialize_socket(server_IP,server_Port)

#when socket has been established, use CLI to send command
def UFTPC_CLI():
    #get user input("Command: ")
    #do error checking for valid command syntax
    #return valid CLI command to send over socket
    
#continuous loop for server parent thread
def UFTPC_Send():
    while(True):
        #command = UFTPC_CLI()
        #socket_send(command)
        
def UFTPC_Receive():
    while(True):
        #wait for server response on socket

def UFTP_Client():
    #when client starts up: request server IP/UDP Port and establish socket
    #socket = UFTPC_Get_Socket():
    #make threads for send/rcv?
        #UFTPC_Send()
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
