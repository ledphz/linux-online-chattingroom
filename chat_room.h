#ifndef CHAT_ROOM_H
#define CHAT_ROOM_H

#include"my_msg.h"

#define IS_ROOT 1
#define IS_NORMAL 0

#define NAME_EXIST 2
#define NAME_NOEXIST 0

#define SUCCESS 3
#define FAIL 0

struct chat_list
{
	char name[MYNAME_LEN];
	int limit;
	int say;
	int confd;
	int file_protect;
	struct chat_list *next;
};

int list_count(struct chat_list *p);
void list_reroot(struct chat_list *p);
int list_find(struct chat_list *p, char name[]);
int list_find_first_confd(struct chat_list *p);
int mylist_add(struct chat_list *p, char name[], int fd);
int list_del(struct chat_list *p, char name[]);
void list_ban(struct chat_list *p, char name[]);
void list_deban(struct chat_list *p, char name[]);

int judge_is_root(struct chat_list *p, char name[]);
int judge_is_exist(struct chat_list *p, char name[]);
int judge_is_can_say(struct chat_list *p, char name[]);

#endif
