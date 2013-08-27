import sys

fr = file("/home/nd222/Music/Daft Punk - Tron Legacy/03 - The Son Of Flynn.ogg", "rb")

d = fr.read(4)
if d != "OggS":
  print "error opening Ogg bitstream"
  sys.exit(-1)

d = fr.read(1)
if d != '\x00':
  print "unknown bitstream version"
  sys.exit(-1)

fr.read(13)
page_no = fr.read(4)

page_no = ord(page_no[0]) + ord(page_no[1]) << 8 + ord(page_no[2]) << 16 + ord(page_no[3]) << 24
print page_no

fr.seek(26)

d = fr.read(1)
print hex(ord(d))
page_segments = ord(d)

d = fr.read(page_segments)
segments = []
for c in d:
  print hex(ord(c)),
  segments.append(ord(c))
fr.seek(27 + page_segments + sum(segments))

print fr.read(4)
