#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

void wait_exit()
{
	printf("\n終了します。何かキーを入力して下さい。\n");
	while (!_kbhit());
	while (_kbhit()) _getch();
	exit(0);
}

int main(int argc, char *argv[])
{
	printf("Intel Microcode List ver0.4\n\n");

	char strbuf[4000];
	if (argc == 2) 	{
		strcpy(strbuf, argv[1]);
	}
	else {
		printf("ファイルをここにドラッグ＆ドロップして、Enterキーを押して下さい。\n");
		do {
			gets(strbuf);
		} while (strbuf[0] == '\0');
	}

	char *filename = strbuf;
	if (filename[0] == '\"') {
		++filename;
		if (filename[strlen(filename)-1] == '\"') {
			filename[strlen(filename)-1] = '\0';
		}
	}
	
	FILE *fp;
	if ((fp=fopen(filename, "rb")) == NULL) {
		printf("エラー：ファイルを開けません %s\n", filename);
		wait_exit();
	}

	int fsize;
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	unsigned char *buf = new unsigned char [fsize];
	fread(buf, 1, fsize, fp);

	fclose(fp);

	int i;
	for (i=0; i<fsize-2000; i++)
	{
		if (
			   *(unsigned int*)(&buf[i+0]) == 1
			&& (buf[i+9]==0x20 || buf[i+9]==0x19)
			&& (buf[i+10]>=0x01 && buf[i+10]<=0x09 || buf[i+10]>=0x10 && buf[i+10]<=0x19 || buf[i+10]>=0x20 && buf[i+10]<=0x29 || buf[i+10]>=0x30 && buf[i+10]<=0x31)
			&& (buf[i+11]>=0x01 && buf[i+11]<=0x09 || buf[i+11]>=0x10 && buf[i+11]<=0x12)
		) {
			
			int totalsize = *(unsigned int*)(&buf[i+32]);
			//printf("   datasize=%d totalsize=%d\n", *(unsigned int*)(&buf[i+28]), totalsize);
			if (totalsize == 0) totalsize = 2048;
			if ((totalsize & 1023) != 0) continue;
			if (i+totalsize > fsize) continue;
			int j,sum=0;
			for (j=0; j<totalsize; j+=4) sum += *(unsigned int*)(&buf[i+j]);
			if (sum != 0) continue;
			
			printf("CPUID=%X UpdateRev=%02X %02X%02X/%02X/%02X Checksum=%08X PlatformID=", *(unsigned int*)(&buf[i+12]), *(unsigned int*)(&buf[i+4]), buf[i+9], buf[i+8], buf[i+11], buf[i+10], *(unsigned int*)(&buf[i+16]));

			bool flag = false;
			for (int j=0; j<=7; ++j)
			{
				if ((1<<j) & *(unsigned int*)(&buf[i+24]))
				{
					if (flag) {
						printf(",", j);
					}
					flag = true;
					printf("%d", j);
				}
			}

			printf("\n");

		}
	}	
	wait_exit();
}
