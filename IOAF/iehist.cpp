/*
   IE History Viewer
   Copyright (c) 2003 Patrik Karlsson

   http://www.cqure.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
//#include "c:\Users\cjy\Desktop\MFTParser\sqlite\Debug\sqlite3.h"
#include <sqlite3.h>
#include "time.h"
#include "define.h"

const int TYPE_URL  = 0x01;
const int TYPE_REDR = 0x02;

const int URL_URL_OFFSET  = 104;  // 0x68 Next, we see that the actual URL the user visited is located at offset 0x68 from the beginning o
const int URL_TIME_OFFSET = 16;  // Last accessed Time Stamp
const int REDR_URL_OFFSET = 16;  

const int Download_URL_OFFSET = 468; // 0x1D4;

DWORD dwURLCount = 0;
// History, COokie, Caches �Ľ� ����ü
struct history {
	int nType;
	char *pURL;
	SYSTEMTIME st;
};

struct history_saved {
	int nType;
	char pURL[1024];
	SYSTEMTIME st;
};

// IEHistoryDownload - index.dat Parsing Struct 
struct history_download {
	int nType;
	char *pURL;
	SYSTEMTIME st;
	WCHAR *pReferer;
	WCHAR *pDownloadURL;
	WCHAR *pLocation;

	//char *Referer;
	//char *DownloadURL;
	//char *pLocation;
};

struct history_download_saved {
	int nType;
	char pURL[1024];
	SYSTEMTIME st;
	CHAR pReferer[1024];
	CHAR pDownloadURL[1024];
	CHAR pLocation[1024];
};


int bMatchPattern( char *pBuf ) {

	// 
	//if ( pBuf[0] == 0x55 && pBuf[1] == 0x52 && pBuf[2] == 0x4c && pBuf[3] == 0x20 && pBuf[5] == 0x00 && pBuf[6] == 0x00)
	if ( pBuf[0] == 0x55 && pBuf[1] == 0x52 && pBuf[2] == 0x4c && pBuf[3] == 0x20 )
	{
		return TYPE_URL;
	}

	if ( pBuf[0] == 0x52 && pBuf[1] == 0x45 && pBuf[2] == 0x44 && pBuf[3] == 0x52 ) 
	{
		return TYPE_REDR;
	}

	return 0;

}

struct history_download *getDownload( char *pBuf, int nType ) {

	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	int a;

	struct history_download *pDownlaod;
	FILETIME ft;

	if ( nType == TYPE_URL ) {

		pDownlaod = (struct history_download*) malloc ( sizeof( struct history_download ) );

		// Last accessed Time Stamp ����ü�� ����.
		memcpy( (DWORD *)&ft.dwLowDateTime, pBuf + URL_TIME_OFFSET, sizeof( DWORD ) );
		memcpy( (DWORD *)&ft.dwHighDateTime, pBuf + URL_TIME_OFFSET + 4, sizeof( DWORD ) );
		                                 
		FileTimeToSystemTime( &ft, &pDownlaod->st );
		pDownlaod->nType = TYPE_URL;

		pBuf += URL_URL_OFFSET;

	}
	// REDR �Ӽ� ���� �ϴ� no-Touch
	else if ( nType == TYPE_REDR ) {
		pDownlaod = (struct history_download*) malloc ( sizeof( struct history_download ) );
		ft.dwHighDateTime = 0;
		ft.dwLowDateTime = 0;
		FileTimeToSystemTime( &ft, &pDownlaod->st );
		pDownlaod->nType = TYPE_REDR;
		pBuf += REDR_URL_OFFSET;
	}

	else {
		return NULL;
	}

	// ------------ GUID Value Parsing Start
	// ���� ��ġ (���� URL�� ����� ��ġ���� ���� NULL�� ������ �κб����� ���̸� ���ϱ� ���ؼ� i�� ������)
	while ( pBuf[i] != 0 ) {  
		i++;
	}

	if ( i > 1024 )
		return NULL;

	pDownlaod->pURL = (char *) malloc( 1024 );
	memset( pDownlaod->pURL, 0, 1024 );

	// �ݺ������� ���� ���� ��ŭ �����Ѵ�.
	strncpy( pDownlaod->pURL, pBuf, i );
	// ------------ GUID Value Parsing End

	pBuf += (Download_URL_OFFSET - URL_URL_OFFSET);		// 0x1D4 - 0x68 = 0x16c
	//printf("pBuf�� ù ���� (������ Ȯ�ο�) = %x\n\n", *pBuf) ;   // 0x68 �� ���´ٸ� offset�� ��Ȯ�� ��ġ��.

	//printf( "pbuf[%d] = %x\n", j, pBuf[j]);		// 0x68
	//printf( "pbuf[%d] = %x\n", j+1, pBuf[j+1]);	// 0x00
	//printf( "pbuf[%d] = %x\n", j+2, pBuf[j+2]);	// 0x74
	//printf( "pbuf[%d] = %x\n", j+3, pBuf[j+3]);	// 0x00

	while (	pBuf[j] != 0x00 || pBuf[j+1] != 0x00 )
	{
		//printf( "pbuf[%d] = %x\n", j, pBuf[j]);
		j++;
	}

	if ( j > 2048 )
		return NULL;

	//printf(" \nString�� ���� = %d\n", j );
	//j = j / 2 ;

	pDownlaod->pReferer = (WCHAR *) malloc( 2048 );
	memset( pDownlaod->pReferer, 0, 2048 );
	//wcsncpy( pDownlaod->pReferer, (WCHAR *)pBuf , j );
	memcpy ( pDownlaod->pReferer, pBuf , j+1);

	char *Temp0;
	int len;
	len = WideCharToMultiByte(CP_ACP, 0, pDownlaod->pReferer, -1, NULL, 0, NULL,NULL);
	Temp0 = new char[len];
	WideCharToMultiByte(CP_ACP, 0, pDownlaod->pReferer, -1, Temp0, 128, NULL, NULL);

	//wprintf(L"\n\n DEBUG pReferer = %s\n", pDownlaod->pReferer);
	//printf(" Convert = %s", Temp0);

	/* Hex dump
		printf("================== HEX DUMP START ===============\n");
		printf("0x%08x   ", &pDownlaod->pReferer);

		for ( a = 0; a < j ; a++)
			printf("%02x", pDownlaod->pReferer[a]);

		printf("\n================== HEX DUMP END ================\n");
	*/


	pBuf += j+3; // DownloadURL�� ������ �̵�.
	//printf("pBuf�� �� ���� (������ Ȯ�ο�) = %x\n\n", *pBuf) ;   // 0x68 �� ���´ٸ� offset�� ��Ȯ�� ��ġ��.

	while (	pBuf[k] != 0x00 || pBuf[k+1] != 0x00 )
	{
		k++;
	}

	if ( k > 2048 )
		return NULL;

	pDownlaod->pDownloadURL = (WCHAR *) malloc( 2048 );
	memset( pDownlaod->pDownloadURL, 0, 2048 );
	//wcsncpy( pDownlaod->pDownloadURL, (WCHAR *)pBuf , j );
	memcpy ( pDownlaod->pDownloadURL, pBuf , k+1);

	char *Temp1;
	int len1;
	len1 = WideCharToMultiByte(CP_ACP, 0, pDownlaod->pDownloadURL, -1, NULL, 0, NULL,NULL);
	Temp1 = new char[len1];

	WideCharToMultiByte(CP_ACP, 0, pDownlaod->pDownloadURL, -1, Temp1, 128, NULL, NULL);

	//wprintf(L"\n\n DEBUG pDownloadURL = %s\n", pDownlaod->pDownloadURL);
	//printf(" Convert = %s", Temp1);

	pBuf += k+3; // Location���� ������ �̵�.
	//printf("pBuf�� �� ���� (������ Ȯ�ο�) = %x\n\n", *pBuf) ;   // 0x68 �� ���´ٸ� offset�� ��Ȯ�� ��ġ��.

	while (	pBuf[l] != 0x00 || pBuf[l+1] != 0x00 )
	{
		l++;
	}

	if ( l > 2048 )
		return NULL;

	pDownlaod->pLocation = (WCHAR *) malloc( 2048 );
	memset( pDownlaod->pLocation, 0, 2048 );
	//wcsncpy( pDownlaod->pLocation, (WCHAR *)pBuf , j );
	memcpy ( pDownlaod->pLocation, pBuf , l+1);

	char *Temp2;
	int len2;
	len2 = WideCharToMultiByte(CP_ACP, 0, pDownlaod->pLocation, -1, NULL, 0, NULL,NULL);
	Temp2 = new char[len2];

	WideCharToMultiByte(CP_ACP, 0, pDownlaod->pLocation, -1, Temp2, 128, NULL, NULL);

	//wprintf(L"\n\n DEBUG pLocation = %s\n", pDownlaod->pLocation);
	//printf(" Convert = %s\n", Temp2);

	//printf(" ############################# function END \n");

	return pDownlaod;	
}

struct history *getURL( char *pBuf, int nType ) {

	int i = 0;
	struct history *pHistory;
	FILETIME ft;

	if ( nType == TYPE_URL ) {

		pHistory = (struct history*) malloc ( sizeof( struct history ) );

		// Fri Nov 22 1963 00:00:00 ������ ���Ѵ�. 
		// Last accessed Time Stamp ����ü�� ����.
		memcpy( (DWORD *)&ft.dwLowDateTime, pBuf + URL_TIME_OFFSET, sizeof( DWORD ) );
		memcpy( (DWORD *)&ft.dwHighDateTime, pBuf + URL_TIME_OFFSET + 4, sizeof( DWORD ) );

		FileTimeToSystemTime( &ft, &pHistory->st );
		pHistory->nType = TYPE_URL;

		pBuf += URL_URL_OFFSET;

	}

	// REDR �Ӽ� ���� �ϴ� no-Touch
	else if ( nType == TYPE_REDR ) {
		pHistory = (struct history*) malloc ( sizeof( struct history ) );
		ft.dwHighDateTime = 0;
		ft.dwLowDateTime = 0;
		FileTimeToSystemTime( &ft, &pHistory->st );
		pHistory->nType = TYPE_REDR;
		pBuf += REDR_URL_OFFSET;
	}

	else {
		return NULL;
	}

	// ���� ��ġ (���� URL�� ����� ��ġ���� ���� NULL�� ������ �κб����� ���̸� ���ϱ� ���ؼ� i�� ������)
	while ( pBuf[i] != 0 ) {  
		i++;
	}

	if ( i > 1024 )
		return NULL;

	pHistory->pURL = (char *) malloc( 1024 );
	memset( pHistory->pURL, 0, 1024 );

	// �ݺ������� ���� ���� ��ŭ �����Ѵ�.
	strncpy( pHistory->pURL, pBuf, i );

	return pHistory;	
}

void print_DownloadHistory( struct history_download *pDownlaod ) {

	char bufType[256];

	memset( bufType, 0, sizeof( bufType ) );
	if ( pDownlaod->nType == TYPE_URL ) {
		strcpy( bufType, "URL");
	}
	else if ( pDownlaod->nType == TYPE_REDR ) {
		strcpy( bufType, "REDR");
	}
	
	
	fprintf(stdout, "%s|", bufType );   // URL or REDR
	
	/* The REDRs do not have a time stamp I think ..... */
	if ( pDownlaod->nType != TYPE_REDR )
		fprintf(stdout, "%d/%d/%d %d:%d:%d|", pDownlaod->st.wYear, pDownlaod->st.wMonth, pDownlaod->st.wDay, pDownlaod->st.wHour, pDownlaod->st.wMinute, pDownlaod->st.wSecond);
	else
		/* skip date and time */
		fprintf(stdout, " |");	// �� ���� ����

	fprintf(stdout, "%s|", pDownlaod->pURL );  // URL �Ľ�.
	fprintf(stdout, "%S|", pDownlaod->pReferer );
	fprintf(stdout, "%S|", pDownlaod->pDownloadURL );
	fprintf(stdout, "%S|\n", pDownlaod->pLocation );
}

void printHistory( struct history *pHistory ) {

	char bufType[256];

	memset( bufType, 0, sizeof( bufType ) );
	if ( pHistory->nType == TYPE_URL ) {
		strcpy( bufType, "URL");
	}
	else if ( pHistory->nType == TYPE_REDR ) {
		strcpy( bufType, "REDR");
	}
	
	
	fprintf(stdout, "%s|", bufType );   // URL or REDR
	
	/* The REDRs do not have a time stamp I think ..... */
	if ( pHistory->nType != TYPE_REDR )
		fprintf(stdout, "%d/%d/%d %d:%d:%d|", pHistory->st.wYear, pHistory->st.wMonth, pHistory->st.wDay, pHistory->st.wHour, pHistory->st.wMinute, pHistory->st.wSecond);
	else
		/* skip date and time */
		fprintf(stdout, " |");	// �� ���� ����
	
	fprintf(stdout, "%s\n", pHistory->pURL );  // URL �Ľ�.

}

time_t TimeFromSystemTime(const SYSTEMTIME pTime)
{
    struct tm tm;
	time_t t;
	
    memset(&tm, 0, sizeof(tm));

	 tm.tm_hour = pTime.wHour;
    tm.tm_min = pTime.wMinute;
    tm.tm_sec = pTime.wSecond;

	tm.tm_mon = pTime.wMonth - 1;
    tm.tm_mday = pTime.wDay;
    tm.tm_year = pTime.wYear-1900;
    

    return mktime(&tm);
}

int MOD1(struct history_saved  *his, DWORD dwURLCount)
{
    sqlite3 *db = NULL; 
    sqlite3_stmt *stmt = NULL; //sqlite3 statement 
    char *sql; 
    int rc; 
    unsigned int i; 
    char *buffer = (char *)malloc(500);  
 
    memset(buffer, 0x00, sizeof(char)*500); 

    int error = sqlite3_open("match.db", &db);
    if(error)
    {
        fprintf(stderr, "DB������ ��ƽ��ϴ�. (���� %s)\n", sqlite3_errmsg(db));
    }
    fprintf(stdout, "DB���� �Ϸ�.\n");
    if(sqlite3_open("match.db", &db) != SQLITE_OK)
    {
        fprintf(stderr, "DB������ ��ƽ��ϴ�. (���� %s)\n", sqlite3_errmsg(db));
    }

    //MFT ���̺� ����
    sql = "CREATE TABLE IF NOT EXISTS HISTORY (URL TEXT, TYPE INT, TIME INT);";
    if( sqlite3_exec(db, sql, NULL, NULL, NULL) == SQLITE_OK) { 
        fprintf(stderr, ">> SQLite Table creation Succeeded!\n");
    } else {
        puts("���̺� ������ �����߽��ϴ�.");
        exit(1);
    }
    
    //if(sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) //SQL �������� �ִ� �ڵ�. �׷��� ���̺��� ������. �������� sql ������ ����
    //{
    //    if ( sqlite3_step(stmt) != SQLITE_DONE )  {
    //        fprintf(stderr, ">> SQLite Table creation failed!\n");
    //        exit(1);
    //    }
    //}
    //else
    //{
    //    puts("���̺� ������ �����߽��ϴ�.");
    //}
    //sqlite3_finalize(stmt);

    //������ �߰� �ڵ�.
    char* errorMsg = NULL;
    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &errorMsg);
    fprintf(stderr, " Commit begin result : %s\n", errorMsg);
    sprintf (buffer,"INSERT INTO HISTORY(URL, TYPE, TIME) VALUES ( ?1, ?2, ?3)");
    //sprintf (buffer,"INSERT INTO MFT(FILENAME, entry, ParentRef, Sl_writeTm, SI_createTm, SI_accessTm, SI_mftTm, FN_writeTm, FN_createTm, FN_accessTm, FN_mftTm) VALUES ( ?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11)", , , , u3[i]., u3[i]., u3[i]., u3[i]., u3[i]., u3[i]., u3[i]., u3[i].);

    if(sqlite3_prepare_v2(db, buffer, strlen(buffer), &stmt, NULL) == SQLITE_OK)
    {
        puts(">> Prepared Statement is ready : Succeeded!\n");
    }
    else
    {
        puts("���̺� �� �Է¿� �����Ͽ����ϴ�.");
    }

    for( i=0; i<dwURLCount; i++){

        //sprintf (buffer,"INSERT INTO MFT(FILENAME, entry, ParentRef, Sl_writeTm, SI_createTm, SI_accessTm, SI_mftTm, FN_writeTm, FN_createTm, FN_accessTm, FN_mftTm) VALUES ( \"%s\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\")", u3[i].FILENAME, (int)u3[i].entry, (int)u3[i].ParentRef, u3[i].SI_writeTm, u3[i].SI_createTm, u3[i].SI_accessTm, u3[i].SI_mftTm, u3[i].FN_writeTm, u3[i].FN_createTm, u3[i].FN_accessTm, u3[i].FN_mftTm);

        sqlite3_bind_text(stmt, 1, his[i].pURL, strlen(his[i].pURL), SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, (int)(his[i].nType));
        sqlite3_bind_int(stmt, 3, (int)TimeFromSystemTime(his[i].st));

        if ( sqlite3_step(stmt) != SQLITE_DONE )  {
            fprintf(stderr, ">> SQLite Insert failed! \n");
        }

        //sqlite3_finalize(stmt);
        sqlite3_reset(stmt);
    }
    rc = sqlite3_exec(db, "COMMIT TRANSACTION;", NULL, NULL, &errorMsg);
    fprintf(stderr, " Commit result : %s\n", errorMsg);
    sqlite3_finalize(stmt);

    sqlite3_close(db);
    for(i=0; i<165000;i++){
        //printf("%s, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n", u3[i].FILENAME, u3[i].entry, u3[i].ParentRef, u3[i].SI_writeTm, u3[i].SI_createTm, u3[i].SI_accessTm, u3[i].SI_mftTm, u3[i].FN_writeTm, u3[i].FN_createTm, u3[i].FN_accessTm, u3[i].FN_mftTm);
    }
    return 0;
 }

int MOD2(struct history_download_saved  *his, DWORD dwURLCount)
{
    sqlite3 *db = NULL; 
    sqlite3_stmt *stmt = NULL; //sqlite3 statement 
    char *sql; 
    int rc; 
    unsigned int i; 
    char *buffer = (char *)malloc(500);  
 
    memset(buffer, 0x00, sizeof(char)*500); 

    int error = sqlite3_open("match.db", &db);
    if(error)
    {
        fprintf(stderr, "DB������ ��ƽ��ϴ�. (���� %s)\n", sqlite3_errmsg(db));
    }
    fprintf(stdout, "DB���� �Ϸ�.\n");
    if(sqlite3_open("match.db", &db) != SQLITE_OK)
    {
        fprintf(stderr, "DB������ ��ƽ��ϴ�. (���� %s)\n", sqlite3_errmsg(db));
    }

    //MFT ���̺� ����
    sql = "CREATE TABLE IF NOT EXISTS DOWNLOAD (URL TEXT, TYPE INT, TIME INT, Referer TEXT, DownloadURL TEXT, Location TEXT);";
    if( sqlite3_exec(db, sql, NULL, NULL, NULL) == SQLITE_OK) { 
        fprintf(stderr, ">> SQLite Table creation Succeeded!\n");
    } else {
        puts("���̺� ������ �����߽��ϴ�.");
        exit(1);
    }
    
    //if(sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) //SQL �������� �ִ� �ڵ�. �׷��� ���̺��� ������. �������� sql ������ ����
    //{
    //    if ( sqlite3_step(stmt) != SQLITE_DONE )  {
    //        fprintf(stderr, ">> SQLite Table creation failed!\n");
    //        exit(1);
    //    }
    //}
    //else
    //{
    //    puts("���̺� ������ �����߽��ϴ�.");
    //}
    //sqlite3_finalize(stmt);



    //������ �߰� �ڵ�.
    char* errorMsg = NULL;
    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &errorMsg);
    fprintf(stderr, " Commit begin result : %s\n", errorMsg);
    sprintf (buffer,"INSERT INTO DOWNLOAD(URL, TYPE, TIME, Referer, DownloadURL, Location) VALUES ( ?1, ?2, ?3, ?4, ?5, ?6)");
    //sprintf (buffer,"INSERT INTO MFT(FILENAME, entry, ParentRef, Sl_writeTm, SI_createTm, SI_accessTm, SI_mftTm, FN_writeTm, FN_createTm, FN_accessTm, FN_mftTm) VALUES ( ?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11)", , , , u3[i]., u3[i]., u3[i]., u3[i]., u3[i]., u3[i]., u3[i]., u3[i].);

    if(sqlite3_prepare_v2(db, buffer, strlen(buffer), &stmt, NULL) == SQLITE_OK)
    {
        puts(">> Prepared Statement is ready : Succeeded!\n");
    }
    else
    {
        puts("���̺� �� �Է¿� �����Ͽ����ϴ�.");
    }

    for( i=0; i<dwURLCount; i++){

        //sprintf (buffer,"INSERT INTO MFT(FILENAME, entry, ParentRef, Sl_writeTm, SI_createTm, SI_accessTm, SI_mftTm, FN_writeTm, FN_createTm, FN_accessTm, FN_mftTm) VALUES ( \"%s\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\")", u3[i].FILENAME, (int)u3[i].entry, (int)u3[i].ParentRef, u3[i].SI_writeTm, u3[i].SI_createTm, u3[i].SI_accessTm, u3[i].SI_mftTm, u3[i].FN_writeTm, u3[i].FN_createTm, u3[i].FN_accessTm, u3[i].FN_mftTm);

        sqlite3_bind_text(stmt, 1, his[i].pURL, strlen(his[i].pURL), SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, (int)(his[i].nType));
        sqlite3_bind_int(stmt, 3, (int)TimeFromSystemTime(his[i].st));
		sqlite3_bind_text(stmt, 4, his[i].pReferer, strlen(his[i].pReferer), SQLITE_STATIC);
		sqlite3_bind_text(stmt, 5, his[i].pDownloadURL, strlen(his[i].pDownloadURL), SQLITE_STATIC);
		sqlite3_bind_text(stmt, 6, his[i].pLocation, strlen(his[i].pLocation), SQLITE_STATIC);

        if ( sqlite3_step(stmt) != SQLITE_DONE )  {
            fprintf(stderr, ">> SQLite Insert failed! \n");
        }

        //sqlite3_finalize(stmt);
        sqlite3_reset(stmt);
    }
    rc = sqlite3_exec(db, "COMMIT TRANSACTION;", NULL, NULL, &errorMsg);
    fprintf(stderr, " Commit result : %s\n", errorMsg);
    sqlite3_finalize(stmt);

    sqlite3_close(db);
    for(i=0; i<165000;i++){
        //printf("%s, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n", u3[i].FILENAME, u3[i].entry, u3[i].ParentRef, u3[i].SI_writeTm, u3[i].SI_createTm, u3[i].SI_accessTm, u3[i].SI_mftTm, u3[i].FN_writeTm, u3[i].FN_createTm, u3[i].FN_accessTm, u3[i].FN_mftTm);
    }
    return 0;
}


int iehist(char* dat_name, int mode) 
{
	//setlocale(LC_ALL, "korean");
	//_wsetlocale(LC_ALL, L"korean");

	FILE *pFD = NULL;
	char *pBuf = NULL;
	long lFileSize, lRead;
	long i = 0;

	//int mode = 2; // ���۸�� ���� 1 = cookie, history, cache �Ľ� / 2 = download ����Ʈ �Ľ�.
	
	struct history *pHistory;
	struct history_saved *pHistory_saved;

	struct history_download *pDownload;
	struct history_download_saved *pDownload_saved;
	
	int nType = 0;
	

	pFD = fopen(dat_name, "rb");

	if ( pFD == NULL ) {
		fprintf(stderr, "ERROR: File \"%s\" not found\n", dat_name);
		exit(1);
	}

	fseek( pFD, 0, SEEK_END ); // ������ ������ ��ġ ����  http://luckyyowu.tistory.com/21
	lFileSize = ftell( pFD );	// ���� ��ġ�� ��ġ ���� ��ȯ �Ѵ�.  -- ������ ��ġ �̹Ƿ� ��������� ������ ũ�⸦ �ǹ��Ѵ�.
	fseek( pFD, 0, SEEK_SET ); // ������ ù ��° ��ġ ����

	pBuf = (char *)malloc( lFileSize );  // ���� ũ�� ��ŭ�� �����޸� �ڵ� �Ҵ�.

	if ( pBuf == NULL ) {
		fprintf(stderr, "Failed to allocate memory");
		exit(1);
	}

	lRead = fread( pBuf, lFileSize, 1, pFD );

	if ( lRead != 1 ) {
		fprintf(stderr, "WARNING: Failed to read complete file\n");
	}

	
	// ����ü ���� �Ҵ��� ���� ��ü ��Ʈ�� ���� ����
	while ( i < lFileSize )
	{
		if ( ( nType = bMatchPattern( pBuf + i ) ) > 0 ) 
		{
				dwURLCount ++;		
		}
		i++;
	}
	
	//printf("@@@@ DEBUG ��ü entry �� �� = %d\n", dwURLCount);
	
	i = 0;
	nType = 0;

	if (mode == 1)
	{
		pHistory_saved = (struct history_saved *)calloc(dwURLCount, sizeof(struct history_saved));
		if(pHistory_saved ==NULL){
			 puts("pHistory_Saved Malloc Failed...");
			 exit(1);
		}
	}

	if (mode == 2)
	{
		pDownload_saved = (struct history_download_saved *) calloc (dwURLCount, sizeof(struct history_download_saved));
		if(pDownload_saved ==NULL){
			 puts("pDownload_Saved Malloc Failed...");
			 exit(1);
		}
	}
	


	int z = 0;
	
	while ( i<lFileSize ) {   // 0 ~ ���� ��ü ũ�� ���� �ݺ�. (1����Ʈ�� �� �ݺ��ؼ� �Ľ��ϴ� ����)

		if ( ( nType = bMatchPattern( pBuf + i ) ) > 0 )   // TYPE URL Ȥ�� REDR�� ���� ��쿡�� �Ʒ� ���� ����.
		{

			// Cookie, Cache, History   index.dat Parsing Part
			if ( mode == 1)
			{
				pHistory = getURL( pBuf + i, nType );

				pHistory_saved[z].nType = (int)pHistory->nType;
				pHistory_saved[z].st = (SYSTEMTIME) pHistory->st;
				strncpy(pHistory_saved[z].pURL, pHistory->pURL, 1024);

				z++;

				if ( pHistory ) 
				{
					//printHistory( pHistory );
					free( pHistory->pURL );
					free( pHistory );
				}
			}

			// DownloadList  index.dat Parsing Part			
			if ( mode == 2)
			{
				pDownload = getDownload( pBuf +i, nType );

				pDownload_saved[z].nType = (int)pDownload->nType;
				pDownload_saved[z].st = (SYSTEMTIME) pDownload->st;

				strncpy(pDownload_saved[z].pURL, pDownload->pURL, 1024);

				char *Referer = NULL;
				int len1;
				len1 = WideCharToMultiByte(CP_ACP, 0, pDownload->pReferer, -1, NULL, 0, NULL,NULL);
				Referer = new char[len1];
				WideCharToMultiByte(CP_ACP, 0, pDownload->pReferer, -1, Referer, 128, NULL, NULL);

				strncpy(pDownload_saved[z].pReferer, Referer, 1024);

				char *DownloadURL = NULL;
				int len2;
				len2 = WideCharToMultiByte(CP_ACP, 0, pDownload->pDownloadURL, -1, NULL, 0, NULL,NULL);
				DownloadURL = new char[len2];
				WideCharToMultiByte(CP_ACP, 0, pDownload->pDownloadURL, -1, DownloadURL, 128, NULL, NULL);

				strncpy(pDownload_saved[z].pDownloadURL, DownloadURL, 1024);

				char *Location = NULL;
				int len3;
				len3 = WideCharToMultiByte(CP_ACP, 0, pDownload->pLocation, -1, NULL, 0, NULL,NULL);
				Location = new char[len3];
				WideCharToMultiByte(CP_ACP, 0, pDownload->pLocation, -1, Location, 128, NULL, NULL);

				strncpy(pDownload_saved[z].pLocation, Location, 1024);
			
				z++;

				if ( pDownload ) 
				{
					//print_DownloadHistory( pDownload );
					free( pDownload->pURL );
					free( pDownload->pReferer );
					free( pDownload->pDownloadURL );
					free( pDownload->pLocation );
					free (pDownload );
				}
			}

		}
		
		i ++;
	}


	fclose( pFD );
	free( pBuf );

	/*
	fprintf(stderr, "Urls retrieved %d\n", dwURLCount);

	if (mode == 1)
	{
		for ( z=0; z<dwURLCount; z++)
		{
			printf(" �� �� Ȯ��1 : %d\n", pHistory_saved[z].nType);
			printf(" �� �� Ȯ��2 : %ld\n", pHistory_saved[z].st);
			printf(" �� �� Ȯ��3 : %s\n", pHistory_saved[z].pURL);
		}
	}

	if (mode == 2)
	{
		for ( z=0; z<dwURLCount; z++)
		{
			printf(" �� �� Ȯ��1 : %d\n", pDownload_saved[z].nType);
			printf(" �� �� Ȯ��2 : %ld\n", pDownload_saved[z].st);
			printf(" �� �� Ȯ��3 : %s\n", pDownload_saved[z].pDownloadURL);
			printf(" �� �� Ȯ��4 : %s\n", pDownload_saved[z].pReferer);
			printf(" �� �� Ȯ��5 : %s\n", pDownload_saved[z].pLocation);
			printf(" �� �� Ȯ��6 : %s\n", pDownload_saved[z].pURL);
		}
	}
	*/

	
	if (mode == 1)
	{
		MOD1(pHistory_saved, dwURLCount);
			// DB�� �ִ� �ڵ� �ۼ�!!!!! 
	}

	if (mode == 2)
	{
		MOD2(pDownload_saved,dwURLCount);
			// DB�ִ� �ڵ� �ۼ� !!!
	}

	if ( mode == 1 )
		free (pHistory_saved);

	if (mode == 2)
		free (pDownload_saved);

}



// SELECT * FROM HISTORY WHERE TIME > 1392499278 ORDER by TIME LIMIT 10