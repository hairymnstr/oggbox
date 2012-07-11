import random

NODE_SIZE = 7
NODE_MID_POINTER = 4
NODE_1ST_KEY = 3
NODE_2ND_KEY = 4
NODE_PASS_KEY = 3

class Node:
  def __init__(self):
    self.pointers = []
    self.keys = []
    self.parent = None
    self.isleaf = False

dbsize = 0
head = Node()
head.isleaf = True

def dbinsert(pointer, key, node, recurse=True):
  global head
  global dbsize

  # until the first node has two entries this doesn't work right
  # so handle the boundary case where these are the first entries
  if dbsize == 0:
    node.pointers.append(pointer)
    node.keys.append(key)
  elif dbsize == 1:
    if node.keys[0] > key:
      node.pointers = [pointer] + node.pointers
    else:
      node.keys[0] = key
      node.pointers.append(pointer)
  else:
    i = 0
    while (i < len(node.keys)):
      if node.keys[i] < key:
        i += 1
      else:
        break

    if recurse and (not node.isleaf):
      dbinsert(pointer, key, node.pointers[i])
    else:
      if len(node.keys) < NODE_SIZE:
        node.pointers = node.pointers[:i+1] + [pointer] + node.pointers[i+1:]
        if isinstance(pointer, Node):
          pointer.parent = node
        node.keys = node.keys[:i] + [key] + node.keys[i:]
      else:
        sibling = Node()
        sibling.pointers = node.pointers[NODE_MID_POINTER:]
        sibling.isleaf = node.isleaf
        if not sibling.isleaf:
          for p in sibling.pointers:
            p.parent = sibling
        sibling.keys = node.keys[NODE_2ND_KEY:]

        if node.parent == None:
          node.parent = Node()
          node.parent.pointers.append(node)
          node.parent.pointers.append(sibling)
          node.parent.keys.append(node.keys[NODE_PASS_KEY])
          head = node.parent
        else:
          dbinsert(sibling, node.keys[NODE_PASS_KEY], node.parent, False)

        node.pointers = node.pointers[:NODE_MID_POINTER]
        node.keys = node.keys[:NODE_1ST_KEY]
        # now actually add the new node
        dbinsert(pointer, key, node.parent)

  dbsize += 1

if __name__ == "__main__":
  random.seed()
  for i in range(30):
    field = (random.randrange(2000), random.randrange(2000))
    print field
    dbinsert(field, field[0], head)
  fw = file("debug.html", "w")

  fw.write("<html><head><title>DB Debug</title></head><body>")
  
  jobs = [("root", head)]
  tableno = 1

  while(len(jobs)):
    fw.write("<h2>%s</h2>\n" % jobs[0][0])

    fw.write("<table>\n")
    fw.write("<tr>");
    fw.write("<td></td>")
    for i in range(NODE_SIZE):
      if(len(jobs[0][1].keys) > i):
        fw.write("<td>%s</td><td></td>" % (str(jobs[0][1].keys[i])))
      else:
        fw.write("<td></td><td></td>")

    fw.write("</tr>\n<tr>")

    for i in range(NODE_SIZE):
      if(len(jobs[0][1].pointers) > i):
        if jobs[0][1].isleaf:
          fw.write("<td>%s</td><td></td>" % (str(jobs[0][1].pointers[i])))
        else:
          fw.write("<td>Table %d</td><td></td>" % (tableno))
          jobs.append(("Table %d" % (tableno), jobs[0][1].pointers[i]))
          tableno += 1
      else:
        fw.write("<td></td><td></td>")

    fw.write("</tr>")
    fw.write("</table>")

    jobs = jobs[1:]

  fw.write("</body></html>")
  fw.close()

