from random import randint


input = [randint(0,1) for i in range(1000)]

file = open('input.in', 'w')
for i in input:
    file.write(str(i))
file.close()