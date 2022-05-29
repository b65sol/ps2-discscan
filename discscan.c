#include <debug.h>  //needed for init_scr();
#include <unistd.h> //needed for sleep();
#include <stdbool.h>
#include <cdvd-ioctl.h>
#include <cdvdman.h>
#include <libcdvd-common.h>
#include <stdlib.h> 

int steps_remain = 30;
int total_steps = 0;
extern u32 initMode;

bool in_progress() {
  return total_steps < 30;
}

void reset_status_line() {
  scr_setXY(4,6);
  scr_printf("                                               ");
  scr_setXY(4,6);
}

void perform_scan() {
  reset_status_line();
  scr_printf("Waiting for drive to become ready.");
  sceCdDiskReady(0);
  reset_status_line();
  scr_printf("Drive ready.");
  u8 *TOC;
  u8 *sector_data;
  sceCdRMode mode;
  u32 lastError = SCECdErNO;
  u32 readErrors = 0;
  u32 otherErrors = 0;
  int num_sectors = 1024;
  TOC = malloc(sizeof(u8)*1024);
  sector_data = malloc(sizeof(u8)*2340*num_sectors);
  int tocok = sceCdGetToc(TOC);
  reset_status_line();
  switch( tocok ) {
    case 1:
      scr_printf("TOC Ok");
      break;
    case 0:
      scr_printf("TOC Retrieval Failed");
      return;
  }
  mode.spindlctrl = SCECdSpinNom;
  u32 currentLsn = 0;
  while ( lastError != 32 ) {
    scr_setXY(4,11);
    scr_printf("Sector: %d", currentLsn);
    sceCdDiskReady(0);
    int cmdok = sceCdRead(currentLsn, num_sectors, sector_data, &mode);
    scr_setXY(4,7);
    sceCdSync(0);
    lastError = sceCdGetError();
    scr_printf("%d Sample: %02X %02X %02X %02X %02X %02X %02X %02X", cmdok, sector_data[0], sector_data[1], sector_data[2], sector_data[3],
      sector_data[4], sector_data[5], sector_data[6], sector_data[7]);
    scr_setXY(4,8);
    currentLsn += num_sectors;
    scr_printf("Last Error: %02d", lastError);
    scr_setXY(4,9);
    if ( lastError == SCECdErREAD ) {
      readErrors++;
    } else if ( lastError != SCECdErNO && lastError != 32 ) {
      otherErrors++;
    }
    scr_printf("Read Errors: %d", readErrors);
    scr_setXY(4,10);
    scr_printf("Other Errors: %d", otherErrors);

  }
  scr_setXY(4,8);
  scr_printf("Last Error: END OF MEDIA");


  //Actually do scan here.
  //Figure out size of disc.
  //Read disc in loop
  //Capture and report non-recoverable errors
  //Error codes: https://ps2dev.github.io/ps2sdk/libcdvd-common_8h.html#acc94c03027ecdedf87578dbedc2bf350
  //Get errors: sceCdGetError
  //Sector stream Read: sceCdStRead
  //Sector stream init: sceCdStInit
}

int main()         //int main() is automatically called/started with all programs
{
  enum SCECdvdInitMode mode;
  int disktype;
  char *message;
  int supported = true;
  mode = SCECdINIT;
  init_scr();         //Initialize Screen
  sleep(1);                       //PS2 is old. Give it a second...
  scr_setCursor(0);
  scr_setbgcolor(0x90993333);
  scr_clear();
  scr_setXY(1,1);
  scr_printf("Native Hardware Disc Scan");
  int result = sceCdInit(mode);
  scr_setXY(2,3);
  if ( result == 0 ) {
    scr_printf("Unable to init CDVD                      ");
  } else {
    scr_printf("CDVD Init                                ");
  }
  scr_setXY(2,5);
  disktype = sceCdGetDiskType();
  switch ( disktype ) {
    case SCECdPSCD:
    case SCECdPSCDDA:
      message = "Media Type: PlayStation CD";
      //sceCdMmode(disktype);
      break;
    case SCECdPS2CD:
    case SCECdPS2CDDA:
      message = "Media Type: PlayStation2 CD";
      //sceCdMmode(disktype);
      break;
    case SCECdPS2DVD:
      message = "Media Type: PlayStation2 DVD";
      //sceCdMmode(disktype);
      break;
    default:
      message = "Media Type: Non-supported disc or no disc!";
      supported = false;
  }
  scr_printf(message);

  if ( supported ) {
    perform_scan();
  }

  scr_setXY(4,12);
  scr_printf("Done! Sleeping for 2 hours. Restart console when ready.\n"); //print our string
  sleep(7200);         //Wait here for 30 seconds instead of exiting (ret 0) with sleep();
  return 0;         //This exits and returns to system menu
}