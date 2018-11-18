try:
    import readline
except ImportError:
    sys.stdout.write("No readline module found, no tab completion available.\n")
else:
    import rlcompleter
    readline.parse_and_bind('tab: complete')

import cmd
from ctypes import *
libUFTP = CDLL("./ClientJSONStuff.dll")

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
    def initTree(self):
        #client-side code to create initial client path object
        return libUFTP.InitDirTree()
#-------------------------------------------------------------------
    def do_greet(self, person):
        """greet [person]
        Greet the named person"""
        if person:
            print("hi,", person)
        else:
            print('hi')

    def do_cd(self, path):
        """cd [path]
        Change Directory to the path specified"""
        # path = self.checkArg(path)
        # print("path = ", path)
        print(path)
        #print(libUFTP.changeDir(path))
        return libUFTP.changeDir(path)
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
        libUFTP.listDir()

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
        libUFTP.purgeDirTree()
        # print("Exiting the CLI...")


if __name__ == '__main__':
    CLI().cmdloop()
