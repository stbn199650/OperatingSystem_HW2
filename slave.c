#include "slave.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<dirent.h>

int count = 0;
int main(int argc, char **argv)
{
	/* write something or nothing */
	FILE *fp;
	char string[1024],keyword[32];
	struct mail_t mail;

	int fd=open("/sys/kernel/hw2/mailbox",O_RDWR);
	receive_from_fd(fd,&mail);
	//printf("fd %d| query word %s| path %s\n",fd,mail.data.query_word,mail.file_path);

	fp=fopen("./path/to/text1","r");   //open file for reading
	if(!fp) {
		puts("Open File Failed");
		exit(1);
	}
    char com[6]="oslab";
	sprintf(keyword,"%s",mail.data.query_word);
	while(fscanf(fp,"%s",string) != EOF) {  //read file
		if(strcmp(string,com) == 0) {   //the word is query_word
			count++;    //count the number of query_word
		}
	}

	//printf("word_count %d\n",count);
	sprintf(mail.data.query_word,"%s",keyword);
    printf("count %d\n",count);
	send_to_fd(fd, &mail);

	close(fd);
	return 0;
}

int send_to_fd(int sysfs_fd, struct mail_t *mail)
{
	/* write something or nothing */
	char str[1100];

	sprintf(str,"%s\t%d\t%s",mail->file_path, count, mail->data.query_word);
	//printf("[send to fd] str %s\n\n",str);

	int ret_val = write(sysfs_fd, str, strlen(str)+10);

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
	char str[1100],*temp[2];
	int ret_val = read(sysfs_fd, str, 1100);
//	printf("\nreceive: str is %s\n",str);

	//split word in str and store it into struct mail
	*temp = strtok(str,"\t");
	*(temp+1) = strtok(NULL," ");
	sprintf(mail->data.query_word,"%s",*temp);
	sprintf(mail->file_path,"%s",*(temp+1));

	//printf("query word str is %s | ",mail->data.query_word);
	//printf("file path is %s\n",mail->file_path);

	if (ret_val == ERR_EMPTY) {
		/* write something or nothing */
	} else {
		/* write something or nothing */
	}

	/* write something or nothing */
	return ret_val;
}
