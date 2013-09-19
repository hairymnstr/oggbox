## script to convert a lookup table of unicode characters from a javascript thing I found on
## github into a lookup table that can be stored in the flash on the oggbox
##
## https://github.com/yvg/js-replace-diacritics/blob/master/replace-diacritics.js
##

fr = open("diacritical_marks_js.txt", "rb")

d = fr.read()

fr.close()

fw = open("unicode_table.c", "wb")

d = d.splitlines()
cs = {}
for line in d:
  k,v = line.strip().split(":")
  if k.strip("\'") == "":
    k = "\x00"
  v = v.strip("/[]ig,\\u")
  v = v.split("\\u")
  v = map(lambda x: int(x,16), v)
  
  for c in v:
    if(c > 127):
      cs[c] = k

keys = cs.keys()
keys.sort()

fw.write("#include \"unicode_table.h\"\n")
fw.write("\n")
fw.write("const int unicode_table_len = %d;\n" % len(keys))
fw.write("\n")
fw.write("const struct unicode_item unicode_table[] = {\n")
for k in keys:
  if len(cs[k]) == 1:
    fw.write("  {%d, \"\\x%02x\"},\n" % (k, ord(cs[k])))
  else:
    fw.write("  {%d, \"\\x%02x\\x%02x\"},\n" % (k, ord(cs[k][0]), ord(cs[k][1])))

print "Total characters:", len(cs)
print "Highest value:", max(cs.keys())

fw.write("};\n")
fw.close()