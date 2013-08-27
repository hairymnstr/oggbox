#ifndef OGG_META_H
#define OGG_META_H

#include <stdint.h>

struct page_header {
  char capture_pattern[4];
  uint8_t stream_structure_version;
  uint8_t header_type_flag;
  int64_t absolute_granule_position;
  uint32_t stream_serial_number;
  uint32_t page_sequence_no;
  uint32_t page_checksum;
  uint8_t page_segments;
} __attribute((__packed__));
  
struct identification_header {
  uint32_t vorbis_version;
  uint8_t audio_channels;
  uint32_t audio_sample_rate;
  int32_t bitrate_maximum;
  int32_t bitrate_nominal;
  int32_t bitrate_minimum;
  uint8_t blocksize;
  uint8_t framing;
} __attribute__((__packed__));

uint32_t ogg_track_length_millis(char *filename);

#endif

