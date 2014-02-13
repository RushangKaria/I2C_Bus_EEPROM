/* ----------------------------------------------- DRIVER I2C_DEV --------------------------------------------------
						   VERSION 2.1						
-----------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------

DRIVER AUTHOR=RUSHANG KARIA
	      ASU # 1206337661
----------------------------------------------------------------------------------------------------------------*/

/* ========================================== DRIVER INFORMATION ===============================================

TIMER DRIVER

================================================================================================================*/

#include<linux/module.h>
#include<linux/init.h>
#include<linux/jiffies.h>
#include<linux/kernel.h>
#include<linux/types.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<asm/uaccess.h>
#include<linux/moduleparam.h>
#include<linux/slab.h>
#include<asm/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include<linux/idr.h>
#include<linux/delay.h>

//#include<linux/kdev_t.h>
//--------------------------DECLARATIONS AND DEFINITIONS---------------------------------
#define DEVICE_NAME "i2c-flash" 			//the device will appear as /dev/I2C-FLASH
#define DEVICE_CLASS "i2c-drivers" 		//the device class as shown in /sys/class
#define DEVICES 1
#define MINOR_NUMBER 0

#define START_ADDRESS 0x00
#define LAST_PAGE 511
#define EEPROM_PAGE_SIZE 0x40

int CURRENT_ADDRESS=START_ADDRESS;
int CURRENT_PAGE_NO=0;

int MAJOR_NUMBER;
struct i2c_client *CLIENT;

struct cdev cdev_info;
static dev_t device_no;
static struct class *class_data;

struct i2c_adapter *adapter;


static int __init I2C_DEV_init(void);		//function declarations
static void __exit I2C_DEV_exit(void);

static int I2C_DEV_open(struct inode *,struct file *);
static int I2C_DEV_release(struct inode *,struct file *);

static ssize_t I2C_DEV_ioctl(struct file *,unsigned int, unsigned long);

static ssize_t I2C_DEV_read(struct file *,char *, size_t, loff_t *);
static ssize_t I2C_DEV_write(struct file *, const char *, size_t, loff_t *);
static loff_t I2C_DEV_llseek(struct file *,loff_t,int);
//=======================================================================================

//---------------------------IMPORTANT DRIVER RELATED DECLARATIONS ----------------------
static struct file_operations fops =
{
	.owner=THIS_MODULE,
	.open=I2C_DEV_open,
	.release=I2C_DEV_release,
	.llseek=I2C_DEV_llseek,
	.read=I2C_DEV_read,
	.write=I2C_DEV_write
};
//=======================================================================================

/*-----------------------------------------------------------------------------------------------------------------------------------------------------*/


static int __init I2C_DEV_init(void)
{
	int result=1;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		registration:
		{		
		result=alloc_chrdev_region(&device_no,MINOR_NUMBER,DEVICES,DEVICE_NAME);
		
				if(!result)
				{
				MAJOR_NUMBER=MAJOR(device_no);
				//printk(KERN_DEBUG "Device Registration complete with Major,Minor pair (%d,%d)\n",MAJOR_NUMBER,MINOR_NUMBER);
				}	
				else
				{
				goto RETURN_ERROR; //ERROR cant register
				}	
				
		}//if code reaches here that means that registering was successful

		class_creation:
		{
			if((class_data=class_create(THIS_MODULE,DEVICE_CLASS))==NULL)
			{
			goto UNREGISTER;
			}
		}//if code reaches here that means that class creation was successful

		device_creation:
		{					
			if(device_create(class_data,NULL,device_no,NULL,DEVICE_NAME)==NULL)
			{
			goto CLASS_DESTROY;
			}
		}

///////////////////////////////////////////////////// GET THE CLIENT TO BE GLOBAL SINCE IT IS CONVINIENT /////////////////////////////////////////////////////
		client_initialization:
		{

				adapter = i2c_get_adapter(2);

					if(adapter==NULL)
					{
					printk(KERN_ALERT " ADAPTER FAILED");
					goto DEVICE_DESTROY;
					}

		//	module_put(adapter->owner);

			CLIENT=kzalloc(sizeof(*CLIENT),GFP_KERNEL);
			CLIENT->addr=0x52;	
			CLIENT->adapter=adapter;

					snprintf(CLIENT->name,I2C_NAME_SIZE,"i2c_flash_client");
		}

///////////////////////////////////////////////////// INITIALIZE THE EEPROM TO ALL X /////////////////////////////////////////////////////

		EEPROM_INITIALIZATION:
		{
		int address;
		int current_page_no=0;

					char *x=kmalloc(66,GFP_KERNEL);
					memset(x,'X',66);

				while(current_page_no<512)
				{
				address=START_ADDRESS+(current_page_no)*(0x40);
					
								x[0]=address >> 8;
								x[1]=address & 0xff;

				i2c_master_send(CLIENT,x,66);
				memset(x,'X',66);
				msleep(20);
				current_page_no++;
						
				}
						kfree(x);
		}

		cdev_initialization:
		{

			
			cdev_init(&cdev_info,&fops);

				if(cdev_add(&cdev_info,device_no,1)<0)
				{
				goto DEVICE_DESTROY;
				}

		}

		success:
		{			
			MAJOR_NUMBER=MAJOR(device_no); //get the final major no used by device
			printk(KERN_DEBUG "Device Registration complete with Major,Minor pair (%d,%d)\n",MAJOR_NUMBER,MINOR_NUMBER);
		return 0;
		}
			
///////////////////////////////////////////////////////// ERROR TABLE //////////////////////////////////////////////////////////////////////////////////////////
//THIS TABLE MATCHES THE ORDER IN WHICH THE THINGS SHOULD BE ROLLED BACK. EXAMPLE IF CLASS CREATION FAILS...ROLLBACK ALL THINGS WHICH WERE SUCCESSFUL BEFORE IT
	DEVICE_DESTROY:	device_destroy(class_data,device_no);
	CLASS_DESTROY:	class_destroy(class_data);
	UNREGISTER:	unregister_chrdev_region(device_no,DEVICES);
	PRINT_ERROR:	printk(KERN_ALERT "Device could not be registered...Please reboot and try again\n");
	RETURN_ERROR:	return -1;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}


static void __exit I2C_DEV_exit(void)
{
int x;
	kfree(CLIENT);
	cdev_del(&cdev_info);
	device_destroy(class_data,device_no);
	class_destroy(class_data);
	unregister_chrdev_region(device_no,DEVICES);

	printk(KERN_ALERT "Bye \n");
	return;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------*/
static int I2C_DEV_open(struct inode *node,struct file *filp)
{

	printk(KERN_ALERT "DRIVER OPENED\n");

	return 0;
}

static int I2C_DEV_release(struct inode *node,struct file *filp)
{
	printk(KERN_ALERT "DRIVER RELEASED\n");

	return 0;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------*/
static ssize_t I2C_DEV_read(struct file *filp,char *buff,size_t length,loff_t *off)	//same as task1 except for copying to and from user space
{
//printk(KERN_ALERT "Inside read \n");

int i;
				if((int)length>512 || (int)length<=0)
				return -1;

//printk(KERN_ALERT "The no of pages that will be read is %d\n",(int)length);

int result,BYTES_TO_READ;
int loops=1;
char *temp=NULL,*main=NULL;
BYTES_TO_READ=(int)length*64;

main=kmalloc(BYTES_TO_READ,GFP_KERNEL);
memset(main,0,BYTES_TO_READ);

					if(BYTES_TO_READ>8192)
					{
					loops=BYTES_TO_READ/8192;
					loops=BYTES_TO_READ%8192==0?loops:loops+1;
					temp=kmalloc(8192,GFP_KERNEL);
					memset(temp,0,8192);
					}

result=i2c_master_recv(CLIENT,main,BYTES_TO_READ);
BYTES_TO_READ-=result;
loops--;
msleep(100);

			while(loops-- >0)
			{
			result=i2c_master_recv(CLIENT,temp,BYTES_TO_READ);
				if(result==-1)
				{
				kfree(temp);
				kfree(main);
				return -1;
				}
			BYTES_TO_READ-=result;

			strcat(main,temp);
				memset(temp,0,8192);
			msleep(100);
			
			}


			copy_to_user(buff,main,(int)(length*64));

//	printk(KERN_ALERT "The String was %s\n",main);
if(temp!=NULL)
kfree(temp);
kfree(main);


CURRENT_ADDRESS+=(int)(length)*(0x40);

return 0;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------*/

static ssize_t I2C_DEV_write(struct file *filp,const char *buff,size_t length,loff_t *off)
{
				if((int)length>512 || (int)length<=0)
				return -1;

int result,j,total=0;
int size=0;
int address,index=0,start=2;
int current_page_no=0;
int count=(int)length;
printk(KERN_ALERT "The string length of the buffer is %d\n",strlen(buff));
printk(KERN_ALERT "The no of pages will be %d\n",count);

char *buffer=kmalloc((int)(length*64),GFP_KERNEL);
copy_from_user(buffer,buff,(count*64));

//printk(KERN_ALERT "The string to be copied is %s and  the length is %d\n",buffer,strlen(buffer));

int pages_required;

		if(strlen(buffer)<65)
		{
		pages_required=1;
		}
		else
		{
		pages_required=strlen(buffer)/64;
		pages_required=strlen(buffer)%64==0?pages_required:pages_required+1;
		}



					char *x=kmalloc(66,GFP_KERNEL);
					address=CURRENT_ADDRESS;	

							while((pages_required--)>0 && count -- >0)
							{

							current_page_no=(address-START_ADDRESS)/0x40;
		
										if(current_page_no>=512)
										{
										CURRENT_ADDRESS=START_ADDRESS;

										address=CURRENT_ADDRESS;
										current_page_no=0;
										}

								*(x+0)=address >> 8;
								*(x+1)=address & 0xff;

								
														while(index<strlen(buffer))
														{
																if(size+2>65)
																break;

																*(x+start++)=*(buffer+index++);

																//printf("Index = %d element at x  is %c  ",index,x[start-1]);

														size++;
														}

/*
							printk(KERN_ALERT "========== DEBUGGING ==========\n");
							printk(KERN_ALERT "String to write\n");
							
							j=2;
							while(j<66 && j <strlen(buffer))
							printk(KERN_ALERT "%c",*(x+j++));
							printk(KERN_ALERT "\nWriting at address %x%x\n",x[0],x[1]);
							printk(KERN_ALERT "The page no for this page is %d\n",current_page_no);
							printk(KERN_ALERT "Writing %d bytes\n",size);
							printk(KERN_ALERT "===============================\n");
*/

												#ifdef __arm__
												result=i2c_master_send(CLIENT,x,(size+2));
												#endif


								address+=0x40;

											#ifdef __arm__
											if(result==-1)
											{
											kfree(x);
											kfree(buffer);
											return -1;
											}
											else
											total+=size;
											#endif

								//printk(KERN_ALERT "new address is %x\n",address);

											msleep(100);
											memset(x,0,66);
											size=0;
											start=2;

										//printk(KERN_ALERT "The value of result is %d\n",result);


						}

CURRENT_ADDRESS=address;

							if((CURRENT_ADDRESS-START_ADDRESS)/0x40>=511)
							CURRENT_ADDRESS=START_ADDRESS;
char *p=kmalloc(2,GFP_KERNEL);
*(p+0)=CURRENT_ADDRESS >> 8;
*(p+1)=CURRENT_ADDRESS & 0xff;

i2c_master_send(CLIENT,p,2);


kfree(x);
kfree(p);
kfree(buffer);

return 0;
//return total;

}

static loff_t I2C_DEV_llseek(struct file  *filp,loff_t offset,int whence)
{

//	printk(KERN_ALERT "SEEK\n");
int result;

	if((int)offset>511 || (int)offset <0)
	{
	//printk(KERN_ALERT "SEEK FALSE\n");
	return -1;
	}
	else
 CURRENT_ADDRESS=START_ADDRESS+(int)(offset)*0x40;

char *x=kmalloc(2,GFP_KERNEL);
*(x+0)=CURRENT_ADDRESS >> 8;
*(x+1)=CURRENT_ADDRESS & 0xff;


//	printk(KERN_ALERT "CURRENT_ADDRESS IS NOW %x%x\n",x[0],x[1]);
#ifdef __arm__
result=i2c_master_send(CLIENT,x,2);
#endif

kfree(x);
return result>0?0:-1;
//return offset;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------*/

//*************************
module_init(I2C_DEV_init);
module_exit(I2C_DEV_exit);
//*************************

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MODULE_LICENSE("GPL");
MODULE_AUTHOR("RUSHANG KARIA {ASU # 1206337661}");
MODULE_DESCRIPTION("Timing module!!!");
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


