import sys
import traceback

#when server starts up: Initialize JSON Tree, variables, and client states

#continuous loop for server parent thread
def UFTP_Server_Parent():
    while(True):
        #accept socket conn
        #if new socket : get IP addr and track it
        #create server child thread and pass socket ID
        #threads must be detatched

def UFTP_Server_Child():
    while(True):
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
        sys.exit(UFTP_Server_Parent())
    except SystemExit:
        print('Quit the Thread.\n\n')
        sys.exit() 
    except Exception as e:
        print ("Error!")
        print(e) 
        print('-' * 60)
        traceback.print_exc(file=sys.stdout)
        print('-' * 60)
