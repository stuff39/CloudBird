/*****************************************************************************
 *
 *   SkyBoy GB Emulator
 *
 *   Copyright (c) 2021 Skyler "Sky" Saleh
 *
 **/

#ifndef SB_TYPES_H 
#define SB_TYPES_H 1

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#if defined(__GNUC__) || defined(__clang__)
  #define FORCE_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
  #define FORCE_INLINE __forceinline
#else
  #define FORCE_INLINE inline
#endif

// Macro for hinting that an expression is likely to be false.
#if defined(__GNUC__) || defined(__clang__)
#define SB_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define SB_UNLIKELY(x) (x)
#endif  // defined(COMPILER_GCC)
#if defined(COMPILER_GCC) || defined(__clang__)
#define SB_LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define SB_LIKELY(x) (x)
#endif  // defined(COMPILER_GCC)

#define SB_FILE_PATH_SIZE 1024
#define MAX_CARTRIDGE_SIZE 8 * 1024 * 1024
#define MAX_CARTRIDGE_RAM 128 * 1024
#define SB_U16_LO(A) ((A)&0xff)
#define SB_U16_LO_SET(A,VAL) A = (((A)&0xff00)|(((int)(VAL))&0xff))
#define SB_U16_HI(A) ((A >> 8) & 0xff)
#define SB_U16_HI_SET(A,VAL) A = (((A)&0x00ff)|((((int)(VAL))&0xff)<<8))


// Extract bits from a bitfield
#define SB_BFE(VALUE, BITOFFSET, SIZE)                                         \
  (((VALUE) >> (BITOFFSET)) & ((1llu << (SIZE)) - 1))
#define SB_BIT_TEST(VALUE,BITOFFSET) ((VALUE)&(1u<<(BITOFFSET)))
#define SB_MODE_PAUSE 0
#define SB_MODE_RESET 1
#define SB_MODE_RUN 2
#define SB_MODE_STEP 3
#define SB_MODE_REWIND 4

#define SB_LCD_W 160
#define SB_LCD_H 144
#define SB_PPU_BG_COLOR_PALETTES 64
#define SB_PPU_SPRITE_COLOR_PALETTES 64
#define SB_VRAM_BANK_SIZE 8192
#define SB_VRAM_NUM_BANKS 2

#define SB_WRAM_BANK_SIZE 4096
#define SB_WRAM_NUM_BANKS 8

#define SB_GB 0 
#define SB_GBC 1 

#define SE_BIND_KEYBOARD 0
#define SE_BIND_KEY 1
#define SE_BIND_ANALOG 2
#define SE_KEY_A 0 
#define SE_KEY_B 1 
#define SE_KEY_X 2 
#define SE_KEY_Y 3 
#define SE_KEY_UP 4
#define SE_KEY_DOWN 5
#define SE_KEY_LEFT 6
#define SE_KEY_RIGHT 7
#define SE_KEY_L 8
#define SE_KEY_R 9
#define SE_KEY_START 10
#define SE_KEY_SELECT 11
#define SE_KEY_FOLD_SCREEN 12
#define SE_KEY_PEN_DOWN 13
#define SE_KEY_EMU_PAUSE 14
#define SE_KEY_EMU_REWIND 15
#define SE_KEY_EMU_FF_2X 16
#define SE_KEY_EMU_FF_MAX 17

#define SE_NUM_KEYBINDS 18

//Should be power of 2 for perf, 8192 samples gives ~85ms maximal latency for 48kHz
#define SB_AUDIO_RING_BUFFER_SIZE (2048*4)

#define SYSTEM_UNKNOWN 0
#define SYSTEM_GB 1
#define SYSTEM_GBA 2
#define SYSTEM_NDS 3

typedef struct{
  float inputs[SE_NUM_KEYBINDS];
  float rumble; 
} sb_joy_t;
  
typedef struct{
  int16_t data[SB_AUDIO_RING_BUFFER_SIZE];
  uint32_t read_ptr;
  uint32_t write_ptr;
}sb_ring_buffer_t;
static FORCE_INLINE uint32_t sb_ring_buffer_size(sb_ring_buffer_t* buff){
  if(buff->read_ptr>SB_AUDIO_RING_BUFFER_SIZE){
    buff->write_ptr-=SB_AUDIO_RING_BUFFER_SIZE;
    buff->read_ptr-=SB_AUDIO_RING_BUFFER_SIZE;
  }
  uint32_t v = (buff->write_ptr-buff->read_ptr);
  v= v%SB_AUDIO_RING_BUFFER_SIZE;
  return v;
}
typedef struct {
  int run_mode;          // [0: Reset, 1: Pause, 2: Run, 3: Step ]
  int step_instructions; // Number of instructions to advance while stepping
  int step_frames; 
  int pc_breakpoint;     // PC to run until
  int panel_mode;
  bool rom_loaded;
  int system;            // Enum to emulated system Ex. SYSTEM_GB, SYSTEM_GBA
  sb_joy_t joy;
  sb_joy_t prev_frame_joy;  //Used for tracking button press changes in a frame 

  int frame;
  bool render_frame;
  double avg_frame_time; 
  sb_ring_buffer_t audio_ring_buff;
  float audio_channel_output[6];
  float mix_l_volume, mix_r_volume;
  float master_volume;
  int cmd_line_arg_count;
  char** cmd_line_args;
  //Temporary storage for use by cores that persists across frames but not in save states
  //or rewind buffers
  uint8_t core_temp_storage[1024*1024];
  uint32_t frames_since_rewind_push;
  char save_data_base_path[SB_FILE_PATH_SIZE];
  char save_file_path[SB_FILE_PATH_SIZE]; 
} sb_emu_state_t;
typedef struct{
  uint32_t addr;
  const char * name;
  struct{
    uint8_t start;
    uint8_t size;
    const char* name; 
  } bits[32]; 
}mmio_reg_t; 

static inline float sb_random_float(float min, float max){
  float v = rand()/(float)RAND_MAX;
  return min + v*(max-min);
}
static inline bool sb_path_has_file_ext(const char * path, const char * ext){
  int ext_len = strlen(ext);
  int path_len = strlen(path);
  if(path_len<ext_len)return false;
  for(int i=0;i<ext_len;++i){
    if(tolower(path[path_len-ext_len+i])!=tolower(ext[i]))return false;
  }
  return true;
}
static bool sb_file_exists(const char * path){
  FILE * f = fopen(path,"r");
  if(f){fclose(f);return true;}
  printf("%s does not exist\n",path);
  return false; 
}
static bool sb_load_file_data_into_buffer(const char* path, void* buffer, size_t buffer_size){
  FILE *f = fopen(path, "rb");
  if(f){
    size_t size = 0; 
    fseek(f, 0,SEEK_END);
    size = ftell(f);
    fseek(f, 0,SEEK_SET);
    if(size!=buffer_size){
      printf("%s is the wrong size. Expected: %zu got: %zu\n",path,buffer_size,size);
      return false; 
    }
    size =fread(buffer, 1, size, f);
    printf("Loaded file %s file_size %zu\n",path,size);
    fclose(f);
    return true;
  }else{
    printf("Failed to open file %s\n",path);
  }
  return false;
}
static uint8_t* sb_load_file_data(const char* path, size_t *file_size){
  FILE *f = fopen(path, "rb");
  if(file_size)*file_size = 0; 
  if(f){
    size_t size = 0; 
    fseek(f, 0,SEEK_END);
    size = ftell(f);
    fseek(f, 0,SEEK_SET);
    uint8_t *data = (uint8_t*)malloc(size);
    if(!data)return NULL;
    size =fread(data, 1, size, f);
    if(size==EOF){size = 0; free(data);} 
    if(file_size)*file_size = size;
    printf("Loaded file %s file_size %zu\n",path,*file_size);
    return data;
  }else{
    printf("Failed to open file %s\n",path);
  }
  return NULL;
}
static bool sb_save_file_data(const char* path, uint8_t* data, size_t file_size){
  FILE *f = fopen(path, "wb");
  size_t written = -1; 
  if(f){
    written = fwrite(data,1,file_size, f);
    fclose(f);
  }
  if(written!=file_size){
    printf("Error failed to save: %s (wrote: %zu out of %zu)\n",path,written,file_size);
  }else{
    printf("Saved: %s (size: %zu)\n",path,written);

  }
  return written ==file_size;
}
static void sb_free_file_data(uint8_t* data){
  if(data)free(data);
}
static void sb_breakup_path(const char* path, const char** base_path, const char** file_name, const char** ext){
  static char tmp_path[SB_FILE_PATH_SIZE];
  strncpy(tmp_path,path,SB_FILE_PATH_SIZE-1);
  tmp_path[SB_FILE_PATH_SIZE-1]='\0';
  size_t sz = strlen(tmp_path);
  *base_path = "";
  *file_name = tmp_path;
  *ext = ""; 
  // Search for end of extension or start of base path
  bool found_ext = false;
  while(sz--){
    if(tmp_path[sz]=='.'&&!found_ext){
      tmp_path[sz] = '\0';
      found_ext = true;
      *ext = tmp_path + sz+1;
      *file_name = tmp_path;
    }
    if(tmp_path[sz]=='\\'||tmp_path[sz]=='/'){
      tmp_path[sz]='\0';
      *file_name = tmp_path+sz+1; 
      *base_path = tmp_path;
      break; 
    }
  }
}
static void se_join_path(char * dest_path, int dest_size, const char * base_path, const char* file_name, const char* add_extension){
  char * seperator = base_path[0]==0? "" : "/"; 
  if(add_extension){
    const char * ext_sep = add_extension[0]=='.' ? "": ".";
    snprintf(dest_path,dest_size,"%s%s%s%s%s",base_path, seperator, file_name,ext_sep,add_extension);
  }else snprintf(dest_path,dest_size,"%s%s%s",base_path, seperator, file_name);
  dest_path[dest_size-1]=0;
}
static bool se_load_bios_file(const char* name, const char* base_path, const char* file_name, uint8_t * data, size_t data_size){
  bool loaded_bios=false;
  const char* base, *file, *ext; 
  sb_breakup_path(base_path, &base,&file, &ext);
  static char bios_path[SB_FILE_PATH_SIZE];
  se_join_path(bios_path,SB_FILE_PATH_SIZE,base,file_name,NULL);
  size_t bios_bytes=0;
  uint8_t *bios_data = sb_load_file_data(bios_path, &bios_bytes);
  if(bios_data){
    if(bios_bytes==data_size){
      printf("Loaded %s from %s\n",name, bios_path);
      memcpy(data,bios_data,data_size);
      loaded_bios=true;
    }else{
      printf("%s file at %s is incorrectly sized. Expected %zu bytes, got %zu bytes",name,file_name,data_size,bios_bytes);
    }
  }
  free(bios_data);
  return loaded_bios;
}
static FILE * se_load_log_file(const char* rom_path, const char* log_name){
  bool loaded_bios=false;
  const char* base, *file, *ext; 
  sb_breakup_path(rom_path, &base,&file, &ext);
  static char log_path[SB_FILE_PATH_SIZE];
  se_join_path(log_path,SB_FILE_PATH_SIZE,base,file,log_name);
  log_path[SB_FILE_PATH_SIZE-1]=0;
  FILE * f = fopen(log_path, "rb");
  if(f)printf("Loaded log file:%s\n",log_path);
  return f; 
}
#endif
