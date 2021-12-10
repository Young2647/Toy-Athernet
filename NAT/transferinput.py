import os

if __name__ == "__main__" :
    if os.path.exists("origininput.txt") :
        f = open("origininput.txt")
        out = open("input.txt", "w")
        data = f.read()
        line_data = []
        i = 0
        for message in data :
            line_data.append(message)
            if len(line_data) == 39 :               
                out.write(''.join(line_data).replace('\n', ' ') + '\n')
                line_data.clear()
                i += 1
                if i == 30 :
                    break
        f.close()
        out.close()