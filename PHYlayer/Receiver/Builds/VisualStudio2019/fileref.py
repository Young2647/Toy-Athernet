
file = open("record.out", "r")
outputfile = open("record_ref.out", "w")
output_str = file.read()
for i in range(len(output_str)):
    outputfile.write(output_str[i])
    if ((i + 1) % 10 == 0):
        outputfile.write(" ")
    if ((i + 1) % 100 == 0):
        outputfile.write("\n")