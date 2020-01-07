#ifndef RM_MANAGER_H_H
#define RM_MANAGER_H_H

#include "PF_Manager.h"
#include "str.h"
#include "bitmanager.h"


typedef int SlotNum;

typedef struct {	
	PageNum pageNum;	//记录所在页的页号
	SlotNum slotNum;		//记录的插槽号
	bool bValid; 			//true表示为一个有效记录的标识符
}RID;

typedef struct{
	bool bValid;		 // False表示还未被读入记录
	RID  rid; 		 // 记录的标识符 
	char *pData; 		 //记录所存储的数据 
}RM_Record;


typedef struct {
    int recNum;
    int recSize;
    int recPerPage;
    int recordOffset;
    //int fileNum;
}RM_recControl;

typedef struct
{
	int bLhsIsAttr,bRhsIsAttr;//左、右是属性（1）还是值（0）
	AttrType attrType;
	int LattrLength,RattrLength;
	int LattrOffset,RattrOffset;
	CompOp compOp;
	void *Lvalue,*Rvalue;
}Con;

typedef struct{//文件句柄
	bool bOpen;//句柄是否打开（是否正在被使用）
	int recSize;
    int recPerPage;
    int recOffset;
    int bitmapLength;
    bitmanager *pageCtlBitmap;   //页面位图
    bitmanager *recCtlBitmap;    //记录位图
    PF_FileHandle file;

	//需要自定义其内部结构
}RM_FileHandle;

typedef struct{
	bool  bOpen;		//扫描是否打开 
	RM_FileHandle  *pRMFileHandle;		//扫描的记录文件句柄
	int  conNum;		//扫描涉及的条件数量 
	Con  *conditions;	//扫描涉及的条件数组指针
    PF_PageHandle  PageHandle; //处理中的页面句柄
	PageNum  pn; 	//扫描即将处理的页面号
	SlotNum  sn;		//扫描即将处理的插槽号
}RM_FileScan;


//通过对文件扫描信息查询，然后查询出每一条的记录
RC GetNextRec(RM_FileScan *rmFileScan,RM_Record *rec);


//将表文件（文件句柄）与文件扫描信息联系在一起
RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions);

RC CloseScan(RM_FileScan *rmFileScan);


//更新记录，参数文件句柄，
RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec);

//删除记录
RC DeleteRec (RM_FileHandle *fileHandle,const RID *rid);


//插入记录  ，文件句柄
RC InsertRec (RM_FileHandle *fileHandle, char *pData, RID *rid); 

RC GetRec (RM_FileHandle *fileHandle, RID *rid, RM_Record *rec); 

RC RM_CloseFile (RM_FileHandle *fileHandle);

RC RM_OpenFile (char *fileName, RM_FileHandle *fileHandle);

RC RM_CreateFile (char *fileName, int recordSize);


bool CmpString(char *left, char *right, CompOp oper);

bool CmpValue(float left, float right, CompOp oper);
#endif