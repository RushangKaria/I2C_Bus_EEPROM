#include<stdio.h>
#include<stdlib.h>
#include<sys/ioctl.h>
#include<linux/i2c-dev.h>
#include<fcntl.h>

#define I2C_DEVICE "/dev/i2c-2"			//driver to be opened
#define ADDRESS 0x52					//EEPROM address
#define START_ADDRESS 0x0000		//start address
#define PAGE_SIZE 0x40					
int current_address=START_ADDRESS;

int open_device()
{
int descriptor=open(I2C_DEVICE,O_RDWR);
ioctl(descriptor,I2C_SLAVE_FORCE,ADDRESS);

return descriptor;
}

int read_EEPROM(void*buffer,int count)
{
int i;
				if(count>512 || count <=0)
				return -1;


int descriptor=open(I2C_DEVICE,O_RDWR);
ioctl(descriptor,I2C_SLAVE_FORCE,ADDRESS);

int result,BYTES_TO_READ;
int loops=1;
char *temp=NULL;
BYTES_TO_READ=count*64;			

					if(BYTES_TO_READ>8192)			//to handle requests of many bytes...since they are restricted to 8192 in i2c.....so now  you can read infinite bytes (provided you have memory)
					{
					loops=BYTES_TO_READ/8192;							
					loops=BYTES_TO_READ%8192==0?loops:loops+1;
					temp=malloc(8192);
					memset(temp,0,8192);
					}

result=read(descriptor,buffer,BYTES_TO_READ);
BYTES_TO_READ-=result;
loops--;
usleep(100);

			while(loops-- >0)		//if no of bytes are >8192
			{
			result=read(descriptor,temp,BYTES_TO_READ);
				if(result==-1)
				return -1;
			BYTES_TO_READ-=result;

			strcat(buffer,temp);
				memset(temp,0,8192);
			usleep(100);
			
			}


//printf("The value of result is %d\n",result);
//	printf("The String was %s\n",temp);

if(temp!=NULL)
free(temp);

current_address+=(count)*(0x40);		//update the address but dont write to eeprom since eeprom automatically does that for read and not write
return 0;
}

int initialize()
{
#ifdef __arm__
int descriptor=open(I2C_DEVICE,O_RDWR);
ioctl(descriptor,I2C_SLAVE_FORCE,ADDRESS);
#endif

	int address;
		int current_page_no=0;

char x[66];
memset(x,'X',66);

				while(current_page_no<512)									//for all pages write X
				{
				address=START_ADDRESS+(current_page_no)*(0x40);
					
								x[0]=address >> 8;
								x[1]=address & 0xff;

				write(descriptor,x,66);
				memset(x,'X',66);
						//printf("Current page no is %d\n",current_page_no);			
				current_page_no++;



				usleep(10000);
						
				}
return 0;

}

int byte_EEPROM(void *c,int page,int position) //NOTE WILL NOT UPDATE THE ADDRESS COUNTER
{


		if(page > 511 || page < 0 || position > 63)			//invalid count
		return -1;

#ifdef __arm__
int descriptor=open(I2C_DEVICE,O_RDWR);
ioctl(descriptor,I2C_SLAVE_FORCE,ADDRESS);
#endif
char *buff=(char *)c;

	int address=START_ADDRESS+page*(0x40);

char *x=malloc(3);
*(x+0)=address >> 8;
address+=postion*(0x01);
*(x+1)=address & 0xff;
*(x+2)=*(buff+0);

//printf("The char to be written is %c\n",*(x+2));

int result=write(descriptor,x,3);

//printf("Result is %d\n",result);
free(x);

return result>0?0:-1;
}

int write_EEPROM(void *buf,int count)
{

		if(count > 511 || count <= 0)			//invalid count
		return -1;

//printf("INSIDE WRITE\n");
#ifdef __arm__
int descriptor=open(I2C_DEVICE,O_RDWR);
ioctl(descriptor,I2C_SLAVE_FORCE,ADDRESS);
#endif

int result,j,total=0;
int size=0;
int address,index=0,start=2;
int current_page_no=0;

char *buffer=(char *)buf;

int pages_required;

//printf("tne string length of buffers is %d\n",strlen(buffers));

		if(strlen(buffer)<65)			//determine the stopping condition......even if user provides less or greater length than that of pages, the program will automatically truncate them
		pages_required=1;
		else
		{
		pages_required=strlen(buffer)/64;
		pages_required=strlen(buffer)%64==0?pages_required:pages_required+1;
		}
//printf("the no of pages required are  %d\n",pages_required);

					char *x=malloc(66);
					address=current_address;

							while((pages_required--)>0 && count -- >0)		//either the size of input was less than the no of pages to write to, or the size was more respectively
							{
							current_page_no=(address-START_ADDRESS)/0x40;
		
										if(current_page_no==512)
										{
										current_address=START_ADDRESS;
										address=current_address;
										current_page_no=(address-START_ADDRESS)/0x40;	
										}

								x[0]=address >> 8;
								x[1]=address & 0xff;

														while(index<strlen(buffer))	//to prepare for writing
														{
																if(size+2>65)
																break;

																*(x+start++)=*(buffer+index++);

																//printf("Index = %d element at x  is %c  ",index,x[start-1]);

														size++;
														}

/*
							printf("========== DEBUGGING ==========\n");
							printf("String to write\n");
							
							j=2;
							while(j<66 && j <strlen(buffer))
							printf("%c",*(x+j++));
							printf("\nWriting at address %x%x\n",x[0],x[1]);
							printf("The page no for this page is %d\n",current_page_no);
							printf("Writing %d bytes\n",size);
							printf("===============================\n");
*/

												#ifdef __arm__
												result=write(descriptor,x,(size+2));	//send length+2 address bytes
												#endif


								address+=0x40;

											#ifdef __arm__
											if(result==-1)
											return -1;
											else
											total+=size;
											#endif

								//printf("new address is %x\n",address);

											usleep(500000);			//sleep to allow eeprom to catch up and reset and restart 
											memset(x,0,66);
											size=0;
											start=2;

										//printf("The value of result is %d\n",result);


						}

current_address=address;			//update the address since eeprom will not automatically do it

							if((current_address-START_ADDRESS)/0x40>=511)
							current_address=START_ADDRESS;
char *p=malloc(2);
*(p+0)=current_address >> 8;
*(p+1)=current_address & 0xff;

write(descriptor,p,2);
free(p);				//free allocated memory
free(x);
//return total;
return 0;
}

int seek_EEPROM(int offset)
{
			if(offset>=512 || offset <0)
			return -1;

int result;
#ifdef __arm__
int descriptor=open(I2C_DEVICE,O_RDWR);
ioctl(descriptor,I2C_SLAVE_FORCE,ADDRESS);
#endif

	current_address=START_ADDRESS+(offset)*0x40;

char *x=malloc(2);
*(x+0)=current_address >> 8;
*(x+1)=current_address & 0xff;

#ifdef __arm__
result=write(descriptor,x,2);
#endif

//printf("Current address set to %x%x\n",x[0],x[1]);
	
free(x);
return 0;
}
