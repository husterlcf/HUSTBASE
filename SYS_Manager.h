#ifndef SYS_MANAGER_H_H
#define SYS_MANAGER_H_H

#include "IX_Manager.h"
#include "PF_Manager.h"
#include "RM_Manager.h"
#include "EditArea.h"
#include "str.h"

void ExecuteAndMessage(char * ,CEditArea *);

bool CanButtonClick();

typedef struct {
	char tablename[21];//�������
	int attrcount ;//��������
}table; 
typedef struct  {
	char tablename[21];//����
	char attrname[21];//������
	AttrType attrtype;//��������
	int attrlength;//���Եĳ���
	int attroffset;//�����ڼ�¼�е�ƫ����
	char ix_flag;//�����Ƿ��������
	char indexname[21];//��������
}column;



RC CreateDB(char *dbpath,char *dbname);
RC DropDB(char *dbname);
RC OpenDB(char *dbname);
RC CloseDB();

RC execute(char * sql);

RC CreateTable(char *relName,int attrCount,AttrInfo *attributes);
RC DropTable(char *relName);
RC CreateIndex(char *indexName,char *relName,char *attrName);
RC DropIndex(char *indexName);
RC Insert(char *relName,int nValues,Value * values);
RC Delete(char *relName,int nConditions,Condition *conditions);
RC Update(char *relName,char *attrName,Value *value,int nConditions,Condition *conditions);




#endif