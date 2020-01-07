#ifndef RM_MANAGER_H_H
#define RM_MANAGER_H_H

#include "PF_Manager.h"
#include "str.h"
#include "bitmanager.h"


typedef int SlotNum;

typedef struct {	
	PageNum pageNum;	//��¼����ҳ��ҳ��
	SlotNum slotNum;		//��¼�Ĳ�ۺ�
	bool bValid; 			//true��ʾΪһ����Ч��¼�ı�ʶ��
}RID;

typedef struct{
	bool bValid;		 // False��ʾ��δ�������¼
	RID  rid; 		 // ��¼�ı�ʶ�� 
	char *pData; 		 //��¼���洢������ 
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
	int bLhsIsAttr,bRhsIsAttr;//���������ԣ�1������ֵ��0��
	AttrType attrType;
	int LattrLength,RattrLength;
	int LattrOffset,RattrOffset;
	CompOp compOp;
	void *Lvalue,*Rvalue;
}Con;

typedef struct{//�ļ����
	bool bOpen;//����Ƿ�򿪣��Ƿ����ڱ�ʹ�ã�
	int recSize;
    int recPerPage;
    int recOffset;
    int bitmapLength;
    bitmanager *pageCtlBitmap;   //ҳ��λͼ
    bitmanager *recCtlBitmap;    //��¼λͼ
    PF_FileHandle file;

	//��Ҫ�Զ������ڲ��ṹ
}RM_FileHandle;

typedef struct{
	bool  bOpen;		//ɨ���Ƿ�� 
	RM_FileHandle  *pRMFileHandle;		//ɨ��ļ�¼�ļ����
	int  conNum;		//ɨ���漰���������� 
	Con  *conditions;	//ɨ���漰����������ָ��
    PF_PageHandle  PageHandle; //�����е�ҳ����
	PageNum  pn; 	//ɨ�輴�������ҳ���
	SlotNum  sn;		//ɨ�輴������Ĳ�ۺ�
}RM_FileScan;


//ͨ�����ļ�ɨ����Ϣ��ѯ��Ȼ���ѯ��ÿһ���ļ�¼
RC GetNextRec(RM_FileScan *rmFileScan,RM_Record *rec);


//�����ļ����ļ���������ļ�ɨ����Ϣ��ϵ��һ��
RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions);

RC CloseScan(RM_FileScan *rmFileScan);


//���¼�¼�������ļ������
RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec);

//ɾ����¼
RC DeleteRec (RM_FileHandle *fileHandle,const RID *rid);


//�����¼  ���ļ����
RC InsertRec (RM_FileHandle *fileHandle, char *pData, RID *rid); 

RC GetRec (RM_FileHandle *fileHandle, RID *rid, RM_Record *rec); 

RC RM_CloseFile (RM_FileHandle *fileHandle);

RC RM_OpenFile (char *fileName, RM_FileHandle *fileHandle);

RC RM_CreateFile (char *fileName, int recordSize);


bool CmpString(char *left, char *right, CompOp oper);

bool CmpValue(float left, float right, CompOp oper);
#endif