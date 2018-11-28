#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"chat_room.h"

int list_count(struct chat_list *p)
{
	int count = 0;
	struct chat_list * temp = p->next;
	while(temp)
	{
		++count;
		temp = temp->next;
	}
	return count;
}

void list_reroot(struct chat_list *p)
{
	struct chat_list * temp = p->next;
	if(temp)
	{
		temp->limit = IS_ROOT;
	}
}

int list_find(struct chat_list *p, char name[])
{
	struct chat_list * temp = p->next;
	while(temp)
	{
		if(strcmp(temp->name, name) == 0)
		{
			return temp->confd;
		}
		temp = temp->next;
	}
	return NAME_NOEXIST;	
}

int list_find_first_confd(struct chat_list *p)
{
	struct chat_list * temp = p->next;
	if(temp)
	{
		return temp->confd;
	}
	return FAIL;
}

int mylist_add(struct chat_list *p, char name[], int fd)
{
	struct chat_list *temp = p, *be;
	if(NAME_NOEXIST == judge_is_exist(p, name))
	{
		while(temp->next)
		{ 
			temp = temp->next;
		}
		be = (struct chat_list *)malloc(sizeof(struct chat_list));
		if(list_count(p))
		{
			be->limit = IS_NORMAL;
		}
		else
		{
			be->limit = IS_ROOT;
		}
		be->say = 1;
		be->confd = fd;
		be->next = NULL;
		be->file_protect = 0;
		strcpy(be->name, name);
		temp->next = be;
		return be->limit;
	}
	return NAME_EXIST;
}

int list_del(struct chat_list *p, char name[])
{
	struct chat_list *pre = p, *mid = p->next;
	if(NAME_EXIST == judge_is_exist(p, name))
	{
		for(; mid; pre = pre->next, mid = mid->next)
		{
			if(strcmp(mid->name, name) == 0)
			{
				pre->next = mid->next;
				free(mid);
				if(judge_is_root(p, name))
				{
					list_reroot(p);
				}
				return SUCCESS;
			}
		}
	}
	return NAME_NOEXIST;
}

void list_ban(struct chat_list *p, char name[])
{
	struct chat_list * temp = p->next;
	while(temp)
	{
		if(0 == strcmp(temp->name, name))
		{
			temp->say = 0;
			break;
		}
		temp = temp->next;
	}
}

void list_deban(struct chat_list *p, char name[])
{
	struct chat_list * temp = p->next;
	while(temp)
	{
		if(0 == strcmp(temp->name, name))
		{
			temp->say = 1;
			break;
		}
		temp = temp->next;
	}
}

int judge_is_root(struct chat_list *p, char name[])
{
	struct chat_list * temp = p->next;
	if(temp)
	{
		if(strcmp(temp->name, name) == 0)
		{
			return 1;
		}
	}
	return 0;
}

int judge_is_exist(struct chat_list *p, char name[])
{
	struct chat_list * temp = p->next;
	while(temp)
	{
		if(strcmp(temp->name, name) == 0)
		{
			return NAME_EXIST;
		}
		temp = temp->next;
	}
	return NAME_NOEXIST;
}

int judge_is_can_say(struct chat_list *p, char name[])
{
	struct chat_list * temp = p->next;
	while(temp)
	{
		if(strcmp(temp->name, name) == 0)
		{
			return temp->say;
		}
		temp = temp->next;
	}
	return NAME_NOEXIST;
}
