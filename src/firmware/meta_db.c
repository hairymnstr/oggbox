#include <meta_db.h>

int meta_db_init() {
  return 0;
}

int meta_db_get_artist_byid(int id, struct artist *result) {

}

int meta_db_get_artist_byname(char *name, struct artist *result) {

}

int meta_db_get_track_byid(int id, struct track *result) {

}

int meta_db_get_album_byname(char *name, struct album *result) {
  struct db_context context;

  context.record = 0;

  while(meta_db_next_album(result, &context)) {
    if(strcmp(temp.title, name) == 0) {
      return 1;
    }
  }
  return 0;
}

int meta_db_next_album(struct album *result, struct db_context *context) {
  if(context->record >= db_global.album_record_count) {
    return 0;
  }
  fseek(db_global.dbf, db_global.album_data_start + (context->record * sizeof(struct album)), SEEK_SET);
  if(fread(result, sizeof(struct album), 1, db_global.dbf) < sizeof(struct album)) {
    return 0;
  }
  return 1;
}

int meta_db_insert_album(struct album *record) {
  
}
