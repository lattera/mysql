#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "sql.h"
#include "misc.h"

SQL_ROW *sql_backend_mysql(SQL_CTX *sqldb, char *statement)
{
	SQL_ROW *rows=NULL, *row=NULL;
	SQL_COL *col;
	MYSQL_ROW mysqlrow;
	MYSQL_RES *res;
	MYSQL_FIELD *fields;
	unsigned int i;
	unsigned int num_fields;
	
	mysql_query(sqldb->db, statement);
	res = mysql_store_result(sqldb->db);
	if (!(res)) {
		// NULL results do not mean an error
	//	fprintf(stderr, "[-] Query Error: %s\n", mysql_error(sqldb->db));
		goto end;
	}
	
	num_fields = mysql_num_fields(res);
	fields = mysql_fetch_fields(res);
	
	while ((mysqlrow = mysql_fetch_row(res))) {
		
		if (!(row)) {
			
			rows = xmalloc(sizeof(SQL_ROW));
			
			if (!(rows))
				goto end;
			
			row = rows;
			
		} else {
			
			row->next = xmalloc(sizeof(SQL_ROW));
			
			if (!(row->next))
				goto end;
			
			row = row->next;
		}
		
		for (i=0; i<num_fields; i++) {
			
			if (!(row->cols)) {
				
				row->cols = xmalloc(sizeof(SQL_COL));
				
				if (!(row->cols))
					goto end;
				
				col = row->cols;
				
			} else {
				
				col->next = xmalloc(sizeof(SQL_COL));
				
				if (!(col->next))
					goto end;
				
				col = col->next;
			}
			
			if (fields[i].name)
				col->name = strdup(fields[i].name);
			
			if (mysqlrow[i])
				col->data = strdup(mysqlrow[i]);
			
		}
	}
	
end:
	if (res)
		mysql_free_result(res);
	
	return rows;
}

SQL_ROW *runsql(SQL_CTX *sqldb, char *statement)
{
	return sql_backend_mysql(sqldb, statement);
}

/* Please only use this function if needed */
SQL_ROW *sqlfmt(SQL_CTX *sqldb, char *buf, size_t bufsz, char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	vsnprintf(buf, bufsz, fmt, ap);
	va_end(ap);
	
	return runsql(sqldb, buf);
}

char *get_column(SQL_ROW *row, char *name)
{
	SQL_COL *col;
	if (!(row))
		return NULL;
	
	for (col = row->cols; col; col = col->next)
		if ((col->name))
			if (!strcmp(col->name, name))
				return col->data;
	
	return NULL;
			
}

void sqldb_free_rows(SQL_ROW *rows)
{
	SQL_ROW *cur_row, *next_row;
	SQL_COL *cur_col, *next_col;
	
	cur_row = rows;
	while (cur_row) {
		next_row = cur_row->next;
		
		cur_col = cur_row->cols;
		while (cur_col) {
			next_col = cur_col->next;
			
			if (cur_col->name)
				free(cur_col->name);
			if (cur_col->data)
				free(cur_col->data);
			free(cur_col);
			
			cur_col = next_col;
		}
		
		free(cur_row);
		cur_row = next_row;
	}
}



void print_rows(SQL_ROW *rows)
{
	SQL_ROW *row;
	SQL_COL *col;
	unsigned int i=0;
	
	row = rows;
	while (row) {
		fprintf(stderr, "[*] Row #%d\n", i++);
		col = row->cols;
		while (col) {
			if ((col->name) && (col->data))
				fprintf(stderr, "[*] \t%s = %s\n", col->name, col->data);
			col = col->next;
		}
		row = row->next;
	}
}
