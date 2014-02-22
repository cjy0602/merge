#include<stdio.h>
#include<stdlib.h>
#include<sqlite3.h>
#include<malloc.h>
#include<string.h>
#include<tchar.h>
#include "match.h"


void get_signature_REG(SIGREGQ * q)
{

	sqlite3 *sigDB = NULL;


	sqlite3_stmt *sigSTMT = NULL;
	char *sql;
	//int rc; 
	char* errorMsg = NULL;
	if(sqlite3_open("sig.db", &sigDB) != SQLITE_OK)
	{
		fprintf(stderr, "DB������ ��ƽ��ϴ�. (���� %s)\n", sqlite3_errmsg(sigDB));
	}

	sql = "select * from Registry;";
	if( sqlite3_exec(sigDB, sql, NULL, NULL, NULL) == SQLITE_OK) { 

	} else {

		exit(1);
	}


	if(sqlite3_prepare_v2(sigDB, sql, strlen(sql), &sigSTMT, NULL) == SQLITE_OK) 
	{
		//puts(">> Prepared Statement is ready : Succeeded!\n");
	}
	else
	{
		puts("���̺� �� �Է¿� �����Ͽ����ϴ�.");
	}

	int row = 0;
	while (1) {
		int s;


		s = sqlite3_step (sigSTMT);
		if (s == SQLITE_ROW) {
			sig_registry * tmp = (sig_registry * ) malloc(sizeof(sig_registry));
			memset(tmp, 0, sizeof(sig_registry));
			int bytes;
			const unsigned char * text;
			////////////////////////////////////////////////////////////////////
			tmp->action = sqlite3_column_int(sigSTMT, 0);
			tmp->tool = sqlite3_column_int (sigSTMT, 1);
			sprintf(tmp->key, "%s", (char * )sqlite3_column_text(sigSTMT, 2));
			////////////////////////////////////////////////////////////////////
			q->push(tmp);
			row++;
		}
		else if (s == SQLITE_DONE) {
			break;
		}
		else {
			fprintf (stderr, "Failed.\n");
			exit (1);
		}
	}
	sqlite3_finalize(sigSTMT);
	sqlite3_close(sigDB);
}
void get_signature_FILE(SIGFILEQ * q)
{

	sqlite3 *sigDB = NULL;


	sqlite3_stmt *sigSTMT = NULL;
	char *sql;
	int rc; 
	char* errorMsg = NULL;

	if(sqlite3_open("sig.db", &sigDB) != SQLITE_OK)
	{
		fprintf(stderr, "DB������ ��ƽ��ϴ�. (���� %s)\n", sqlite3_errmsg(sigDB));
	}

	sql = "select * from file;";
	if( sqlite3_exec(sigDB, sql, NULL, NULL, NULL) == SQLITE_OK) { 

	} else {

		exit(1);
	}


	if(sqlite3_prepare_v2(sigDB, sql, strlen(sql), &sigSTMT, NULL) == SQLITE_OK) 
	{
		puts(">> Prepared Statement is ready : Succeeded!\n");
	}
	else
	{
		puts("���̺� �� �Է¿� �����Ͽ����ϴ�.");
	}

	int row = 0;
	while (1) {
		int s;


		s = sqlite3_step (sigSTMT);
		if (s == SQLITE_ROW) {
			sig_file * tmp = (sig_file * ) malloc(sizeof(sig_file));
			memset(tmp, 0, sizeof(sig_file));
			int bytes;
			const unsigned char * text;
			////////////////////////////////////////////////////////////////////
			tmp->action = sqlite3_column_int(sigSTMT, 1);
			tmp->tool = sqlite3_column_int (sigSTMT, 0);
			sprintf(tmp->file, "%s", (char * )sqlite3_column_text(sigSTMT, 2));
			sprintf(tmp->path, "%s", (char * )sqlite3_column_text(sigSTMT, 3));
			////////////////////////////////////////////////////////////////////
			q->push(tmp);
			row++;
		}
		else if (s == SQLITE_DONE) {
			break;
		}
		else {
			fprintf (stderr, "Failed.\n");
			exit (1);
		}
	}
	sqlite3_finalize(sigSTMT);
	sqlite3_close(sigDB);
}
void matching_REG(SIGREGQ *q, MATCHREGQ *mq)
{
	sqlite3 *infoDB = NULL;
	sqlite3_stmt *infoSTMT = NULL;
	char *sql;
	int rc; 
	char* errorMsg = NULL;

	if(sqlite3_open("info.db", &infoDB) != SQLITE_OK)
	{
		fprintf(stderr, "DB������ ��ƽ��ϴ�. (���� %s)\n", sqlite3_errmsg(infoDB));
	}
	if( sqlite3_exec(infoDB, "create table IF NOT EXISTS Registry(action NUMERIC, tool NUMERIC, key TEXT, mtime NUMERIC);", NULL, NULL, NULL) == SQLITE_OK) { 
	} else {
		exit(1);
	}
	while(!q->empty()){
		sig_registry * stmp;
		stmp=q->front();
		printf("%s\n", q->front()->key);
		char sql[0x1000];
		memset(sql, 0, 0x1000);
		sprintf(sql, "select * from Registry where PATH like \"%%%s\";", q->front()->key);
		if(sqlite3_prepare_v2(infoDB, sql, strlen(sql), &infoSTMT, NULL) == SQLITE_OK) 
		{
		}
		while (1) {
			int row = 0;
			int s;
			s = sqlite3_step (infoSTMT);
			if (s == SQLITE_ROW) {
				match_registry * mtmp = (match_registry * )malloc(sizeof(match_registry));
				memset(mtmp, 0, sizeof(match_registry));
				////////////////////////////////////////////////////////////////////
				mtmp->action = stmp->action;
				sprintf(mtmp->key, "%s", (char * )sqlite3_column_text(infoSTMT, 0));
				mtmp->time =  sqlite3_column_int64 (infoSTMT, 1);
				mtmp->tool = stmp->tool;
				////////////////////////////////////////////////////////////////////
				printf("%d : %s : %d : %d\n", stmp->action, mtmp->key, mtmp->time, mtmp->tool);
				mq->push(mtmp);
				row++;

			}
			else if (s == SQLITE_DONE) {
				break;
			}
			else {
				fprintf (stderr, "Failed.\n");
				exit (1);
			}

		}
		sqlite3_finalize(infoSTMT);
		q->pop();
		free(stmp);
	}

	sqlite3_close(infoDB);
}
void Deep_matching_REG(SIGREGQ *q, MATCHREGQ *mq)
{
	sqlite3 *infoDB = NULL;
	sqlite3_stmt *infoSTMT = NULL;
	char *sql;
	int rc; 
	char* errorMsg = NULL;

	if(sqlite3_open("info.db", &infoDB) != SQLITE_OK)
	{
		fprintf(stderr, "DB������ ��ƽ��ϴ�. (���� %s)\n", sqlite3_errmsg(infoDB));
	}

	while(!q->empty()){
		sig_registry * stmp;
		stmp=q->front();
		printf("%s\n", q->front()->key);
		char sql[0x1000];
		memset(sql, 0, 0x1000);
		sprintf(sql, "select * from Registry where PATH like \"%%%s%%\";", q->front()->key);//modify
		if(sqlite3_prepare_v2(infoDB, sql, strlen(sql), &infoSTMT, NULL) == SQLITE_OK) 
		{
		}
		while (1) {
			int row = 0;
			int s;
			s = sqlite3_step (infoSTMT);
			if (s == SQLITE_ROW) {
				match_registry * mtmp = (match_registry * )malloc(sizeof(match_registry));
				memset(mtmp, 0, sizeof(match_registry));
				////////////////////////////////////////////////////////////////////
				mtmp->action = stmp->action;
				sprintf(mtmp->key, "%s", (char * )sqlite3_column_text(infoSTMT, 0));
				mtmp->time =  sqlite3_column_int64 (infoSTMT, 1);
				mtmp->tool = stmp->tool;
				////////////////////////////////////////////////////////////////////
				printf("%d : %s : %d : %d\n", stmp->action, mtmp->key, mtmp->time, mtmp->tool);
				mq->push(mtmp);
				row++;

			}
			else if (s == SQLITE_DONE) {
				break;
			}
			else {
				fprintf (stderr, "Failed.\n");
				exit (1);
			}

		}
		sqlite3_finalize(infoSTMT);
		q->pop();
		free(stmp);
	}

	sqlite3_close(infoDB);
}
void matching_FILE(SIGFILEQ *q, MATCHFILEQ *mq)
{
	sqlite3 *infoDB = NULL;
	sqlite3_stmt *infoSTMT = NULL;
	char *sql;
	int rc; 
	char* errorMsg = NULL;

	if(sqlite3_open("info.db", &infoDB) != SQLITE_OK)
	{
		fprintf(stderr, "DB������ ��ƽ��ϴ�. (���� %s)\n", sqlite3_errmsg(infoDB));
	}

	while(!q->empty()){
		sig_file * stmp;
		stmp=q->front();
		printf("%s\n", q->front()->path);
		char sql[0x1000];
		memset(sql, 0, 0x1000);
		sprintf(sql, "select * from mft where FULLPATH like \"%%%s\";", q->front()->path, q->front()->file);
		printf("%s\n", sql);
		if(sqlite3_prepare_v2(infoDB, sql, strlen(sql), &infoSTMT, NULL) == SQLITE_OK) 
		{
		}
		while (1) {
			int row = 0;
			int s;
			s = sqlite3_step (infoSTMT);
			if (s == SQLITE_ROW) {
				match_file * mtmp = (match_file * )malloc(sizeof(match_file));
				memset(mtmp, 0, sizeof(match_file));
				////////////////////////////////////////////////////////////////////
				mtmp->action = stmp->action;
				sprintf(mtmp->path, "%s", (char * )sqlite3_column_text(infoSTMT, 0));
				mtmp->SI_Atime =  sqlite3_column_int64 (infoSTMT, 1);
				mtmp->SI_Mtime =  sqlite3_column_int64 (infoSTMT, 2);
				mtmp->SI_Ctime =  sqlite3_column_int64 (infoSTMT, 3);
				mtmp->FN_Atime =  sqlite3_column_int64 (infoSTMT, 4);
				mtmp->FN_Mtime =  sqlite3_column_int64 (infoSTMT, 5);
				mtmp->FN_Ctime =  sqlite3_column_int64 (infoSTMT, 6);
				mtmp->tool = stmp->tool;
				////////////////////////////////////////////////////////////////////
				//printf("%d : %s : %d : %d\n", stmp->action, mtmp->key, mtmp->time, mtmp->tool);
				mq->push(mtmp);
				row++;

			}
			else if (s == SQLITE_DONE) {
				break;
			}
			else {
				fprintf (stderr, "Failed.\n");
				exit (1);
			}

		}
		sqlite3_finalize(infoSTMT);
		q->pop();
		free(stmp);
	}

	sqlite3_close(infoDB);
}
void Deep_matching_FILE(SIGFILEQ *q, MATCHFILEQ *mq)
{
	sqlite3 *infoDB = NULL;
	sqlite3_stmt *infoSTMT = NULL;
	char *sql;
	int rc; 
	char* errorMsg = NULL;

	if(sqlite3_open("info.db", &infoDB) != SQLITE_OK)
	{
		fprintf(stderr, "DB������ ��ƽ��ϴ�. (���� %s)\n", sqlite3_errmsg(infoDB));
	}
	
	while(!q->empty()){
		sig_file * stmp;
		stmp=q->front();
		printf("%s\n", q->front()->path);
		char sql[0x1000];
		memset(sql, 0, 0x1000);
		sprintf(sql, "select * from mft where FULLPATH like \"%%%s%%\";", q->front()->file);
		printf("%s\n", sql);
		if(sqlite3_prepare_v2(infoDB, sql, strlen(sql), &infoSTMT, NULL) == SQLITE_OK) 
		{
		}
		while (1) {
			int row = 0;
			int s;
			s = sqlite3_step (infoSTMT);
			if (s == SQLITE_ROW) {
				match_file * mtmp = (match_file * )malloc(sizeof(match_file));
				memset(mtmp, 0, sizeof(match_file));
				////////////////////////////////////////////////////////////////////
				mtmp->action = stmp->action;
				sprintf(mtmp->path, "%s", (char * )sqlite3_column_text(infoSTMT, 0));
				mtmp->SI_Atime =  sqlite3_column_int64 (infoSTMT, 1);
				mtmp->SI_Mtime =  sqlite3_column_int64 (infoSTMT, 2);
				mtmp->SI_Ctime =  sqlite3_column_int64 (infoSTMT, 3);
				mtmp->FN_Atime =  sqlite3_column_int64 (infoSTMT, 4);
				mtmp->FN_Mtime =  sqlite3_column_int64 (infoSTMT, 5);
				mtmp->FN_Ctime =  sqlite3_column_int64 (infoSTMT, 6);
				mtmp->tool = stmp->tool;
				////////////////////////////////////////////////////////////////////
				//printf("%d : %s : %d : %d\n", stmp->action, mtmp->key, mtmp->time, mtmp->tool);
				mq->push(mtmp);
				row++;

			}
			else if (s == SQLITE_DONE) {
				break;
			}
			else {
				fprintf (stderr, "Failed.\n");
				exit (1);
			}

		}
		sqlite3_finalize(infoSTMT);
		q->pop();
		free(stmp);
	}

	sqlite3_close(infoDB);
}
void REG2SQL(MATCHREGQ * mq)
{
	 sqlite3 *db = NULL; 
    sqlite3_stmt *stmt = NULL;
    char *sql;
    int rc; 
    unsigned int i;
    char *buffer = (char *)malloc(500); 
    char* errorMsg = NULL;

    memset(buffer, 0x00, sizeof(char)*500); 


    if(sqlite3_open("info.db", &db) != SQLITE_OK)
    {
        fprintf(stderr, "DB������ ��ƽ��ϴ�. (���� %s)\n", sqlite3_errmsg(db));
    }

	if( sqlite3_exec(db, "create table IF NOT EXISTS match_Registry(action NUMERIC, tool NUMERIC, key TEXT, mtime NUMERIC);", NULL, NULL, NULL) == SQLITE_OK) { 
	} else {
		exit(1);
	}

    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &errorMsg); 
  
	sprintf (buffer,"INSERT INTO match_Registry(action, tool, key, mtime) VALUES ( ?1, ?2, ?3, ?4)");

    if(sqlite3_prepare_v2(db, buffer, strlen(buffer), &stmt, NULL) == SQLITE_OK) 
    {
        puts(">> Prepared Statement is ready : Succeeded!\n");
    }
    else
    {
        puts("���̺� �� �Է¿� �����Ͽ����ϴ�.");
    }

	 while (!mq->empty())
    {
		match_registry * tmp;
		tmp=mq->front();
		sqlite3_bind_int(stmt, 1, mq->front()->action);
		sqlite3_bind_int(stmt, 2, mq->front()->tool);
		sqlite3_bind_text(stmt, 3, mq->front()->key, strlen(mq->front()->key), SQLITE_STATIC);
		sqlite3_bind_int64(stmt, 4, mq->front()->time);
        mq->pop();
		if ( sqlite3_step(stmt) != SQLITE_DONE )  {
        }
        sqlite3_reset(stmt); 
		free(tmp);
    }
    rc = sqlite3_exec(db, "COMMIT TRANSACTION;", NULL, NULL, &errorMsg);
    //fprintf(stderr, " Commit result : %s\n", errorMsg);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

}
void FILE2SQL(MATCHFILEQ * mq)
{

	sqlite3 *db = NULL; 
    sqlite3_stmt *stmt = NULL;
    char *sql;
    int rc; 
    unsigned int i;
    char *buffer = (char *)malloc(500); 
    char* errorMsg = NULL;

    memset(buffer, 0x00, sizeof(char)*500); 


    if(sqlite3_open("info.db", &db) != SQLITE_OK)
    {
        fprintf(stderr, "DB������ ��ƽ��ϴ�. (���� %s)\n", sqlite3_errmsg(db));
    }

	if( sqlite3_exec(db, "create table IF NOT EXISTS match_file(action NUMERIC, tool NUMERIC, path TEXT, FN_mtime NUMERIC, FN_ctime NUMERIC, FN_atime NUMERIC, SI_mtime NUMERIC, SI_atime NUMERIC, SI_ctime NUMERIC);", NULL, NULL, NULL) == SQLITE_OK) { 
	} else {
		exit(1);
	}

    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &errorMsg); 
  
	sprintf (buffer,"INSERT INTO match_file(action, tool, path, FN_mtime, FN_ctime, FN_atime, SI_mtime, SI_ctime, SI_atime) VALUES ( ?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9)");

    if(sqlite3_prepare_v2(db, buffer, strlen(buffer), &stmt, NULL) == SQLITE_OK) 
    {
        puts(">> Prepared Statement is ready : Succeeded!\n");
    }
    else
    {
        puts("���̺� �� �Է¿� �����Ͽ����ϴ�.");
    }

	 while (!mq->empty())
    {
		match_file * tmp;
		tmp=mq->front();
		sqlite3_bind_int(stmt, 1, mq->front()->action);
		sqlite3_bind_int(stmt, 2, mq->front()->tool);
		sqlite3_bind_text(stmt, 3, mq->front()->path, strlen(mq->front()->path), SQLITE_STATIC);
		sqlite3_bind_int64(stmt, 4, mq->front()->FN_Mtime);
		sqlite3_bind_int64(stmt, 5, mq->front()->FN_Ctime);
		sqlite3_bind_int64(stmt, 6, mq->front()->FN_Atime);
		sqlite3_bind_int64(stmt, 7, mq->front()->SI_Mtime);
		sqlite3_bind_int64(stmt, 8, mq->front()->SI_Ctime);
		sqlite3_bind_int64(stmt, 9, mq->front()->FN_Atime);
        mq->pop();
		if ( sqlite3_step(stmt) != SQLITE_DONE )  {
        }
        sqlite3_reset(stmt); 
		free(tmp);
    }
    rc = sqlite3_exec(db, "COMMIT TRANSACTION;", NULL, NULL, &errorMsg);
    //fprintf(stderr, " Commit result : %s\n", errorMsg);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}