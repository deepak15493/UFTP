import sys
import traceback
import UFTP_Sockets
import threading
import CLI
import UFTP_DLL

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
        #return codes 0-2 reserved for Directory Commands
        #return serialized valid CLI command to send over socket
        if return_code == 0:
            #success
            pass
        if return_code == 1:
            #error, directory DNE
            break
        if return_code == 2:
            #run DGET again b/c directory is empty
            rqpath1 = "DGET " + CLI_Class.rqpath + "/"
            print(rqpath1)
            socket_info = UFTPC_Get_Socket()
            socket_info[0].bind(('',0))
            client_send_port = socket_info[0].getsockname()[1]
            UFTP_Sockets.Socket_Send(socket_info[0],socket_info[1],socket_info[2],rqpath1.encode("utf-8"))
            data,addr = UFTP_Sockets.Socket_Rcv(socket_info[0])
            print("Received : " + data + " from " + addr[0] + ":" + str(addr[1]))
            print(UFTP_DLL.Client_JTI(data.encode("utf-8")))
        if return_code == 3:
            #get command
            rqpath1 = "GET " + CLI_Class.rqpath + "/" + CLI_Class.filename
            print(rqpath1)

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
    print(UFTP_DLL.Client_JTI(data.encode("utf-8")))

def UFTPC_Receive():
    while(True):
        #wait for server response on socket
        pass

def UFTP_Client():
    #when client starts up: request server IP/UDP Port and establish socket
    socket_info = UFTPC_Get_Socket()
    print(socket_info[0])
    rqpath = UFTP_DLL.Client_InitTree()
    rqpathptr = UFTP_DLL.Client_StringAt(rqpath).decode("utf-8")
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
    except KeyboardInterrupt:
        UFTP_DLL.Client_PDT()
        print("Purging dir tree and exiting")
        raise
    except Exception as e:
        print ("Error!")
        print(e)
        print('-' * 60)
        traceback.print_exc(file=sys.stdout)
        print('-' * 60)
