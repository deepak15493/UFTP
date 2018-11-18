try:
    import readline
except ImportError:
    sys.stdout.write("No readline module found, no tab completion available.\n")
else:
    import rlcompleter
    readline.parse_and_bind('tab: complete')

import cmd
from ctypes import *
libUFTP = CDLL("ClientJSONStuff.dll")


class CLI(cmd.Cmd):
    prompt = "\n$ "
    intro = "Please select a file to download.  Use standard Linux commands to navigate the file hierarchy.\nType 'help' for a list of commands.  "
    g_path = "~/"

    def checkArg(self, arg):
        if(" " in arg):
            print("Path must not contain spaces.")
            return ""
        return arg
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
        print(libUFTP.changeDir(path))

    def do_pwd(self, arg):
        """pwd
        Prints the path of the current Working Directory"""
        print("path = ", self.g_path)

    def do_ls(self):
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
        pass
        # print("Exiting the CLI...")


if __name__ == '__main__':
    CLI().cmdloop()
