
    Welcome to PSPInt

Original Author of Jzintv

  Joseph Zbiciak

Author of the PSP port version 

  Ludovic.Jacomme also known as Zx-81 (zx81.zx81@gmail.com)

1. INTRODUCTION
   ------------

  Jzintv is an emulator of the Intellivision videogame system
  running on many systems such as Linux, Unix, Windows and MacOS.
  See http://spatula-city.org/~im14u2c/intv/ for further informations.

  PSPInt is a port on PSP of the version 20051204 of Jzintv.

  First, i would like to thanks DrNicket, for his help and support.  
  Without him, this port would never have been what it is today.

  Special thanks to Danzel and Jeff Chen for their virtual keyboard,
  and to all PSPSDK developpers.

  This package is under GPL Copyright, read COPYING file for
  more information about it.


2. INSTALLATION
   ------------

  Unzip the zip file, and copy the content of the directory fw3x
  on the psp/game or psp/game3xx directory (depending on your firmware).

  Put your rom image files on "roms" sub-directory.

  It has been developped on linux for custom Firmware 3.40-OE.

  For any comments or questions on this version, please visit 
  http://zx81.zx81.free.fr, http://www.dcemu.co.uk, or 
  http://forums.qj.net.

3. CONTROL
   ------------

  In the Intellivision emulator window

    PSP        Intellivision 
  
    Square     KP1
    Triangle   A-T 
    Cross      A-L
    Circle     A-R
    Up         D-N
    Down       D-S
    Left       D-W
    Right      D-E
    LTrigger   RESET
    RTrigger   KPE

    A-Pad      J-N/J-E/J-S/J-W
  
    Press Start  + L + R   to exit and return to eloader.
    Press Select           to enter in emulator main menu.
    Press Start            open/close the On-Screen keyboard

  In the main menu

    L+R+Start  Exit the emulator
    R Trigger  Reset the Intellivision console

    Triangle   Go Up directory
    Cross      Valid
    Circle     Valid
    Square     Go Back to the emulator window

  The On-Screen Keyboard of "Danzel" and "Jeff Chen"

    Use Analog stick to choose one of the 9 squares, and
    use Triangle, Square, Cross and Circle to choose one
    of the 4 letters of the highlighted square.


4. LOADING INTELLIVISION ROM FILES
   ------------

  If you want to load rom image in your emulator, you have to put your rom file
  (with .zip, .rom, .int, .itv or .bin file extension) on your PSP memory stick 
  in the 'rom' directory.

  Then, while inside Jzintv emulator, just press SELECT to enter in 
  the emulator main menu, and then using the file selector choose one 
  rom file to load in your emulator.

  Back to the emulator window, the rom should stard automatically.

5. LOADING KEY MAPPING FILES
   ------------

  For given games, the default keyboard mapping between PSP Keys and
  Intellivision keys, is not suitable, and the game can't be played on Jzintv.

  To overcome the issue, you can write your own mapping file. Using notepad for
  example you can edit a file with the .kbd extension and put it in the kbd 
  directory.

  For the exact syntax of those mapping files, have a look on sample files already
  presents in the kbd directory (default.kbd etc ...).

  After writting such keyboard mapping file, you can load them using the main menu
  inside the emulator.

  If the keyboard filename is the same as the rom filename (.zip etc ...)
  then when you load this rom, the corresponding keyboard file is automatically 
  loaded !

  You can now use the Keyboard menu and edit, load and save your
  keyboard mapping files inside the emulator. The Save option save the .kbd
  file in the kbd directory using the "Game Name" as filename. The game name
  is displayed on the right corner in the emulator menu.

6. COMPILATION
   ------------

  It has been developped under Linux using gcc with PSPSDK. 
  To rebuild the homebrew run the Makefile in the src archive.
