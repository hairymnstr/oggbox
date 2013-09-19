const char *font[] = {
  "\x00\x00\x00\x00\x00\x00",
  "\x3E\x55\x61\x55\x3E\x00",
  "\x3E\x6B\x5F\x6B\x3E\x00",
  "\x0C\x1E\x3C\x1E\x0C\x00",
  "\x08\x1C\x3E\x1C\x08\x00",
  "\x0C\x2E\x33\x2E\x0C\x00",
  "\x04\x2E\x37\x2E\x04\x00",
  "\x00\x18\x3C\x3C\x18\x00",
  "\xFF\xE7\xC3\xC3\xE7\xFF",
  "\x18\x24\x42\x42\x24\x18",
  "\xE7\xDB\xBD\xBD\xDB\xE7",
  "\x30\x48\x4D\x33\x07\x00",
  "\x06\x29\x79\x29\x06\x00",
  "\x60\x70\x3F\x05\x05\x00",
  "\x60\x7F\x05\x65\x7F\x00",
  "\x2A\x1C\x77\x1C\x2A\x00",
  "\x00\x7F\x3E\x1C\x08\x00",
  "\x00\x08\x1C\x3E\x7F\x00",
  "\x00\x12\x3F\x12\x00\x00",
  "\x00\x5F\x00\x00\x5F\x00",
  "\x02\x05\x3F\x01\x3F\x00",
  "\x02\x4D\x55\x59\x20\x00",
  "\x70\x70\x70\x70\x70\x00",
  "\x00\x52\x7F\x52\x00\x00",
  "\x00\x02\x3F\x02\x00\x00",
  "\x00\x10\x3F\x10\x00\x00",
  "\x08\x08\x08\x1C\x08\x00",
  "\x08\x1C\x08\x08\x08\x00",
  "\x00\x18\x10\x10\x10\x00",
  "\x08\x1C\x08\x1C\x08\x00",
  "\x30\x3C\x3F\x3C\x30\x00",
  "\x03\x0F\x3F\x0F\x03\x00",
  "\x00\x00\x00\x00\x00\x00",  /*   */
  "\x00\x00\x2E\x00\x00\x00",  /* ! */
  "\x00\x06\x00\x06\x00\x00",  /* " */
  "\x14\x3E\x14\x3E\x14\x00",  /* # */
  "\x04\x2A\x6B\x2A\x10\x00",  /* $ */
  "\x24\x16\x08\x34\x12\x00",  /* % */
  "\x14\x2A\x2A\x10\x28\x00",  /* & */
  "\x00\x00\x06\x00\x00\x00",  /* ' */
  "\x00\x0C\x12\x21\x00\x00",  /* ( */
  "\x00\x21\x12\x0C\x00\x00",  /* ) */
  "\x24\x18\x0E\x18\x24\x00",  /* * */
  "\x08\x08\x3E\x08\x08\x00",  /* + */
  "\x00\x40\x30\x00\x00\x00",  /* , */
  "\x00\x08\x08\x08\x00\x00",  /* - */
  "\x00\x00\x30\x00\x00\x00",  /* . */
  "\x20\x10\x08\x04\x02\x00",  /* / */
  "\x1E\x31\x2D\x23\x1E\x00",  /* 0 */
  "\x00\x22\x3F\x20\x00\x00",  /* 1 */
  "\x22\x31\x29\x29\x26\x00",  /* 2 */
  "\x12\x21\x25\x25\x1A\x00",  /* 3 */
  "\x18\x14\x12\x3F\x10\x00",  /* 4 */
  "\x17\x25\x25\x25\x19\x00",  /* 5 */
  "\x1E\x25\x25\x25\x18\x00",  /* 6 */
  "\x01\x01\x39\x05\x03\x00",  /* 7 */
  "\x1A\x25\x25\x25\x1A\x00",  /* 8 */
  "\x06\x29\x29\x29\x1E\x00",  /* 9 */
  "\x00\x00\x12\x00\x00\x00",  /* : */
  "\x00\x40\x32\x00\x00\x00",  /* ; */
  "\x08\x14\x14\x22\x22\x00",  /* < */
  "\x14\x14\x14\x14\x14\x00",  /* = */
  "\x22\x22\x14\x14\x08\x00",  /* > */
  "\x02\x01\x29\x05\x02\x00",  /* ? */
  "\x1E\x25\x2B\x2D\x0E\x00",  /* @ */
  "\x3C\x0A\x09\x0A\x3C\x00",  /* A */
  "\x3F\x25\x25\x25\x1A\x00",  /* B */
  "\x1E\x21\x21\x21\x12\x00",  /* C */
  "\x3F\x21\x21\x21\x1E\x00",  /* D */
  "\x3F\x25\x25\x25\x21\x00",  /* E */
  "\x3F\x05\x05\x05\x01\x00",  /* F */
  "\x1E\x21\x21\x29\x3A\x00",  /* G */
  "\x3F\x04\x04\x04\x3F\x00",  /* H */
  "\x00\x21\x3F\x21\x00\x00",  /* I */
  "\x10\x20\x20\x20\x1F\x00",  /* J */
  "\x3F\x04\x04\x0A\x31\x00",  /* K */
  "\x3F\x20\x20\x20\x20\x00",  /* L */
  "\x3F\x02\x04\x02\x3F\x00",  /* M */
  "\x3F\x02\x0C\x10\x3F\x00",  /* N */
  "\x1E\x21\x21\x21\x1E\x00",  /* O */
  "\x3F\x09\x09\x09\x06\x00",  /* P */
  "\x1E\x21\x21\x11\x2E\x00",  /* Q */
  "\x3F\x09\x09\x19\x26\x00",  /* R */
  "\x12\x25\x25\x25\x19\x00",  /* S */
  "\x01\x01\x3F\x01\x01\x00",  /* T */
  "\x1F\x20\x20\x20\x1F\x00",  /* U */
  "\x0F\x10\x20\x10\x0F\x00",  /* V */
  "\x3F\x10\x08\x10\x3F\x00",  /* W */
  "\x31\x0A\x04\x0A\x31\x00",  /* X */
  "\x03\x04\x38\x04\x03\x00",  /* Y */
  "\x31\x29\x25\x23\x21\x00",  /* Z */
  "\x00\x3F\x21\x21\x00\x00",  /* [ */
  "\x02\x04\x08\x10\x20\x00",  /* \ */
  "\x00\x21\x21\x3F\x00\x00",  /* ] */
  "\x00\x02\x01\x02\x00\x00",  /* ^ */
  "\x40\x40\x40\x40\x40\x00",  /* _ */
  "\x00\x01\x02\x00\x00\x00",  /* ` */
  "\x10\x2A\x2A\x2A\x3C\x00",  /* a */
  "\x3F\x24\x24\x24\x18\x00",  /* b */
  "\x18\x24\x24\x24\x24\x00",  /* c */
  "\x18\x24\x24\x24\x3F\x00",  /* d */
  "\x1C\x2A\x2A\x2A\x2C\x00",  /* e */
  "\x00\x3E\x09\x09\x00\x00",  /* f */
  "\x0C\x52\x52\x52\x3E\x00",  /* g */
  "\x3F\x04\x04\x04\x38\x00",  /* h */
  "\x00\x04\x3D\x00\x00\x00",  /* i */
  "\x00\x44\x3D\x00\x00\x00",  /* j */
  "\x3F\x08\x14\x22\x00\x00",  /* k */
  "\x00\x1F\x20\x00\x00\x00",  /* l */
  "\x3C\x04\x38\x04\x38\x00",  /* m */
  "\x3C\x04\x04\x38\x00\x00",  /* n */
  "\x18\x24\x24\x18\x00\x00",  /* o */
  "\x7C\x24\x24\x18\x00\x00",  /* p */
  "\x18\x24\x24\x7C\x20\x00",  /* q */
  "\x3C\x04\x04\x08\x00\x00",  /* r */
  "\x04\x2A\x2A\x2A\x10\x00",  /* s */
  "\x00\x1E\x24\x24\x10\x00",  /* t */
  "\x1C\x20\x20\x3C\x00\x00",  /* u */
  "\x0C\x10\x20\x10\x0C\x00",  /* v */
  "\x3C\x20\x10\x20\x3C\x00",  /* w */
  "\x24\x18\x18\x24\x00\x00",  /* x */
  "\x0C\x50\x50\x30\x1C\x00",  /* y */
  "\x24\x34\x2C\x24\x00\x00",  /* z */
  "\x00\x08\x36\x41\x00\x00",  /* { */
  "\x00\x00\x7F\x00\x00\x00",  /* | */
  "\x00\x41\x36\x08\x00\x00",  /* } */
  "\x08\x04\x08\x10\x08\x00",  /* ~ */
  "\x00\x00\x00\x00\x00\x00",
  "\x0E\x51\x71\x11\x0A\x00",
  "\x18\x22\x20\x3A\x00\x00",
  "\x38\x54\x56\x55\x18\x00",
  "\x20\x56\x55\x56\x78\x00",
  "\x20\x55\x54\x55\x78\x00",
  "\x20\x55\x56\x54\x78\x00",
  "\x20\x54\x57\x57\x78\x00",
  "\x08\x54\x74\x14\x00\x00",
  "\x38\x56\x55\x56\x18\x00",
  "\x38\x55\x54\x55\x18\x00",
  "\x38\x55\x56\x54\x18\x00",
  "\x00\x25\x3C\x21\x00\x00",
  "\x00\x2A\x39\x22\x00\x00",
  "\x00\x29\x3A\x20\x00\x00",
  "\x70\x29\x24\x29\x70\x00",
  "\x70\x2B\x27\x2B\x70\x00",
  "\x7C\x54\x56\x55\x44\x00",
  "\x64\x68\x10\x2C\x4C\x00",
  "\x38\x0C\x0A\x3E\x2A\x00",
  "\x18\x26\x25\x26\x18\x00",
  "\x18\x25\x24\x25\x18\x00",
  "\x18\x25\x26\x24\x18\x00",
  "\x1C\x22\x21\x22\x3C\x00",
  "\x1C\x21\x22\x20\x3C\x00",
  "\x00\x4D\x50\x51\x3C\x00",
  "\x38\x45\x44\x45\x38\x00",
  "\x3C\x41\x40\x41\x3C\x00",
  "\x1C\x22\x63\x22\x14\x00",
  "\x28\x3E\x29\x21\x22\x00",
  "\x15\x16\x3C\x16\x15\x00",
  "\x3F\x05\x3D\x55\x42\x00",
  "\x20\x48\x3E\x09\x02\x00",
  "\x20\x54\x56\x55\x78\x00",
  "\x00\x28\x3A\x21\x00\x00",
  "\x30\x48\x4A\x49\x30\x00",
  "\x38\x40\x42\x41\x78\x00",
  "\x78\x0A\x09\x0A\x71\x00",
  "\x00\x7A\x11\x22\x79\x00",
  "\x00\x12\x15\x17\x00\x00",
  "\x00\x12\x15\x12\x00\x00",
  "\x30\x48\x45\x40\x20\x00",
  "\x00\x0C\x04\x04\x04\x00",
  "\x00\x04\x04\x04\x0C\x00",
  "\x17\x08\x4C\x6A\x51\x00",
  "\x17\x28\x34\x2A\x7D\x00",
  "\x00\x00\x7D\x00\x00\x00",
  "\x08\x14\x00\x08\x14\x00",
  "\x14\x08\x00\x14\x08\x00",
  "\x4A\x94\x21\x4A\x94\x21",
  "\x55\xAA\x55\xAA\x55\xAA",
  "\xB6\x6D\xDB\xB6\x6D\xDB",
  "\xFF\xFF\x00\x00\xFF\xFF",
  "\x10\x10\xFF\x00\x00\x00",
  "\x14\x14\xFF\x00\x00\x00",
  "\x10\x10\xFF\x00\xFF\x00",
  "\x10\xF0\x10\xF0\x00\x00",
  "\x14\x14\xFC\x00\x00\x00",
  "\x14\x14\xF7\x00\xFF\x00",
  "\x00\x00\xFF\x00\xFF\x00",
  "\x14\x14\xF4\x04\xFC\x00",
  "\x14\x14\x17\x10\x1F\x00",
  "\x10\x10\x1F\x10\x1F\x00",
  "\x14\x14\x1F\x00\x00\x00",
  "\x10\x10\xF0\x00\x00\x00",
  "\x00\x00\x1F\x10\x10\x10",
  "\x10\x10\x1F\x10\x10\x10",
  "\x10\x10\xF0\x10\x10\x10",
  "\x00\x00\xFF\x10\x10\x10",
  "\x10\x10\x10\x10\x10\x10",
  "\x10\x10\xFF\x10\x10\x10",
  "\x00\x00\xFF\x14\x14\x14",
  "\x00\x00\xFF\x00\xFF\x10",
  "\x00\x00\x1F\x10\x17\x14",
  "\x00\x00\xFC\x04\xF4\x14",
  "\x14\x14\x17\x10\x17\x14",
  "\x14\x14\xF4\x04\xF4\x14",
  "\x00\x00\xFF\x00\xF7\x14",
  "\x14\x14\x14\x14\x14\x14",
  "\x14\x14\xF7\x00\xF7\x14",
  "\x14\x14\x17\x14\x14\x14",
  "\x10\x10\x1F\x10\x1F\x10",
  "\x14\x14\xF4\x14\x14\x14",
  "\x10\x10\xF0\x10\xF0\x10",
  "\x00\x00\x1F\x10\x1F\x10",
  "\x00\x00\x1F\x14\x14\x14",
  "\x00\x00\xFC\x14\x14\x14",
  "\x00\x00\xF0\x10\xF0\x10",
  "\x10\x10\xFF\x10\xFF\x10",
  "\x14\x14\xFF\x14\x14\x14",
  "\x10\x10\x1F\x00\x00\x00",
  "\x00\x00\xF0\x10\x10\x10",
  "\xFF\xFF\xFF\xFF\xFF\xFF",
  "\xF0\xF0\xF0\xF0\xF0\xF0",
  "\xFF\xFF\xFF\x00\x00\x00",
  "\x00\x00\x00\xFF\xFF\xFF",
  "\x0F\x0F\x0F\x0F\x0F\x0F",
  "\x18\x24\x24\x18\x24\x00",
  "\x7E\x01\x25\x26\x18\x00",
  "\x3F\x01\x01\x01\x03\x00",
  "\x04\x3C\x04\x1C\x24\x00",
  "\x31\x2B\x25\x21\x33\x00",
  "\x18\x24\x2C\x14\x04\x00",
  "\x7C\x10\x10\x0C\x00\x00",
  "\x08\x04\x78\x08\x04\x00",
  "\x00\x2D\x33\x2D\x00\x00",
  "\x1E\x25\x25\x25\x1E\x00",
  "\x2C\x32\x02\x32\x2C\x00",
  "\x00\x14\x2A\x2A\x10\x00",
  "\x18\x24\x18\x24\x18\x00",
  "\x18\x64\x18\x26\x18\x00",
  "\x1C\x2A\x2A\x2A\x2A\x00",
  "\x3C\x02\x02\x02\x3C\x00",
  "\x2A\x2A\x2A\x2A\x2A\x00",
  "\x44\x44\x5F\x44\x44\x00",
  "\x51\x51\x4A\x4A\x44\x00",
  "\x44\x4A\x4A\x51\x51\x00",
  "\x00\x00\xFE\x01\x02\x00",
  "\x20\x40\x3F\x00\x00\x00",
  "\x08\x08\x2A\x08\x08\x00",
  "\x14\x0A\x14\x28\x14\x00",
  "\x00\x02\x05\x02\x00\x00",
  "\x00\x00\x0C\x0C\x00\x00",
  "\x00\x00\x08\x08\x00\x00",
  "\x10\x20\x7F\x01\x01\x00",
  "\x00\x07\x01\x06\x00\x00",
  "\x00\x09\x0D\x0A\x00\x00",
  "\x00\x1C\x1C\x1C\x00\x00",
  "\x00\x00\x00\x00\x00\x00",
};