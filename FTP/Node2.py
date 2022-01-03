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
        if os.path.exists("output.txt") :
            with open("output.txt") as f :
                cmd, data = self.decodeMessage(f.read())
                self.sendCmd(cmd, data)
    
    def decodeMessage(self, data):
        ftp_cmd = data[0]
        if len(data) > 1:
            ftp_data = data[1:]
        else:
            ftp_data = ""
        return ftp_cmd, ftp_data

    def sendCmd(self, cmd, data):
        if cmd == CONT:
            response = self.ftp.CONT(data, 21)
        elif cmd == RETR:
            data = data.strip().split()
            if len(data) == 1:
                response = self.ftp.RETR(data[0], data[0])
                localpath = data[0]
            if len(data) == 2:
                response = self.ftp.RETR(data[0], data[1])
                localpath = data[1]    
        else:
            response = "Invalid Command."
        print(response)
        self.WritetoFile(response)

        if cmd == RETR and response[0:3] == "226": # file transfer complete
            self.NotifyCpp(localpath)
        else:
            self.NotifyCpp()
    
    def NotifyCpp(self,localpath = ""):
        with open("WRITE_DOWN.txt","w") as f:
            if (self.debug_on) :
                print("Notify the AtherNode to work.")
            if localpath is not "":
                with open("filename.txt","w") as f1:
                    f1.write(localpath)


    def WritetoFile(self, response):
        with open("response.bin", "wb") as file:
            file.write(response.encode('utf8'))
    
if __name__ == "__main__" :
    input("Print any key to start FTP client.\n")
    node2 = Node2()
    while True:
        if msvcrt.kbhit(): break
        node2.receiveFromNode1()

    