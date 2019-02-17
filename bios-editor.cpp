#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <algorithm>
#include <iostream>

#define VERSION_INFO "v2.0a 20190217"
#define DEBUG_OUTPUT 1

typedef struct MICROCODE
{
	unsigned int dataSize;
	unsigned int totalSize;
	unsigned int cpuId;
	unsigned int rev;
	unsigned int yf;
	unsigned int yl;
	unsigned int mm;
	unsigned int dd;
	unsigned int platformId;
	unsigned int crc;
	unsigned char *data;
}MicroCode;

bool load(const char* fileName,std::list<MicroCode> *lst,unsigned char *binaryData,unsigned int *binarySize);
void listMicrocode(std::list<MicroCode> *lst);

int main()
{
	setvbuf (stdout, NULL, _IONBF, 0);
	//data
	std::list<MicroCode> bios;
	std::list<MicroCode> lib;
	unsigned char *biosBin;
	unsigned char *libBin;
	unsigned int biosBinSize;
	unsigned int libBinSize;

	//user inputs
	char biosName[1024];
	char libName[1024];

	//load bios
	printf("Enter bios file path : ");
	scanf("%s",biosName);
	if(!load(biosName,&bios,biosBin,&biosBinSize))
	{
		printf("Error : cannot open file %s.",biosName);
		return -1;
	}
    if(DEBUG_OUTPUT)
		listMicrocode(&bios);

	//load lib
	printf("Enter lib file path : ");
	scanf("%s",libName);
    if(!load(libName,&lib,libBin,&libBinSize))
	{
		printf("Error : cannot open file %s.",biosName);
		return -1;
	}
    if(DEBUG_OUTPUT)
		listMicrocode(&lib);

    return 0;
}

bool load(const char* fileName,std::list<MicroCode> *lst,unsigned char *binaryData,unsigned int *binarySize)
{
	printf("Trying to load from %s.\n",fileName);

	FILE *filePointer;
	if ((filePointer=fopen(fileName, "rb")) == NULL) 
	{
		printf("[Error] Cannot open file %s\n", fileName);
		return false;
	}
    else
	    printf("Loaded file : %s\n",fileName);

	//get bios size
	int fsize;
	fseek(filePointer, 0, SEEK_END);
	fsize = ftell(filePointer);
	fseek(filePointer, 0, SEEK_SET);

    //load file to buffer
	unsigned char *buf = new unsigned char [fsize];
	fread(buf, 1, fsize, filePointer);
	fclose(filePointer);

	//making a copy of binary file
	*binarySize=fsize;
	binaryData=new unsigned char [fsize];
	memcpy(binaryData,buf,fsize);

	int entryCounter = 0;

	for (int i=0; i<fsize-2000; i++)
	{
		if (*(unsigned int*)(&buf[i+0]) == 1
			&& (buf[i+9]==0x20 || buf[i+9]==0x19)
			&& (buf[i+10]>=0x01 && buf[i+10]<=0x09 || buf[i+10]>=0x10 && buf[i+10]<=0x19 || buf[i+10]>=0x20 && buf[i+10]<=0x29 || buf[i+10]>=0x30 && buf[i+10]<=0x31)
			&& (buf[i+11]>=0x01 && buf[i+11]<=0x09 || buf[i+11]>=0x10 && buf[i+11]<=0x12)) 
		{
			int totalsize = *(unsigned int*)(&buf[i+32]);
			
			if (totalsize == 0)
            {
                totalsize = 2048;
            }
			if ((totalsize & 1023) != 0) continue;
			if (i+totalsize > fsize) continue;
			int j,sum=0;
			for (j=0; j<totalsize; j+=4) sum += *(unsigned int*)(&buf[i+j]);
			if (sum != 0) continue;

			entryCounter++;
			printf("No.%d\t",entryCounter);

			MicroCode tmp;

			printf("CPUID=%X\t",*(unsigned int*)(&buf[i+12]));									
			tmp.cpuId=*(unsigned int*)(&buf[i+12]);

			printf("Rev=%02X\t",*(unsigned int*)(&buf[i+4]));									
			tmp.rev=*(unsigned int*)(&buf[i+4]);

			printf("%02X%02X/%02X/%02X\t",buf[i+9], buf[i+8], buf[i+11], buf[i+10]);			
			tmp.yf=buf[i+9];
			tmp.yl=buf[i+8];
			tmp.mm=buf[i+11];
			tmp.dd=buf[i+10];

			printf("CRC=%08X\t",*(unsigned int*)(&buf[i+16]));									
			tmp.crc=*(unsigned int*)(&buf[i+16]);

			printf("datasize=%4d\t", *(unsigned int*)(&buf[i+28]));								
			tmp.dataSize=*(unsigned int*)(&buf[i+28]);
			
            //totalsize							
			tmp.totalSize=*(unsigned int*)(&buf[i+32]);
            if(tmp.totalSize==0)
                printf("total=%4d*\t",totalsize);
            else
                printf("total=%4d\t",totalsize);

			printf("offset=%-8X\t",i);															

			printf("PlatformID=(%3d):",*(unsigned int*)(&buf[i+24]));							
			tmp.platformId=*(unsigned int*)(&buf[i+24]);	
			bool flag = false;
			for (int j=0; j<=7; ++j)
			{
				if ((1<<j) & *(unsigned int*)(&buf[i+24]))
				{
					if (flag) 
					{
						printf(",");															
					}
					flag = true;
					printf("%d", j);															
				}
			}
			printf("\n");																		

			//save microcode data to node
			tmp.data = new unsigned char [totalsize];
			memcpy(tmp.data,buf+i,totalsize);
            lst->push_back(tmp);
		}
	}	
    printf("Done.\n\n");
	return true;
}

void listMicrocode(std::list<MicroCode> *lst)
{
    printf("List microcode\n");
    int entryCounter = 0;	
    for (MicroCode microcode : *lst) 
    {
        entryCounter++;
		printf("No.%d\t",entryCounter);
		printf("CPUID=%X\t",microcode.cpuId);
		printf("Rev=%02X\t",microcode.rev);
		printf("%02X%02X/%02X/%02X\t",microcode.yf, microcode.yl, microcode.mm, microcode.dd);
		printf("CRC=%08X\t",microcode.crc);
		printf("datasize=%4d\t", microcode.dataSize);
        if(microcode.totalSize==0)
		    printf("total=2048*\t",microcode.totalSize);
        else
            printf("total=%4d\t",microcode.totalSize);
        
		printf("PlatformID=(%3d):",microcode.platformId);
		bool flag = false;
		for (int j=0; j<=7; ++j)
		{
			if ((1<<j) & microcode.platformId)
			{
				if (flag) 
				    printf(",", j);
				flag = true;
				printf("%d", j);
			}
		}
		printf("\n");
    }
    printf("done.\n\n");
}