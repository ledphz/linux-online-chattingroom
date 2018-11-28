#include<stdio.h>
#include<mysql.h>
#include"chat_room.h"
#include"my_mysql_operation.h"

int my_mysql_signup(struct msg  rm)
{
    char sql[MAXSIZE];
    MYSQL *conn_ptr;
    MYSQL_RES *res_ptr;
    int re;
    conn_ptr = mysql_init(NULL);
    if(!conn_ptr)
    {
        printf("mysql_init failed\n");
        return -1;
    }
    conn_ptr = mysql_real_connect(conn_ptr, "localhost", "root", "", "xh2", 0, NULL, 0);
    if(conn_ptr)
    {
        sprintf(sql, "select * from user where name='%s'", rm.name_from);
        re = mysql_query(conn_ptr, sql);
        if(re)
        {
            printf("SELECT error:%s\n",mysql_error(conn_ptr));
        }
        else
        {
            res_ptr = mysql_store_result(conn_ptr);
            if(mysql_num_rows(res_ptr))
            {
                mysql_close(conn_ptr);
                mysql_free_result(res_ptr);
                return NAME_EXIST;
            }
            else
            {
                sprintf(sql, "insert into user values ('%s','%s','%s','%s')", rm.name_from, rm.passwd, rm.data, rm.name_des);
                re = mysql_query(conn_ptr, sql);
                if(!re)
                {
                    mysql_close(conn_ptr);
                    mysql_free_result(res_ptr);
                    return SUCCESS;
                }
                else
                {
                    fprintf(stderr, "Insert error %d: %sn",mysql_errno(conn_ptr),mysql_error(conn_ptr));
                }
            }
        }
    }
    else
    {
        printf("connect mysql error!\n");
        mysql_close(conn_ptr);
        mysql_free_result(res_ptr);
        return -1;
    }

    mysql_close(conn_ptr);
    mysql_free_result(res_ptr);
    return -1;
}

int my_mysql_signin(struct msg  rm)
{
    char sql[MAXSIZE];
    MYSQL *conn_ptr;
    MYSQL_RES *res_ptr;
    int re;
    conn_ptr = mysql_init(NULL);
    if(!conn_ptr)
    {
        printf("mysql_init failed\n");
        return -1;
    }
    conn_ptr = mysql_real_connect(conn_ptr, "localhost", "root", "", "xh2", 0, NULL, 0);
    if(conn_ptr)
    {
        sprintf(sql, "select * from user where name='%s' and passwd='%s'", rm.name_from, rm.passwd);
        re = mysql_query(conn_ptr, sql);
        if(re)
        {
            printf("SELECT error:%s\n",mysql_error(conn_ptr));
        }
        else
        {
            res_ptr = mysql_store_result(conn_ptr);
            if(mysql_num_rows(res_ptr))
            {
                mysql_close(conn_ptr);
                mysql_free_result(res_ptr);
                return SUCCESS;
            }
            else
            {
                mysql_close(conn_ptr);
                mysql_free_result(res_ptr);
                return FAIL;
            }
        }
    }
    else
    {
        printf("connect mysql error!\n");
        return -1;
    }
    mysql_close(conn_ptr);
    mysql_free_result(res_ptr);
    return -1;
}

int my_mysql_name_exist(char name[])
{
    char sql[MAXSIZE];
    MYSQL *conn_ptr;
    MYSQL_RES *res_ptr;
    int re;
    conn_ptr = mysql_init(NULL);
    if(!conn_ptr)
    {
        printf("mysql_init failed\n");
        return -1;
    }
    conn_ptr = mysql_real_connect(conn_ptr, "localhost", "root", "", "xh2", 0, NULL, 0);
    if(conn_ptr)
	{
        sprintf(sql, "select * from user where name='%s'", name);
		re = mysql_query(conn_ptr, sql);
		if(re)
		{
			printf("SELECT error:%s\n",mysql_error(conn_ptr));
		}
		else
		{
			res_ptr = mysql_store_result(conn_ptr);
			if(mysql_num_rows(res_ptr))
			{
				mysql_close(conn_ptr);
                mysql_free_result(res_ptr);
                return 1;
			}
			else
            {
                mysql_close(conn_ptr);
                mysql_free_result(res_ptr);
                return 0;
            }
		}
	}
	else
	{
		printf("connect mysql error!\n");
		return -1;
	}
	mysql_close(conn_ptr);
	mysql_free_result(res_ptr);
	return -1;
}

char *my_mysql_mibao_q(char name[])
{
    char sql[MAXSIZE];
    MYSQL *conn_ptr;
    MYSQL_RES *res_ptr;
	MYSQL_ROW sqlrow;
    int re;
	char *re_string;
    conn_ptr = mysql_init(NULL);
    if(!conn_ptr)
    {
        printf("mysql_init failed\n");
        return NULL;
    }
    conn_ptr = mysql_real_connect(conn_ptr, "localhost", "root", "", "xh2", 0, NULL, 0);
    if(conn_ptr)
	{
        sprintf(sql, "select change_question from user where name='%s'", name);
		re = mysql_query(conn_ptr, sql);
		if(re)
		{
			printf("SELECT error:%s\n",mysql_error(conn_ptr));
		}
		else
		{
			res_ptr = mysql_store_result(conn_ptr);
			sqlrow = mysql_fetch_row(res_ptr);
			mysql_close(conn_ptr);
            mysql_free_result(res_ptr);
			re_string = sqlrow[0];
            return re_string;
		}
	}
	else
	{
		printf("connect mysql error!\n");
		return NULL;
	}
	mysql_close(conn_ptr);
	mysql_free_result(res_ptr);
	return NULL;
}

int my_mysql_mibao_if_right(char name[], char mibao[])
{
    char sql[MAXSIZE];
    MYSQL *conn_ptr;
    MYSQL_RES *res_ptr;
    int re;
    conn_ptr = mysql_init(NULL);
    if(!conn_ptr)
    {
        printf("mysql_init failed\n");
        return -1;
    }
    conn_ptr = mysql_real_connect(conn_ptr, "localhost", "root", "", "xh2", 0, NULL, 0);
    if(conn_ptr)
	{
        sprintf(sql, "select * from user where name='%s' and change_answer='%s'", name, mibao);
		re = mysql_query(conn_ptr, sql);
		if(re)
		{
			printf("SELECT error:%s\n",mysql_error(conn_ptr));
		}
		else
		{
			res_ptr = mysql_store_result(conn_ptr);
			if(mysql_num_rows(res_ptr))
			{
				mysql_close(conn_ptr);
                mysql_free_result(res_ptr);
                return 1;
			}
			else
            {
                mysql_close(conn_ptr);
                mysql_free_result(res_ptr);
                return 0;
            }
		}
	}
	else
	{
		printf("connect mysql error!\n");
		return -1;
	}
	mysql_close(conn_ptr);
	mysql_free_result(res_ptr);
	return -1;
}

int my_mysql_change(char name[], char new_passwd[])
{
    char sql[MAXSIZE];
    MYSQL *conn_ptr;
    int re;
    conn_ptr = mysql_init(NULL);
    if(!conn_ptr)
    {
        printf("mysql_init failed\n");
        return -1;
    }
    conn_ptr = mysql_real_connect(conn_ptr, "localhost", "root", "", "xh2", 0, NULL, 0);
    if(conn_ptr)
	{
        sprintf(sql, "update  user set passwd='%s' where name='%s'", new_passwd, name);
		re = mysql_query(conn_ptr, sql);
		if(re)
		{
			printf("SELECT error:%s\n",mysql_error(conn_ptr));
		}
	}
	else
	{
		printf("connect mysql error!\n");
		return -1;
	}
	mysql_close(conn_ptr);
	return 1;
}
