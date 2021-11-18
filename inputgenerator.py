from random import randint


input = [randint(0,1) for i in range(10000)]

file1 = open('input.in', 'w')
for i in input:
    file1.write(str(i))
file1.close()

file = open('input_ref.in', 'w')
cnt = 0
for i in input:
    cnt += 1
    file.write(str(i))
    if cnt % 100 == 0:
        file.write('\n')
    elif cnt % 10 == 0:
        file.write('  ')
file.close()

file = open('input_byte.in', 'w')
cnt = 0
for i in input:
    cnt += 1
    file.write(str(i))
    if cnt % 8 == 0 :
        file.write('\n')
file.close()