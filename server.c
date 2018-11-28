#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<time.h>
#include"chat_room.h"
#include"my_msg.h"
#include"my_mysql_operation.h"

#define BACKLOG 1024

int sockfd;
struct chat_list *my_list;

int main()
{
	int confd;
	struct sockaddr_in servaddr, conaddr;
	int i, len, re, fd_count, fd_max, re_is_root;
	struct timeval tv;
	fd_set inset, tempset;
	char buf[MAXSIZE];
	struct msg *sm = (struct msg *)malloc(sizeof(struct msg));
	struct msg *rm = (struct msg *)malloc(sizeof(struct msg));

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
	{
		perror("socket error:");
		return -1;
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(7092);
	servaddr.sin_addr.s_addr = INADDR_ANY;
	re = bind(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr));
	if(re == -1)
	{
		perror("bind error:");
		return -1;
	}
	re = listen(sockfd, BACKLOG);
	if(re == -1)
	{
		perror("listen error:");
		return -1;
	}

	my_list = (struct chat_list *)malloc(sizeof(struct chat_list));
	my_list->next = NULL;
	fd_count = 0;
	FD_ZERO(&inset);
	FD_SET(sockfd, &inset);
	fd_max = sockfd + fd_count;

	while(1)
	{
		tempset = inset;
		tv.tv_sec = 20;
		tv.tv_usec = 0;
		re = select(fd_max + 1, &tempset, NULL, NULL, &tv);
		if(re < 0)
		{
			perror("select error:");
			break;
		}
		if(re == 0)
		{
			continue;
		}

		for(i = 3; i <= fd_max ; ++i)
		{
			if (FD_ISSET(i, &tempset))
			{
				if(i == sockfd)
				{
					if(fd_count < BACKLOG)
					{
						len = sizeof(servaddr);
						bzero(&conaddr, sizeof(conaddr));
						confd = accept(sockfd,  (struct sockaddr*)&conaddr, &len);
						if(-1 == confd)
						{
							perror("accept error:");
							break;
						}
						FD_SET(confd, &inset);
						printf("connect confd:%d\n", confd);
						memset(buf, 0, sizeof(buf));
						sprintf(buf, "连接服务器成功,my confd is %d\n", confd);
						send(confd, buf, sizeof(buf), 0);
						++fd_count;
						if(fd_max < confd)
						{
							fd_max = confd;
						}

					}
					else
					{
						printf("fd_count is max\n");
						continue;
					}
				}
				else
				{
					recv(i, (void *)rm, sizeof(struct msg), 0);
					if(rm->type == MSG_EXIT) //退出
					{	
						sprintf(sm->data, "已与服务器断开连接\n");
						send(i, (void *)sm, sizeof(struct msg), 0);
						printf("与客户端断开连接 confd :%d\n", i);
						FD_CLR(i, &inset);
						close(i);
						--fd_count;
						continue;
					}
					else if(rm->type == MSG_SIGNUP) //注册
					{
						re = my_mysql_signup(*rm);
						if(NAME_EXIST == re)
						{
							sm->type = MSG_ERROR;
							sprintf(sm->data, "注册的用户名已存在\n");
						}
						else if(SUCCESS == re)
						{
							sm->type = MSG_SUCCESS;
							sprintf(sm->data, "注册成功,你的用户名为%s\n密码为%s\n密保问题为%s\n密保答案为%s\n", rm->name_from, rm->passwd, rm->data, rm->name_des);
						}
						else
						{
							printf("my_mysql_signup error!\n");
							sm->type = MSG_ERROR;
						}
						send(i, (void *)sm, sizeof(struct msg), 0);
					}
					else if(rm->type == MSG_CHANGE1) //去数据库查找用户名
					{
						if(my_mysql_name_exist(rm->name_from))
						{
							sm->type = MSG_SUCCESS;
							sprintf(sm->data, "密保问题:%s\n", my_mysql_mibao_q(rm->name_from));
						}
						else
						{
							sm->type = MSG_ERROR;
							sprintf(sm->data, "该用户名不存在\n");
						}
						send(i, (void *)sm, sizeof(struct msg), 0);
					}
					else if(rm->type == MSG_CHANGE2) //去数据库校对用户名的密保
					{
						if(my_mysql_mibao_if_right(rm->name_from, rm->name_des))
						{
							sm->type = MSG_SUCCESS;
							sprintf(sm->data, "密保正确\n");
						}
						else
						{
							sm->type = MSG_ERROR;
							sprintf(sm->data, "密保错误\n");
						}
						send(i, (void *)sm, sizeof(struct msg), 0);
					}
					else if(rm->type == MSG_CHANGE3)//去数据库更改密码
					{
						my_mysql_change(rm->name_from, rm->passwd);
						sm->type = MSG_SUCCESS;
						sprintf(sm->data, "修改成功!用户名%s的新密码为:%s\n", rm->name_from, rm->passwd);
						send(i, (void *)sm, sizeof(struct msg), 0);
					}
					else if(rm->type == MSG_SIGNIN) //登录
					{
						re = my_mysql_signin(*rm);
						if(SUCCESS == re)
						{
							re = mylist_add(my_list, rm->name_from, i);
							if(NAME_EXIST == re)
							{
								sm->type = MSG_ERROR;
								sprintf(sm->data, "该用户名已登录\n");
							}
							else if(IS_ROOT == re)
							{
								sm->type = MSG_SUCCESS;
								sprintf(sm->data, "已登录，权限为群主\n");
							}
							else
							{
								sm->type = MSG_SUCCESS;
								sprintf(sm->data, "已登录，权限为普通成员\n");
							}
							send(i, (void *)sm, sizeof(struct msg), 0);
						}
						else
						{
							sm->type = MSG_ERROR;
							sprintf(sm->data, "用户名或密码错误，请检查!\n");
							send(i, (void *)sm, sizeof(struct msg), 0);
						}
					}
					else if(rm->type == MSG_SIGNOUT) //注销
					{
						re_is_root = judge_is_root(my_list, rm->name_from);
						re = list_del(my_list, rm->name_from);
						if(SUCCESS == re)
						{
							sm->type = MSG_SIGNOUT;
							sprintf(sm->data, "注销成功!\n");
							send(i, (void *)sm, sizeof(struct msg), 0);
							if(re_is_root)
							{
								re = list_find_first_confd(my_list);
								if(FAIL != re)
								{
									sm->type = MSG_ERROR;
									sprintf(sm->data, "群主退出，您成为了新群主!\n");
									send(re, (void *)sm, sizeof(struct msg), 0);
								}
							}
						}
						else
						{
							sm->type = MSG_ERROR;
							sprintf(sm->data, "你本来就不在聊天室里!\n");
							send(i, (void *)sm, sizeof(struct msg), 0);
						}
					}
					else if(rm->type == MSG_SEE) //查看
					{
						sm->type = MSG_SEE;
						sprintf(sm->data, "聊天室人数为:%d\n有这些用户:", list_count(my_list));
						send(i, (void *)sm, sizeof(struct msg), 0);
						struct chat_list * temp_see = my_list->next;
						while(temp_see)
						{
							sprintf(sm->data, "%s", temp_see->name);
							send(i, (void *)sm, sizeof(struct msg), 0);
							temp_see = temp_see->next;
						}
					}
					else if(rm->type == MSG_PRIVATE) //私聊
					{
						if(judge_is_can_say(my_list, rm->name_from))
						{
							if(judge_is_exist(my_list, rm->name_des))
							{
								send(list_find(my_list, rm->name_des), (void *)rm, sizeof(struct msg), 0);
							}
							else
							{
								sm->type = MSG_ERROR;
								sprintf(sm->data, "此人不存在,输入exit可退出发言!\n");
								send(i, (void *)sm, sizeof(struct msg), 0);
							}
						}
						else
						{
							sm->type = MSG_ERROR;
							sprintf(sm->data, "你被禁言了,无法说话!\n");
							send(i, (void *)sm, sizeof(struct msg), 0);
						}
					}
					else if(rm->type == MSG_NORMAL) //群聊
					{
						if(judge_is_can_say(my_list, rm->name_from))
						{
							struct chat_list * temp = my_list->next;
							while(temp)
							{
								if(temp->confd != i)
								{
									send(temp->confd, (void *)rm, sizeof(struct msg), 0);
								}
								temp = temp->next;
							}
						}
						else
						{
							sm->type = MSG_ERROR;
							sprintf(sm->data, "你被禁言了,无法说话!\n");
							send(i, (void *)sm, sizeof(struct msg), 0);
						}
					}
					else if(rm->type == MSG_BAN) //禁言
					{
						if(judge_is_root(my_list, rm->name_from))
						{
							if(judge_is_exist(my_list, rm->name_des))
							{
								list_ban(my_list, rm->name_des);
								sm->type = MSG_SUCCESS;
								sprintf(sm->data, "您已被群主禁言!\n");
								send(list_find(my_list, rm->name_des), (void *)sm, sizeof(struct msg), 0);
								sprintf(sm->data, "禁言%s成功!\n", rm->name_des);
								send(list_find(my_list, rm->name_from), (void *)sm, sizeof(struct msg), 0);
							}
							else
							{
								sm->type = MSG_ERROR;
								sprintf(sm->data, "此人不存在!\n");
								send(i, (void *)sm, sizeof(struct msg), 0);
							}
						}
						else
						{
							sm->type = MSG_ERROR;
							sprintf(sm->data, "你不是管理员,无法禁言!\n");
							send(i, (void *)sm, sizeof(struct msg), 0);
						}
					}
					else if(rm->type == MSG_DEBAN) //解禁
					{
						if(judge_is_root(my_list, rm->name_from))
						{
							if(judge_is_exist(my_list, rm->name_des))
							{
								list_deban(my_list, rm->name_des);
								sm->type = MSG_SUCCESS;
								sprintf(sm->data, "您已被群主解禁!\n");
								send(list_find(my_list, rm->name_des), (void *)sm, sizeof(struct msg), 0);
								sprintf(sm->data, "解禁%s成功!\n", rm->name_des);
								send(list_find(my_list, rm->name_from), (void *)sm, sizeof(struct msg), 0);
							}
							else
							{
								sm->type = MSG_ERROR;
								sprintf(sm->data, "此人不存在!\n");
								send(i, (void *)sm, sizeof(struct msg), 0);
							}
						}
						else
						{
							sm->type = MSG_ERROR;
							sprintf(sm->data, "你不是管理员,无法解禁!\n");
							send(i, (void *)sm, sizeof(struct msg), 0);
						}
					}
					else if(rm->type == MSG_KICK) //踢人
					{
						if(judge_is_root(my_list, rm->name_from))
						{
							if(judge_is_exist(my_list, rm->name_des))
							{
								sm->type = MSG_SIGNOUT;
								sprintf(sm->data, "您已被群主踢出聊天室!\n");
								send(list_find(my_list, rm->name_des), (void *)sm, sizeof(struct msg), 0);
								list_del(my_list, rm->name_des);
								sprintf(sm->data, "踢出%s成功!\n", rm->name_des);
								send(list_find(my_list, rm->name_from), (void *)sm, sizeof(struct msg), 0);
							}
							else
							{
								sm->type = MSG_ERROR;
								sprintf(sm->data, "此人不存在!\n");
								send(i, (void *)sm, sizeof(struct msg), 0);
							}
						}
						else
						{
							sm->type = MSG_ERROR;
							sprintf(sm->data, "你不是管理员,无法踢人!\n");
							send(i, (void *)sm, sizeof(struct msg), 0);
						}
					}
					else if(rm->type == MSG_FILESURE) //发送文件前的确认
					{
						if(judge_is_can_say(my_list, rm->name_from))
						{
							if(judge_is_exist(my_list, rm->name_des))
							{
								sm->type = MSG_FILESURE;
								sprintf(sm->data, "%s", rm->data);
							}
							else
							{
								sm->type = MSG_ERROR;
								sprintf(sm->data, "该用户不存在,无法发文件!\n");
							}
						}
						else
						{
							sm->type = MSG_ERROR;
							sprintf(sm->data, "你被禁言了,无法发文件!\n");
						}
						send(i, (void *)sm, sizeof(struct msg), 0);
					}
					else if(rm->type == MSG_FILENAME) //发送文件名
					{
						send(list_find(my_list, rm->name_des), (void *)rm, sizeof(struct msg), 0);
					}
					else if(rm->type == MSG_FILEIN) //发送文件内容
					{
						send(list_find(my_list, rm->name_des), (void *)rm, sizeof(struct msg), 0);
					}
					else if(rm->type == MSG_FILEEND) //发送文件结尾
					{
						send(list_find(my_list, rm->name_des), (void *)rm, sizeof(struct msg), 0);
					}
					else
					{
						printf("nothing!\n");
					}
				}
			}
		}
	}
	free(rm);
	free(sm);
	close(sockfd);
}
