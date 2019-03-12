#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <iostream>

#define VERSION_INFO "v2.0a 20190305"
#define DEBUG 0

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
	int offset;
}MicroCode;

bool load(const char* fileName,std::vector<MicroCode> *vtr,unsigned int *binarySize,bool print);
void listMicrocode(std::vector<MicroCode> *vtr);
void showMicroCode(MicroCode microCode);
bool swapMicroCode(MicroCode source,MicroCode dest,const char* fileName);
bool verify(const char* originFileName,bool unsafe);

int main(int argc, char *argv[])
{
	setvbuf (stdout, NULL, _IONBF, 0);

	bool unsafe = false;
	if(argc > 1)
		if(strcmp(argv[1],"-unsafe") == 0)
		{
			printf("Warning : Unsafe mode is enabled!\n");
			unsafe = true;
		}

	//data
	std::vector<MicroCode> bios;
	std::vector<MicroCode> lib;
	unsigned int biosBinSize;
	unsigned int libBinSize;

	//user inputs
	char biosName[1024];
	char libName[1024];

	//load bios
	printf("Enter bios file path : ");
	scanf("%s",biosName);
	if(!load(biosName,&bios,&biosBinSize,true))
	{
		printf("Error : cannot open file %s.",biosName);
		return -1;
	}
    if(DEBUG)
		listMicrocode(&bios);

	printf("\n");

	//load lib
	printf("Enter lib file path : ");
	scanf("%s",libName);
    if(!load(libName,&lib,&libBinSize,true))
	{
		printf("Error : cannot open file %s.",biosName);
		return -1;
	}
    if(DEBUG)
		listMicrocode(&lib);

	printf("\n");
	// user input - slot entry
	int destPos = 1;
	do
	{
		if(destPos<1 || destPos>bios.size())
			printf("Error : invalid input!\n");
		printf("Enter slot to be replaced : ");
		scanf("%d",&destPos);
	}
	while (destPos<1 || destPos>bios.size());

	int sourcePos = 1;
	do
	{
		if(sourcePos<1 || sourcePos>lib.size())
			printf("Error : invalid input!\n");
		printf("Enter slot to import : ");
		scanf("%d",&sourcePos);
	}
	while(sourcePos<1 || sourcePos>lib.size());

	printf("\n");
	
	if(bios[destPos-1].totalSize != lib[sourcePos-1].totalSize)
	{
		if(!unsafe)
		{
			printf("Error : Source and destination MicroCodes are not the same size!\n");
			return -1;
		}
	}
	//skip the microcode with totalSize = 0
	if((bios[destPos-1].totalSize == 0) || (lib[sourcePos-1].totalSize == 0))
	{
		if(!unsafe)
		{
			printf("Error : Source or destination MicroCodes are not safe to be swapped.\n");
			return -1;
		}
	}

	//confirm dialog
	printf("The following microcode of %s will be replaced : \n",biosName);
	showMicroCode(bios[destPos-1]);
	printf("The following microcode of %s will be imported : \n",libName);
	showMicroCode(lib[sourcePos-1]);
	printf("Is that okay? y/n : ");
	char confirm[12];
	scanf("%s",confirm);
	if(confirm[0]!='y')
	{
		printf("Exit!\n");
		return 1;
	}
	printf("\n");

	printf("Swapping microcode ...\n");
	if(swapMicroCode(lib[sourcePos-1],bios[destPos-1],biosName))
	{
		printf("\nVerifying modified bios file...\n");
		if(verify(biosName,unsafe))
		{
			printf("Success!\n");
			return 0;
		}
		else
		{
			printf("Failed!\n");
			return -1;
		}
	}
	else
	{
		printf("Operation aborted.\n");
		return -1;
	}
}

bool load(const char* fileName,std::vector<MicroCode> *vtr,unsigned int *binarySize,bool print)
{
	if(print)
		printf("Trying to load from %s.\n",fileName);

	FILE *filePointer;
	if ((filePointer=fopen(fileName, "rb")) == NULL) 
	{
		if(print)
			printf("Error : Cannot open file %s\n", fileName);
		return false;
	}
    else
	    if(print)
			printf("Loaded file : %s\n",fileName);

	//get bios size
	int fsize;
	fseek(filePointer, 0, SEEK_END);
	fsize = ftell(filePointer);
	fseek(filePointer, 0, SEEK_SET);

	*binarySize=fsize;

    //load file to buffer
	unsigned char *buf = new unsigned char [fsize];
	fread(buf, 1, fsize, filePointer);
	fclose(filePointer);

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
			if(print)
				printf("No.%d\t",entryCounter);

			MicroCode tmp;

			if(print)
				printf("CPUID=%X\t",*(unsigned int*)(&buf[i+12]));									
			tmp.cpuId=*(unsigned int*)(&buf[i+12]);

			
			if(print)
				printf("Rev=%02X\t",*(unsigned int*)(&buf[i+4]));									
			tmp.rev=*(unsigned int*)(&buf[i+4]);

			if(print)
				printf("%02X%02X/%02X/%02X\t",buf[i+9], buf[i+8], buf[i+11], buf[i+10]);			
			tmp.yf=buf[i+9];
			tmp.yl=buf[i+8];
			tmp.mm=buf[i+11];
			tmp.dd=buf[i+10];

			if(print)
				printf("CRC=%08X\t",*(unsigned int*)(&buf[i+16]));									
			tmp.crc=*(unsigned int*)(&buf[i+16]);

			if(print)
				printf("datasize=%4d\t", *(unsigned int*)(&buf[i+28]));								
			tmp.dataSize=*(unsigned int*)(&buf[i+28]);
			
            //totalsize							
			tmp.totalSize=*(unsigned int*)(&buf[i+32]);
            if(print)
			{
				if(tmp.totalSize == 0)
					printf("total=%4d*\t",totalsize);
				else
					printf("total=%4d\t",totalsize);
			}

			if(print)
				printf("offset=%-8X\t",i);
			tmp.offset = i;															

			if(print)
				printf("PlatformID=(%3d):",*(unsigned int*)(&buf[i+24]));							
			tmp.platformId=*(unsigned int*)(&buf[i+24]);	
			bool flag = false;
			for (int j=0; j<=7; ++j)
			{
				if ((1<<j) & *(unsigned int*)(&buf[i+24]))
				{
					if (flag) 
					{
						if(print)
							printf(",");															
					}
					flag = true;
					if(print)
						printf("%d", j);															
				}
			}
			if(print)
				printf("\n");																		

			//save microcode data to node
			tmp.data = new unsigned char [totalsize];
			memcpy(tmp.data,buf+i,totalsize);

            vtr->push_back(tmp);
		}
	}

    if(vtr->size() < 1)
	{
		printf("Error : Cannot find any microcode in file!");
		return false;
	}
	return true;
}

void listMicrocode(std::vector<MicroCode> *vtr)
{
    int entryCounter = 0;	
    for (MicroCode microcode : *vtr) 
    {
        entryCounter++;
		printf("No.%d\t",entryCounter);
		showMicroCode(microcode);
    }
}

void showMicroCode(MicroCode microCode)
{
	printf("CPUID=%X\t",microCode.cpuId);
		printf("Rev=%02X\t",microCode.rev);
		printf("%02X%02X/%02X/%02X\t",microCode.yf, microCode.yl, microCode.mm, microCode.dd);
		printf("CRC=%08X\t",microCode.crc);
		printf("datasize=%4d\t", microCode.dataSize);
        if(microCode.totalSize==0)
		    printf("total=2048*\t",microCode.totalSize);
        else
            printf("total=%4d\t",microCode.totalSize);
        
		printf("PlatformID=(%3d):",microCode.platformId);
		bool flag = false;
		for (int j=0; j<=7; ++j)
		{
			if ((1<<j) & microCode.platformId)
			{
				if (flag) 
				    printf(",", j);
				flag = true;
				printf("%d", j);
			}
		}
		printf("\n");
}

bool swapMicroCode(MicroCode source,MicroCode dest,const char* fileName)
{
	FILE *filePointer;
	if ((filePointer=fopen(fileName, "rb")) == NULL) 
	{
		printf("Error : Cannot open file %s\n", fileName);
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
			
			if(
				dest.cpuId==*(unsigned int*)(&buf[i+12]) &&						
				dest.rev==*(unsigned int*)(&buf[i+4]) &&
				dest.yf==buf[i+9] &&
				dest.yl==buf[i+8] &&
				dest.mm==buf[i+11] &&
				dest.dd==buf[i+10] &&								
				dest.crc==*(unsigned int*)(&buf[i+16]) &&						
				dest.dataSize==*(unsigned int*)(&buf[i+28]) &&				
				dest.totalSize==*(unsigned int*)(&buf[i+32]) &&					
				dest.platformId==*(unsigned int*)(&buf[i+24])
			)
			{
				printf("Found microcode to be swapped at offset %-8X\n",i);
				//swap microcode data
				memcpy(buf+i,source.data,source.totalSize);

				//output file name
				char *outputFileName = new char [strlen(fileName)+5];
				strcpy(outputFileName,fileName);
				strcat(outputFileName,".out");

				//write output
				FILE *outputWriter;
				outputWriter=fopen(outputFileName,"wb");
				if(outputWriter==NULL)
				{
					printf("Error : Cannot create output file! \nOperation aborted!");
					return false;
				}
				printf("Write %d bytes to output file : %s\n",fsize,outputFileName);
				fwrite (buf ,1, fsize, outputWriter);
				fclose(outputWriter);
				return true;
			}
		}
	}
	printf("Error : Original bios file mismatch!\n");
	return false;
}

bool verify(const char* originFileName,bool unsafe)
{
	char *moddedFileName = new char [strlen(originFileName)+5];
	strcpy(moddedFileName,originFileName);
	strcat(moddedFileName,".out");

	//load origin and modded files
	std::vector<MicroCode> origin;
	std::vector<MicroCode> modded;
	unsigned int originSize;
	unsigned int moddedSize;
	if(load(originFileName,&origin,&originSize,false) && load(moddedFileName,&modded,&moddedSize,false))
	{
		if(!unsafe)
		{
			if(originSize != moddedSize)
			{
				printf("Error : File size not matched!\n");
				return false;
			}

			if(origin.size() != modded.size())
			{
				printf("Error : Number of microcodes not matched!\n");
				return false;
			}

			int moddedMicrocode = 0;
			for(int i=0;i< origin.size();i++)
			{
				if(
					(origin[i].totalSize != modded[i].totalSize) ||
					(origin[i].offset != modded[i].offset))
				{
					printf("Error : Microcode data structure error!\n");
					return false;
				}

				if((origin[i].dataSize!=modded[i].dataSize) || (origin[i].crc!=modded[i].crc))
					moddedMicrocode++;
			}

			if(moddedMicrocode != 1)
			{
				printf("Error : Ouput file has been occurred!\n");
				return false;
			}
		}
		listMicrocode(&modded);
		return true;
	}
	return false;
}