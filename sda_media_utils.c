#include "sda_media_utils.h"
#include "utils/dr_mp3.h"
#include "SDA_OS/SDA_OS.h"

uint32_t wavplay_getDuration(uint8_t *fname) {
  svp_file fil;
  wav_header_t wavHeader;
  uint32_t count = 0;

  if(!svp_fopen_read(&fil, fname)) {
    return 0;
  }

  if(svp_fread(&fil, &wavHeader, 44)) {
    return 0;
  }

  svp_fclose(&fil);

  return (wavHeader.Subchunk2Size/4)/wavHeader.SampleRate;
}

uint32_t wavplay_getSampleRate(uint8_t *fname) {
  svp_file fil;
  wav_header_t wavHeader;
  uint32_t count = 0;

  if(!svp_fopen_read(&fil, fname)) {
    return 0;
  }

  if(svp_fread(&fil, &wavHeader, 44)) {
    return 0;
  }

  svp_fclose(&fil);

  return wavHeader.SampleRate;
}

uint32_t mp3play_getDuration(uint8_t *fname) {
  drmp3 mp3_info;
  
  if (!drmp3_init_file(&mp3_info, fname, NULL)) {
    printf("%s: Failed to open mp3 file.\n", __FUNCTION__);  
    return 0;
  }

  // mp3_info.totalPCMFrames is sometimes UINT64_t max and needs to be recalculated
  drmp3_uint64 frameCount = drmp3_get_pcm_frame_count(&mp3_info);

  uint32_t duration = (uint32_t)(frameCount/((drmp3_uint64)(mp3_info.sampleRate)));

  drmp3_uninit(&mp3_info);
  return duration;
}

uint32_t mp3play_getSampleRate(uint8_t *fname) {
  drmp3 mp3_info;
  
  if (!drmp3_init_file(&mp3_info, fname, NULL)) {
    printf("%s: Failed to open mp3 file.\n", __FUNCTION__);  
    return 0;
  }

  uint32_t rate = (uint32_t)(mp3_info.sampleRate);

  drmp3_uninit(&mp3_info);
  return rate;
}

uint32_t mp3play_getBitRate(uint8_t *fname) {
  drmp3 mp3_info;
  
  if (!drmp3_init_file(&mp3_info, fname, NULL)) {
    printf("%s: Failed to open mp3 file.\n", __FUNCTION__);  
    return 0;
  }

  uint32_t rate = (uint32_t) (((float)(svp_get_size_n(fname)-(uint32_t)mp3_info.streamStartOffset) / (float) mp3play_getDuration(fname)));

  drmp3_uninit(&mp3_info);
  return rate;
}

uint32_t sda_media_getDuration(uint8_t* fname) {
  if(sda_validate_extension(fname, "wav")){
    return wavplay_getDuration(fname);
  } else if(sda_validate_extension(fname, "mp3")) {
    return mp3play_getDuration(fname);
  } else {
    printf("%s: unknown file \"%s\".\n", __FUNCTION__, fname);
    sda_show_error_message("Unknown media type!");
  }

  return 0;
}

uint8_t sda_media_seek(uint32_t seek_s) {
  printf("%s: seek to: %us\n", __FUNCTION__, seek_s);
  return 0;
}

extern uint32_t playStart;
extern svpStatusStruct svpSGlobal;

uint32_t sda_media_getPos() {
  //printf("%s: getting pos in sim...\n", __FUNCTION__);
  if(playStart != 0) {
    return svpSGlobal.uptime - playStart;
  } else {
    return 0;
  }
}

uint32_t mediaSampleRate;

uint32_t sda_media_getSampleRate(uint8_t* fname) {
  if(sda_validate_extension(fname, "wav")){
    return wavplay_getSampleRate(fname);
  } else if(sda_validate_extension(fname, "mp3")) {
    return mp3play_getSampleRate(fname);
  } else {
    printf("%s: unknown file \"%s\".\n", __FUNCTION__, fname);
    sda_show_error_message("Unknown media type!");
  }

  return 0;
}

uint32_t sda_media_getBitRate(uint8_t* fname) {
  if(sda_validate_extension(fname, "wav")){
    return 0;
  } else if(sda_validate_extension(fname, "mp3")) {
    return mp3play_getBitRate(fname);
  } else {
    printf("%s: unknown file \"%s\".\n", __FUNCTION__, fname);
    sda_show_error_message("Unknown media type!");
  }

  return 0;
}

void sda_media_pause(uint8_t pause_on) {
  sdl_pause_wav(pause_on);
}
