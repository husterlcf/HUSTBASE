#ifndef IX_MANAGER_H_H
#define IX_MANAGER_H_H

#include "RM_Manager.h"
#include "PF_Manager.h"

typedef struct{
	int attrLength;
	int keyLength;
	AttrType attrType;
	PageNum rootPage;
	PageNum first_leaf;
	int order;
}IX_FileHeader;

typedef struct{
	bool bOpen;
	PF_FileHandle fileHandle;
	IX_FileHeader fileHeader;
}IX_IndexHandle;

typedef struct{
	int is_leaf;
	int keynum;
	PageNum parent;
	PageNum brother;
	char *keys;
	RID *rids;
}IX_Node;

typedef struct{
	bool bOpen;		/*扫描是否打开 */
	IX_IndexHandle *pIXIndexHandle;	//指向索引文件操作的指针
	CompOp compOp;  /* 用于比较的操作符*/
	char *value;		 /* 与属性行比较的值 */
    PF_PageHandle pfPageHandles[PF_BUFFER_SIZE]; // 固定在缓冲区页面所对应的页面操作列表
	PageNum pnNext; 	//下一个将要被读入的页面号
	int ridIx;
}IX_IndexScan;

typedef struct Tree_Node{
	int  keyNum;		//节点中包含的关键字（属性值）个数
	char  **keys;		//节点中包含的关键字（属性值）数组
	Tree_Node  *parent;	//父节点
	Tree_Node  *sibling;	//右边的兄弟节点
	Tree_Node  *firstChild;	//最左边的孩子节点
}Tree_Node; //节点数据结构

typedef struct{
	AttrType  attrType;	//B+树对应属性的数据类型
	int  attrLength;	//B+树对应属性值的长度
	int  order;			//B+树的序数
	Tree_Node  *root;	//B+树的根节点
}Tree;


RC IXCreateIndex(char * fileName,AttrType attrType,int attrLength);
RC OpenIndex(char *fileName,IX_IndexHandle *indexHandle);
RC CloseIndex(IX_IndexHandle *indexHandle);

RC InsertEntry(IX_IndexHandle *indexHandle,void *pData,RID * rid);
RC DeleteEntry(IX_IndexHandle *indexHandle,void *pData,RID * rid);
RC OpenIndexScan(IX_IndexScan *indexScan,IX_IndexHandle *indexHandle,CompOp compOp,char *value);
RC IX_GetNextEntry(IX_IndexScan *indexScan,RID * rid);
RC CloseIndexScan(IX_IndexScan *indexScan);
RC GetIndexTree(char *fileName, Tree *index);

void searchpath(PF_PageHandle * pagehandle, int order, AttrType attrtype, int attrlength, void * pdata, RID * rid);
void insertkeyandrid(PF_PageHandle * pagehandle, int order, AttrType attrtype, int attrlength, void * pdata, RID * rid);
void copykeyandrid(PF_PageHandle *pagehandle, void * keydata, int attrlength, int num, int order, void * riddata);
void if_existence(PF_PageHandle * pagehandle, int order, AttrType attrtype, int attrlength, void *pdata, RID *rid, bool *existence);
void lessthanleastnum(PF_PageHandle *pagehandle, PF_FileHandle *filehandle, int order, AttrType attrtype, int attrlength, int *status);
void leftbrother(PF_PageHandle *pagehandle, PF_FileHandle *filehandle, int order, AttrType attrtype, int attrlength, PageNum * leftbrothernum);
void solvewithleft(PF_PageHandle *pagehandle, PF_PageHandle *lefthandle, int order, AttrType attrtype, int attrlength, int *status);
void solvewithright(PF_PageHandle *pagehandle, PF_PageHandle *righthandle, int order, AttrType attrtype, int attrlength, int *status);
void deleteNode(PF_PageHandle *parentpagehandle, PF_FileHandle *filehandle, int order, AttrType attrtype, int attrlength, PageNum nodepagenum, bool ifdelete, void *pdata);

void scan_lequal(IX_IndexHandle *indexHandle, void *pdata, PageNum *startpagenum, int *ridIx);
int comparekeys(void *data1, void *data2, AttrType attrtype);
void scan_thefirstequal(IX_IndexHandle * indexhandle, void *pdata, PageNum *startpagenum, int *ridIx, bool *existence);
void update_parent(PF_FileHandle *filehandle, IX_FileHeader fileheader);

#endif