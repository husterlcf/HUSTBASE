#include "stdafx.h"
#include "RM_Manager.h"
#include "str.h"


RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//��ʼ��ɨ��
{
	RC result;
	if(rmFileScan->bOpen) {
		return RM_FSOPEN; //ɨ���ļ��Ѿ���
	}
    if(fileHandle->file.pFileSubHeader->nAllocatedPages<=2)
    {
        rmFileScan->pn=rmFileScan->sn=0;
        return SUCCESS;
    }
    rmFileScan->bOpen=TRUE;
    rmFileScan->conNum=conNum;
    rmFileScan->conditions=conditions;
    rmFileScan->pRMFileHandle=fileHandle;
	result = GetThisPage(&fileHandle->file,2,&rmFileScan->PageHandle);
	if (result) {
		return result;
	}
    rmFileScan->pn=2;
    rmFileScan->sn=0;
    return SUCCESS;
}

RC CloseScan(RM_FileScan *rmFileScan){

	rmFileScan->bOpen=false;
    rmFileScan->conditions=NULL;
    rmFileScan->conNum=0;
    rmFileScan->pn=rmFileScan->sn=0;
    //CloseFile(&rmFileScan->pRMFileHandle->file);
    UnpinPage(&rmFileScan->PageHandle);
    return SUCCESS;
}



RC GetNextRec(RM_FileScan *rmFileScan,RM_Record *rec)
{
	if(!rmFileScan->bOpen) {
        return RM_FSCLOSED;
    }
    //���ȼ���ļ��Ƿ�������
    if(rmFileScan->pn==0) {
        return RM_NOMORERECINMEM;
    }
    bitmanager recBitmap(1,NULL);
    while(1)
    {
        GetThisPage(&rmFileScan->pRMFileHandle->file,rmFileScan->pn,&rmFileScan->PageHandle);
        //recPerPage��Ϊ8�ı���
        recBitmap.redirectBitmap(rmFileScan->pRMFileHandle->recPerPage/8,rmFileScan->PageHandle.pFrame->page.pData);
        rmFileScan->sn=recBitmap.firstBit(rmFileScan->sn,1);
        while(((rmFileScan->sn=recBitmap.firstBit(rmFileScan->sn,1))!=-1))
        {
            Con *condition;
            bool correct=true;
            //���������бȽ�
            int recOffset=rmFileScan->pRMFileHandle->recOffset+rmFileScan->sn*rmFileScan->pRMFileHandle->recSize;
            char *recAddr=rmFileScan->PageHandle.pFrame->page.pData+recOffset;
            int conNumber;
            int leftVal,rightVal;
            float leftF,rightF;
            char *leftStr,*rightStr;
            for(conNumber=0; conNumber<rmFileScan->conNum; conNumber++)
            {
                condition=(Con *)(rmFileScan->conditions+conNumber);
                switch (condition->attrType)
                {
                case ints:
                    leftVal=(condition->bLhsIsAttr==1)?*((int *)(recAddr+condition->LattrOffset)):*((int *)condition->Lvalue);
                    if(condition->compOp==NO_OP) {
                        if(leftVal==0) {
                            correct=false;
                            break;
                        }
                        else
                            break;
                    }
                    rightVal=(condition->bRhsIsAttr==1)?*((int *)(recAddr+condition->RattrOffset)):*((int *)condition->Rvalue);
                    correct=CmpValue(leftVal,rightVal,condition->compOp);
                    break;
                case floats:
                    leftF=(condition->bLhsIsAttr==1)?*((float *)(recAddr+condition->LattrOffset)):*((float *)condition->Lvalue);
                    if(condition->compOp==NO_OP) {
                        if(leftVal==0) {
                            correct=false;
                            break;
                        }
                        else {
                            break;
                        }
                    }
                    rightF=(condition->bRhsIsAttr==1)?*((float *)(recAddr+condition->RattrOffset)):*((float *)condition->Rvalue);
                    correct=CmpValue(leftF,rightF,condition->compOp);
                    break;
                case chars:
                    if(condition->compOp==NO_OP) {
                        correct=false;
                        break;
                    }
                    leftStr=(condition->bLhsIsAttr==1)?(recAddr+condition->LattrOffset):(char *)condition->Lvalue;
                    rightStr=(condition->bRhsIsAttr==1)?(recAddr+recOffset+condition->RattrOffset):(char *)condition->Rvalue;
                    //�Ƚ������ַ���
                    correct=CmpString(leftStr,rightStr,condition->compOp);
                    break;
                }
                if(!correct) {
                    break;
                }
            }
            //if ����Ƚ�����
            if(conNumber==rmFileScan->conNum)
            {
                rec->bValid=true;
                rec->pData=recAddr;
                rec->rid.bValid=true;
                rec->rid.pageNum=rmFileScan->pn;
                rec->rid.slotNum=rmFileScan->sn;
                rmFileScan->sn++;  //����Ҫ
                return SUCCESS;
            } else {
                rmFileScan->sn++;
            }
        }
        //������һҳ
        UnpinPage(&rmFileScan->PageHandle);
        if(rmFileScan->pRMFileHandle->pageCtlBitmap->firstBit(rmFileScan->pn+1,1)==-1) {
            return RM_NOMORERECINMEM;
        }
        rmFileScan->pn=rmFileScan->pRMFileHandle->pageCtlBitmap->firstBit(rmFileScan->pn+1,1);
        GetThisPage(&rmFileScan->pRMFileHandle->file,rmFileScan->pn,&rmFileScan->PageHandle);
        rmFileScan->sn=0;
    }
}

RC GetRec (RM_FileHandle *fileHandle,RID *rid, RM_Record *rec) 
{
	RC result;
	 rec->bValid=false;
    //char bitmapPage=*(fileHandle->file->pBitmap+rid->pageNum/8);
	if(!fileHandle->pageCtlBitmap->atPos(rid->pageNum) || (rid->pageNum>fileHandle->file.pFileSubHeader->pageCount) || (rid->slotNum>fileHandle->recPerPage-1)||(rid->pageNum<2)) {
		return RM_INVALIDRID; //RID��Ч
	}
    PF_PageHandle *targetPage = new PF_PageHandle;
	result = GetThisPage(&fileHandle->file,rid->pageNum,targetPage);
	if (result) {
		free(targetPage);
		return result;
	}
    char bitmapRec=*(targetPage->pFrame->page.pData+rid->slotNum/8);
    if((bitmapRec&(0x01<<(rid->slotNum%8)))==0)
    {
        UnpinPage(targetPage);
        free(targetPage);
        return RM_INVALIDRID; //��¼��Ч
    }
    rec->rid.bValid=TRUE;
    rec->rid.pageNum=rid->pageNum;
    rec->rid.slotNum=rid->slotNum;
    rec->bValid=TRUE;
    rec->pData=targetPage->pFrame->page.pData+fileHandle->recOffset+rid->slotNum*fileHandle->recSize;
    UnpinPage(targetPage);
    free(targetPage);
    return SUCCESS;
}

RC InsertRec (RM_FileHandle *fileHandle,char *pData, RID *rid)
{
	RC result;
	int unfillPage=fileHandle->recCtlBitmap->firstBit(2,0);
	if(unfillPage == -1) {
		return FAIL; //û�п���λ�ÿɲ���
	}
    //���Ȳ����Ƿ�����ѷ����δ��ҳ
	while((!fileHandle->pageCtlBitmap->atPos(unfillPage)) && unfillPage<=fileHandle->file.pFileSubHeader->pageCount) {
	unfillPage=fileHandle->recCtlBitmap->firstBit(unfillPage+1,0);
	}
//��û���Ѿ������δ��ҳ��������ҳ
    if(unfillPage>fileHandle->file.pFileSubHeader->pageCount) {
        if(!fileHandle->pageCtlBitmap->anyZero())
        {
            rid->bValid=false;
            return FAIL; //û�п���ҳ��
        }
        PF_PageHandle *newPage=new PF_PageHandle;
	result = AllocatePage(&fileHandle->file,newPage);
	if(result) {
	return result;
	}
        bitmanager bmpNewPage(fileHandle->recOffset,newPage->pFrame->page.pData);
		memcpy(newPage->pFrame->page.pData+fileHandle->recOffset, pData, fileHandle->recSize);
        rid->bValid=true;
        rid->pageNum=newPage->pFrame->page.pageNum;
        rid->slotNum=0;
        bmpNewPage.setBitmap(0,1);
        MarkDirty(newPage);
        UnpinPage(newPage);
        free(newPage);
    } else {
		//��ȡ�ѷ����δ��ҳ
        PF_PageHandle *page=new PF_PageHandle;
		result = GetThisPage(&fileHandle->file,unfillPage,page);
		if(result) {
			free(page);
			return result;
		}
        bitmanager bmp(fileHandle->recOffset,page->pFrame->page.pData);
        int emptySlot=bmp.firstBit(0,0);
        memcpy(page->pFrame->page.pData+fileHandle->recOffset+emptySlot*fileHandle->recSize, pData, fileHandle->recSize);
        bmp.setBitmap(emptySlot,1); //�ҵ��ղ۲�����
        //��ҳ����������Ҫ���¼�¼����λͼ
        if(!bmp.anyZero())
        {
            PF_PageHandle *ctrPage=new PF_PageHandle;
            GetThisPage(&fileHandle->file,1,ctrPage);
            fileHandle->recCtlBitmap->setBitmap(unfillPage,1);
            MarkDirty(ctrPage);
            UnpinPage(ctrPage);
            free(ctrPage);
        }
        rid->bValid=true;
        rid->pageNum=unfillPage;
        rid->slotNum=emptySlot;
        MarkDirty(page);
        UnpinPage(page);
        free(page);
    }
    //���¼�¼��
    PF_PageHandle *fileCtlPage=new PF_PageHandle;
    GetThisPage(&fileHandle->file,1,fileCtlPage);
    (*((int *)fileCtlPage->pFrame->page.pData))++;
    MarkDirty(fileCtlPage);
    UnpinPage(fileCtlPage);
    free(fileCtlPage);
    return SUCCESS;
}

RC DeleteRec (RM_FileHandle *fileHandle,const RID *rid)
{
	RC result;
	
    if(rid->pageNum>fileHandle->file.pFileSubHeader->pageCount || (!fileHandle->pageCtlBitmap->atPos(rid->pageNum)) || (rid->pageNum<2)) {
		return RM_INVALIDRID; //RID��Ч
	}
	//ȡĿ��ҳ����
	PF_PageHandle *target=new PF_PageHandle;
	result = GetThisPage(&fileHandle->file,rid->pageNum,target);
	if(result) {
		free(target);
		return result;
	}
    bitmanager bitmap(fileHandle->recOffset,target->pFrame->page.pData);
    if(!bitmap.atPos(rid->slotNum))
    {
        UnpinPage(target);
        free(target);
        return RM_INVALIDRID; //��¼��Ч
    }
    bitmap.setBitmap(rid->slotNum,0); //����λͼ
    //����ҳ���Ѿ�û���κ���Ч��¼
    if(bitmap.firstBit(0,1)==-1)
    {
        PF_PageHandle *ctrPage=new PF_PageHandle;
        GetThisPage(&fileHandle->file,1,ctrPage);
        fileHandle->recCtlBitmap->setBitmap(rid->pageNum,0);
        MarkDirty(ctrPage);
        UnpinPage(ctrPage);
        free(ctrPage);
        DisposePage(&fileHandle->file,target->pFrame->page.pageNum);
    }
    //�޸�ʣ���¼��
    PF_PageHandle *fileCtlPage=new PF_PageHandle;
    GetThisPage(&fileHandle->file,1,fileCtlPage);
    (*((int *)fileCtlPage->pFrame->page.pData))--;
    MarkDirty(fileCtlPage);
    UnpinPage(fileCtlPage);
    free(fileCtlPage);
    MarkDirty(target);
    UnpinPage(target);
    free(target);
    return SUCCESS;
}

RC UpdateRec (RM_FileHandle *fileHandle,const RM_Record *rec)
{
	 RC result;
		if(rec->rid.pageNum>fileHandle->file.pFileSubHeader->pageCount || (!fileHandle->pageCtlBitmap->atPos(rec->rid.pageNum)) || (rec->rid.pageNum<2)) {
			return RM_INVALIDRID; //RID��Ч
		}
    //ȡĿ��ҳ����
    PF_PageHandle *target=new PF_PageHandle;
    result = GetThisPage(&fileHandle->file,rec->rid.pageNum,target);
    if(result) {
        return result;
    }
    bitmanager bitmap(fileHandle->recOffset,target->pFrame->page.pData);
    if(!bitmap.atPos(rec->rid.slotNum))
    {
        UnpinPage(target);
        free(target);
        return RM_INVALIDRID; //��¼��Ч
    }
    memcpy(target->pFrame->page.pData+fileHandle->recOffset+rec->rid.slotNum * fileHandle->recSize,rec->pData,fileHandle->recSize); //���¼�¼
    MarkDirty(target); //�����ҳ��
    UnpinPage(target); //�������������
    free(target);
    return SUCCESS;

}

RC RM_CreateFile (char *fileName, int recordSize){

	RC result;
    result = CreateFile(fileName);
    if(result) {
		return result;
	}
    //����ɹ������ɼ�¼��Ϣ����ҳ�����Ȼ�ô����ļ��ľ��
    PF_FileHandle *file=new PF_FileHandle;
	result = openFile(fileName,file);
	if(result) {
		return result;
	}
    //������ҳ��
    PF_PageHandle *ctrPage=new PF_PageHandle;
	result = AllocatePage(file,ctrPage);
	if(result) {
		return result;
	}
    //�ڼ�¼����ҳ��ż�¼��Ϣ���ݽṹ
    RM_recControl *recCtl;
    recCtl=(RM_recControl *) ctrPage->pFrame->page.pData;
    recCtl->recNum=0;
    recCtl->recSize=recordSize;
    recCtl->recPerPage=PF_PAGE_SIZE/(recordSize+0.125);
    recCtl->recPerPage=(int(recCtl->recPerPage/8))*8;
    recCtl->recordOffset=recCtl->recPerPage/8;
    //recCtl->fileNum=1;
    //�ر��ļ�
    MarkDirty(ctrPage); //�����ҳ��
    UnpinPage(ctrPage); //�������������
    CloseFile(file);
	return SUCCESS;
}
RC RM_OpenFile(char *fileName, RM_FileHandle *fileHandle)
{
	RC result;
	if(fileHandle->bOpen) {
		return RM_FHOPENNED; // �ļ��Ѿ���
	}
	result = openFile(fileName,&fileHandle->file);
	if(result) {
		return result;
	}
    fileHandle->bOpen=TRUE;
    //fileHandle->fileName=fileName;
    //fileHandle->file[0]=filePF;
    //��ȡҳ�������Ϣ
    PF_PageHandle *ctrPage=new PF_PageHandle;
    result = GetThisPage(&fileHandle->file, 1 , ctrPage);
    if(result) {
        CloseFile(&fileHandle->file);
		return result;
    }
    //��ȡ��¼������Ϣ
    RM_recControl *recCtl=NULL;
    recCtl=(RM_recControl *)ctrPage->pFrame->page.pData;
    fileHandle->recOffset=recCtl->recordOffset;
    fileHandle->recPerPage=recCtl->recPerPage;
    fileHandle->recSize=recCtl->recSize;
    fileHandle->bitmapLength=PF_PAGE_SIZE-sizeof(RM_recControl);
    //��ȡ����λͼ
    fileHandle->recCtlBitmap=new bitmanager(fileHandle->bitmapLength,ctrPage->pFrame->page.pData + sizeof(RM_recControl));
    fileHandle->pageCtlBitmap=new bitmanager(fileHandle->bitmapLength,fileHandle->file.pBitmap);
    UnpinPage(ctrPage);
    return SUCCESS;
}

RC RM_CloseFile(RM_FileHandle *fileHandle)
{
	RC result;
	 if(!fileHandle->bOpen) {
		return RM_FHCLOSED; //�ļ��Ѿ��ر�
	}
	result = CloseFile(&fileHandle->file);
	if(result) {
		return result;
}
    fileHandle->bOpen=FALSE;
    return SUCCESS;
}

// �Ƚ��ַ�������
bool CmpString(char *left, char *right, CompOp oper)
{
    int cmpResult=strcmp(left,right);
    switch(oper)
    {
        case EQual:
            return (cmpResult==0)?true:false;
        case LessT:
            return (cmpResult<0)?true:false;
        case GreatT:
            return (cmpResult>0)?true:false;
        case NEqual:
            return (cmpResult==0)?false:true;
        case LEqual:
            return (cmpResult==0||cmpResult<0)?true:false;
        case GEqual:
            return (cmpResult==0||cmpResult>0)?true:false;
        default:
            return false;
    }
}

// �Ƚ���ֵ����
bool CmpValue(float left, float right, CompOp oper)
{
    switch(oper)
    {
        case EQual:
            return (left==right);
        case LessT:
            return (left<right);
        case GreatT:
            return (left>right);
        case NEqual:
            return (left!=right);
        case LEqual:
            return (left<=right);
        case GEqual:
            return (left>=right);
        default:
            return false;
    }
}