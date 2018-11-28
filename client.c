#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>
#include<pthread.h>
#include "my_msg.h"

//需要经常修改的宏定义区域
#define MY_IP "192.168.0.26"
#define MY_PORT 7092

#define SUCCESS 1
#define FAIL -2

//结构体区域

//全局变量区域
int sockfd;
struct msg *sm, *rm, *sm_file;
int in_room;

//函数区域
void *client_filesend(void * am)
{
	char path[500];
	FILE *fp;
	struct msg *sm_file = (struct msg *)malloc(sizeof(struct msg));
	sm_file->type = MSG_FILENAME;
	strcpy(sm_file->name_from, sm->name_from);
	strcpy(sm_file->name_des, sm->name_des);
	strcpy(sm_file->data, sm->data);
	strcpy(path, sm->data);
	send(sockfd, (void *)sm_file, sizeof(struct msg), 0);
	printf("开始发送文件: %s\n", path);
	fp = fopen(path, "r");

	sm_file->type = MSG_FILEIN;
	bzero(sm_file->data, sizeof(sm_file->data));
	while(fread(sm_file->data, 1, sizeof(sm_file->data) - 1, fp) != 0)
	{
		send(sockfd, (void *)sm_file, sizeof(struct msg), 0);
		bzero(sm_file->data, sizeof(sm_file->data));
		usleep(500);
	}

	if(feof(fp))
	{
		sm_file->type = MSG_FILEEND;
		send(sockfd, (void *)sm_file, sizeof(struct msg), 0);
		printf("文件传输完毕!\n");
	}
	fclose(fp);
	free(sm_file);
}

void *client_recv(void *without)
{
	pthread_t tid_file;
	int open_file_flag = 0;
	char path[500], *path_temp;
	FILE *fp;
	struct msg *rm = (struct msg *)malloc(sizeof(struct msg));
	printf("已进入聊天室\n");
	while(1)
	{
		recv(sockfd, (void *)rm, sizeof(struct msg), 0);
		if(rm->type == MSG_SIGNOUT)
		{
			printf("%s\n", rm->data);
			in_room = 0;
			break;
		}
		else if(rm->type == MSG_FILESURE)
		{
			pthread_create(&tid_file, NULL, (void *)client_filesend, NULL);
			sleep(2);
			pthread_detach(tid_file);
		}
		else if(rm->type == MSG_FILENAME)
		{
			if(0 != access("./recvfile", F_OK))
			{
				mkdir("./recvfile", 0777);
			}
			path_temp = strrchr(rm->data, '/');
			sprintf(path, "./recvfile/%s", path_temp + 1);
			fp = fopen(path, "w");
			if(!fp)
			{
				printf("打开写入文件%s失败\n", rm->data);
				perror(":");
			}
			open_file_flag = 1;
		}
		else if(rm->type == MSG_FILEIN)
		{
			fwrite(rm->data, 1, strlen(rm->data), fp);
			usleep(500);
		}
		else if(rm->type == MSG_FILEEND)
		{
			printf("文件 %s 接收完毕\n", path);
			fclose(fp);
			open_file_flag = 0;
		}
		else if(rm->type == MSG_NORMAL)
		{
			printf("用户 %s :%s\n", rm->name_from, rm->data);
		}
		else if(rm->type == MSG_PRIVATE)
		{
			printf("用户 %s 悄悄地对你说:%s\n", rm->name_from, rm->data);
		}
		else if(rm->type == MSG_SEE)
		{
			printf("%s\n", rm->data);
		}
		else if(rm->type == MSG_SUCCESS)
		{
			printf("%s\n", rm->data);
		}
		else if(rm->type == MSG_ERROR)
		{
			printf("%s\n", rm->data);
		}
	}
	if(open_file_flag)
	{
		fclose(fp);
	}
}

void user_signup()
{
	sm->type = MSG_SIGNUP;
	printf("现在请输入注册信息：\n");
	printf("请输入用户名：");
	scanf("%s", sm->name_from);
	printf("请输入密码：");
	scanf("%s", sm->passwd);
	printf("请输入密保问题：");
	scanf("%s", sm->data);
	printf("请输入密保答案：");
	scanf("%s", sm->name_des);
	send(sockfd, (void *)sm, sizeof(struct msg), 0);
	recv(sockfd, (void *)rm, sizeof(struct msg), 0);
	printf("%s\n", rm->data);
}

void user_change()
{
	sm->type = MSG_CHANGE1;
	printf("现在请输入要更改的信息：\n");
	printf("请输入用户名：");
	scanf("%s", sm->name_from);
	send(sockfd, (void *)sm, sizeof(struct msg), 0);//确认用户名存在
	recv(sockfd, (void *)rm, sizeof(struct msg), 0);
	printf("%s\n", rm->data); //success:密保问题 error:用户名不存在
	if(rm->type == MSG_SUCCESS)
	{
		sm->type = MSG_CHANGE2;
		printf("请输入密保答案：");
		scanf("%s", sm->name_des);
		send(sockfd, (void *)sm, sizeof(struct msg), 0);//确认密保答案
		recv(sockfd, (void *)rm, sizeof(struct msg), 0);
		printf("%s\n", rm->data);//success:密保正确 error:密保错误
		if(rm->type == MSG_SUCCESS)
		{
			sm->type = MSG_CHANGE3;
			printf("请输入新密码：");
			scanf("%s", sm->passwd);
			send(sockfd, (void *)sm, sizeof(struct msg), 0);//用户名和新密码
			recv(sockfd, (void *)rm, sizeof(struct msg), 0);//更改成功
			printf("%s\n", rm->data);
		}
	}
}

void user_exit()
{
	sm->type = MSG_EXIT;
	send(sockfd, (void *)sm, sizeof(struct msg), 0);
	recv(sockfd, (void *)rm, sizeof(struct msg), 0);
	if(rm->type == MSG_EXIT)
	{
		printf("%s", rm->data);
	}
}

int user_signin()
{
	sm->type = MSG_SIGNIN;
	printf("请输入用户名：");
	scanf("%s", sm->name_from);
	printf("请输入密码：");
	scanf("%s", sm->passwd);
	send(sockfd, (void *)sm, sizeof(struct msg), 0);
	recv(sockfd, (void *)rm, sizeof(struct msg), 0);
	if(rm->type == MSG_SUCCESS)
	{
		printf("%s", rm->data);
		in_room = 1;
		return SUCCESS;
	}
	else if(rm->type == MSG_ERROR)
	{
		printf("%s", rm->data);
	}
	return FAIL;
}

void user_chat_signout()
{
	sm->type = MSG_SIGNOUT;
	send(sockfd, (void *)sm, sizeof(struct msg), 0);
}

void user_char_see()
{
	sm->type = MSG_SEE;
	send(sockfd, (void *)sm, sizeof(struct msg), 0);
}

void user_char_say()
{
	int input_option;
	while(1)
	{
		printf("请输入你的聊天类型\n");
		printf("1.群聊\n");
		printf("2.私聊\n");
		scanf("%d", &input_option);
		if(1 == input_option)
		{
			sm->type = MSG_NORMAL;
			break;
		}
		else if(2 == input_option)
		{
			sm->type = MSG_PRIVATE;
			printf("请输入你想要私聊的用户名:\n");
			scanf("%s", sm->name_des);
			break;
		}
		else
		{
			printf("\n非法输入，请重新输入!\n");
		}
	}
	printf("输入exit后可取消发言(不会退出聊天室)\n");
	while(1)
	{
		scanf("%s", sm->data);
		if(0 == strcmp(sm->data, "exit"))
		{
			break;
		}
		else
		{
			send(sockfd, (void *)sm, sizeof(struct msg), 0);
		}
	}
}

void user_chat_ban()
{
	sm->type = MSG_BAN;
	printf("请输入你想要禁言的用户名:\n");
	scanf("%s", sm->name_des);
	send(sockfd, (void *)sm, sizeof(struct msg), 0);
}

void user_chat_deban()
{
	sm->type = MSG_DEBAN;
	printf("请输入你想要解禁的用户名:\n");
	scanf("%s", sm->name_des);
	send(sockfd, (void *)sm, sizeof(struct msg), 0);
}

void user_chat_kick()
{
	sm->type = MSG_KICK;
	printf("请输入你想要踢人的用户名:\n");
	scanf("%s", sm->name_des);
	send(sockfd, (void *)sm, sizeof(struct msg), 0);
}

void user_chat_sendfile()
{
	sm->type = MSG_FILESURE;
	printf("请输入发送文件的文件名:\n");
	scanf("%s", sm->data);
	if(0 != access(sm->data, F_OK))
	{
		printf("该文件不存在\n");
		return ;
	}
	if(0 != access(sm->data, R_OK))
	{
		printf("该文件不可读\n");
		return ;
	}
	printf("请输入想要发送的用户名:\n");
	scanf("%s", sm->name_des);
	send(sockfd, (void *)sm, sizeof(struct msg), 0);
}

void p_menu_chat()
{
	printf("1.查看群成员\n");
	printf("2.发言(可选 群聊/私聊)\n");
	printf("3.禁言\n");
	printf("4.解禁\n");
	printf("5.踢人\n");
	printf("6.发送文件\n");
	printf("7.注销\n");
	printf("请输入你的选择:\n\n");
}

#define MENU_CHAT_SEE 1
#define MENU_CHAT_SAY 2
#define MENU_CHAT_BAN 3
#define MENU_CHAT_DEBAN 4
#define MENU_CHAT_KICK 5
#define MENU_CHAT_SENDFILE 6
#define MENU_CHAT_SIGNOUT 7

void user_chat()
{
	pthread_t tid_r;
	int input_option;
	int chat_flag = 1;
	pthread_create(&tid_r, NULL, (void *)client_recv, NULL);
	while(in_room)
	{
		p_menu_chat();
		scanf("%d", &input_option);
		if(!in_room)
		{
			break;
		}
		switch(input_option)
		{
			case MENU_CHAT_SEE:
			{
				user_char_see();
				break;
			}
			case MENU_CHAT_SAY:
			{
				user_char_say();
				break;
			}
			case MENU_CHAT_BAN:
			{
				user_chat_ban();
				break;
			}
			case MENU_CHAT_DEBAN:
			{
				user_chat_deban();
				break;
			}
			case MENU_CHAT_KICK:
			{
				user_chat_kick();
				break;
			}
			case MENU_CHAT_SENDFILE:
			{
				user_chat_sendfile();
				break;
			}
			case MENU_CHAT_SIGNOUT:
			{
				user_chat_signout();
				in_room = 0;
				break;
			}
			default:
			{
				printf("\n非法输入，请重新输入!\n");
			}
		}
	}
	pthread_join(tid_r, NULL);
	printf("即将退出聊天室\n");
}

void p_menu_init()
{
	printf("1.注册\n");
	printf("2.登陆\n");
	printf("3.更改密码\n");
	printf("4.退出\n");
	printf("请输入你的选择:\n\n");
}

#define MENU_SIGNUP 1
#define MENU_SIGNIN 2
#define MENU_CHANGE 3
#define MENU_EXIT 4

void *client_send(void *without)
{
	int input_option;
	while(1)
	{
		p_menu_init();
		scanf("%d", &input_option);
		switch(input_option)
		{
			case MENU_SIGNUP:
			{
				user_signup();
				break;
			}
			case MENU_SIGNIN:
			{
				if(SUCCESS == user_signin())
				{
					user_chat();
				}
				break;
			}
			case MENU_CHANGE:
			{
				user_change();
				break;
			}
			case MENU_EXIT:
			{
				user_exit();
				return NULL;
			}
			default:
			{
				printf("\n非法输入，请重新输入!\n");
			}
		}

	}
}

int connect_init()
{
	struct sockaddr_in servaddr;
	int re;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
	{
		perror("socket error:");
		return -1;
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(MY_PORT);
	re = inet_pton(AF_INET, MY_IP, &servaddr.sin_addr);
	if(re == -1)
	{
		perror("inet_ntop error:");
		return -1;
	}
	re = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr));
	if(re == -1)
	{
		perror("connect error:");
		return -1;
	}
	recv(sockfd, (void *)rm, sizeof(struct msg), 0);
	printf("%s\n", rm->data);
	return 0;
}

int main()
{
	pthread_t tid_s;
	int re;
	sm = (struct msg *)malloc(sizeof(struct msg));
	rm = (struct msg *)malloc(sizeof(struct msg));
	re = connect_init();
	if(re == -1)
	{
		printf("connect_init failed\n");
		close(sockfd);
		return -1;
	}
	
	pthread_create(&tid_s, NULL, (void *)client_send, NULL);

	pthread_join(tid_s, NULL);
	printf("即将关闭客户端\n");	
	free(sm);
	free(rm);
	close(sockfd);
	return 0;
}
