from ctypes import *
#API for DLL C Functions
libUFTPC = CDLL("./ClientJSONStuff.dll")
libUFTPS = CDLL("./ServerJSONStuff.dll")

def Client_JTI(data_bin):
    return libUFTPC.JSONTreeInterpret(data_bin)

def Client_InitTree():
    return libUFTPC.InitDirTree()

def Client_StringAt(rqpath):
    return string_at(rqpath)

def Client_PDT():
    return libUFTPC.purgeDirTree()

def Client_Get(filename):
    return libUFTPC.getCommand(filename)

def Client_CD(path):
    return libUFTPC.changeDir(path)

def Client_LS():
    return libUFTPC.listDir()

def Client_GetPath():
    return libUFTPC.getrqpath()

def Server_FJB(data,final_tree):
    return libUFTPS.fetchJSONBuffer(data,final_tree)

def Server_JTB():
    return libUFTPS.JSONTreeBuild()

def Server_PDT():
    return libUFTPS.purgeJSONTree()

def Server_StringAt(rqpath):
    return string_at(rqpath)
