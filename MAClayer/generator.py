import os
def randomString(n):
    return os.urandom(n)

 
result = randomString(6250)
f = open("test.in", "wb")
f.write(result)