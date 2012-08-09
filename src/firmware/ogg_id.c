#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ogg_meta.h"

uint32_t ogg_track_length_millis(char *filename) {
  uint8_t segments;
  int32_t audio_sample_rate;
  int64_t granule;
  float samples_per_milli;
  uint32_t millis;
  char buf[4];
  FILE *fr;
  if(!(fr = fopen(filename, "rb"))) {
    return -1;
  }
  fseek(fr, 26, SEEK_SET);
  
  fread(&segments, 1, 1, fr);
  fseek(fr, segments + 7 + 5, SEEK_CUR);
  
  fread(&audio_sample_rate, 4, 1, fr);
  
  fseek(fr, -4, SEEK_END);
  fread(buf, 1, 4, fr);
  fseek(fr, -5, SEEK_END);
  while(strncmp("OggS", buf, 4) != 0) {
    buf[3] = buf[2];
    buf[2] = buf[1];
    buf[1] = buf[0];
    fread(&buf[0], 1, 1, fr);
      
    if(ftell(fr) > 1) {
      fseek(fr, -2, SEEK_CUR);
    } else {
      return -2;
    }
  }
  
  fseek(fr, 6 + 1, SEEK_CUR); /* we want the 6th byte on, looking 1 byte before now */
  
  fread(&granule, 8, 1, fr);
  
  samples_per_milli = audio_sample_rate / 1000.0;
  
  millis = granule / samples_per_milli;
  fclose(fr);
  return millis;
}
  
//int main(int argc, char *argv[]) {
//  printf("Track is %d milliseconds long\n", ogg_track_length_millis(argv[1]));
//   uint8_t segment_table, page_segments;
//   int i, page_len;
//   struct identification_header id;
//   
//   FILE *fr = fopen(argv[1], "rb");
//   struct page_header ph;
//   
//   fread(&ph, sizeof(struct page_header), 1, fr);
//   
//   printf("Capture pattern: %c%c%c%c\n", ph.capture_pattern[0], ph.capture_pattern[1], ph.capture_pattern[2], ph.capture_pattern[3]);
//   printf("Stream structure version: %u\n", ph.stream_structure_version);
//   printf("Header type flag: %x\n", ph.header_type_flag);
//   printf("Absolute granule position: %ld\n", ph.absolute_granule_position);
//   printf("Stream serial number: %d\n", ph.stream_serial_number);
//   printf("Page Sequence No: %d\n", ph.page_sequence_no);
//   printf("Page checksum: %x\n", ph.page_checksum);
//   printf("Page Segments: %u\n", ph.page_segments);
//   
//   for(i=0;i<ph.page_segments;i++) {
//     fread(&segment_table, 1, 1, fr);
//     page_len += segment_table;
//   }
// 
//   fseek(fr, 7, SEEK_CUR);
// 
//   fread(&id, sizeof(struct identification_header), 1, fr);
// 
//   printf("Vorbis Version: %u\n", id.vorbis_version);
//   printf("Audio Channels: %u\n", id.audio_channels);
//   printf("Audio Sample Rate: %u\n", id.audio_sample_rate);
//   printf("Bitrate Maximum: %d\n", id.bitrate_maximum);
//   printf("Bitrate Nominal: %d\n", id.bitrate_nominal);
//   printf("Bitrate Minimum: %d\n", id.bitrate_minimum);
//   printf("Blocksize: %x\n", id.blocksize);
//   
//   int blocksize_0 = (1 << (id.blocksize & 0xf));
//   printf("blocksize_0: %d\n", blocksize_0);
//   fseek(fr, -4, SEEK_END);
//   
//   char buf[4];
//   fread(buf, 1, 4, fr);
//   fseek(fr, -5, SEEK_END);
//   while(strncmp("OggS", buf, 4) != 0) {
//     buf[3] = buf[2];
//     buf[2] = buf[1];
//     buf[1] = buf[0];
//     fread(&buf[0], 1, 1, fr);
//       
//     if(ftell(fr) > 1) {
//       fseek(fr, -2, SEEK_CUR);
//     } else {
//       printf("Error, didn't find an end of file packet to check length\n");
//       exit(1);
//     }
//   }
//   
//   
// //   fseek(fr, -4, SEEK_CUR);
//   fseek(fr, 1, SEEK_CUR);
//   fread(&ph, sizeof(struct page_header), 1, fr);
//   printf("Capture pattern: %c%c%c%c\n", ph.capture_pattern[0], ph.capture_pattern[1], ph.capture_pattern[2], ph.capture_pattern[3]);
//   printf("Stream structure version: %u\n", ph.stream_structure_version);
//   printf("Header type flag: %x\n", ph.header_type_flag);
//   printf("Absolute granule position: %ld\n", ph.absolute_granule_position);
//   printf("Stream serial number: %d\n", ph.stream_serial_number);
//   printf("Page Sequence No: %d\n", ph.page_sequence_no);
//   printf("Page checksum: %x\n", ph.page_checksum);
//   printf("Page Segments: %u\n", ph.page_segments);
//   printf("Last granule position %ld\n", ph.absolute_granule_position);
//   printf("File length %lf\n", (double)ph.absolute_granule_position / (double)id.audio_sample_rate);
//   
//  exit(0);
//}
