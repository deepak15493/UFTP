import sys
import os
import traceback
import UFTP_Sockets
import threading
import CLI
import UFTP_DLL
import UFTP_SR
debug = 0

def UFTPC_Get_Socket():
    global debug
    #get IP of server from user input
    server_IP = input("Server IP Address: ")

    #if no IP given, use IP of this machine
    if(len(server_IP) < 1):
        server_IP = UFTP_Sockets.Socket_GetIP()
        print("Using default '" + server_IP + "'")

    #get UDP Port # from user input
    server_Port_input = input("Server UDP Port: ")

    #if no port given, use default port for this application
    if(len(server_Port_input) < 1):
        print("Using default '5731'")
        server_Port = 5731
    else:
        server_Port = int(server_Port_input)

    #return initialized socket and IP/Port info
    return (UFTP_Sockets.Initialize_Socket(debug),server_IP,server_Port)

def UFTPC_CLI(socket_info):
    print("Use standard Linux commands to navigate the file hierarchy.\nType 'help' for a list of commands.  \n")
    while(True):
        CLI_Class.cmdloop()
        return_code = CLI_Class.retVal
        #return codes 0-2 reserved for Directory Commands
        #return serialized valid CLI command to send over socket
        if debug : print("return_code = " + str(return_code))
        rqpathptr = UFTP_DLL.Client_GetPath()
        rqpath = UFTP_DLL.Client_StringAt(rqpathptr).decode("utf-8")
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
            UFTPC_Send(socket_info, rqpath1)
        if return_code == 3:
            #get command
            rqpath1 = "GET " + rqpath + "/" + CLI_Class.filename
            UFTPC_Send(socket_info, rqpath1)

        #return command.encode("utf-8")

def UFTPC_Send(socket_info,msg):
    client_send_port = socket_info[0].getsockname()[1]
    #send message to selective repeat send code
    UFTP_SR.SR_Sender(socket_info[0],socket_info[1],socket_info[2],msg,debug,"Client")
    #wait for response from server via selective repeat receive code
    data,addr = UFTP_SR.SR_Receiver(socket_info[0],debug,"Client")

    #if we asked for a file, print name of file that we received and save it
    if msg.startswith("GET",0,3):
        print("Received " + CLI_Class.filename + " from server")
        fileName = os.path.join(os.getcwd(),"Client",CLI_Class.filename)
        if os.path.exists(fileName):
            os.remove(fileName)
        with open(fileName, "w") as f:
            f.write(data)

    #print contents of response from server
    if debug : print("Received : " + data + " from " + addr[0] + ":" + str(addr[1]))

    jtiRetVal = UFTP_DLL.Client_JTI(data.encode("utf-8"))

def UFTPC_Init_Tree(socket_info,rqpathptr):
    #initialize JSON tree representation of working directory
    rqpath1 = "DGET " + rqpathptr + "/"
    UFTPC_Send(socket_info,rqpath1)

def UFTP_Client():
    #when client starts up: request server IP/UDP Port and establish socket
    socket_info = UFTPC_Get_Socket()
    socket_info[0].bind(('',0))
    rqpath = UFTP_DLL.Client_InitTree()
    rqpathptr = UFTP_DLL.Client_StringAt(rqpath).decode("utf-8")
    UFTPC_Init_Tree(socket_info,rqpathptr)
    UFTPC_CLI(socket_info)

if __name__ == "__main__":
    try:
        #grab CLI object for setting variables
        CLI_Class = CLI.CLI()
        #ask user if debugs should be enabled
        flag = input("Debug? <y/n>  ").lower()
        if(flag == 'y'):
            #if debugs are to be enabled, set appropriate flags
            debug=1
            CLI_Class.debug = 1
            UFTP_Sockets.debug = 1
        else:
            #if debugs are not to be enabled, clear appropriate flags
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
