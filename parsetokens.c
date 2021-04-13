/*
 * BASIC parser for Sharp PV-1
 * Based on information from P&E
 * Artemio Urbina 27/01/2015
 */

#include <stdio.h>

#define TOKEN_LIST	0x1d6cd
#define TOKEN_NUM	161

struct token_st
{
	int id;
	int size;
	char data[10];
};

int main(int argc, char **argv)
{
	int i = 0, j = 0, pos = 0;
	int buff, found = 0;
	FILE *mrom;
	FILE *card;
	struct token_st tokens[TOKEN_NUM];

	mrom = NULL;
	card = NULL;
	mrom = fopen("mask rom basic.bin", "rb");
	if(!mrom)
	{
		printf("mask rom basic.bin not found\n");
		return 0;
	}	

    if(argc < 2)
    {
        fprintf(stderr, "Please specify a file for %s\n", argv[0]);
        return 0;
    }

	fseek(mrom, TOKEN_LIST, SEEK_SET);

	for(i = 0; i < TOKEN_NUM; i++)
	{
		buff = fgetc(mrom);
		if(buff == 0x00)
		{
			buff = fgetc(mrom);
			printf("+");
		}
		tokens[pos].size = buff;
		for(j = 0; j < tokens[pos].size; j++)
		{
			buff = fgetc(mrom);
			tokens[pos].data[j] = buff;
		}
		tokens[pos].data[j] = '\0';
		buff = fgetc(mrom);
		if(buff == 0x00)
		{
			buff = fgetc(mrom);
			printf("*");
		}
		tokens[pos].id = buff;
		//printf("%d ID: 0x%X DATA: %s\n", pos, tokens[pos].id, tokens[pos].data); // List tokens
		pos++;
	}
	
	fclose(mrom);

	card = fopen(argv[1], "rb");
	do
	{
		buff = fgetc(card);
		if(buff == 0xFE)
		{
			found = 0;
			buff = fgetc(card);
			for(i = 0; i < TOKEN_NUM; i++)
			{
				if(tokens[i].id == buff)
				{
					printf("%s ", tokens[i].data);
					found = 1;
					break;
				}
			}
			if(!found)
				printf("{NF:%0.2X}", buff);
		}
		else if(buff == 0x3A) // :, possible comments
		{
			buff = fgetc(card);
			if(buff != 0x27)
			{
				ungetc(buff, card);
				printf(":");
			}
			else
			{
				buff = fgetc(card);
				if(buff == 0x1E) // COMMENT
				{
					buff = fgetc(card); // length
					printf("' ");
				}	
				else
				{
					printf("{0x3A:27%0.2X", buff);
					buff = fgetc(card);
					printf("%0.2X}", buff);
				}
			}
		}
		else if(buff == 0x1D) // constants
		{
			int num[6], ammount = 0, number = 0, bnum = 2, left = 1;

			//printf("{0x1D:");
			for(j = 0; j < 7; j++)
			{
				buff = fgetc(card);
				num[j] = buff;
				//printf("%0.2X", buff);
			}
			//printf("}");
			
			ammount = num[1] + 1;
			if(ammount > 1)
			{
				for(j = 0; j < ammount; j++)
				{
					number = number << 4;
					if(left)
						number |= (num[bnum] & 0xF0) >> 4;
					else
					{
						number |= num[bnum] & 0x0F;
						bnum ++;
					}
					left = !left;
				}
				printf("%X", number);
			}
			else
				printf("%d", num[2]/0x0F);
		}
		else if(buff == 0x1E) // Assembler flag in IF statements? skipped
		{
			buff = fgetc(card);
			//printf("{0x1E:%0.2X}", buff);  // not shown
		}
		else if(buff == 0x1F)  // No longer found after parsing comments
		{
			buff = fgetc(card);
			printf("{0x1F:%0.2X}", buff);
		}
		else if(buff == 0x2A) // *, designates Subroutines. Skipped ATM.
		{
			buff = fgetc(card);
			if(buff == 0x1C || buff == 0x00)
			{
				//printf("{0x2A:%0.2X", buff); // not shown
				for(j = 0; j < 2; j++)
				{
					buff = fgetc(card);
					//printf("%0.2X", buff); // not shown
				}
				//printf("}"); // not shown
			}
			else			
				ungetc(buff, card);
			printf("*");
		}
		else
		{			
			printf("%c", buff);	
			if(buff == 0xD) // address
			{
				int line = 0;

				putchar(0xA); // show them correctly on Windows OS

				buff = fgetc(card);
				line = buff << 8;
				buff = fgetc(card);
				line |= buff;

				// length (discarded ATM)
				buff = fgetc(card);
				printf("%0.5d ", line);
			}
		}
	}while(buff != EOF);
	fclose(card);
	return 0;
}