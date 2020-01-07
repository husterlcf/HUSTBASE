#include "stdafx.h"
#include "RM_Manager.h"
#include "str.h"


RC OpenScan(RM_FileScan *rmFileScan,RM_FileHandle *fileHandle,int conNum,Con *conditions)//初始化扫描
{
	RC result;
	if(rmFileScan->bOpen) {
		return RM_FSOPEN; //扫描文件已经打开
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
    //首先检查文件是否有内容
    if(rmFileScan->pn==0) {
        return RM_NOMORERECINMEM;
    }
    bitmanager recBitmap(1,NULL);
    while(1)
    {
        GetThisPage(&rmFileScan->pRMFileHandle->file,rmFileScan->pn,&rmFileScan->PageHandle);
        //recPerPage必为8的倍数
        recBitmap.redirectBitmap(rmFileScan->pRMFileHandle->recPerPage/8,rmFileScan->PageHandle.pFrame->page.pData);
        rmFileScan->sn=recBitmap.firstBit(rmFileScan->sn,1);
        while(((rmFileScan->sn=recBitmap.firstBit(rmFileScan->sn,1))!=-1))
        {
            Con *condition;
            bool correct=true;
            //与条件进行比较
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
                    //比较两个字符串
                    correct=CmpString(leftStr,rightStr,condition->compOp);
                    break;
                }
                if(!correct) {
                    break;
                }
            }
            //if 满足比较条件
            if(conNumber==rmFileScan->conNum)
            {
                rec->bValid=true;
                rec->pData=recAddr;
                rec->rid.bValid=true;
                rec->rid.pageNum=rmFileScan->pn;
                rec->rid.slotNum=rmFileScan->sn;
                rmFileScan->sn++;  //很重要
                return SUCCESS;
            } else {
                rmFileScan->sn++;
            }
        }
        //跳到下一页
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
		return RM_INVALIDRID; //RID无效
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
        return RM_INVALIDRID; //记录无效
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
		return FAIL; //没有空余位置可插入
	}
    //首先查找是否存在已分配的未满页
	while((!fileHandle->pageCtlBitmap->atPos(unfillPage)) && unfillPage<=fileHandle->file.pFileSubHeader->pageCount) {
	unfillPage=fileHandle->recCtlBitmap->firstBit(unfillPage+1,0);
	}
//若没有已经分配的未满页，分配新页
    if(unfillPage>fileHandle->file.pFileSubHeader->pageCount) {
        if(!fileHandle->pageCtlBitmap->anyZero())
        {
            rid->bValid=false;
            return FAIL; //没有空闲页面
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
		//获取已分配的未满页
        PF_PageHandle *page=new PF_PageHandle;
		result = GetThisPage(&fileHandle->file,unfillPage,page);
		if(result) {
			free(page);
			return result;
		}
        bitmanager bmp(fileHandle->recOffset,page->pFrame->page.pData);
        int emptySlot=bmp.firstBit(0,0);
        memcpy(page->pFrame->page.pData+fileHandle->recOffset+emptySlot*fileHandle->recSize, pData, fileHandle->recSize);
        bmp.setBitmap(emptySlot,1); //找到空槽并更新
        //若页面已满，需要更新记录控制位图
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
    //更新记录数
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
		return RM_INVALIDRID; //RID无效
	}
	//取目标页面句柄
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
        return RM_INVALIDRID; //记录无效
    }
    bitmap.setBitmap(rid->slotNum,0); //更新位图
    //若该页面已经没有任何有效记录
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
    //修改剩余记录数
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
			return RM_INVALIDRID; //RID无效
		}
    //取目标页面句柄
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
        return RM_INVALIDRID; //记录无效
    }
    memcpy(target->pFrame->page.pData+fileHandle->recOffset+rec->rid.slotNum * fileHandle->recSize,rec->pData,fileHandle->recSize); //更新记录
    MarkDirty(target); //标记脏页面
    UnpinPage(target); //解除缓冲区限制
    free(target);
    return SUCCESS;

}

RC RM_CreateFile (char *fileName, int recordSize){

	RC result;
    result = CreateFile(fileName);
    if(result) {
		return result;
	}
    //如果成功，生成记录信息控制页，首先获得创建文件的句柄
    PF_FileHandle *file=new PF_FileHandle;
	result = openFile(fileName,file);
	if(result) {
		return result;
	}
    //申请新页面
    PF_PageHandle *ctrPage=new PF_PageHandle;
	result = AllocatePage(file,ctrPage);
	if(result) {
		return result;
	}
    //在记录控制页存放记录信息数据结构
    RM_recControl *recCtl;
    recCtl=(RM_recControl *) ctrPage->pFrame->page.pData;
    recCtl->recNum=0;
    recCtl->recSize=recordSize;
    recCtl->recPerPage=PF_PAGE_SIZE/(recordSize+0.125);
    recCtl->recPerPage=(int(recCtl->recPerPage/8))*8;
    recCtl->recordOffset=recCtl->recPerPage/8;
    //recCtl->fileNum=1;
    //关闭文件
    MarkDirty(ctrPage); //标记脏页面
    UnpinPage(ctrPage); //解除缓冲区限制
    CloseFile(file);
	return SUCCESS;
}
RC RM_OpenFile(char *fileName, RM_FileHandle *fileHandle)
{
	RC result;
	if(fileHandle->bOpen) {
		return RM_FHOPENNED; // 文件已经打开
	}
	result = openFile(fileName,&fileHandle->file);
	if(result) {
		return result;
	}
    fileHandle->bOpen=TRUE;
    //fileHandle->fileName=fileName;
    //fileHandle->file[0]=filePF;
    //获取页面管理信息
    PF_PageHandle *ctrPage=new PF_PageHandle;
    result = GetThisPage(&fileHandle->file, 1 , ctrPage);
    if(result) {
        CloseFile(&fileHandle->file);
		return result;
    }
    //获取记录管理信息
    RM_recControl *recCtl=NULL;
    recCtl=(RM_recControl *)ctrPage->pFrame->page.pData;
    fileHandle->recOffset=recCtl->recordOffset;
    fileHandle->recPerPage=recCtl->recPerPage;
    fileHandle->recSize=recCtl->recSize;
    fileHandle->bitmapLength=PF_PAGE_SIZE-sizeof(RM_recControl);
    //获取管理位图
    fileHandle->recCtlBitmap=new bitmanager(fileHandle->bitmapLength,ctrPage->pFrame->page.pData + sizeof(RM_recControl));
    fileHandle->pageCtlBitmap=new bitmanager(fileHandle->bitmapLength,fileHandle->file.pBitmap);
    UnpinPage(ctrPage);
    return SUCCESS;
}

RC RM_CloseFile(RM_FileHandle *fileHandle)
{
	RC result;
	 if(!fileHandle->bOpen) {
		return RM_FHCLOSED; //文件已经关闭
	}
	result = CloseFile(&fileHandle->file);
	if(result) {
		return result;
}
    fileHandle->bOpen=FALSE;
    return SUCCESS;
}

// 比较字符串函数
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

// 比较数值函数
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