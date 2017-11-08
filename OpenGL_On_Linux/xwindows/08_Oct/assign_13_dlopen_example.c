#include<stdio.h>
#include<stdlib.h>
#include<dlfcn.h>


typedef int (*sum)(int,int);
void print_help(char * exe_name){
	printf("%s <number> <number>\n",exe_name);
}
int main(int argc, char *argv[]){
	void *lib_handle=NULL;
	sum sum_func_ptr=NULL;
	int num1,num2,sum_of_two_numbers;
	const char *lib_file_name="/home/jeevan/OpenGL/jeevangaikwadrtrassignments/OpenGL_On_Linux/xwindows/08_Oct/sample_so.so";
	const char *func_name="sum_of_two_integers";
	if(argc < 2){
		print_help(argv[0]);
		exit(EXIT_FAILURE);
	}
	num1=atoi(argv[1]);
	num2=atoi(argv[2]);

	lib_handle=dlopen(lib_file_name,RTLD_NOW);
	if(lib_handle==NULL){
		fprintf(stderr,"Unable to load %s\n",lib_file_name);
		exit(EXIT_FAILURE);
	}
	sum_func_ptr=(sum)dlsym(lib_handle,func_name);
	if(sum_func_ptr==NULL){
		fprintf(stderr,"Failed to locate %s in the %s  \n",func_name,lib_file_name);
		exit(EXIT_FAILURE);
	}else
		fprintf(stdout,"Successfully loaded %s\n",func_name);
	sum_of_two_numbers=(*sum_func_ptr)(num1,num2);
	printf("Sum of %d and %d is:%d\n",num1,num2,sum_of_two_numbers);

	dlclose(lib_handle);
	exit(EXIT_SUCCESS);
}
