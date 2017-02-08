#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define VERSION_INFO "v1.4b 2017-02-09"
#define BIOS_LOG_FILE "BiosMicrocodeList.txt"
#define LIB_LOG_FILE "LibMicrocodeList.txt"
#define TOS ""
#define BLANK -1
#define ORIGIN 0
#define EDITED 1

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
	int status;
}MicroCode;


//global var
bool con=true;
unsigned char *clone;
unsigned char *original;
bool biosLoaded=false;
bool libLoaded=false;
int originalSize;
int libCounter=0;
int entryCounter=0;
MicroCode microCodeList[1024];
MicroCode microCodeLib[1024];
MicroCode editedBiosMicroCodeList[1024];
char biosFileName[1024];
char libFileName[1024];

//function
void showMenu();
bool loadBios();
bool editBios();
void showMicrocodeList();
void showEditedBios();
void showSingleEntry(MicroCode);
bool loadLib();
void showLib();
void updateDatabase();
bool exportBios();
int str_cpy(char*,char*);

//function body 
void showMenu()
{
	printf("Intel Microcode editor %s\n",VERSION_INFO);
	printf("-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
	printf("Bios file :\t");
	if(biosLoaded)
		printf("%s\n",biosFileName);
	else
		printf("Not loaded\n");
	printf("Lib file :\t");
	if(libLoaded)
		printf("%s\n",libFileName);
	else
		printf("Not loaded\n");
	printf("-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

	printf("[1] Load bios file\n");
	printf("[2] Display bios microcode list\n");
	printf("[3] Load microcode lib\n");
	printf("[4] Display lib microcode list\n");
	printf("[5] Edit bios microcode\n");
	printf("[6] Show edited bios microcode list\n");
	printf("[7] Export bios\n");
	printf("[0] Quit\n");

	printf("\nEnter your choice and press Enter key \n");
	
	
	char choice[100];
	scanf("%s",&choice);
	switch (choice[0])
	{
		case '0':	//quit
			con=false;
			break;
		
		case '1':	//load bios
			if(!biosLoaded)
				biosLoaded=loadBios();
			else
			{
				system("cls");
				printf("[Error] Bios has already been loaded!\n");
				printf("Press any key to continue.\n");
				_getch();
				system("cls");
			}
			break;

		case '2':	//show bios microcode list
			if(!biosLoaded)
			{
				system("cls");
				printf("[Error] Please load bios before using this function!\n");
				printf("Press any key to continue.\n");
				_getch();
				system("cls");
			}
			else
			{
				system("cls");
				printf("[2] Bios Microcode list\n\n"); 
				showMicrocodeList();
					printf("\nComplete! press any key to continue.\n");
					_getch();
					system("cls");
			}	
			break;

		case '3':	//load lib
			if(!libLoaded)
				libLoaded=loadLib();
			else
			{
				system("cls");
				printf("[Error] Lib has already been loaded!\n");
				printf("Press any key to continue.\n");
				_getch();
				system("cls");
			}
			break;



		case '4':	//show lib microcode list
			if(!libLoaded)
			{
				system("cls");
				printf("[Error] Please load lib before using this function!\n");
				printf("Press any key to continue.\n");
				_getch();
				system("cls");
			}
			else
			{
				system("cls");
				printf("[4] Lib Microcode list\n\n"); 
				showLib();
				printf("\nComplete! press any key to continue.\n");
				_getch();
				system("cls");
			}
			break;


		case '5':	//edit
			if(!biosLoaded || !libLoaded)
			{
				system("cls");
				printf("[Error] Please load bios and lib before using this function!\n");
				printf("Press any key to continue.\n");
				_getch();
				system("cls");
			}
			else
				editBios();
			break;

		case '6':	//show edited bios microcode list
			if(!biosLoaded || !libLoaded)
			{
				system("cls");
				printf("[Error] Please load bios and lib before using this function!\n");
				printf("Press any key to continue.\n");
				_getch();
				system("cls");
			}
			else
			{
				system("cls");
				printf("[6] Edited bios Microcode list\n\n"); 
				showEditedBios();
				printf("\nComplete! press any key to continue.\n");
				_getch();
				system("cls");
			}
			break;

		case '7':	//export bios
			if(!biosLoaded || !libLoaded)
			{
				system("cls");
				printf("[Error] Please load bios and lib before using this function!\n");
				printf("Press any key to continue.\n");
				_getch();
				system("cls");
			}
			else
				exportBios();
			break;

		default:
			system("cls");
			printf("[Error] invalid input!\n"); 
			printf("Press any key to continue.\n");
			_getch();
			system("cls");
			break;

	}
}

void showMicrocodeList()
{
	for(int i=0;i<entryCounter;i++)
	{
		printf("Entry No.%d\t",i+1);
		printf("CPUID=%X\t",microCodeList[i].cpuId);
		printf("Rev=%02X\t",microCodeList[i].rev);
		printf("%02X%02X/%02X/%02X  ",microCodeList[i].yf, microCodeList[i].yl, microCodeList[i].mm, microCodeList[i].dd);
		printf("CRC=%08X  ",microCodeList[i].crc);
		printf("datasize=%4d  ", microCodeList[i].dataSize);
		printf("total=%4d  ",microCodeList[i].totalSize);
		printf("PlatformID=(%3d):",microCodeList[i].platformId);
			bool flag = false;
			for (int j=0; j<=7; ++j)
			{
				if ((1<<j) & microCodeList[i].platformId)
				{
					if (flag) 
					{
						printf(",", j);
					}
					flag = true;
					printf("%d", j);
				}
			}
			printf("\n");
	}
	return;
}

bool loadBios()
{
	system("cls");
	printf("[1] Load bios\n\n");

	char strbuf[4000];
	printf("Drag the file here and press Enter\n");
		do 
		{
			gets(strbuf);
		} 
		while (strbuf[0] == '\0');
	

	char *filename = strbuf;
	if (filename[0] == '\"') 
	{
		++filename;
		if (filename[strlen(filename)-1] == '\"') 
		{
			filename[strlen(filename)-1] = '\0';
		}
	}
	
	FILE *fp,*logFp;
	if ((fp=fopen(filename, "rb")) == NULL) 
	{
		system("cls");
		printf("[Error] Cannot open file %s\n", filename);
		printf("Press any key to continue.\n");
		_getch();
		system("cls");
		return false;
	}

	if((logFp=fopen(BIOS_LOG_FILE,"w"))==NULL)
	{
		system("cls");
		printf("[Error] Cannot open file %s\n",BIOS_LOG_FILE);
		fclose(fp);
		printf("Press any key to continue.\n");
		_getch();
		system("cls");
		return false;
	}

	//copy bios file name to global var
	str_cpy(filename,biosFileName);

	//file processing from here
	system("cls");
	printf("Loaded bios file : %s\n",filename);
	printf("Log file wil be exported to %s\n\n",BIOS_LOG_FILE);

	//get bios size
	int fsize;
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	unsigned char *buf = new unsigned char [fsize];
	fread(buf, 1, fsize, fp);
	fclose(fp);

	//copy data to global var
	originalSize=fsize;
	original=new unsigned char [fsize];
	memcpy(original,buf,fsize);
	clone = new unsigned char [fsize];
	memcpy(clone,buf,fsize);


	fprintf(logFp,"Bios file : %s\n\n",filename);

	entryCounter=0;
	for (int i=0; i<fsize-2000; i++)
	{
		if (*(unsigned int*)(&buf[i+0]) == 1
			&& (buf[i+9]==0x20 || buf[i+9]==0x19)
			&& (buf[i+10]>=0x01 && buf[i+10]<=0x09 || buf[i+10]>=0x10 && buf[i+10]<=0x19 || buf[i+10]>=0x20 && buf[i+10]<=0x29 || buf[i+10]>=0x30 && buf[i+10]<=0x31)
			&& (buf[i+11]>=0x01 && buf[i+11]<=0x09 || buf[i+11]>=0x10 && buf[i+11]<=0x12)) 
		{
			int totalsize = *(unsigned int*)(&buf[i+32]);
			
			if (totalsize == 0) totalsize = 2048;
			if ((totalsize & 1023) != 0) continue;
			if (i+totalsize > fsize) continue;
			int j,sum=0;
			for (j=0; j<totalsize; j+=4) sum += *(unsigned int*)(&buf[i+j]);
			if (sum != 0) continue;

			entryCounter++;
			printf("Entry No.%d\t",entryCounter);

			MicroCode tmp;

			printf("CPUID=%X\t",*(unsigned int*)(&buf[i+12]));									fprintf(logFp,"CPUID=%X\t",*(unsigned int*)(&buf[i+12]));
			tmp.cpuId=*(unsigned int*)(&buf[i+12]);

			printf("Rev=%02X\t",*(unsigned int*)(&buf[i+4]));									fprintf(logFp,"Rev=%02X\t",*(unsigned int*)(&buf[i+4]));
			tmp.rev=*(unsigned int*)(&buf[i+4]);

			printf("%02X%02X/%02X/%02X  ",buf[i+9], buf[i+8], buf[i+11], buf[i+10]);			fprintf(logFp,"%02X%02X/%02X/%02X  ",buf[i+9], buf[i+8], buf[i+11], buf[i+10]);
			tmp.yf=buf[i+9];
			tmp.yl=buf[i+8];
			tmp.mm=buf[i+11];
			tmp.dd=buf[i+10];

			printf("CRC=%08X  ",*(unsigned int*)(&buf[i+16]));									fprintf(logFp,"CRC=%08X  ",*(unsigned int*)(&buf[i+16]));
			tmp.crc=*(unsigned int*)(&buf[i+16]);

			printf("datasize=%4d  ", *(unsigned int*)(&buf[i+28]));								fprintf(logFp,"datasize=%4d  ", *(unsigned int*)(&buf[i+28]));
			tmp.dataSize=*(unsigned int*)(&buf[i+28]);

			//printf("total=%4d  ",*(unsigned int*)(&buf[i+32]));								//fprintf(logFp,"total=%4d  ",*(unsigned int*)(&buf[i+32]));
			//tmp.totalSize=*(unsigned int*)(&buf[i+32]);

			//totalsize
			printf("total=%4d  ",totalsize);													fprintf(logFp,"total=%4d  ",totalsize);
			tmp.totalSize=totalsize;

			printf("offset=%-8X  ",i);															fprintf(logFp,"offset=%-8x  ",i);

			printf("PlatformID=(%3d):",*(unsigned int*)(&buf[i+24]));							fprintf(logFp,"PlatformID=(%3d):",*(unsigned int*)(&buf[i+24]));
			tmp.platformId=*(unsigned int*)(&buf[i+24]);	
			bool flag = false;
			for (int j=0; j<=7; ++j)
			{
				if ((1<<j) & *(unsigned int*)(&buf[i+24]))
				{
					if (flag) 
					{
						printf(",");															fprintf(logFp,",");
					}
					flag = true;
					printf("%d", j);															fprintf(logFp,"%d", j);
				}
			}
			printf("\n");																		fprintf(logFp,"\n");

			//save microcode data to node
			tmp.data = new unsigned char [totalsize];
			memcpy(tmp.data,buf+i,totalsize);
			tmp.status=ORIGIN;
			microCodeList[entryCounter-1]=tmp;
			editedBiosMicroCodeList[entryCounter-1]=tmp;
		}
	}	
	fclose(logFp);

	printf("\nComplete! press any key to continue.\n");
	_getch();
	system("cls");
	return true;
}

bool editBios()
{
	system("cls");
	printf("[5] Edit bios microcode\n\n");
	
	//print out the list
	printf("Current bios microcode list:\n");
	showEditedBios();
	//get the entry number to edit
	int destChoice = 0;
	do
	{
		printf("\nEnter the entry number you need edit :");
		scanf("%d",&destChoice);
	}
	while(destChoice<=0 || destChoice>entryCounter);

	printf("\n--------------------------------------------------------------------------------------------------------\n");

	//show the lib
	printf("Lib microcode list:\n");
	showLib();
	//get the entry number to insert
	int srcChoice=0;
	do
	{
		
		printf("\n\nEnter the entry number you need insert :");
		scanf("%d",&srcChoice);
	}
	while(srcChoice<=0 || srcChoice>libCounter);

	//check size (1st time)
	if(editedBiosMicroCodeList[destChoice-1].totalSize!=microCodeLib[srcChoice-1].totalSize)
	{
		printf("\n[error] Src and dest size not match ! \nOperation aborted!");
		printf("\nPress any key to continue.\n");
		_getch();
		system("cls");
		return false;
	}


	//search data
	int counter = 0;
	for (int i=0; i<originalSize-2000; i++)
	{
		if (*(unsigned int*)(&clone[i+0]) == 1
			&& (clone[i+9]==0x20 || clone[i+9]==0x19)
			&& (clone[i+10]>=0x01 && clone[i+10]<=0x09 || clone[i+10]>=0x10 && clone[i+10]<=0x19 || clone[i+10]>=0x20 && clone[i+10]<=0x29 || clone[i+10]>=0x30 && clone[i+10]<=0x31)
			&& (clone[i+11]>=0x01 && clone[i+11]<=0x09 || clone[i+11]>=0x10 && clone[i+11]<=0x12)) 
		{
			
			int totalsize = *(unsigned int*)(&clone[i+32]);
			if (totalsize == 0) totalsize = 2048;
			if ((totalsize & 1023) != 0) continue;
			if (i+totalsize > originalSize) continue;
			int j,sum=0;
			for (j=0; j<totalsize; j+=4) sum += *(unsigned int*)(&clone[i+j]);
			if (sum != 0) continue;

			counter++;

			if(
				*(unsigned int*)(&clone[i+12])==editedBiosMicroCodeList[destChoice-1].cpuId
				&&
				*(unsigned int*)(&clone[i+4])==editedBiosMicroCodeList[destChoice-1].rev
				&&
				clone[i+9]==editedBiosMicroCodeList[destChoice-1].yf
				&&
				clone[i+8]==editedBiosMicroCodeList[destChoice-1].yl
				&&
				clone[i+11]==editedBiosMicroCodeList[destChoice-1].mm
				&&
				clone[i+10]==editedBiosMicroCodeList[destChoice-1].dd
				&&
				*(unsigned int*)(&clone[i+16])==editedBiosMicroCodeList[destChoice-1].crc
				&&
				*(unsigned int*)(&clone[i+24])==editedBiosMicroCodeList[destChoice-1].platformId
				)
			{
				//database and file not match
				if(counter!=destChoice)
				{
					printf("\n[error] Database error ! \nOperation aborted!");
					printf("\nPress any key to continue.\n");
					_getch();
					system("cls");
					return false;
				}

				system("cls");
				printf("\nEntry to edit : Entry No.%d\n",counter);
				showSingleEntry(editedBiosMicroCodeList[counter-1]);
				printf("\nLib to insert : Entry No.%d\n",srcChoice);
				showSingleEntry(microCodeLib[srcChoice-1]);
				
				//check size
				if(microCodeLib[srcChoice-1].totalSize==totalsize)
				{
					//save pos
					int modPos=i;

					//modify code from here
					printf("\nWrite %d bytes to output buffer.\n",totalsize);
					memcpy(clone+i,microCodeLib[srcChoice-1].data,totalsize);

					

					//check
					printf("Checking output buffer.....\n");
					if(memcmp(clone,original,modPos)!=0)
					{
						//rollback
						memcpy(clone,original,originalSize);

						printf("Verify step 1: Error\n");
						printf("Operation aborted!");
						printf("\nPress any key to continue.\n");
						_getch();
						system("cls");
						return false;
					}
					else
					{
						printf("Verify step 1: OK\n");
					}

					if(memcmp(clone+modPos,microCodeLib[srcChoice-1].data,totalsize)!=0)
					{
						//rollback
						memcpy(clone,original,originalSize);

						printf("Verify step 2: Error\n");
						printf("Operation aborted!");
						printf("\nPress any key to continue.\n");
						_getch();
						system("cls");
						return false;
					}
					else
					{
						printf("Verify step 2: OK\n");
					}

					if(memcmp(clone+modPos+totalsize,original+modPos+totalsize,originalSize-modPos-totalsize)!=0)
					{
						//rollback
						memcpy(clone,original,originalSize);

						printf("Verify step 3: Error\n");
						printf("Operation aborted!");
						printf("\nPress any key to continue.\n");
						_getch();
						system("cls");
						return false;
					}
					else
					{
						printf("Verify step 3: OK\n");
					}
					//update current database
					updateDatabase();

					printf("\nOperation successed!\n");
					printf("\nPress any key to continue.\n");
					_getch();
					system("cls");
					return true;
				}
				else
				{
					printf("\n[error] Src and Dest size not match ! \nOperation aborted!");
					printf("\nPress any key to continue.\n");
					_getch();
					system("cls");
					return false;
				}
			}
		}
	}
	printf("\nComplete! press any key to continue.\n");
	_getch();
	system("cls");
	return true;
}

void showEditedBios()
{
	for(int i=0;i<entryCounter;i++)
	{
		printf("Entry No.%2d",i+1);
		if(editedBiosMicroCodeList[i].status==ORIGIN)
			printf(" [Origin]\t");
		else
			printf(" [Edited]\t");
		printf("CPUID=%X\t",editedBiosMicroCodeList[i].cpuId);
		printf("Rev=%02X\t",editedBiosMicroCodeList[i].rev);
		printf("%02X%02X/%02X/%02X  ",editedBiosMicroCodeList[i].yf, editedBiosMicroCodeList[i].yl, editedBiosMicroCodeList[i].mm, editedBiosMicroCodeList[i].dd);
		printf("CRC=%08X  ",editedBiosMicroCodeList[i].crc);
		printf("datasize=%4d  ", editedBiosMicroCodeList[i].dataSize);
		printf("total=%4d  ",editedBiosMicroCodeList[i].totalSize);
		printf("PlatformID=(%3d):",editedBiosMicroCodeList[i].platformId);
			bool flag = false;
			for (int j=0; j<=7; ++j)
			{
				if ((1<<j) & editedBiosMicroCodeList[i].platformId)
				{
					if (flag) 
					{
						printf(",", j);
					}
					flag = true;
					printf("%d", j);
				}
			}
			printf("\n");
			
	}
	return;
}

void updateDatabase()
{
	memcpy(original,clone,originalSize);

	int counter=0;
	for (int i=0; i<originalSize-2000; i++)
	{
		if (*(unsigned int*)(&clone[i+0]) == 1
			&& (clone[i+9]==0x20 || clone[i+9]==0x19)
			&& (clone[i+10]>=0x01 && clone[i+10]<=0x09 || clone[i+10]>=0x10 && clone[i+10]<=0x19 || clone[i+10]>=0x20 && clone[i+10]<=0x29 || clone[i+10]>=0x30 && clone[i+10]<=0x31)
			&& (clone[i+11]>=0x01 && clone[i+11]<=0x09 || clone[i+11]>=0x10 && clone[i+11]<=0x12)) 
		{
			int totalsize = *(unsigned int*)(&clone[i+32]);
			if (totalsize == 0) totalsize = 2048;
			if ((totalsize & 1023) != 0) continue;
			if (i+totalsize > originalSize) continue;
			int j,sum=0;
			for (j=0; j<totalsize; j+=4) sum += *(unsigned int*)(&clone[i+j]);
			if (sum != 0) continue;

			counter++;

			if(
				editedBiosMicroCodeList[counter-1].cpuId!=*(unsigned int*)(&clone[i+12])
				||
				editedBiosMicroCodeList[counter-1].rev!=*(unsigned int*)(&clone[i+4])
				||
				editedBiosMicroCodeList[counter-1].yf!=clone[i+9]
				||
				editedBiosMicroCodeList[counter-1].yl!=clone[i+8]
				||
				editedBiosMicroCodeList[counter-1].mm!=clone[i+11]
				||
				editedBiosMicroCodeList[counter-1].dd!=clone[i+10]
				||
				editedBiosMicroCodeList[counter-1].crc!=*(unsigned int*)(&clone[i+16])
				||
				editedBiosMicroCodeList[counter-1].dataSize!=*(unsigned int*)(&clone[i+28])
				||
				//editedBiosMicroCodeList[counter-1].totalSize!=*(unsigned int*)(&clone[i+32])
				editedBiosMicroCodeList[counter-1].totalSize!=totalsize
				||
				editedBiosMicroCodeList[counter-1].platformId!=*(unsigned int*)(&clone[i+24])
				||
				memcmp(editedBiosMicroCodeList[counter-1].data,clone+i,totalsize)!=0
				)
				{
					editedBiosMicroCodeList[counter-1].cpuId=*(unsigned int*)(&clone[i+12]);
					editedBiosMicroCodeList[counter-1].rev=*(unsigned int*)(&clone[i+4]);
					editedBiosMicroCodeList[counter-1].yf=clone[i+9];
					editedBiosMicroCodeList[counter-1].yl=clone[i+8];
					editedBiosMicroCodeList[counter-1].mm=clone[i+11];
					editedBiosMicroCodeList[counter-1].dd=clone[i+10];
					editedBiosMicroCodeList[counter-1].crc=*(unsigned int*)(&clone[i+16]);
					editedBiosMicroCodeList[counter-1].dataSize=*(unsigned int*)(&clone[i+28]);
					//editedBiosMicroCodeList[counter-1].totalSize=*(unsigned int*)(&clone[i+32]);
					editedBiosMicroCodeList[counter-1].totalSize=totalsize;
					editedBiosMicroCodeList[counter-1].platformId=*(unsigned int*)(&clone[i+24]);
					memcpy(editedBiosMicroCodeList[counter-1].data,clone+i,totalsize);
					editedBiosMicroCodeList[counter-1].status=EDITED;
				}
		}
	}	
	printf("Database update completed!\n");
	return;
}

bool loadLib()
{
	system("cls");
	printf("[3] Load microcode lib\n\n");

	char strbuf[4000];
	printf("Drag the file here and press Enter\n");
		do 
		{
			gets(strbuf);
		} 
		while (strbuf[0] == '\0');
	

	char *filename = strbuf;
	if (filename[0] == '\"') 
	{
		++filename;
		if (filename[strlen(filename)-1] == '\"') 
		{
			filename[strlen(filename)-1] = '\0';
		}
	}
	
	FILE *fp,*logFp;
	if ((fp=fopen(filename, "rb")) == NULL) 
	{
		system("cls");
		printf("[Error] Cannot open file %s\n", filename);
		printf("Press any key to continue.\n");
		_getch();
		system("cls");
		return false;
	}

	if((logFp=fopen(LIB_LOG_FILE,"w"))==NULL)
	{
		system("cls");
		printf("[Error] Cannot open file %s\n",LIB_LOG_FILE);
		fclose(fp);
		printf("Press any key to continue.\n");
		_getch();
		system("cls");
		return false;
	}

	//copy lib file name to global var
	str_cpy(filename,libFileName);

	//file processing from here
	system("cls");
	printf("Loaded microcode lib file : %s\n",filename);
	printf("Log file wil be exported to %s\n\n",LIB_LOG_FILE);

	int fsize;
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	unsigned char *buf = new unsigned char [fsize];
	fread(buf, 1, fsize, fp);
	fclose(fp);


	fprintf(logFp,"Microcode lib file : %s\n\n",filename);

	libCounter=0;
	for (int i=0; i<fsize-2000; i++)
	{
		if (*(unsigned int*)(&buf[i+0]) == 1
			&& (buf[i+9]==0x20 || buf[i+9]==0x19)
			&& (buf[i+10]>=0x01 && buf[i+10]<=0x09 || buf[i+10]>=0x10 && buf[i+10]<=0x19 || buf[i+10]>=0x20 && buf[i+10]<=0x29 || buf[i+10]>=0x30 && buf[i+10]<=0x31)
			&& (buf[i+11]>=0x01 && buf[i+11]<=0x09 || buf[i+11]>=0x10 && buf[i+11]<=0x12)) 
		{
			int totalsize = *(unsigned int*)(&buf[i+32]);
			//printf("datasize=%d totalsize=%d\n", *(unsigned int*)(&buf[i+28]), totalsize);
			if (totalsize == 0) totalsize = 2048;
			if ((totalsize & 1023) != 0) continue;
			if (i+totalsize > fsize) continue;
			int j,sum=0;
			for (j=0; j<totalsize; j+=4) sum += *(unsigned int*)(&buf[i+j]);
			if (sum != 0) continue;

			libCounter++;
			printf("Entry No.%d\t",libCounter);

			MicroCode tmp;

			printf("CPUID=%X\t",*(unsigned int*)(&buf[i+12]));									fprintf(logFp,"CPUID=%X\t",*(unsigned int*)(&buf[i+12]));
			tmp.cpuId=*(unsigned int*)(&buf[i+12]);

			printf("Rev=%02X\t",*(unsigned int*)(&buf[i+4]));									fprintf(logFp,"Rev=%02X\t",*(unsigned int*)(&buf[i+4]));
			tmp.rev=*(unsigned int*)(&buf[i+4]);

			printf("%02X%02X/%02X/%02X  ",buf[i+9], buf[i+8], buf[i+11], buf[i+10]);			fprintf(logFp,"%02X%02X/%02X/%02X  ",buf[i+9], buf[i+8], buf[i+11], buf[i+10]);
			tmp.yf=buf[i+9];
			tmp.yl=buf[i+8];
			tmp.mm=buf[i+11];
			tmp.dd=buf[i+10];

			printf("CRC=%08X  ",*(unsigned int*)(&buf[i+16]));									fprintf(logFp,"CRC=%08X  ",*(unsigned int*)(&buf[i+16]));
			tmp.crc=*(unsigned int*)(&buf[i+16]);

			printf("datasize=%4d  ", *(unsigned int*)(&buf[i+28]));								fprintf(logFp,"datasize=%4d  ", *(unsigned int*)(&buf[i+28]));
			tmp.dataSize=*(unsigned int*)(&buf[i+28]);

			//printf("total=%4d  ",*(unsigned int*)(&buf[i+32]));								//fprintf(logFp,"total=%4d  ",*(unsigned int*)(&buf[i+32]));
			//tmp.totalSize=*(unsigned int*)(&buf[i+32]);

			//totalsize
			printf("total=%4d  ",totalsize);									fprintf(logFp,"total=%4d  ",totalsize);
			tmp.totalSize=totalsize;


			printf("offset=%-8X  ",i);															fprintf(logFp,"offset=%-8x  ",i);

			printf("PlatformID=(%3d):",*(unsigned int*)(&buf[i+24]));							fprintf(logFp,"PlatformID=(%3d):",*(unsigned int*)(&buf[i+24]));
			tmp.platformId=*(unsigned int*)(&buf[i+24]);	
			bool flag = false;
			for (int j=0; j<=7; ++j)
			{
				if ((1<<j) & *(unsigned int*)(&buf[i+24]))
				{
					if (flag) 
					{
						printf(",");															fprintf(logFp,",");
					}
					flag = true;
					printf("%d", j);															fprintf(logFp,"%d", j);
				}
			}
			printf("\n");																		fprintf(logFp,"\n");

			//save microcode data to node
			tmp.data = new unsigned char [totalsize];
			memcpy(tmp.data,buf+i,totalsize);
			microCodeLib[libCounter-1]=tmp;
		}
	}	
	fclose(logFp);

	printf("\nComplete! press any key to continue.\n");
	_getch();
	system("cls");
	return true;
}

void showLib()
{
	for(int i=0;i<libCounter;i++)
	{
		printf("Entry No.%d\t",i+1);
		printf("CPUID=%X\t",microCodeLib[i].cpuId);
		printf("Rev=%02X\t",microCodeLib[i].rev);
		printf("%02X%02X/%02X/%02X  ",microCodeLib[i].yf, microCodeLib[i].yl, microCodeLib[i].mm, microCodeLib[i].dd);
		printf("CRC=%08X  ",microCodeLib[i].crc);
		printf("datasize=%4d  ", microCodeLib[i].dataSize);
		printf("total=%4d  ",microCodeLib[i].totalSize);
		printf("PlatformID=(%3d):",microCodeLib[i].platformId);
			bool flag = false;
			for (int j=0; j<=7; ++j)
			{
				if ((1<<j) & microCodeLib[i].platformId)
				{
					if (flag) 
					{
						printf(",", j);
					}
					flag = true;
					printf("%d", j);
				}
			}
			printf("\n");	
	}
	return;
}

void showSingleEntry(MicroCode microCode)
{
	printf("CPUID=%X\t",microCode.cpuId);
	printf("Rev=%02X\t",microCode.rev);
	printf("%02X%02X/%02X/%02X  ",microCode.yf, microCode.yl, microCode.mm, microCode.dd);
	printf("CRC=%08X  ",microCode.crc);
	printf("datasize=%d  ", microCode.dataSize);
	printf("total=%d  ",microCode.totalSize);
	printf("PlatformID=(%3d):",microCode.platformId);
	bool flag = false;
	for (int j=0; j<=7; ++j)
	{
		if ((1<<j) & microCode.platformId)
		{
			if (flag) 
			{
				printf(",", j);
			}
			flag = true;
			printf("%d", j);
		}
	}
	printf("\n");
}

bool exportBios()
{
	system("cls");
	printf("[7] Export bios\n\n");

	//write data to output file
	char outputFileName[256];
	printf("Enter output file name : ");
	scanf("%s",outputFileName);

	FILE *outputWriter;
	outputWriter=fopen(outputFileName,"wb");
	if(outputFileName==NULL)
	{
		printf("\n[error] Cannot create output file! \nOperation aborted!");
		printf("\nPress any key to continue.\n");
		_getch();
		system("cls");
		return false;
	}
	printf("\nWrite %d bytes to output file : %s\n",originalSize,outputFileName);
	fwrite (clone ,1, originalSize, outputWriter);
	fclose(outputWriter);
	
	printf("\nComplete! press any key to continue.\n");
	_getch();
	system("cls");
	return true;
}

int str_cpy(char* src,char* dest)
{
	int i=0;
	for(i=0;src[i]!='\0';i++)
		dest[i]=src[i];
	dest[i]='\0';
	return i;
}

//Main
int main(int argc,char *argv[])
{
	//TOS here

	//main menu
	do showMenu();
	while(con);
	return 0;
}


