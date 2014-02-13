#include<stdio.h>
#include<stdlib.h>
#include<time.h>

//	EXTERN DECLARATIONS

extern int read_EEPROM(void *,int);
extern int write_EEPROM(void *,int);
extern int seek_EEPROM(int);
extern int open_device();
extern int initialize();
extern int byte_EEPROM(void *,int,int);

#define SIZE 512*64	//MAX PAGE_SIZE

int main()
{
srand(time(NULL));
int x,choice;
char c[10];
int page,position;
int result;
char buffer[SIZE+200];	//+200 is to fix some garbage errors
char tt[SIZE+200];

printf("INITIALIZING THE EPPROM TO ALL X\n");	
result=initialize();
printf("INITIALIZATION COMPLETE\n");

	
			do
			{

				printf("-----------------MENU---------------------\n");
				printf("1. SEEK\n");
				printf("2. READ\n");
				printf("3. WRITE\n");
				printf("4. WRITE A BYTE TO THE EEPROM AT A LOCATION (DOES NOT UPDATE ADDRESS COUNTER)\n");
				printf("5. EXIT\n");
				printf("ENTER YOUR OPTION\n");

					scanf("%d",&choice);

						switch(choice)
						{
							case 1:	printf("To which page do you want to seek [0-511]\n");

								scanf("%d",&x);

									result=seek_EEPROM(x);

										if(result==0)
										printf("SEEK SUCESSFUL\n");
										else
										printf("SEEK FAILED\n");
							break;

							case 2:	printf("How many pages do you want to read\n");
								scanf("%d",&x);

									result=read_EEPROM(buffer,x);

										if(result==0)
										{

											int i,j,index=0;
														memcpy(tt,buffer,(x*64));
														printf("========== RESULT OF READ OPERATION ===========\n");
														printf("*********ALL PAGES RELATIVE TO THE CURRENT PAGE NUMBER********\n");
													
														for(i=0;i<x;i++)						//format the output in a nice way
														{
														printf("PAGE %d -->",i);
																
																for(j=0;j<64;j++)
																printf("%c",tt[index++]);

														printf("\n");
														}	
											
														memset(buffer,0,SIZE+200);	//reset them so that another operation will not display garbage
														memset(tt,0,SIZE+200);
										}
										else
										printf("FAILED TO READ\n");
								
							break;
			
							case 3:	printf("How many pages do you want to write\n");

											scanf("%d",&x);
		
										char temp[10000];
										int randomness;

												printf("Do you want to randomly generate a string (not guaranteed to fit the page size)or do you want to enter one....\n\nPRESS 1 to randomly generate a string\n");
												scanf("%d",&randomness);

																if(randomness==1)				//if you choose to randomly generate a string
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
								{
								printf("WARNING YOU HAVE PROVIDED LESS/MORE DATA....SOME DATA IN THE PAGE MAY BE GARBAGE/NOT WRITTEN\n");
								sleep(2);
								}

									result=write_EEPROM(temp,x);

										if(result==0)
										printf("WRITE SUCESSFUL\n");
										else
										printf("WRITE FAILED\n");
							

							break;

							case 4: 
										printf("Enter the page no [0-511] at which you want to write\n");
										scanf("%d",&page);

										printf("Enter the position [0-63] for the page at which you want to write\n");
										scanf("%d",&position);

										printf("Enter the character that you want to write\n");
										scanf("%s",c);

									result=byte_EEPROM(c,page,position);

												if(result==0)
												printf("SUCESS\n");
												else
												printf("FAILED\n");
									
							default:
							break;
						}
			}
			while(choice !=5);

return 0;
}
