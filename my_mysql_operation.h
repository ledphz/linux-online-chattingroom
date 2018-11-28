#ifndef MY_MYSQL_OPERATION_H
#define MY_MYSQL_OPERATION_H

#include"chat_room.h"

int my_mysql_signup(struct msg  rm);
int my_mysql_signin(struct msg  rm);
int my_mysql_name_exist(char name[]);
char *my_mysql_mibao_q(char name[]);
int my_mysql_mibao_if_right(char name[], char mibao[]);
int my_mysql_change(char name[], char new_passwd[]);

#endif

