install:
	gcc server.c chat_room.c my_mysql_operation.c -o server -I /usr/include/mysql -L /usr/lib/mysql -l mysqlclient
	gcc client.c -o client -lpthread
