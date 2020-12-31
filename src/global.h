#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//LUDO:
# define INTEL_RENDER_NORMAL   0
# define INTEL_RENDER_X2       1
# define INTEL_LAST_RENDER     1

# define MAX_PATH           256
# define INTEL_MAX_SAVE_STATE 5

  typedef struct Intel_t {
 
    char intel_save_used[INTEL_MAX_SAVE_STATE];
    char intel_save_name[MAX_PATH];
    char intel_home_dir[MAX_PATH];
    int  psp_screenshot_id;
    int  psp_cpu_clock;
    int  psp_reverse_analog;
    int  psp_active_joystick;
    int  intel_snd_enable;
    int  intel_render_mode;
    int  psp_skip_max_frame;
    int  psp_skip_cur_frame;
    int  intel_slow_down_max;

  } Intel_t;

  extern Intel_t INTEL;
  
  extern int psp_screenshot_mode;

  extern void intel_global_init(void);

#endif
