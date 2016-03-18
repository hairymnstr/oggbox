import struct

fr = open("tracks.db", "rb")

track_struct_format = "IIIiII64s256s"

fr.seek(0,2)  # seek end
flen = fr.tell()
fr.seek(0)    # seek set

l = struct.unpack("I", fr.read(4))
l = l[0]
print "Found %d records" % l
print "File size should be:"
print "  count      4"
print "  tracks  %d" % (l * struct.calcsize(track_struct_format))
print "  index    %d" % (4 * l)
print "           ----"
print "  total    %d" % (l * (struct.calcsize(track_struct_format) + 4) + 4)
print "  actual   %d" % flen

x = []
idx = []
for i in range(l):
  d = struct.unpack(track_struct_format, fr.read(struct.calcsize(track_struct_format)))

  x.append(d)

for i in range(l):
  d = struct.unpack("I", fr.read(4))
  idx.append(d[0])
  
print idx

n = 0
for v in idx:
  print n, x[v][6].strip("\x00")
  n += 1
