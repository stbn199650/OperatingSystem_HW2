#include "master.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<dirent.h>

int slave_num;
#define SIZE 1024
int main(int argc, char **argv)
{
	/* write something or nothing*/
	struct mail_t mail,receive_mail;
	int dir_num = 0, file_num = 0;
	char file[SIZE][SIZE], directory[SIZE][SIZE];
	//char *data="hello world";

	//open for reading and writing
	int fd=open("/sys/kernel/hw2/mailbox",O_RDWR|O_CREAT);
	if(fd == -1) {
		printf("File is not exist %d\n",fd);
		exit(-1);
	}

	//printf("argc=%d\n",argc);
	if(argc == 5) {     //slave = 1

		slave_num = 1;
		if(strcmp(argv[1],"-q") == 0) { //if argv[1]=="-q",then argv[3]=="-d"
			strcpy(mail.data.query_word,argv[2]);
			strcpy(mail.file_path,argv[4]);
		} else if(strcmp(argv[1],"-d") == 0) { //if argv[1]=="-d", then argv[3]=="-q"
			strcpy(mail.file_path,argv[2]);
			strcpy(mail.data.query_word,argv[4]);
		}
	} else if(argc == 7) {
		if(strcmp(argv[1],"-q") == 0) { //argv[1]=="-q"
			if(strcmp(argv[3],"-d") == 0) { //argv[3]=="-d"=>-q -d -s
				strcpy(mail.data.query_word,argv[2]);
				strcpy(mail.file_path,argv[4]);
				slave_num=atoi(argv[6]);
			} else if(strcmp(argv[3],"-s") == 0) {   //argv[3]=="-s"=>-q -s -d
				strcpy(mail.data.query_word,argv[2]);
				slave_num=atoi(argv[4]);
				strcpy(mail.file_path,argv[6]);
			}
		} else if(strcmp(argv[1],"-d") == 0) { //argv[1]=="-d"
			if(strcmp(argv[3],"-q") == 0) { //argv[3]=="-q"=>-d -q -s
				strcpy(mail.file_path,argv[2]);
				strcpy(mail.data.query_word,argv[4]);
				slave_num=atoi(argv[6]);
			} else if(strcmp(argv[3],"-s") == 0) {   //argv[3]=="-s"=>-d -s -q
				strcpy(mail.file_path,argv[2]);
				slave_num=atoi(argv[4]);
				strcpy(mail.data.query_word,argv[6]);
			}
		} else if(strcmp(argv[1],"-s") == 0) { //argv[1]=="-s"
			if(strcmp(argv[3],"-q") == 0) { //argv[3]=="-q"=>-s -q -d
				strcpy(mail.file_path,argv[6]);
				strcpy(mail.data.query_word,argv[4]);
				slave_num=atoi(argv[2]);
			} else if(strcmp(argv[3],"-d") == 0) {   //argv[3]=="-d"=>-s -d -q
				strcpy(mail.file_path,argv[4]);
				slave_num=atoi(argv[2]);
				strcpy(mail.data.query_word,argv[6]);
			}
		}
	}
	//printf("query word: %s | ",mail.data.query_word);
	//printf("filepath: %s | ",mail.file_path);
	//printf("slave num: %d\n",slave_num);

	char dot='.';
	DIR *dir, *dir2;
	struct dirent *ent = NULL;
	struct dirent *ent2 = NULL;
	int i = 0, j = 0, total = 0;
	char name[1024];

	//filepath include /home/ =>absolute path
	if(strstr(mail.file_path,"/home/") != NULL) {
		//puts("absolute path");
		sprintf(name,"%c%s",dot,mail.file_path);
	} else {
		//puts("relative path");
		//concatenate file name with '.'
		sprintf(name,"%c%s",dot,mail.file_path);
		strcpy(mail.file_path,name);
	}

	dir=opendir(mail.file_path);
	if(!dir) {
		puts("No such filepath");
		exit(1);
	}
	pid_t pid[1024];
	//get directory under file path
	while((ent=readdir(dir)) != NULL) {
		//printf("in readdir %s\n",ent->d_name);
		if(strstr(ent->d_name,".") == NULL) { //filename is not . or ..
			sprintf(directory[i],"%s%s",name,ent->d_name);
			//printf("dir path: %s\n",directory[i]);
			dir2=opendir(directory[i]);

			while((ent2=readdir(dir2)) != NULL) {
				//filename is not . or ..
				if(strstr(ent2->d_name,".") == NULL) {
					sprintf(mail.file_path,"%s/%s",directory[i],ent2->d_name);
					//printf("path %s\n",mail.file_path);
					send_to_fd(fd, &mail);  //send data

					/*---------fork & exec child process-----------*/
					//pid_t pid = fork();
					pid[j] = fork();
					if(pid[j] < 0) {
						puts("Fork Failed");
						exit(1);
					} else if(pid[j]==0) {  //fork child process to kill process easily
						execvp("./slave",NULL); //execute child process
					} else { //fork parent process
						wait(NULL);
					}

					int status=0;
					pid[j]=wait(&status);      //wait until slave finish
					//printf("\n**back to master**\n");
					receive_from_fd(fd,&receive_mail);  //receive data from slave

					total+=receive_mail.data.word_count;
					pid[j]=wait(NULL);  //kill child process
					j++;
					file_num=j;
				}
			}
			i++;
			dir_num=i;
		}
	}
	printf("The total number of query word %s is %d\n",mail.data.query_word,total);

	close(fd);
	return 0;
}

int send_to_fd(int sysfs_fd, struct mail_t *mail)
{
	/* write something or nothing */
	char temp,str[1100];
	//printf("[master send] file path %s\n",mail->file_path);

	sprintf(str,"%s\t%s",mail->data.query_word,mail->file_path);
	int ret_val = write(sysfs_fd, &str, strlen(str)+10);

	if (ret_val == ERR_FULL) {
		/* write something or nothing */
	} else {
		/* write something or nothing */
	}

	/* write something or nothing */

	return ret_val;
}

int receive_from_fd(int sysfs_fd, struct mail_t *mail)
{
	/* write something or nothing */
	char str[1100], *temp[3];

	int fd=open("/sys/kernel/hw2/mailbox",O_RDWR);
	if(fd == -1) {
		printf("File is not exist %d\n",fd);
		exit(-1);
	}

	int ret_val = read(fd, str, strlen(str)+10);
	//printf("master receive from slave %s\n",str);

	//split string into file path and word count
	char re[32];
	*temp = strtok(str,"\t");
	*(temp+1)= strtok(NULL,"\t");
	*(temp+2)= strtok(NULL," ");
	//printf("temp1 %s\ temp2 %s\ temp3 %s\n ",*(temp),*(temp+1),*(temp+2));
	sprintf(mail->file_path,"%s",*temp);
	sprintf(mail->data.query_word,"%s",*(temp+2));
	//word_count type is unsigned long,strtoul could convert string to unsigned long
	mail->data.word_count=strtoul(*(temp+1),NULL,10);

	if (ret_val == ERR_EMPTY) {
		/* write something or nothing */
	} else {
		/* write something or nothing */
	}

	/* write something or nothing */
	close(fd);
	return ret_val;
}
