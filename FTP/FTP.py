from ftplib import FTP, all_errors

class FTPClient:
    def __init__(self):
        self.ftpclient = FTP()

    def CONT(self, host, port):
        try:
            response = self.ftpclient.connect(host, port)
            #print(self.ftpclient.getwelcome())
            return response
        except all_errors as e:
            return str(e)

    def USER(self):
        try:
            return self.ftpclient.sendcmd("USER anonymous")
        except all_errors as e:
            return str(e)
    
    def PASS(self):
        try:
            return self.ftpclient.sendcmd("PASS ")
        except all_errors as e:
            return str(e)

    def PWD(self):
        try:
            return self.ftpclient.sendcmd("PWD")
        except all_errors as e:
            return str(e)

    def CWD(self, dirpath):
        try:
            return self.ftpclient.cwd(dirpath)
        except all_errors as e:
            return str(e)
    
    def PASV(self):
        try:
            return self.ftpclient.sendcmd("PASV")
        except all_errors as e:
            return str(e)
    
    def LIST(self, dirpath):
        try:
            lines = []
            self.ftpclient.retrlines('LIST'+ dirpath, lines.append)
            return "\n".join(lines)
        except all_errors as e:
            return str(e)
    
    def RETR(self, dirpath):
        try:
            f = open(dirpath, "wb")
            response = self.ftpclient.retrbinary("RETR " + dirpath, f.write, 1024)
            f.close()
            return response
        except all_errors as e:
            return str(e)
    
    def QUIT(self):
        try :
            return self.ftpclient.quit()
        except all_errors as e:
            return str(e)

if __name__ == "__main__":
    ftp = FTPClient()
    print(ftp.CONT("ftp.sjtu.edu.cn", 21))
    print(ftp.USER())
    print(ftp.PASS())
    print(ftp.PWD())
    print(ftp.CWD("csw/"))