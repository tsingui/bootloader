/*==============================================================
 #Copyright (c), 1991-2008, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 #Function:��ѹ�����bootloader image_second������� �ļ�ͷ��
 #Designed by xuanguanglei 
 #Date:2008-07-11
 ==============================================================*/
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>
#include "config.h"				/*��ȡ��ǰCPU�Ǵ�˻���С��*/
#include "../include/mkimage.h"

#define DEFAULT_LZMAFILE "u-boot.bin.lzmaimg"

//#define DEBUG 1

#if 1
typedef struct
{
    char  BootloaderSecondName[256];
    char  OutputBootloaderSecondName[256];
    unsigned int   loadaddr;
}ImageInfo;

#endif

int ConvertEndian(int val)
{
	int i;
	unsigned char *p;
	unsigned char *q;

	i = val;
	p = ( unsigned char * ) &val;
	q = ( unsigned char * ) &i;

#ifdef __BIG_ENDIAN
	*q = *(p+3);
	*(q+1) = *(p+2);
	*(q+2) = *(p+1);
	*(q+3) = *p;
#endif

	return i;
}

void PrintHelp(void)
{
    printf("This utility lzmas bootloader second part and add image header.\n\n"); 
    printf("Usage:  mkimage [-h,--help] [-b filename] [-o filename] [-l loadaddr]\n");
    printf("   -h,--help\tPrint this infomation\n");
    printf("   -b\t\tThe name of second part of bootloader file (required)\n"); 
    printf("   -o\t\tThe name of output lzmaed bootloader  file \n");
    printf("\n");
}



int AnalyseParameter(int argc, char *argv[], ImageInfo *stImgInfo)
{
	int i,j;
	unsigned int loadaddr[1];
	char strCurrentDir[256];
	char strTempPath[256];
	char strSlash[2] = "/";
	
	for(i=1;i<argc;i++)
	{
		 if(strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"--help") == 0)
		 {
			PrintHelp();
			return 0;
		 }
		 else if(strcmp(argv[i],"-b") == 0)            /* deal with bootload name parameter */
		 {
			i++;

			if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
			{
				printf("The -b parameter is wrong.\n");
				return 0;
			}

			strcpy(stImgInfo->BootloaderSecondName, argv[i]);
		 }
		 else if(strcmp(argv[i],"-o") == 0)            /* deal with kernel name parameter */
		 {
			i++;

			if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
			{
				printf("The -o parameter is wrong.\n");
				return 0;
			}

			strcpy(stImgInfo->OutputBootloaderSecondName, argv[i]);
		 }

		else if(strcmp(argv[i],"-l") == 0)            /* deal with IP address parameter */
		{
			i++;

			if(i >= argc || *argv[i] == '-')            /* if the next parameter is wrong */
			{
				printf("The -l parameter is wrong.\n");
				return 0;
			}

			if(sscanf(argv[i],"%x",&loadaddr[0]) != 1) 
			{
				printf("The -l parameter is wrong.\n");
				return 0;
			}
			stImgInfo->loadaddr = loadaddr[0];
		}

	}
	if(stImgInfo->OutputBootloaderSecondName == 0)
	{
		strcpy(stImgInfo->OutputBootloaderSecondName, DEFAULT_LZMAFILE);
	}

	/* Generate absolute path */

	getcwd(strCurrentDir,256);           /* Get current dir name */

	if(*(stImgInfo->BootloaderSecondName) != '/')
	{
		strcpy(strTempPath,strCurrentDir);  /* Check bootloader file */
		strcat(strTempPath,strSlash);
	}
	else
	{
		strTempPath[0] = '\0';
	}

	strcat(strTempPath,stImgInfo->BootloaderSecondName);
	strcpy(stImgInfo->BootloaderSecondName,strTempPath);


	if(access(stImgInfo->BootloaderSecondName,F_OK))
	{
		printf("The bootloader file is not exist.\n");
		return 0;
	}
	return 1;
}

int CreateLzmaimgFile(ImageInfo *stImgInfo, bootloader_second_image_hdr_t *bootloader_second_image_hdr)
{
	char buf[4096];

	FILE *pfin;
	FILE *pfout;

	int iImgFileLength = 0;      /* count how many bytes have been write to IMG file */
	int iReadCount;
	int iWriteCount;
	int tmp;
	int i;


#if DEBUG
	printf("Open output lzmaimg file: %s\n",stImgInfo->OutputBootloaderSecondName);
#endif
   
	memset( bootloader_second_image_hdr , 0 , sizeof( bootloader_second_image_hdr_t ) );
	memcpy( bootloader_second_image_hdr->tag , "lzma" , 4 );     /* fill tag */

	pfout = fopen(stImgInfo->OutputBootloaderSecondName,"w");
	if(pfout == NULL)
	{
		printf("Can't open output file: %s\n",stImgInfo->OutputBootloaderSecondName);
		return 0;
	} 

	for(i=0; i<sizeof(bootloader_second_image_hdr_t); i++)
	{
		fputc(0xffff,pfout);
		iImgFileLength++;
	}
    

	pfin = fopen(stImgInfo->BootloaderSecondName,"r");                          /* Copy bootloader file */
	if(pfin == NULL)
	{
		printf("Can't open bootloader file: %s\n",stImgInfo->BootloaderSecondName);
		return 0;
	}
    
	while(1)
	{
		iReadCount = fread(buf,1,4096,pfin);
		iWriteCount = fwrite(buf,1,iReadCount,pfout);
		if(iReadCount != iWriteCount && iReadCount > 0)  /* fail to write ? */
		{
			printf("Fail to write Lama file.\n");
			fclose(pfin);
			fclose(pfout);
			remove(stImgInfo->OutputBootloaderSecondName);
			return 0;
		}

		iImgFileLength += iWriteCount ;

		if(iReadCount < 4096)
		{
			if(!feof(pfin))        /* fail to read ? */
			{
				printf("Fail to read bootloader file.\n");
				fclose(pfin);
				fclose(pfout);
				remove(stImgInfo->OutputBootloaderSecondName);
				return 0;
			}

			fclose(pfin);
			break;
		}
	}

	bootloader_second_image_hdr->image_len = iImgFileLength - sizeof( bootloader_second_image_hdr_t );

	/* Dealwith image header */
	
	bootloader_second_image_hdr->image_len = ConvertEndian(bootloader_second_image_hdr->image_len);
	bootloader_second_image_hdr->image_load = ConvertEndian(stImgInfo->loadaddr);


	 /* Write image header to IMG file */    

	if(fseek(pfout,0,SEEK_SET) == -1)
	{
		printf("Fail to wirite image header.\n");
		fclose(pfout);
		remove(stImgInfo->OutputBootloaderSecondName);
		return 0;                         /* fail to lseek */
	}

	iWriteCount = fwrite(bootloader_second_image_hdr, 1,sizeof(bootloader_second_image_hdr_t) ,pfout);
	if(iWriteCount != sizeof(bootloader_second_image_hdr_t))  /* fail to write ? */
	{
		printf("Fail to write header to IMG file.\n");
		fclose(pfout);
		remove(stImgInfo->OutputBootloaderSecondName);
		return 0;
	}
 
    fclose(pfout);
    return 1;
}


int main(int argc, char *argv[])
{
	int i;
	bootloader_second_image_hdr_t	bootloader_second_image_hdr;
	ImageInfo g_stImgInfo;

	memset( &g_stImgInfo , 0 , sizeof( ImageInfo ) );

	if(AnalyseParameter(argc, argv, &g_stImgInfo))    /* if analyse parameter fail,exit  */
	{    	
		if(CreateLzmaimgFile(&g_stImgInfo, &bootloader_second_image_hdr))
		{
			printf("Generate lzma file successfully!\n");
		}
	}

	return 0 ;
}

