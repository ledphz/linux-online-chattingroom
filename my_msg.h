#ifndef MY_MSG_H
#define MY_MSG_H

#define MYNAME_LEN 20
#define PASSWD_LEN 8
#define MAXSIZE 1024

#define MSG_EXIT 0
#define MSG_FILENAME 22
#define MSG_FILEIN 24
#define MSG_FILEEND 25
#define MSG_FILESURE 26
#define MSG_NORMAL 33
#define MSG_PRIVATE 44
#define MSG_BAN 11
#define MSG_DEBAN 13
#define MSG_KICK 21
#define MSG_SEE 59

#define MSG_SIGNUP 1
#define MSG_SIGNIN 2
#define MSG_SIGNOUT 3
#define MSG_CHANGE1 4
#define MSG_CHANGE2 5
#define MSG_CHANGE3 6
#define MSG_SUCCESS 10000
#define MSG_ERROR 20000

struct msg
{
    int type;
    char name_from[MYNAME_LEN];
    char name_des[MYNAME_LEN];
    char passwd[PASSWD_LEN];
    char data[MAXSIZE];
};

#endif
