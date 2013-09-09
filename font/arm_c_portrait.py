##
##    This file is part of Fontedit.
##    Copyright 2010 Nathan Dumont
##
##    Fontedit is free software: you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation, either version 3 of the License, or
##    (at your option) any later version.
##
##    Fontedit is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with Fontedit.  If not, see <http://www.gnu.org/licenses/>.
##

##
## Simple exporter for fontedit
##
## Exports to a C array of strings called font
## Each character row is represented by a byte, suitable for 
## use with monochrome LCDs that have bytes aligned horizontally.
##

## export function, name is unimportant but must be in the exporters
## dictionary below.  Must take a font object and filename as arguments.
def export(font, filename):

  # useful properties of the font object:
  # font.cols - the number of columns per character (i.e. width in px)
  # font.rows - the number of rows per character (i.e. height in px)
  # font.chars - total number of characters in the font
  # font.get_character(i) - get character number i (where 0<=i<font.chars)
  #                         returns a list of rows of integers
  #                         1 represents foreground in the editor,
  #                         0 represents background

  # prepare the strings to start and end the file, add anything you like
  preamble = """const char *font[] = {\n"""
  postamble = """};\n"""

  # open a file for writing
  fw = file(filename, "w")

  # write the pre-amble
  fw.write(preamble)

  # itterate over all characters
  for i in range(font.chars):
    fw.write("  \"")
    # fetch the array of pixels
    c = font.get_character(i)
    # itterate over the rows
    for j in range(font.rows):
      # do a shift to make a single byte bitmask of one row
      b = 0
      for k in range(font.cols-1,-1,-1):
        b = (b << 1) | c[j][k]
      # write the byte formatted as an escaped hex value for C
      fw.write("\\x%02X" % b)
    # add a handy comment at the end of the row if the char
    # is a printable ascii character
    if (i >= 32) and (i < 127):
      comm = "  /* %c */" % i
    else:
      comm = ""
    # write the end of the string and newline
    fw.write("\",%s\n" % comm)

  # finally write the closure of the C list and close the file
  fw.write(postamble)
  fw.close()
  # return zero to indicate that we were successful
  return 0

# the exporters dict is where the program finds the importer from
# needs name, desc and func parts.
exporters = {"name": "Byte-rows", 
             "desc": "Formatted for bytes-in-rows type display", 
             "func": export}

