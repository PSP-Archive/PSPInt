/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <pspctrl.h>
#include <zlib.h>

#include "global.h"
#include "config.h"

#include "macros.h"

#include "macros.h"
#include "plat/plat.h"
#include "periph/periph.h"
#include "cp1600/cp1600.h"
#include "mem/mem.h"
#include "icart/icart.h"
#include "bincfg/bincfg.h"
#include "bincfg/legacy.h"
#include "pads/pads.h"
#include "pads/pads_cgc.h"
#include "pads/pads_intv2pc.h"
#include "gfx/gfx.h"
#include "snd/snd.h"
#include "ay8910/ay8910.h"
#include "demo/demo.h"
#include "stic/stic.h"
#include "speed/speed.h"
#include "debug/debug_.h"
#include "event/event.h"
#include "file/file.h"
#include "ivoice/ivoice.h"
#include "cfg/cfg.h"

#include <zlib.h>
#include "psp_fmgr.h"
#include "psp_danzeff.h"
  
  Intel_t INTEL;

  int psp_screenshot_mode;

static int
loc_intel_load_settings(char *chFileName)
{
  char  Buffer[512];
  char *Scan;
  unsigned int Value;
  FILE* FileDesc;

  FileDesc = fopen(chFileName, "r");
  if (FileDesc == (FILE *)0 ) return 0;

  while (fgets(Buffer,512, FileDesc) != (char *)0) {

    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';
    if (Buffer[0] == '#') continue;

    Scan = strchr(Buffer,'=');
    if (! Scan) continue;

    *Scan = '\0';
    Value = atoi(Scan+1);

    if (!strcasecmp(Buffer,"psp_cpu_clock"))       INTEL.psp_cpu_clock = Value;
    else
    if (!strcasecmp(Buffer,"psp_reverse_analog"))  INTEL.psp_reverse_analog = Value;
    else
    if (!strcasecmp(Buffer,"psp_skip_max_frame"))  INTEL.psp_skip_max_frame = Value;
    else
    if (!strcasecmp(Buffer,"intel_snd_enable"))    INTEL.intel_snd_enable = Value;
    else
    if (!strcasecmp(Buffer,"intel_render_mode"))   INTEL.intel_render_mode = Value;
    else
    if (!strcasecmp(Buffer,"intel_slow_down_max")) INTEL.intel_slow_down_max = Value;
    else
    if (!strcasecmp(Buffer,"intel_slow_down_max")) INTEL.intel_slow_down_max = Value;
    else
    if (!strcasecmp(Buffer,"psp_kbd_skin"))        psp_kbd_skin = Value;
  }

  fclose(FileDesc);

  scePowerSetClockFrequency(INTEL.psp_cpu_clock, INTEL.psp_cpu_clock, INTEL.psp_cpu_clock/2);

  return 0;
}

int
intel_load_settings()
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/set/%s.set", INTEL.intel_home_dir, INTEL.intel_save_name);
  error = loc_intel_load_settings(FileName);

  return error;
}

int
intel_load_file_settings(char *FileName)
{
  return loc_intel_load_settings(FileName);
}

void
intel_global_init(void)
{
  memset(&INTEL, 0, sizeof(Intel_t));
  getcwd(INTEL.intel_home_dir,256);

  intel_default_settings();

  intel_update_save_name("");

  intel_load_settings();

  // emulator_reset();

  scePowerSetClockFrequency(INTEL.psp_cpu_clock, INTEL.psp_cpu_clock, INTEL.psp_cpu_clock/2);
}

extern cfg_t top_intv;
void
emulator_reset(void)
{
  psp_kbd_run_command_reset();
}

void
intel_default_settings()
{
  INTEL.intel_snd_enable       = 1;
  INTEL.intel_render_mode      = INTEL_RENDER_NORMAL;
# if 0
  INTEL.skip_cur_frame      = 0;
  INTEL.skip_max_frame      = 0;
# endif
  INTEL.psp_reverse_analog  = 0;
  INTEL.psp_active_joystick = 0;
  INTEL.psp_cpu_clock       = 222;
  INTEL.psp_screenshot_id   = 0;

}

static int
loc_intel_save_settings(char *chFileName)
{
  FILE* FileDesc;
  int   error = 0;

  FileDesc = fopen(chFileName, "w");
  if (FileDesc != (FILE *)0 ) {

    fprintf(FileDesc, "psp_cpu_clock=%d\n"         , INTEL.psp_cpu_clock);
    fprintf(FileDesc, "psp_reverse_analog=%d\n"    , INTEL.psp_reverse_analog);
    fprintf(FileDesc, "psp_skip_max_frame=%d\n"    , INTEL.psp_skip_max_frame);
    fprintf(FileDesc, "intel_snd_enable=%d\n"      , INTEL.intel_snd_enable);
    fprintf(FileDesc, "intel_render_mode=%d\n"     , INTEL.intel_render_mode);
    fprintf(FileDesc, "intel_slow_down_max=%d\n"   , INTEL.intel_slow_down_max);
    fprintf(FileDesc, "psp_kbd_skin=%d\n"          , psp_kbd_skin);

    fclose(FileDesc);

  } else {
    error = 1;
  }

  return error;
}

int
intel_save_settings(void)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/set/%s.set", INTEL.intel_home_dir, INTEL.intel_save_name);
  error = loc_intel_save_settings(FileName);

  return error;
}


//Load Functions

typedef struct {
   char *pchZipFile;
   char *pchExtension;
   char *pchFileNames;
   char *pchSelection;
   int iFiles;
   unsigned int dwOffset;
} t_zip_info;

t_zip_info zip_info;

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

#define ERR_FILE_NOT_FOUND       13
#define ERR_FILE_BAD_ZIP         14
#define ERR_FILE_EMPTY_ZIP       15
#define ERR_FILE_UNZIP_FAILED    16

FILE *pfileObject;
char *pbGPBuffer = NULL;

static dword
loc_get_dword(byte *buff)
{
  return ( (((dword)buff[3]) << 24) |
           (((dword)buff[2]) << 16) |
           (((dword)buff[1]) <<  8) |
           (((dword)buff[0]) <<  0) );
}

static void
loc_set_dword(byte *buff, dword value)
{
  buff[3] = (value >> 24) & 0xff;
  buff[2] = (value >> 16) & 0xff;
  buff[1] = (value >>  8) & 0xff;
  buff[0] = (value >>  0) & 0xff;
}

static word
loc_get_word(byte *buff)
{
  return( (((word)buff[1]) <<  8) |
          (((word)buff[0]) <<  0) );
}


int 
zip_dir(t_zip_info *zi)
{
   int n, iFileCount;
   long lFilePosition;
   dword dwCentralDirPosition, dwNextEntry;
   word wCentralDirEntries, wCentralDirSize, wFilenameLength;
   byte *pbPtr;
   char *pchStrPtr;
   dword dwOffset;

   iFileCount = 0;
   if ((pfileObject = fopen(zi->pchZipFile, "rb")) == NULL) {
      return ERR_FILE_NOT_FOUND;
   }

   if (pbGPBuffer == (char *)0) {
     pbGPBuffer = (char *)malloc( sizeof(byte) * 128*1024); 
   }

   wCentralDirEntries = 0;
   wCentralDirSize = 0;
   dwCentralDirPosition = 0;
   lFilePosition = -256;
   do {
      fseek(pfileObject, lFilePosition, SEEK_END);
      if (fread(pbGPBuffer, 256, 1, pfileObject) == 0) {
         fclose(pfileObject);
         return ERR_FILE_BAD_ZIP; // exit if loading of data chunck failed
      }
      pbPtr = (byte*)(pbGPBuffer + (256 - 22)); // pointer to end of central directory (under ideal conditions)
      while (pbPtr != (byte *)pbGPBuffer) {
         if (loc_get_dword(pbPtr) == 0x06054b50) { // check for end of central directory signature
            wCentralDirEntries = loc_get_word(pbPtr + 10);
            wCentralDirSize = loc_get_word(pbPtr + 12);
            dwCentralDirPosition = loc_get_dword(pbPtr + 16);
            break;
         }
         pbPtr--; // move backwards through buffer
      }
      lFilePosition -= 256; // move backwards through ZIP file
   } while (wCentralDirEntries == 0);
   if (wCentralDirSize == 0) {
      fclose(pfileObject);
      return ERR_FILE_BAD_ZIP; // exit if no central directory was found
   }
   fseek(pfileObject, dwCentralDirPosition, SEEK_SET);
   if (fread(pbGPBuffer, wCentralDirSize, 1, pfileObject) == 0) {
      fclose(pfileObject);
      return ERR_FILE_BAD_ZIP; // exit if loading of data chunck failed
   }

   pbPtr = (byte *)pbGPBuffer;
   if (zi->pchFileNames) {
      free(zi->pchFileNames); // dealloc old string table
   }
   zi->pchFileNames = (char *)malloc(wCentralDirSize); // approximate space needed by using the central directory size
   pchStrPtr = zi->pchFileNames;

   for (n = wCentralDirEntries; n; n--) {
      wFilenameLength = loc_get_word(pbPtr + 28);
      dwOffset = loc_get_dword(pbPtr + 42);
      dwNextEntry = wFilenameLength + loc_get_word(pbPtr + 30) + loc_get_word(pbPtr + 32);
      pbPtr += 46;
      char *pchThisExtension = zi->pchExtension;
      while (*pchThisExtension != '\0') { // loop for all extensions to be checked
         if (strncasecmp((char *)pbPtr + (wFilenameLength - 4), pchThisExtension, 4) == 0) {
            strncpy(pchStrPtr, (char *)pbPtr, wFilenameLength); // copy filename from zip directory
            pchStrPtr[wFilenameLength] = 0; // zero terminate string
            pchStrPtr += wFilenameLength+1;
            loc_set_dword((byte*)pchStrPtr, dwOffset);
            pchStrPtr += 4;
            iFileCount++;
            break;
         }
         pchThisExtension += 4; // advance to next extension
      }
      pbPtr += dwNextEntry;
   }
   fclose(pfileObject);

   if (iFileCount == 0) { // no files found?
      return ERR_FILE_EMPTY_ZIP;
   }

   zi->iFiles = iFileCount;
   return 0; // operation completed successfully
}

int 
zip_extract(char *pchZipFile, char *pchFileName, dword dwOffset, char *ext)
{
   int iStatus, iCount;
   dword dwSize;
   byte *pbInputBuffer, *pbOutputBuffer;
   FILE *pfileOut, *pfileIn;
   z_stream z;

   strcpy(pchFileName, INTEL.intel_home_dir);
   strcat(pchFileName, "/unzip.");
   strcat(pchFileName, ext);

   if (!(pfileOut = fopen(pchFileName, "wb"))) {
      return ERR_FILE_UNZIP_FAILED; // couldn't create output file
   }
   if (pbGPBuffer == (char *)0) {
     pbGPBuffer = (char *)malloc( sizeof(byte) * 128*1024); 
   }
   pfileIn = fopen(pchZipFile, "rb"); // open ZIP file for reading
   fseek(pfileIn, dwOffset, SEEK_SET); // move file pointer to beginning of data block
   fread(pbGPBuffer, 30, 1, pfileIn); // read local header
   dwSize = loc_get_dword((byte *)(pbGPBuffer + 18)); // length of compressed data
   dwOffset += 30 + loc_get_word((byte *)(pbGPBuffer + 26)) + loc_get_word((byte *)(pbGPBuffer + 28));
   fseek(pfileIn, dwOffset, SEEK_SET); // move file pointer to start of compressed data

   pbInputBuffer = (byte *)pbGPBuffer; // space for compressed data chunck
   pbOutputBuffer = pbInputBuffer + 16384; // space for uncompressed data chunck
   z.zalloc = (alloc_func)0;
   z.zfree = (free_func)0;
   z.opaque = (voidpf)0;
   iStatus = inflateInit2(&z, -MAX_WBITS); // init zlib stream (no header)
   do {
      z.next_in = pbInputBuffer;
      if (dwSize > 16384) { // limit input size to max 16K or remaining bytes
         z.avail_in = 16384;
      } else {
         z.avail_in = dwSize;
      }
      z.avail_in = fread(pbInputBuffer, 1, z.avail_in, pfileIn); // load compressed data chunck from ZIP file
      while ((z.avail_in) && (iStatus == Z_OK)) { // loop until all data has been processed
         z.next_out = pbOutputBuffer;
         z.avail_out = 16384;
         iStatus = inflate(&z, Z_NO_FLUSH); // decompress data
         iCount = 16384 - z.avail_out;
         if (iCount) { // save data to file if output buffer is full
            fwrite(pbOutputBuffer, 1, iCount, pfileOut);
         }
      }
      dwSize -= 16384; // advance to next chunck
   } while ((dwSize > 0) && (iStatus == Z_OK)) ; // loop until done
   if (iStatus != Z_STREAM_END) {
      return ERR_FILE_UNZIP_FAILED; // abort on error
   }
   iStatus = inflateEnd(&z); // clean up
   fclose(pfileIn);
   fclose(pfileOut);

   return 0; // data was successfully decompressed
}

static int 
loc_load_rom(char *TmpName)
{
  int error  = cfg_load_bin(TmpName);
  return error;
}

int
intel_kbd_save(void)
{
  char TmpFileName[MAX_PATH + 1];
  snprintf(TmpFileName, MAX_PATH, "%s/kbd/%s.kbd", INTEL.intel_home_dir, INTEL.intel_save_name );
  return( psp_kbd_save_mapping(TmpFileName) );
}

void
intel_kbd_load(void)
{
  char        TmpFileName[MAX_PATH + 1];
  struct stat aStat;

  snprintf(TmpFileName, MAX_PATH, "%s/kbd/%s.kbd", INTEL.intel_home_dir, INTEL.intel_save_name );
  if (! stat(TmpFileName, &aStat)) {
    psp_kbd_load_mapping(TmpFileName);
  }
}


int
intel_load_rom(char *FileName, int zip_format)
{
  char *pchPtr;
  char *scan;
  char  SaveName[MAX_PATH+1];
  char  TmpFileName[MAX_PATH + 1];
  dword n;
  int   format;
  int   error;

  error = 1;

  if (zip_format) {

    zip_info.pchZipFile   = FileName;
    zip_info.pchExtension = ".rom.bin.int.itv";

    if (!zip_dir(&zip_info)) 
    {
      pchPtr = zip_info.pchFileNames;
      for (n = zip_info.iFiles; n != 0; n--) 
      {
        format = psp_fmgr_getExtId(pchPtr);
        if (format == FMGR_FORMAT_ROM) break;
        pchPtr += strlen(pchPtr) + 5; // skip offset
      }
      if (n) {
        strncpy(SaveName,pchPtr,MAX_PATH);
        scan = strrchr(SaveName,'.');
        if (scan) *scan = '\0';
        intel_update_save_name(SaveName);
        zip_info.dwOffset = loc_get_dword((byte *)(pchPtr + (strlen(pchPtr)+1)));
        if (!zip_extract(FileName, TmpFileName, zip_info.dwOffset, scan+1)) {
          error = loc_load_rom(TmpFileName);
          remove(TmpFileName);
        }
      }
    }

  } else {
    strncpy(SaveName,FileName,MAX_PATH);
    scan = strrchr(SaveName,'.');
    if (scan) *scan = '\0';
    intel_update_save_name(SaveName);
    error = loc_load_rom(FileName);
  }

  if (! error ) {
    emulator_reset();
    intel_kbd_load();
  }

  return error;
}

extern int fn_decode_1st();

# define CACHEABLE_SIZE_ARRAY (1 << (CP1600_MEMSIZE - CP1600_DECODE_PAGE - 5))
# define INSTR_SIZE_ARRAY     (1 << CP1600_MEMSIZE)

# if 0
  typedef struct intel_save_t {
    /* CPU */
    uint_16         cpu_r[8];
    uint_16         cpu_oldpc;
    uint_16         cpu_ext;
    uint_16         cpu_int_vec;
    int             cpu_S;
    int             cpu_C;
    int             cpu_O;
    int             cpu_Z;
    int             cpu_I;
    int             cpu_D;
    int             cpu_intr;
    req_bus_t       cpu_req_bus;

    uint_8          gfx_vid[160 * 200];
    uint_8          gfx_bbox[4 * 8];
    int             gfx_b_color;
    int             gfx_b_dirty;
    int             gfx_x_blank;
    int             gfx_y_blank;
    int             gfx_x_delay;
    int             gfx_y_delay;

    /* STIC */
    uint_32         stic_raw [0x40];
    uint_8          stic_gmem[0x140 * 8];
    int             stic_fifo_ptr;
    uint_16         stic_btab_sr [240];
    uint_16         stic_btab    [240];
    uint_32         stic_last_bg [12];
    uint_8          stic_bt_bmp  [240*8];
    uint_32         stic_mob_img [ 16*16  / 8];
    uint_16         stic_mob_bmp [8][16];
    uint_32         stic_mpl_img [192*224 / 8];
    uint_32         stic_mpl_vsb [192*224 /32];
    uint_32         stic_mpl_pri [192*224 /32];
    uint_32         stic_xbt_img [192*112 / 8];
    uint_32         stic_xbt_bmp [192*112 /32];
    uint_32         stic_image   [192*224 / 8];

    /* RAM */
    uint_16         scr_ram_image [256];
    uint_16         sys_ram_image [512];
    uint_16         sys_ram2_image[256];
    uint_16         ecs_ram_image [2048];

  } intel_save_t;

static int
loc_load_state(char *filename)
{
  intel_save_t sd;
  gzFile      *fd;
  int          index;

  memset(&sd,0, sizeof(sd));

  if ((fd = gzopen(filename, "rb")) != NULL) {
    if (gzread(fd, &sd, sizeof(sd)) <= 0) {
      gzclose(fd);
      return 1;
    }
    gzclose(fd);
  } else {
    return 1;
  }

  for (index = 0; index < 8; index++) {
    top_intv.cp1600.r[index] = sd.cpu_r[index];
  }
  top_intv.cp1600.oldpc = sd.cpu_oldpc;
  top_intv.cp1600.ext = sd.cpu_ext;
  top_intv.cp1600.int_vec = sd.cpu_int_vec;
  top_intv.cp1600.S = sd.cpu_S;
  top_intv.cp1600.C = sd.cpu_C;
  top_intv.cp1600.O = sd.cpu_O;
  top_intv.cp1600.Z = sd.cpu_Z;
  top_intv.cp1600.I = sd.cpu_I;
  top_intv.cp1600.D = sd.cpu_D;
  top_intv.cp1600.intr = sd.cpu_intr;
  top_intv.cp1600.req_bus = sd.cpu_req_bus;

  memcpy(top_intv.gfx.vid , sd.gfx_vid , 160 * 200);
  memcpy(top_intv.gfx.bbox, sd.gfx_bbox, 4 * 8);
  top_intv.gfx.dirty = 1;
  top_intv.gfx.b_color = sd.gfx_b_color;
  top_intv.gfx.b_dirty = sd.gfx_b_dirty;
  top_intv.gfx.y_blank = sd.gfx_x_blank;
  top_intv.gfx.y_delay = sd.gfx_x_delay;

  /* stic */
  memcpy(top_intv.stic.raw , sd.stic_raw , sizeof(top_intv.stic.raw));
  memcpy(top_intv.stic.gmem, sd.stic_gmem, sizeof(top_intv.stic.gmem));
  top_intv.stic.fifo_ptr = sd.stic_fifo_ptr;
  memcpy(top_intv.stic.btab_sr, sd.stic_btab_sr, sizeof(top_intv.stic.btab_sr ));
  memcpy(top_intv.stic.btab   , sd.stic_btab   , sizeof(top_intv.stic.btab    ));
  memcpy(top_intv.stic.last_bg, sd.stic_last_bg, sizeof(top_intv.stic.last_bg ));
  memcpy(top_intv.stic.bt_bmp , sd.stic_bt_bmp , sizeof(top_intv.stic.bt_bmp  ));
  memcpy(top_intv.stic.mob_img, sd.stic_mob_img, sizeof(top_intv.stic.mob_img ));
  memcpy(top_intv.stic.mob_bmp, sd.stic_mob_bmp, sizeof(top_intv.stic.mob_bmp ));
  memcpy(top_intv.stic.mpl_img, sd.stic_mpl_img, sizeof(top_intv.stic.mpl_img ));
  memcpy(top_intv.stic.mpl_vsb, sd.stic_mpl_vsb, sizeof(top_intv.stic.mpl_vsb ));
  memcpy(top_intv.stic.mpl_pri, sd.stic_mpl_pri, sizeof(top_intv.stic.mpl_pri ));
  memcpy(top_intv.stic.xbt_img, sd.stic_xbt_img, sizeof(top_intv.stic.xbt_img ));
  memcpy(top_intv.stic.xbt_bmp, sd.stic_xbt_bmp, sizeof(top_intv.stic.xbt_bmp ));
  memcpy(top_intv.stic.image  , sd.stic_image  , sizeof(top_intv.stic.image   ));

  memcpy(top_intv.scr_ram.image , sd.scr_ram_image , 256);
  memcpy(top_intv.sys_ram.image , sd.sys_ram_image , 512);
  memcpy(top_intv.sys_ram2.image, sd.sys_ram2_image, 256);
  if (top_intv.ecs_ram.image != NULL) {
    memcpy(top_intv.ecs_ram.image, sd.ecs_ram_image, 2048);
  }

  for (index = 0; index < CACHEABLE_SIZE_ARRAY; index++)
  {
    top_intv.cp1600.cacheable[index] = 0;
  }
  for (index = 0; index < INSTR_SIZE_ARRAY; index++)
  {
    top_intv.cp1600.execute[index] = fn_decode_1st;
    if (top_intv.cp1600.instr[index] != 0) {
      put_instr(top_intv.cp1600.instr[index]);
      top_intv.cp1600.instr[index]   = 0;
    }
    top_intv.cp1600.disasm[index]  = 0;
  }

  return 0;
}

static int
intel_save_state(char *filename)
{
  intel_save_t sd;
  gzFile      *fd;
  int          index;

  for (index = 0; index < 8; index++) {
    sd.cpu_r[index] = top_intv.cp1600.r[index];
  }
  /* cp1600 */
  sd.cpu_oldpc   = top_intv.cp1600.oldpc;
  sd.cpu_ext     = top_intv.cp1600.ext;
  sd.cpu_int_vec = top_intv.cp1600.int_vec;
  sd.cpu_S       = top_intv.cp1600.S;
  sd.cpu_C       = top_intv.cp1600.C;
  sd.cpu_O       = top_intv.cp1600.O;
  sd.cpu_Z       = top_intv.cp1600.Z;
  sd.cpu_I       = top_intv.cp1600.I;
  sd.cpu_D       = top_intv.cp1600.D;
  sd.cpu_intr    = top_intv.cp1600.intr;
  sd.cpu_req_bus = top_intv.cp1600.req_bus;

  /* gfx */
  memcpy(sd.gfx_vid , top_intv.gfx.vid , 160 * 200);
  memcpy(sd.gfx_bbox, top_intv.gfx.bbox, 4 * 8 );
  sd.gfx_b_color = top_intv.gfx.b_color;
  sd.gfx_b_dirty = top_intv.gfx.b_dirty;
  sd.gfx_x_blank = top_intv.gfx.y_blank;
  sd.gfx_x_delay = top_intv.gfx.y_delay;

  /* snd : no need ? */
  
  /* stic */
  memcpy(sd.stic_raw , top_intv.stic.raw , sizeof(top_intv.stic.raw));
  memcpy(sd.stic_gmem, top_intv.stic.gmem, sizeof(top_intv.stic.gmem));
# if 0
  sd.stic_fifo_ptr = top_intv.stic.fifo_ptr;
# endif
  memcpy(sd.stic_btab_sr, top_intv.stic.btab_sr, sizeof(sd.stic_btab_sr ));
  memcpy(sd.stic_btab   , top_intv.stic.btab   , sizeof(sd.stic_btab    ));
  memcpy(sd.stic_last_bg, top_intv.stic.last_bg, sizeof(sd.stic_last_bg ));
  memcpy(sd.stic_bt_bmp , top_intv.stic.bt_bmp , sizeof(sd.stic_bt_bmp  ));
  memcpy(sd.stic_mob_img, top_intv.stic.mob_img, sizeof(sd.stic_mob_img ));
  memcpy(sd.stic_mob_bmp, top_intv.stic.mob_bmp, sizeof(sd.stic_mob_bmp ));
  memcpy(sd.stic_mpl_img, top_intv.stic.mpl_img, sizeof(sd.stic_mpl_img ));
  memcpy(sd.stic_mpl_vsb, top_intv.stic.mpl_vsb, sizeof(sd.stic_mpl_vsb ));
  memcpy(sd.stic_mpl_pri, top_intv.stic.mpl_pri, sizeof(sd.stic_mpl_pri ));
  memcpy(sd.stic_xbt_img, top_intv.stic.xbt_img, sizeof(sd.stic_xbt_img ));
  memcpy(sd.stic_xbt_bmp, top_intv.stic.xbt_bmp, sizeof(sd.stic_xbt_bmp ));
  memcpy(sd.stic_image  , top_intv.stic.image  , sizeof(sd.stic_image   ));

  memcpy(sd.scr_ram_image , top_intv.scr_ram.image , 256);
  memcpy(sd.sys_ram_image , top_intv.sys_ram.image , 512);
  memcpy(sd.sys_ram2_image, top_intv.sys_ram2.image, 256);
  if (top_intv.ecs_ram.image != NULL) {
    memcpy(sd.ecs_ram_image, top_intv.ecs_ram.image, 2048);
  }

  if ((fd = gzopen(filename, "wb")) != NULL) {
    if (! gzwrite(fd, &sd, sizeof(sd))) {
      gzclose(fd);
      return 1;
    }
    gzclose(fd);

  } else {
    return 1;
  }
  return 0;
}
# endif

int
intel_update_save_name(char *Name)
{
  char        TmpFileName[MAX_PATH];
  struct stat aStat;
  int         index;
  char       *SaveName;
  char       *Scan1;
  char       *Scan2;

  SaveName = strrchr(Name,'/');
  if (SaveName != (char *)0) SaveName++;
  else                       SaveName = Name;

  if (!strncasecmp(SaveName, "sav_", 4)) {
    Scan1 = SaveName + 4;
    Scan2 = strrchr(Scan1, '_');
    if (Scan2 && (Scan2[1] >= '0') && (Scan2[1] <= '5')) {
      strncpy(INTEL.intel_save_name, Scan1, MAX_PATH);
      INTEL.intel_save_name[Scan2 - Scan1] = '\0';
    } else {
      strncpy(INTEL.intel_save_name, SaveName, MAX_PATH);
    }
  } else {
    strncpy(INTEL.intel_save_name, SaveName, MAX_PATH);
  }

  memset(INTEL.intel_save_used,0,sizeof(INTEL.intel_save_used));

  if (INTEL.intel_save_name[0] == '\0') {
    strcpy(INTEL.intel_save_name,"default");
  }

  for (index = 0; index < INTEL_MAX_SAVE_STATE; index++) {
    snprintf(TmpFileName, MAX_PATH, "%s/save/sav_%s_%d.sta", INTEL.intel_home_dir, INTEL.intel_save_name, index);
    if (! stat(TmpFileName, &aStat)) {
      INTEL.intel_save_used[index] = 1;
    }
  }

  return 0;
}

void
intel_reset_save_name()
{
  intel_update_save_name("");
}

# if 0
int
intel_snapshot_save_slot(int save_id)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  if (save_id < INTEL_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.sta", INTEL.intel_home_dir, INTEL.intel_save_name, save_id);
    error = intel_save_state(FileName);
    if (! error) INTEL.intel_save_used[save_id] = 1;
  }

  return error;
}

int
intel_snapshot_load_slot(int load_id)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  if (load_id < INTEL_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.sta", INTEL.intel_home_dir, INTEL.intel_save_name, load_id);
    error = loc_load_state(FileName);
  }
  return error;
}

int
intel_snapshot_del_slot(int save_id)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  if (save_id < INTEL_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.sta", INTEL.intel_home_dir, INTEL.intel_save_name, save_id);
    error = remove(FileName);
    if (! error) INTEL.intel_save_used[save_id] = 0;
  }

  return error;
}

# endif
