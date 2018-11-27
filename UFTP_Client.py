import sys
import traceback
import UFTP_Sockets
import threading
import CLI
import UFTP_DLL
debug = 0

#get socket info from user input
def UFTPC_Get_Socket():
    server_IP = input("Server IP Address: ")
    if(len(server_IP) < 1):
        print("Using default '127.0.0.1'")
        server_IP = "127.0.0.1"
    ##check to verify correct IP format
    server_Port_input = input("Server UDP Port: ")
    if(len(server_Port_input) < 1):
        print("Using default '5731'")
        server_Port = 5731
    else:
        server_Port = int(server_Port_input)
    ##check to verify correct port format
    return (UFTP_Sockets.Initialize_Socket(),server_IP,server_Port)

#when socket has been established, use CLI to send command
def UFTPC_CLI(socket_info):
    #get user input("Command: ")
    #command = input("Command: ")
    #CLI_Class = CLI.CLI()
    print("Use standard Linux commands to navigate the file hierarchy.\nType 'help' for a list of commands.  \n")
    while(True):
        CLI_Class.cmdloop()
        return_code = CLI_Class.retVal
        #return codes 0-2 reserved for Directory Commands
        #return serialized valid CLI command to send over socket
        if debug : print("After CLI_Class.cmdloop()")
        if debug : print("return_code = " + str(return_code))
        rqpathptr = UFTP_DLL.Client_GetPath()
        rqpath = UFTP_DLL.Client_StringAt(rqpathptr).decode("utf-8")
        if debug : print("PYTHON rqpath = " + rqpath)
        if return_code == -1:
            #success
            pass
        if return_code == 0:
            #success
            pass
        if return_code == 1:
            #error, directory DNE
            pass
        if return_code == 2:
            #run DGET again b/c directory is empty
            rqpath1 = "DGET " + rqpath + "/"
            if debug : print(rqpath1)
            UFTPC_Send(socket_info, rqpath1)
        if return_code == 3:
            #get command
            rqpath1 = "GET " + rqpath + "/" + CLI_Class.filename
            if debug : print(rqpath1)
            UFTPC_Send(socket_info, rqpath1)

        #return command.encode("utf-8")

#continuous loop for server parent thread
def UFTPC_Send(socket_info,msg):
    client_send_port = socket_info[0].getsockname()[1]
    UFTP_Sockets.Socket_Send(socket_info[0],socket_info[1],socket_info[2],msg.encode("utf-8"))
    data,addr = UFTP_Sockets.Socket_Rcv(socket_info[0])
    if debug : print("Received : " + data + " from " + addr[0] + ":" + str(addr[1]))
    jtiRetVal = UFTP_DLL.Client_JTI(data.encode("utf-8"))
    if debug : print(jtiRetVal)

def UFTPC_Init_Tree(socket_info,rqpathptr):
    rqpath1 = "DGET " + rqpathptr + "/"
    if debug : print(rqpath1)
    UFTPC_Send(socket_info,rqpath1)

def UFTPC_Receive():
    while(True):
        #wait for server response on socket
        pass

def UFTP_Client():
    #when client starts up: request server IP/UDP Port and establish socket
    socket_info = UFTPC_Get_Socket()
    if debug : print(socket_info[0])
    socket_info[0].bind(('',0))
    rqpath = UFTP_DLL.Client_InitTree()
    rqpathptr = UFTP_DLL.Client_StringAt(rqpath).decode("utf-8")
    UFTPC_Init_Tree(socket_info,rqpathptr)
    #make threads for send/rcv?
    UFTPC_CLI(socket_info)
    #UFTPC_Receive()

if __name__ == "__main__":
    try:
        CLI_Class = CLI.CLI()
        flag = input("Debug? <y/n>  ").lower()
        if(flag == 'y'):
            debug=1
            CLI_Class.debug = 1
        else:
            debug=0
            CLI_Class.debug = 0
        client_send_port = 0
        sys.exit(UFTP_Client())
    except SystemExit:
        print('Quit the Thread.')
        UFTP_DLL.Client_PDT()
        print("Purging dir tree and exiting.\n\n")
        sys.exit()
    except KeyboardInterrupt:
        print("^C")
        UFTP_DLL.Client_PDT()
        print("Purging dir tree and exiting\n\n")
        sys.exit()
    except Exception as e:
        print ("Error!")
        print(e)
        print('-' * 60)
        traceback.print_exc(file=sys.stdout)
        print('-' * 60)
