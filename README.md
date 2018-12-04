# UFTP
This application was created for NCSU ECE 573 Internet Protocols.  The purpose of creating it is to implement a client to server file download application utilizing the lightweight transport-layer protocol UDP instead of the overhead-heavy TCP.

## Environment Settings
This application is written and tested using both Python 3.6 and C.  Since the C code written was compiled into a DLL, running this application requires using a Windows OS that has Python version 3.6+ installed.

## How to run this application
This application is to be run in two or more instances: the server and one or more clients.  All instances can be run on the same physical machine or on separate ones, as long as each machine is able to connect to the internet and be assigned an IPv4 address.
 
One can clone the git repository in one of two ways:
Using the git clone command : git clone https://github.ncsu.edu/dpatil/UFTP.git
Downloading the repository as a zip and extract it

Once the git repository is downloaded on the desired machine(s), one just needs to navigate to the target directory via terminal and execute the following commands in this order:
To start the server run “python .\UFTP_Server.py”
To start the client run “python .\UFTP_Client.py”

## Application outputs
On the startup of the client side application, it prompts the user to enter the IP address and the port number of the server. The port number is always fixed for this application (5731), and the IP address of the server should be known beforehand. After entering this information, the client requests the directory tree of the server and waits till it is received successfully. After which, the client’s console input prompt ‘$’ appears and can take the user’s input for file requests, change directories and print current directories. Depending on the debug flag state, the client could also print out the payload as it is received on the socket. 

On the server’s side, the application first prompts for a debug option during its operation, after which it goes into listening mode. When client requests are received, depending on the debug flag state, the commands are printed out as they enter the socket. The transmitted data which goes into the socket is printed out as well.

In general, on the sender or receiver side, if the debugging option is enabled, information about the various packet transmissions, timeouts, window states and acknowledgement statuses can be viewed. One can observe the intentional drop of packets and retransmissions, detailing the selective repeat operation in real time. 

After requesting a file using the get command and the subsequent download of the file, it is placed in the ./Client/ folder. The procedural generation of the directory tree can be observed as well when a directory is requested after a cd command to a directory which is empty and the debug output displays the JSON tree string payload being received from the server.

## Examples
![alt text](https://raw.githubusercontent.com/dpatil/UFTP/master/Example.png)
