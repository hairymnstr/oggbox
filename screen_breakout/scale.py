# -*- coding: utf-8 -*-
fr = file("OSHW_COPPER_10MM.mod", "r")
fw = file("OSHW_COPPER_10MM.2.mod", "w")

d = fr.read()
fr.close()

d = d.splitlines()

for l in d:
  if l[0:2] == "Sh":
    l = l.split()
    l[3] = str(int(int(l[3]) / 4.8))
    l[4] = str(int(int(l[4]) / 4.8))
    l[6] = str(int(int(l[6]) / 4.8))
    l = " ".join(l)
  elif l[0:2] == "Po":
    l = l.split()
    l[1] = str(int(int(l[1]) / 4.8))
    l[2] = str(int(int(l[2]) / 4.8))
    l = " ".join(l)
  fw.write("%s\n" % l)

fw.close()
