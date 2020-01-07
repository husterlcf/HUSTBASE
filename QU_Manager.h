#ifndef __QUERY_MANAGER_H_
#define __QUERY_MANAGER_H_
#include "str.h"

typedef struct SelResult{
	int col_num;
	int row_num;
	AttrType type[20];	//��������ֶε���������
	int offset[20];
	int length[20];		//��������ֶ�ֵ�ĳ���
	char fields[20][20];//����ʮ���ֶ���������ÿ���ֶεĳ��Ȳ�����20
	char *res[100];	//���һ������¼
	//char *pData[100];
	
	SelResult * next_res;
}SelResult;




void Init_Result(SelResult * res);
void Destory_Result(SelResult * res);

RC Query(char * sql,SelResult * res);

RC Select(int nSelAttrs,RelAttr **selAttrs,int nRelations,char **relations,int nConditions,Condition *conditions,SelResult * res);

RC single_nocon(int nSelAttrs, RelAttr **selAttrs, int nRelations, char **relations, int nConditions, Condition *conditions, SelResult * res);
RC single_con(int nSelAttrs, RelAttr **selAttrs, int nRelations, char **relations, int nConditions, Condition *conditions, SelResult * res);

RC multi(int nSelAttrs, RelAttr **selAttrs, int nRelations, char **relations, int nConditions, Condition *conditions, SelResult * res) ;
#endif
