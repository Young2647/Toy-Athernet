import os
def randomString(n):
    return os.urandom(n)

 
result = randomString(5000)
f = open("test.in", "wb")
f.write(result)