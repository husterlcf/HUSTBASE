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
	char tablename[21];//表的名称
	int attrcount ;//属性数量
}table; 
typedef struct  {
	char tablename[21];//表名
	char attrname[21];//属性名
	AttrType attrtype;//属性类型
	int attrlength;//属性的长度
	int attroffset;//属性在记录中的偏移量
	char ix_flag;//该列是否存在索引
	char indexname[21];//索引名称
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