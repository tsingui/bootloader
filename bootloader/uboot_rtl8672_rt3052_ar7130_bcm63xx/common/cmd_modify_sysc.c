#include <common.h>
#include <command.h>
#include <flash.h>

sys_config_t * stSysConfig = ( sys_config_t * ) ( CFG_SDRAM_BASE + SYSCFG_OFFSET );
extern int  sysdata_save(sys_config_t * 	syscfg,flash_info_t *  	info);
void deal_mac(char *argv, unsigned char iTempMac[]);
void deal_ip(char *argv, unsigned char iTempIP[]);

void deal_ip(char *argv, unsigned char iTempIP[])
{
	unsigned char i,k;	
	unsigned char ip_right=0;
	unsigned char ip_data[4][3];
	int j;

	for(i=0;i<4;i++)
	{
		for(k=0;k<3;k++)
			ip_data[i][k]=0;		
	}
	
	i=4;
	j=0;
	k=2;	

	argv[strlen(argv)]='.';
	argv[strlen(argv)+1]='\0';
	for(j=(strlen(argv)-1);j>=0;j--)
	{		
		if(argv[j]=='.')
		{				
			i--;
			k=2;
			continue;
		}
		ip_data[i][k]=argv[j];
		k--;		
	}
	for(i=0;i<15;i++)
		argv[i]=0;
	for(i=0;i<4;i++)
	{		
		for(k=0;k<3;k++)
		{
			if(ip_data[i][k]>'9')
			{
				printf("your ip is not suitability\n");
				ip_right=1;
				break;
			}
			else if(ip_data[i][k]>=48 && ip_data[i][k]<=57)     
			ip_data[i][k]-=48;
		}
	}
	if(ip_right!=1)
	{
		for(i=0;i<4;i++)
		{		
			iTempIP[i]=ip_data[i][0]*100+ip_data[i][1]*10+ip_data[i][2];		
		}
	}
}

void deal_mac(char *argv, unsigned char iTempMAC[])
{
	unsigned char j,k;
	unsigned char mac_right=0;

	for(j=0;j<17;j++)
	{
		if(argv[j]==':')
			continue;
		if(argv[j]>'f')
		{
			printf("your mac is not suitability\n");
			mac_right=1;
			break;
		}
		else if(argv[j]>=65 && argv[j]<=90)       /*'A'<=argv[j]<='Z'*/
			argv[j]-=55;
		else if(argv[j]>=97 && argv[j]<=122)     /*'a'<=argv[j]<='z'*/
			argv[j]-=87;	
		else if(argv[j]>=48 && argv[j]<=57)      /*'0'<=argv[j]<='9'*/
			argv[j]-=48;
	}
	if(mac_right!=1)
	{
		for(j=0,k=0;j<17;)
		{
			iTempMAC[k]=argv[j]*16+argv[j+1];
			k++;
			j+=3;
		}
	}
}


int
do_modify_sysc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned char  i,j,temp;	
	unsigned char iTempIP[4];
	unsigned char iTempMAC[6];
	union 
	{  
		unsigned int iIpVal;  
		char   ch[4];  
	}iptmp = {0}; 	
	temp=0;       //说明修改的内容与原有相同
	for(i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-mac") == 0)            /* deal with MAC address parameter */
		{
              	i++;    
              	if(i >= argc || *argv[i] == '-')            /* if the next parameter is wrong */
              	{
                  		 printf("The -mac parameter is wrong.\n");
                  		 return 0;
              	}
			deal_mac(argv[i],  iTempMAC);
			for(j=0;j<6;j++)
			{
				if(stSysConfig->mac[j]!=iTempMAC[j])
				temp=1;
			}
			if(temp==1)
			{
				for(j=0;j<6;j++)
				{
					stSysConfig->mac[j]=iTempMAC[j];			
				}		
			}
		}

		else if(strcmp(argv[i],"-ip") == 0)            /* deal with IP address parameter */
         	{
              	i++;
              	if(i >= argc || *argv[i] == '-')            /* if the next parameter is wrong */
              	{
                   		printf("The -ip parameter is wrong.\n");
                   		return 0;
              	}
			deal_ip(argv[i] ,  iTempIP);
			for(j=0;j<4;j++)
			{
				if(iptmp.ch[j]!=iTempIP[j])
				temp=1;
			}
			if(temp==1)
			{
				for(j=0;j<4;j++)
              		{				
					iptmp.ch[j] = iTempIP[j];
              		}
				stSysConfig->ip = iptmp.iIpVal;
			}               
            
		}
	}
	if(temp==1)
	{			
		sysdata_save(stSysConfig,info); 
	}
	
	return 0;
}

U_BOOT_CMD(
	modify_sysc,	5,	1,	do_modify_sysc,
 	"modify_sysc    - sysc modify \n",
	"-mac xx.xx.xx.xx.xx.xx -ip x.x.x.x\n"
);
