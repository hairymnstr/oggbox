import random

class node:
  entries = []
  parent = None
  firstkey = None

def dbinsert(data, index):
  i = 0
  while (i < len(index.entries)):
    if isinstance(data, node):
      if index.entries[i][0] < data.firstkey:
        i += 1
      else:
        break
    else:
      if index.entries[i][0] < data[0]:
        i += 1
      else:
        break
  if isinstance(index.entries[i], node):
    dbinsert(data, index[i])
  else:
    if len(index.entries) < NODE_SIZE:
      index.entries = index.entries[:i] + [data] + index.entries[i:]
    else:
      # need to split and add a parent and stuff
      if index.parent == None:
        index.parent = node()
        index.parent.entries.append(index)
      index.firstkey = index.enties[0][0]
      sibling = node()
      sibling.entries = index.entries[:NODE_SIZE/2]
      sibling.firstkey = sibling.entries[0][0]
      dbinsert(sibling, index.parent)

if __name__ == "__main__":
  index = node()
  random.seed()
  for i in range(30):
    dbinsert((random.randrange(2000), random.randrange(2000)), index)

  print index.entries
