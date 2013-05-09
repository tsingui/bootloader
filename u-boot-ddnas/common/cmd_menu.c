/*
 *      U-boot DownLoad menu 
 *      by Lintel(lintel.huang@gmail.com)
 *
 *      2011.09.02
 */
#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/mach-types.h>

#define bBootFrmNORFlash 1

extern char console_buffer[];
extern int readline (const char *const prompt);

#if defined(CONFIG_DDNAS)
#include<ddnas_board.h>
#endif
//lintel add
int chk_image(void)
{
  
  printf("Start image header check...");
  if (*(unsigned long *)0x800000 == U_BOOT_KWB_IMAGE_HEARD)
  {
    printf("U-boot kirkwood boot image.\n");
    return 1;
  }
  else if(*(unsigned long *)0x800000 == FIRMWARE_IMAGE_HEARD)
  {
    printf("Firmware image.\n");
    return 0;
  }
  else
  {
  printf("Unknow image header:0x%x.\n",*(unsigned long *)0x800000);
  return -1;
  }
}


static char awaitkey(unsigned long delay, int* error_p)
{
    int i;
    if (delay == -1)
    {
        while (1)
        {
            if (tstc()) /* we got a key press */
                return getc();
        }
    }
    else
    {
        for (i = 0; i < delay; i++)
        {
            if (tstc()) /* we got a key press */
                return getc();
            udelay (10*1000);
        }
    }
    if (error_p)
        *error_p = -1;

    return 0;
}


int download(int modes)
{
      char cmd_buf[256];
    switch(modes)
    {
      case 0:
	     if (getenv("ethact")==NULL)
	     {
	       printf("\nSorry,you have no a ethernet device... \n");
	       return -1;
	       break;
	     }
	      printf("\nLoad file from tftp server ... \n");
	      setenv("filesize","");
	      strcpy(cmd_buf, "tftpboot  0x800000  ");
	      printf("Please input filename from tftp-server:");
	      memset(console_buffer,NULL,CONFIG_SYS_CBSIZE);
	      readline(NULL);
	      strcat(cmd_buf, console_buffer);
	      run_command(cmd_buf, 0);
	      if (simple_strtoul(getenv ("filesize"), NULL, 16)<16)
	      return -1;
	      break;
      case 1:
	      printf("\nLoad file from serial with xmodem ... \n");
	      setenv("filesize","");
	      strcpy(cmd_buf, "loadx  0x800000");
	      run_command(cmd_buf, 0);
	      if (simple_strtoul(getenv ("filesize"), NULL, 16)<16)
	      return -1;
	      break;
    }
    return simple_strtoul(getenv ("filesize"), NULL, 16);
}

void main_menu_usage(int mode)
{
    printf("\r\n*************U-boot Menu****************\r\n");
    printf("\r     by Lintel(Lintel.huang@gmail.com)    \r\n");
    printf("          Download Mode:%s   \r\n",mode==0?"tftp":"serial");
    printf("[0] BootLoader[u-boot.kwb]  --> SPI Flash \r\n");
    printf("[1] BootLoader[u-boot.kwb]  --> Nand Flash\r\n");
    printf("[2] Linux kernel[uImage]    --> SPI Flash \r\n");
    printf("[3] Linux kernel[zImage]    --> SPI Flash \r\n");
    printf("[4] Rootfs filesystem       --> SPI Flash \r\n");
    printf("[5] DDNAS Firmware          --> SPI Flash \r\n");
    printf("[6] QNAP Firmware           --> SPI Flash \r\n");
    printf("[6] Linux kernel            --> Ram & Run \r\n");
    printf("[7] DDNAS Rescue Mode       --> Tftpd     \r\n");
    printf("[8] Set boot command line   --> Linux     \r\n"); 
    printf("[s] Setting  parameter      -->           \r\n");
    printf("[m] Multi boot              -->           \r\n");
    printf("[e] Reset settings to default             \r\n"); 
    printf("[c] Change download Mode                  \r\n"); 
    printf("[b] Boot Linux                            \r\n");
    printf("[r] ReBoot System                         \r\n");
    printf("[q] Quit to shell                         \r\n");
    printf("Boot From:%s FLASH\n",(bBootFrmNORFlash == 1?"SPI":"NAND"));
    printf("Choose a job:");
}

void menu_shell(void)
{
    char c;
    char cmd_buf[256];
    static int mode=0;
    while (1)
    {
      /*
      check the download modes 
      0 for usb download
      1 for tftp download
      3 for serial download
      */
	main_menu_usage(mode);
	
        c = awaitkey(-1, NULL);
        printf("%c\n", c);
        switch (c)
        {
        case 'c':
        {
	    mode=!mode;
            break;
	}
        case 's':
        {
	    system_setting_shell();
            break;
	}
        case 'b':
        {
//            strcpy(cmd_buf, "usbslave 1 0x800000; nand erase boot; nand write 0x800000 boot $(filesize)");
            strcpy(cmd_buf, "bootd");
            run_command(cmd_buf, 0);
            break;
        }
        case 'f':
        {
#if 0
	    if (bBootFrmNORFlash == 1)
            strcpy(cmd_buf, "mtdpart default; nand erase;saveenv");
	    else
	    {
	    printf("\nBoot From NAND !\nNot erase bootloader.\n ");
            strcpy(cmd_buf, "mtdpart default; nand erase 0x00080000; saveenv");
	    }
            run_command(cmd_buf, 0);
#else
	  printf("\nNot Supported!\n ");
#endif
            break;
        }
	case 'm':
	  multiboot_shell();
	  break;
        case 'r':
        {
            run_command("reset", 0);
            break;
        }
        case 'v':
        {
#if 0
            strcpy(cmd_buf, "mtdparts");
            run_command(cmd_buf, 0);
	    printf("\nPress any key...\n");
	    awaitkey(-1, NULL);
#else
	  printf("\nNot Supported!\n ");
#endif
            break;
        }
	case 'e':
	{
	    printf("Reset u-boot-env...");
            run_command("env default -f;saveenv;reset",0);
            break;
	}
        case  27:
        case 'q':
        {
            return;
            break;
        }
        case '0':
        {
	  if(bBootFrmNORFlash)
	  {
	    printf("\nWarning!This operation will be brick your board!!!continue?(y/n)");
	    c = awaitkey(-1, NULL);
	    if (c=='n'||c=='N')
	    break;  
	  }
	    printf("\nDownload U-boot Bootloader ... \n");
	    if(download(mode)<0)
	    break;
	    if(chk_image())
	    {
	    strcpy(cmd_buf, "sf probe 0;sf erase 0x0 0x100000;sf write 0x800000 0x0 0x100000");
	    }
	    else
	    {
	      printf("Image is not U-boot SPI image!!!abort!\n");
	    }
	    {
	    if (simple_strtoul(getenv ("filesize"), NULL, 16)>4)
            run_command(cmd_buf, 0);
	    }
            break;
        }
        case '1':
        {
	    printf("\nDownLoad b-boot nand ... \n");
            break;
        }
	case '2': 
	{
	    printf("\nDownLoad Kernel ... \n");
	    break;
	}
        case '3':
        {
	    printf("\nDownLoad Kernel ... \n");
	    download(mode);
	    break;
        }
	case '4': //jffs2
	{
		printf("\nDownLoad rootfs ... \n");
		break;
	}

	case '5': 
	{

		printf("\nDownLoad DDNAS Firmware ... \n");
		download(mode);
		strcpy(cmd_buf, "sf probe 0;sf erase 0x100000 0xf00000;sf write 0x800000 0x100000 0xf00000");
		if(chk_image()==0)
		{
		run_command(cmd_buf, 0);
//		run_command(" bsetenvootargs noinitrd root=/dev/mtdblock3  console=ttySAC0 rootfstype=yaffs2;saveenv", 0);
		}
		else
		  printf("Firmware error!");
		break;
	}
        case '6':
	{
            break;	  
         }
	case '7': 
	{
	    run_command("rescue", 0);
	    break;
	}
        case '8':
        {
	    printf("Current command line:%s\n",getenv("bootargs"));
	    printf("New command line:");
	    memset(console_buffer,NULL,CONFIG_SYS_CBSIZE);
	    readline(NULL);
//	    strcat(cmd_buf, console_buffer);   
	    printf("Save new command line ?(y/n)");
	    c = awaitkey(-1, NULL);
	    
	    if(c=='y'||c=='Y')
	    {setenv("bootargs",console_buffer); run_command("saveenv",0);break;}
	}
        }
    }
}


void multiboot_menu_usage(void)
{
    printf("\r\n*************U-boot Menu****************\r\n");   
    printf("\r              Multi boot                  \r\n");
    printf("[1] Boot Linux from USB Disk[zImage]        \r\n");
    printf("[2] Boot Linux from MMC/SD Card [zImage]    \r\n");
    printf("[3] Set USB Boot as bootcmd                 \r\n");
    printf("[4] Set SD/MMC Boot as bootcmd              \r\n");
    printf("[q] Back to main menu                       \r\n");
    printf("Boot From:%s FLASH\n",(bBootFrmNORFlash == 1?"SPI":"NAND"));
    printf("Choose a job:");
}

void multiboot_shell(void)
{
    char c;
    char cmd_buf[256];
    while (1)
    {
      /*
      check the download modes 
      0 for usb download
      1 for tftp download
      3 for serial download
      */
	multiboot_menu_usage();
	
        c = awaitkey(-1, NULL);
        printf("%c\n", c);
        switch (c)
        {
        case '1':
        {
	    printf("Load kernel form USB...\n");
	    run_command("usb start", 0);
	    run_command("fatls usb 0", 0);
            strcpy(cmd_buf, "fatload usb 0 0x30008000 ");
	    printf("filename to load from USB:");
	    memset(console_buffer,NULL,CONFIG_SYS_CBSIZE);
	    readline(NULL);
	    strcat(cmd_buf, console_buffer);
	    run_command(cmd_buf, 0);
	    if (simple_strtoul(getenv ("filesize"), NULL, 16)>16)
            run_command("bootz", 0);	
            break;
	}
        case '2':
        {
	    printf("Load kernel form SD/MMC...\n");
	    run_command("mmc init", 0);
	    run_command("fatls mmc 0", 0);
            strcpy(cmd_buf, "fatload mmc 0 0x30008000 ");
	    printf("filename to load from MMC/SD:");
	    memset(console_buffer,NULL,CONFIG_SYS_CBSIZE);
	    readline(NULL);
	    strcat(cmd_buf, console_buffer);
	    run_command(cmd_buf, 0);
	    if (simple_strtoul(getenv ("filesize"), NULL, 16)>16)
	    run_command("bootz", 0);
            break;
	}
        case '3':
        {
	    setenv("bootcmd","mmc init;fatload mmc 0 0x30008000 zImage;bootz");
	    run_command("saveenv", 0);
            break;
	}
        case '4':
        {
	    setenv("bootcmd","usb start;fatload mmc 0 0x30008000 zImage;bootz");
	    run_command("saveenv", 0);
            break;
	}
	case  27:
        case 'q':
        {
            return;
            
	}
	}
    }
}

void system_setting_menu_usage(void)
{
    printf("\r\n*************U-boot Menu****************\r\n");   
    printf("\r             System Settings              \r\n");
    printf("[1] Print U-boot parameter                  \r\n");
    printf("[2] Edit a U-boot parameter                 \r\n");
    printf("[3] Select board types                      \r\n");
    printf("[4] Nand scrub - really clean NAND erasing bad blocks (UNSAFE) \r\n");
    printf("[5] Reset settings to default               \r\n"); 
    printf("[s] Save settings to flash                  \r\n");
    printf("[q] Back to main menu                       \r\n");
    printf("Boot From:%s FLASH\n",(bBootFrmNORFlash == 1?"SPI":"NAND"));
    printf("Choose a job:");
}

void system_setting_shell(void)
{
    char c;
    char cmd_buf[256];
    while (1)
    {
      /*
      check the download modes 
      0 for usb download
      1 for tftp download
      3 for serial download
      */
	system_setting_menu_usage();
	
        c = awaitkey(-1, NULL);
        printf("%c\n", c);
        switch (c)
        {
        case '1':
        {
	  strcpy(cmd_buf, "printenv ");
	  printf("Name(enter to view all paramters): ");
	  readline(NULL);
	  strcat(cmd_buf, console_buffer);
	  run_command(cmd_buf, 0);
	   printf("Press any key...");
	    awaitkey(-1, NULL);
            break;
	}
	case '2':
	{
	sprintf(cmd_buf, "setenv ");
	printf("Name: ");
        readline(NULL);
        strcat(cmd_buf, console_buffer);
	printf("Value: ");
	readline(NULL);
	strcat(cmd_buf, " ");
	strcat(cmd_buf, console_buffer);
	run_command(cmd_buf, 0);
        break;
        }
	case '3':
	{
	  matchtype_setting_shell();
	  break;
	}
	case '4':
	{
#if 0
	strcpy(cmd_buf, "nand scrub ");
	run_command(cmd_buf, 0);
#endif
	break;
	}
        case '5':
        {
            strcpy(cmd_buf, "env default -f;saveenv;");
            run_command(cmd_buf, 0);
            break;
        }
	case 's':
	{
	  run_command("saveenv", 0);
	  break;
	}
	case  27:
        case 'q':
        {
            return;
            
	}
	}
    }
}

void machtype_menu_usage(void)
{
    printf("\r\n*************U-boot Menu****************\r\n");   
    printf("\r           select Board Types             \r\n");
    printf("88F6281:                                    \r\n");
    printf("[1] DDNAS                                   \r\n");
    printf("[2] QNAP TS-219                             \r\n");
    printf("[3] OpenRD                                  \r\n");
    printf("[4] DreamPlug                               \r\n");
    printf("[5] SheevPlug                                \r\n");
    printf("[6] Iconnect                                 \r\n");
    printf("[o] Other Boards                            \r\n");
    printf("[s] Save settings to flash                  \r\n");
    printf("[q] back to upset menu                       \r\n");
    printf("Boot From:%s FLASH\n",(bBootFrmNORFlash == 1?"SPI":"NAND"));
    printf("Choose a job:");
}
void matchtype_setting_shell(void)
{
    char c;
    char cmd_buf[256];
    
    while (1)
    {	
	machtype_menu_usage();
        c = awaitkey(-1, NULL);
        printf("%c\n", c);
        switch (c)
        {
        case '1':
        {
	      setenv("machid","3381");
            break;
	}
        case '2':
        {
	      setenv("machid","2139");
            break;
	}
	case '3':
        {
 	      setenv("machid","2325");
            break;
	}
	case '4':
        {
 	      setenv("machid","2315");
            break;
	}
	case '5':
        {
 	      setenv("machid","2678");
            break;
	}
	case '6':
        {
 	      setenv("machid","2870");
            break;
	}
	case 'o':
	{
	    printf("Current MACH_TYPE_ID:%s\n",getenv("machid"));
	    printf("New MACH_TYPE_ID:");
	    memset(console_buffer,NULL,CONFIG_SYS_CBSIZE);
	    readline(NULL);
	    setenv("machid",console_buffer);
            break;
	}
	case 's':
	{
	  run_command("saveenv", 0);
	  break;
	}
	case 'q':
	case  27:
        {
            return 0;
            break;
	}
        }
    }
}


int do_menu (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    menu_shell();
    return 0;
}
int do_rescue (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    printf("Start rescue mode...\n");
    run_command("tftpd 0x0x800000", 0);

    switch(chk_image())
    {
      case 0:
	    run_command("sf probe 0;sf erase 0x100000 0xf00000;sf write 0x800000 0x100000 0xf00000", 0);
	    run_command("reset", 0);
	    break;
      default:
	    printf("Firmware error!");
	    run_command("reset", 0);
//		run_command(" bsetenvootargs noinitrd root=/dev/mtdblock3  console=ttySAC0 rootfstype=yaffs2;saveenv", 0);
    }
    return 0;
}
U_BOOT_CMD(
     rescue, 1, 0, do_rescue,
    "start rescue mode",
    "DDNAS rescue mode by lintel\n"
);
U_BOOT_CMD(
    menu, 1, 0, do_menu,
    "Download Menu",
    "U-boot Download Menu by lintel\n"
);
