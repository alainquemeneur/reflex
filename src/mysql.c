#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mysql/mysql.h>

#define _MYSQL

#define DBHANDLER MYSQL *
#define DBRESULT MYSQL_RES *

DBHANDLER handler;
DBRESULT result;

char *main_database="mysql";

DBHANDLER db_opendatabase(char *databasename, char *host, char *login, char *password)
{
MYSQL *handler;

handler=(MYSQL *)malloc(sizeof(MYSQL));
mysql_init(handler);
return(mysql_real_connect(handler, host, login, password, databasename, 0, NULL, 0));
}

DBRESULT db_query(DBHANDLER handler, char *query)
{
char *buf;

buf=(char *)malloc(strlen(query)+10);
strcpy(buf, query);
buf[strlen(buf)-1]=0;
mysql_query(handler, buf);
//mysql_real_query(handler,query,strlen(query));
result=mysql_store_result(handler);
free(buf);
return(result);
}

int db_ntuples(DBRESULT result)
{
return((int)mysql_num_rows(result));
}

void db_clear_result(DBRESULT result)
{
mysql_free_result(result);
}

void db_close(DBHANDLER handler)
{
mysql_close(handler);
free(handler);
}

void db_getvalue(DBRESULT result, int row, int col, char *value, int max)
{
MYSQL_ROW rrow;

mysql_data_seek(result, row);
rrow=mysql_fetch_row(result);
strncpy(value, rrow[col], max-1);
value[max]=0;
}
