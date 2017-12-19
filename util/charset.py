count = 0
line = []
for x in range(ord(' ')+1, ord('~')+1):
    line.append(chr(x))
    count += 1
    if count % 16 == 0:
        print "".join(line)
        line = []
print "".join(line)
