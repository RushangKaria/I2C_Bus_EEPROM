#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<time.h>
#include<unistd.h>
#define SIZE 512*64


int main()
{
	int file;
	int result=-1;
	loff_t value=2;

int x,choice;
char buffer[SIZE+200];
char tt[SIZE+200];

				
		
	printf("PLEASE WAIT....INITIALIZING THE DRIVER\n");

		file=open("/dev/i2c-flash",O_RDWR);

				if(file==-1)
				{
				printf("DRIVER INITIALIZATION FAILED \n");
				return -1;
				}

			do
			{

				printf("-----------------MENU---------------------\n");
				printf("1. SEEK\n");
				printf("2. READ\n");
				printf("3. WRITE\n");
				printf("4. EXIT\n");
				printf("ENTER YOUR OPTION\n");

					scanf("%d",&choice);

						switch(choice)
						{
							case 1:	printf("To which page do you want to seek [0-511]\n");

								scanf("%d",&x);
								//printf("X is %d\n",x);

								value=x;

									result=lseek(file,value,0);

										if(result==0)
										printf("SEEK SUCESSFUL\n");
										else
										printf("SEEK FAILED\n");
							break;

							case 2:	printf("How many pages do you want to read\n");
								scanf("%d",&x);

										
				
									result=read(file,buffer,x);

										if(result==0)
										{

											int i,j,index=0;
														memcpy(tt,buffer,(x*64));
														printf("========== RESULT OF READ OPERATION ===========\n");
														printf("*********ALL PAGES RELATIVE TO THE CURRENT PAGE NUMBER********\n");
													
														for(i=0;i<x;i++)
														{
														printf("PAGE %d -->",i);
																
																for(j=0;j<64;j++)
																printf("%c",tt[index++]);

														printf("\n");
														}	
											
														memset(buffer,0,SIZE+200);
														memset(tt,0,SIZE+200);
										}
										else
										printf("FAILED TO READ\n");

										//free(buffer);
								
							break;
			
							case 3:	printf("How many pages do you want to write\n");

											scanf("%d",&x);
		
										char temp[10000];

										int randomness;

												printf("Do you want to randomly generate a string (not guaranteed to fit page size) or do you want to enter one....PRESS 1 to randomly generate a string\n");
												scanf("%d",&randomness);

																if(randomness==1)
																{
																	int length=rand()%((x*64)+1);
																	int i;
																			for(i=0;i<length;i++)
																			temp[i]=rand()%3<2?rand()%26+'A':rand()%10+'0';

																	printf("Here is the random string \n%s\n",temp);
																	sleep(2);			
																}
																else
																{

																printf("Please enter the string that you want to write\n");
																scanf("%s",temp);
																printf("\n");
																}


								if(strlen(temp)<x*64 || strlen(temp)>x*64)
								printf("WARNING YOU HAVE PROVIDED LESS/MORE DATA....SOME DATA IN THE PAGE MAY BE GARBAGE/NOT WRITTEN\n");
								sleep(2);
									
									result=write(file,temp,x);

										if(result==0)
										printf("WRITE SUCESSFUL\n");
										else
										printf("FAILED TO WRITE\n");
						
							break;

							default:
							break;
						}
			}
			while(choice !=4);

		close(file);
return 0;
}