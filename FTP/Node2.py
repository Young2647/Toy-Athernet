from FTP import FTPClient
import os
import msvcrt

CONT = 0
USER = 1
PASS = 2
PWD = 3
CWD = 4
PASV = 5
LIST = 6
RETR = 7
QUIT = 8
RESP = 9

class Node2:
    def __init__(self, debug_on = False):
        self.ftp = FTPClient()
        self.connect_state = False
        self.debug_on = debug_on
    
    def receiveFromNode1(self):
        while True :
            if msvcrt.kbhit() : break
            if os.path.exists("NOTIFY_DONE.txt") :
                os.remove("NOTIFY_DONE.txt")
                break
        if os.path.exists("command.bin") :
            with open("command.bin","rb") as f :
                cmd, data = self.decodeMessage(f.read().decode('utf8'))
                self.sendCmd(cmd, data)
    
    def decodeMessage(self, data):
        data = data.split()
        ftp_cmd = (int.from_bytes(data[0].encode(), byteorder = "big", signed = False))
        if len(data) > 1:
            ftp_data = data[1:]
        else:
            ftp_data = []
            ftp_data.append("")
        return ftp_cmd, ftp_data

    def sendCmd(self, cmd, data):
        if cmd == CONT:
            response = self.ftp.CONT(data[0], 21)
            self.connect_state = True
        elif cmd == USER:
            if self.connect_state == True:
                response = self.ftp.USER(data[0])
            else:
                response = "ERR: NO FTP connection yet!"
        elif cmd == PASS:
            if self.connect_state == True:
                response = self.ftp.PASS(data[0])
            else:
                response = "ERR: NO FTP connection yet!"
        elif cmd == PWD:
            if self.connect_state == True:
                response = self.ftp.PWD()
            else:
                response = "ERR: NO FTP connection yet!"
        elif cmd == CWD:
            if self.connect_state == True:
                response = self.ftp.CWD(data[0])
            else:
                response = "ERR: NO FTP connection yet!"
        elif cmd == PASV:
            if self.connect_state == True:
                response = self.ftp.PASV()
            else:
                response = "ERR: NO FTP connection yet!"
        elif cmd == LIST:
            if self.connect_state == True:
                response = self.ftp.LIST(data[0])
            else:
                response = "ERR: NO FTP connection yet!"                                               
        elif cmd == RETR:
            if len(data) == 1:
                response = self.ftp.RETR(data[0], data[0])
                localpath = data[0]
            if len(data) == 2:
                response = self.ftp.RETR(data[0], data[1])
                localpath = data[1]    
        elif cmd == QUIT:
            if self.connect_state == True:
                response = self.ftp.QUIT()
            else:
                response = "ERR: NO FTP connection yet!" 
        else:
            response = "Invalid Command."
        print(response)
        #print(len(response))
        self.WritetoFile(response)

        if cmd == RETR and response[0:3] == "226": # file transfer complete
            self.NotifyCpp(localpath)
        else:
            self.NotifyCpp()
    
    def NotifyCpp(self,localpath = ""):
        f = open("WRITE_DOWN.txt","w")
        if (self.debug_on) :
            print("Notify the AtherNode to work.")
        f.close()
        if localpath != "":
            with open("filename.txt","w") as f1:
                f1.write(localpath)
    
    def checkNotify(self):
        if os.path.exists("NOTIFY_DONE.txt"):
            try:
                os.remove("NOTIFY_DOWN.txt")
            except:
                pass
    def WritetoFile(self, response):
        with open("response.bin", "wb") as file:
            file.write(response.encode('utf8'))
    
if __name__ == "__main__" :
    input("Print any key to start FTP client.\n")
    node2 = Node2(debug_on=True)
    while True:
        if msvcrt.kbhit(): break
        node2.checkNotify()
        node2.receiveFromNode1()

    