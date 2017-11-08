#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef union {
	char str[20];
	int  number[5];

}DataStore;

int main(void){
	DataStore ds1;	
	//First try
	strcpy(ds1.str,"Jeevan");

	printf("First try:\nstr in union is:%s\n",ds1.str);
	ds1.number[0]=44;
	ds1.number[1]=55;
	printf("ds1.number[0]:%d ds1.number[1]:%d\n",ds1.number[0],ds1.number[1]);
	//Second try
	
	strcpy(ds1.str,"Gaikwad");

	ds1.number[2]=77;	//note the indices
	ds1.number[3]=88;
	printf("Second try:\nstr in union is:%s\n",ds1.str);
	printf("ds1.number[0]:%d ds1.number[1]:%d\n",ds1.number[0],ds1.number[1]);


	return 0;
}
