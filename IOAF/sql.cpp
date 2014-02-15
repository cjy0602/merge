#include "sql.h"

int reg2sql(REGQUEUE *q)
{
    //database ���� �����ڵ�
    sqlite3 *db = NULL; 
    sqlite3_stmt *stmt = NULL; //sqlite3 statement 
    char *sql; //������ ���� ��
    int rc; //sqlite3 �Լ��� ����ؼ� ���� ���� �����ϴ� �κ��ε�. �׳� �����ų�� �޴� ������ �����ϸ� �ɵ�.
    unsigned int i; //for �� ���� �� ����
    char *buffer = (char *)malloc(500);  //sql�� �ص� ������ insert�κ��� ������ �ֱ� ���� ������ ���� �������
    int error = sqlite3_open("test2.db", &db);// db�̸� ��ȯ
    char* errorMsg = NULL; //error ������ �� ��� ���� ����

    memset(buffer, 0x00, sizeof(char)*500); //buffer�κ��� �޸� �ʱ�ȭ

	//���⼭���� ���̺� ���� �������� db���� �����κ�

    if(error)
    {
        fprintf(stderr, "DB������ ��ƽ��ϴ�. (���� %s)\n", sqlite3_errmsg(db));
    }
    fprintf(stdout, "DB���� �Ϸ�.\n");
    if(sqlite3_open("test2.db", &db) != SQLITE_OK)
    {
        fprintf(stderr, "DB������ ��ƽ��ϴ�. (���� %s)\n", sqlite3_errmsg(db));
    }

    //MFT ���̺� ����
    sql = "CREATE TABLE IF NOT EXISTS Regist (PATH TEXT ,TIME INTEGER);";
    if( sqlite3_exec(db, sql, NULL, NULL, NULL) == SQLITE_OK) { //����Բ��� ����� ���Ͻ÷��� �������ֽ� �κ�. ����Ǹ� �ֿܼ� succeeded ��µ�.
        //fprintf(stderr, ">> SQLite Table creation Succeeded!\n");
    } else {
       // puts("���̺� ������ �����߽��ϴ�.");
        exit(1);
    }

    //������ �߰� �ڵ�.
    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &errorMsg); //insert �ӵ� ���� ���� ����� �ڵ�. insert�� ���� �� ������ ���嵵 ����.
    //fprintf(stderr, " Commit begin result : %s\n", errorMsg); //����� ���ϰ� �ϱ� ���� �ֿܼ� ������ִ� �κ�. ���߿��� ������ �ɵ�
	sprintf (buffer,"INSERT INTO Regist(PATH, TIME) VALUES ( ?1, ?2)");
  //buffer��� ������ insert �������� �ִ� ��. �������� ������ �°� �ְ�. values�� �������. 1,2,3,4,5,6~~ ���ڴ� �Ʒ� sqlite3_bind_text stmt, ���ںκ��� �ٲ��ָ� ��. ���ڴ����� ������ִ� ��������)
    if(sqlite3_prepare_v2(db, buffer, strlen(buffer), &stmt, NULL) == SQLITE_OK) //����� Ȯ�� ���� if��
    {
        puts(">> Prepared Statement is ready : Succeeded!\n");
    }
    else
    {
        puts("���̺� �� �Է¿� �����Ͽ����ϴ�.");
    }

	int size = q->size();

	 while (!q->empty())
    {
		sqlite3_bind_text(stmt, 1, q->front()->key, strlen(q->front()->key), SQLITE_STATIC);
		sqlite3_bind_int(stmt, 2, q->front()->time);
        q->pop();
		if ( sqlite3_step(stmt) != SQLITE_DONE )  {
        }
        sqlite3_reset(stmt); 
    }
    rc = sqlite3_exec(db, "COMMIT TRANSACTION;", NULL, NULL, &errorMsg);//���� insert�� �ӵ������� �ϱ����Ѱ� ����ġ�°�.
    fprintf(stderr, " Commit result : %s\n", errorMsg);//��������ϱ� ���� �ֿܼ� �����ٰ� ǥ��
    sqlite3_finalize(stmt);//sqlite3�۵� ������ �Լ�.

    sqlite3_close(db);//db ����.
    return 0;
 }