import struct

fr = open("albums.db", "rb")

fr.seek(0,2)  # seek end
flen = fr.tell()
fr.seek(0)    # seek set

l = struct.unpack("I", fr.read(4))
l = l[0]
print "Found %d records" % l
print "File size should be:"
print "  count      512"
print "  tracks  %d" % (l * 512) #struct.calcsize(track_struct_format))
print "  index    %d" % (4 * (l - 1))
print "           ----"
print "  total    %d" % (l * (512 + 4) - 4)
print "  actual   %d" % flen

x = []
idx = []
for i in range(l):
  fr.seek(512 * i)
  d = fr.read(64)
  d = d.strip("\x00")
  x.append(d)

fr.seek(512 * l)
for i in range(l-1):
  d = struct.unpack("I", fr.read(4))
  idx.append(d[0])

print idx 
print len(x)

n = 0
for v in idx:
  print n, x[v]
  n += 1
