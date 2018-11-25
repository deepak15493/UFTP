try:
    import readline
except ImportError:
    sys.stdout.write("No readline module found, no tab completion available.\n")
else:
    import rlcompleter
    readline.parse_and_bind('tab: complete')

import cmd
import UFTP_DLL
import sys


class CLI(cmd.Cmd):
    prompt = "\n$ "
    intro = ""
    g_path = "~/"
    rqpath = ""
    retVal = -1
    debug = 0

#-------------------------------------------------------------------
    def do_get(self, filename):
        """get [file]
        Get the selected file"""
        if self.debug : print('Getting selected file.')
        #filepath check with .dll code libUFTP.getCommand(filename)
        ##returns rqpath (current directory)
        ##if not null, append filename to current rqpath
        ##create new buffer with new rqpath (absolute path of file)

        if(UFTP_DLL.Client_Get(filename) != NULL):
            self.retVal = 3 # GET command success
        else:
            print("File does not exist.")
            self.retVal = 1 # ERROR

    def do_cd(self, path):
        """cd [path]
        Change Directory to the path specified"""
        if len(path) < 1:
            print("No path given")
            self.retVal = 1
            return True
        retVal = UFTP_DLL.Client_CD(path.encode("utf-8"))
        if self.debug : print("changeDir returned a " + str(retVal))
        if(retVal == 1):
            print("Not a valid path.")
        self.retVal = retVal # SUCCESS; no further action
        # call JSON Function for cd
        # check return value of JSON function
        # 0 : success
        # 1 : error, directory does not exist
        # 2 : send something to server

    # def do_pwd(self, arg):
    #     """pwd
    #     Prints the path of the current Working Directory"""
    #     print("path = ", self.g_path)

    def do_ls(self,arg):
        """ls
        List the contnets of the current directory"""
        UFTP_DLL.Client_LS()
        self.retVal = 0

    # def do_retVal(self, arg):
    #     """retVal
    #     print the retVal"""
    #     print(self.retVal)
#-------------------------------------------------------------------
    def preloop(self):
        if self.debug : print("In preloop")
        self.retVal = -1
        pass
        # print("Exiting the CLI...")

    def do_close(self, arg):
        """close
        Exits the CLI program"""
        sys.exit()
        return True

    def do_quit(self, arg):
        """quit
        Exits the CLI program"""
        sys.exit()
        return True

    def do_exit(self, arg):
        """exit
        Exits the CLI program"""
        sys.exit()
        return True

    def do_EOF(self, line):
        return True

    def postcmd(self, arg1, arg2):
        if self.debug : print("In postcmd")
        # print("arg1 = " + str(arg1))
        # print("arg2 = " + str(arg2))
        return True

    def postloop(self):
        #libUFTP.purgeDirTree()
        if self.debug : print("Exiting the cmd post loop...")


if __name__ == '__main__':
    CLI().cmdloop()
