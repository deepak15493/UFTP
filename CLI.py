try:
    import readline
except ImportError:
    sys.stdout.write("No readline module found, no tab completion available.\n")
else:
    import rlcompleter
    readline.parse_and_bind('tab: complete')

import cmd
import UFTP_DLL

class CLI(cmd.Cmd):
    prompt = "\n$ "
    intro = "Please select a file to download.  Use standard Linux commands to navigate the file hierarchy.\nType 'help' for a list of commands.  "
    g_path = "~/"
    rqpath = ""

    def checkArg(self, arg):
        if(" " in arg):
            print("Path must not contain spaces.")
            return ""
        return arg
#-------------------------------------------------------------------
    def do_get(self, filename):
        """get [file]
        Get the selected file"""
        print('Getting selected file.')
        #filepath check with .dll code libUFTP.getCommand(filename)
        ##returns rqpath (current directory)
        ##if not null, append filename to current rqpath
        ##create new buffer with new rqpath (absolute path of file)
        
        if(UFTP_DLL.Client_Get(filename) != NULL)
            return 3 # GET command success
        else
            print("File does not exist.")
            return 1 # ERROR

    def do_cd(self, path):
        """cd [path]
        Change Directory to the path specified"""
        # path = self.checkArg(path)
        # print("path = ", path)
        print(path)
        if len(path) < 1:
            print("No path given")
            return 1
        #print(libUFTP.changeDir(path))
        retVal = UFTP_DLL.Client_CD(path.encode("utf-8"))
        print("changeDir returned a " + str(retVal) 
        return retVal # SUCCESS; no further action
        # call JSON Function for cd
        # check return value of JSON function
        # 0 : success
        # 1 : error, directory does not exist
        # 2 : send something to server

    def do_pwd(self, arg):
        """pwd
        Prints the path of the current Working Directory"""
        print("path = ", self.g_path)

    def do_ls(self,arg):
        """ls
        List the contnets of the current directory"""
        UFTP_DLL.Client_LS()
        return 0

    def do_config(self, arg):
        """config
        Configure IP addresses"""
        clientIP = input("\tWhat is the client's IP: ")
        serverIP = input("\tWhat is the server's IP: ")
#-------------------------------------------------------------------
    def preloop(self):
        pass
        # print("Exiting the CLI...")

    def do_close(self, arg):
        """close
        Exits the CLI program"""
        return True

    def do_quit(self, arg):
        """quit
        Exits the CLI program"""
        return True

    def do_exit(self, arg):
        """exit
        Exits the CLI program"""
        return True

    def do_EOF(self, line):
        return True

    def postloop(self):
        #libUFTP.purgeDirTree()
        print("Exiting the cmd post loop...")


if __name__ == '__main__':
    CLI().cmdloop()
