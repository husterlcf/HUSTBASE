#include "stdafx.h"
#include "IX_Manager.h"
#include <queue>
using namespace std;

int leastnum;

RC IXCreateIndex(char * fileName,AttrType attrType,int attrLength){
	//����ҳ���ļ�����
	RC crc;
	crc = CreateFile(fileName);
	if(crc){
		return crc;
	}

	PF_FileHandle * filehandle = (PF_FileHandle *)malloc(sizeof(PF_FileHandle));
	openFile(fileName,filehandle);
	
	//���������ļ��ĵ�һ��ҳ��
	PF_PageHandle * headPH = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	AllocatePage(filehandle, headPH);
	PageNum PNum;
	GetPageNum(headPH,&PNum);
	//printf("num:%d",PNum);

	//����������Ϣ
	IX_FileHeader IXhead;
	IXhead.attrLength = attrLength;
	IXhead.attrType = attrType;
	IXhead.keyLength = attrLength + sizeof(RID);//������ֵ�ĳ���+��¼ID�ĳ���
	IXhead.first_leaf = PNum;
	IXhead.rootPage = PNum;
	//printf("%d, %d, %d, %d, %d", PF_PAGE_SIZE,sizeof(IX_FileHeader),sizeof(IX_Node),sizeof(RID),attrLength);
	IXhead.order = (PF_PAGE_SIZE-sizeof(IX_FileHeader)-sizeof(IX_Node))/(2*sizeof(RID)+attrLength);
	leastnum = IXhead.order>>1;

	//�ڵ������Ϣ
	IX_Node index_node;
	index_node.brother = 0;
	index_node.parent = 0;
	index_node.is_leaf = 1;
	index_node.keynum = 0;

	char *pdata = NULL;
	GetData(headPH, &pdata);
	index_node.keys = pdata + sizeof(IX_FileHeader) + sizeof(IX_Node);
	index_node.rids = (RID *)(index_node.keys + IXhead.order*IXhead.attrLength);
	memcpy(pdata,&IXhead,sizeof(IX_FileHeader));
	memcpy(pdata+sizeof(IX_FileHeader),&index_node,sizeof(IX_Node));

	MarkDirty(headPH);
	UnpinPage(headPH);
	free(headPH);
	CloseFile(filehandle);
	
	//free(filehandle);
	
	return SUCCESS;
}
//��������ļ�����������ļ�������Ϣ����һҳ��
RC OpenIndex(char *fileName,IX_IndexHandle *indexHandle){
	RC rc;
	rc = openFile(fileName, &indexHandle->fileHandle);
	if(rc != SUCCESS){
		return rc;
	}

	PF_PageHandle * headPH = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	if(GetThisPage(&indexHandle->fileHandle,1,headPH)){
		CloseFile(&indexHandle->fileHandle);
		//printf("fail\n");
		return FAIL;
	}

	char *pData = NULL;
	GetData(headPH, &pData);

	IX_FileHeader fileheader;
	memcpy(&fileheader, pData, sizeof(IX_FileHeader));

	IX_Node IXnode;
	memcpy(&IXnode, pData+sizeof(IX_FileHeader), sizeof(IX_Node));

	indexHandle->fileHeader.attrLength = fileheader.attrLength;
	indexHandle->fileHeader.attrType = fileheader.attrType;
	indexHandle->fileHeader.first_leaf = fileheader.first_leaf;
	indexHandle->fileHeader.keyLength = fileheader.keyLength;
	indexHandle->fileHeader.order = fileheader.order;
	indexHandle->fileHeader.rootPage = fileheader.rootPage;
	indexHandle->bOpen = true;
	UnpinPage(headPH);
	free(headPH);
	return SUCCESS;
}

RC CloseIndex(IX_IndexHandle *indexHandle){
	CloseFile(&(indexHandle->fileHandle));
	return SUCCESS;
}

RC InsertEntry(IX_IndexHandle *indexHandle,void *pData, RID * rid){
	PF_FileHandle filehandle = indexHandle->fileHandle;
	IX_FileHeader fileheader = indexHandle->fileHeader;
	int order = fileheader.order;
	int attrlength = fileheader.attrLength;
	PF_PageHandle * pagehandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	GetThisPage(&filehandle, fileheader.rootPage, pagehandle);
	char * nodedata;
	GetData(pagehandle, &nodedata);
	IX_Node * IXnode = (IX_Node *)(nodedata + sizeof(IX_FileHeader));
	while(IXnode->is_leaf != 1){
		RID temprid;
		searchpath(pagehandle, order, fileheader.attrType, attrlength, pData, &temprid);
		GetThisPage(&filehandle, temprid.pageNum, pagehandle);
		GetData(pagehandle, &nodedata);
		IXnode = (IX_Node *)(nodedata + sizeof(IX_FileHeader));
	}
	insertkeyandrid(pagehandle, order, fileheader.attrType, attrlength, pData, rid);
	//��keynum����orderʱ�Խڵ���з��Ѵ���
	while(IXnode->keynum == order){
		int keynum = IXnode->keynum;
		char *nodekeys = nodedata+sizeof(IX_FileHeader)+sizeof(IX_Node);
		char *noderids = nodekeys+attrlength*order;
		PageNum pagenum;
		GetPageNum(pagehandle, &pagenum);
		PF_PageHandle * newleafPH = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
		newleafPH->bOpen = true;
		AllocatePage(&filehandle, newleafPH);
		MarkDirty(newleafPH);
		UnpinPage(newleafPH);
		char * newleafdata=NULL;
		GetData(newleafPH,&newleafdata);
		PageNum newleafpagenum;
		GetPageNum(newleafPH, &newleafpagenum);
		int left = keynum>>1;
		int right = keynum - left;//printf("l:%d r:%d\n",left,right);
		if(IXnode->parent!=0){//��ǰ�ڵ㲻�Ǹ��ڵ�
			IX_Node tempixnode;
			tempixnode.brother = 0;
			tempixnode.parent = IXnode->parent;
			tempixnode.is_leaf = IXnode->is_leaf;
			tempixnode.keynum = right;
			tempixnode.keys = newleafdata+sizeof(IX_FileHeader)+sizeof(IX_Node);
			tempixnode.rids = (RID *)(tempixnode.keys+order*attrlength);
			memcpy(newleafdata+sizeof(IX_FileHeader), &tempixnode, sizeof(IX_Node));
			tempixnode.brother = newleafpagenum;
			tempixnode.parent = IXnode->parent;
			tempixnode.is_leaf = IXnode->is_leaf;
			tempixnode.keynum = left;
			tempixnode.keys = nodedata+sizeof(IX_FileHeader)+sizeof(IX_Node);
			tempixnode.rids = (RID *)(tempixnode.keys+order*attrlength);
			memcpy(nodedata+sizeof(IX_FileHeader), &tempixnode, sizeof(IX_Node));
			copykeyandrid(newleafPH, nodekeys+left*attrlength, attrlength, right, order, noderids+left*sizeof(RID));
			MarkDirty(pagehandle);
			UnpinPage(pagehandle);
			MarkDirty(newleafPH);
			UnpinPage(newleafPH);
			RID temprid;
			temprid.bValid = false;
			temprid.pageNum = newleafpagenum;
			//�򸸽ڵ��в���key��rid
			GetThisPage(&filehandle, IXnode->parent, pagehandle);
			insertkeyandrid(pagehandle, order, fileheader.attrType, fileheader.attrLength, newleafdata+sizeof(IX_FileHeader)+sizeof(IX_Node), &temprid);
			GetData(pagehandle, &nodedata);
			IXnode = (IX_Node *)(nodedata+sizeof(IX_FileHeader));
			MarkDirty(pagehandle);
			UnpinPage(pagehandle);
		}
		else{//��ǰ�ڵ��Ǹ��ڵ�����
			PF_PageHandle *newrootPH = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
			newrootPH->bOpen = true;
			AllocatePage(&filehandle, newrootPH);
			PageNum newrootpagenum;
			GetPageNum(newrootPH, &newrootpagenum);
			//printf("newroot:%d", newrootpagenum);
			char *newrootdata;
			GetData(newrootPH, &newrootdata);
			IX_Node tempixnode;
			//�����µĸ��ڵ��IX_Node��Ϣ
			tempixnode.brother = 0;
			tempixnode.parent = 0;
			tempixnode.is_leaf = 0;
			tempixnode.keynum = 0;
			tempixnode.keys = newrootdata+sizeof(IX_FileHeader)+sizeof(IX_Node);
			tempixnode.rids = (RID *)(tempixnode.keys+order*attrlength);
			memcpy(newrootdata+sizeof(IX_FileHeader), &tempixnode, sizeof(IX_Node));
			//�����µķ��ѳ��Ľڵ��IX_Node��Ϣ
			tempixnode.brother = 0;
			tempixnode.parent = newrootpagenum;
			tempixnode.is_leaf = IXnode->is_leaf;
			tempixnode.keynum = right;
			tempixnode.keys = newleafdata+sizeof(IX_FileHeader)+sizeof(IX_Node);
			tempixnode.rids = (RID *)(tempixnode.keys+order*attrlength);
			memcpy(newleafdata+sizeof(IX_FileHeader), &tempixnode, sizeof(IX_Node));
			//����ԭ�ڵ��IX_Node��Ϣ
			tempixnode.brother = newleafpagenum;
			tempixnode.parent = newrootpagenum;
			tempixnode.is_leaf = IXnode->is_leaf;
			tempixnode.keynum = left;
			tempixnode.keys = nodedata+sizeof(IX_FileHeader)+sizeof(IX_Node);
			tempixnode.rids = (RID *)(tempixnode.keys+order*attrlength);
			memcpy(nodedata+sizeof(IX_FileHeader), &tempixnode, sizeof(IX_Node));
			copykeyandrid(newleafPH, nodekeys+left*attrlength, attrlength, right, order, noderids+left*sizeof(RID));
			char *tempdata;
			GetData(pagehandle,&tempdata);
			RID temprid;
			temprid.bValid = false;
			temprid.pageNum = pagenum;
			insertkeyandrid(newrootPH, order, fileheader.attrType, attrlength, tempdata+sizeof(IX_FileHeader)+sizeof(IX_Node), &temprid);
			GetData(newleafPH, &tempdata);
			temprid.pageNum = newleafpagenum;
			insertkeyandrid(newrootPH,order, fileheader.attrType, attrlength, tempdata+sizeof(IX_FileHeader)+sizeof(IX_Node), &temprid);
			indexHandle->fileHeader.rootPage = newrootpagenum;
			PF_PageHandle * tempheadPH = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
			GetThisPage(&indexHandle->fileHandle,1,tempheadPH);
			char *temprootdata;
			GetData(tempheadPH, &temprootdata);
			IX_FileHeader * tempfileheader = (IX_FileHeader *)temprootdata;
			tempfileheader->rootPage = newrootpagenum;
			MarkDirty(tempheadPH);
			UnpinPage(tempheadPH);
			MarkDirty(pagehandle);
			UnpinPage(pagehandle);
			MarkDirty(newleafPH);
			UnpinPage(newleafPH);
			MarkDirty(newrootPH);
			UnpinPage(newrootPH);
			free(newrootPH);
			free(tempheadPH);
		}
		free(newleafPH);
	}
	update_parent(&(indexHandle->fileHandle),indexHandle->fileHeader);
	free(pagehandle);
	return SUCCESS;
}

RC DeleteEntry(IX_IndexHandle *indexHandle,void *pData,RID * rid){
	PF_FileHandle filehandle = indexHandle->fileHandle;
	IX_FileHeader fileheader = indexHandle->fileHeader;
	int order = fileheader.order;
	int attrlength = fileheader.attrLength;
	PF_PageHandle *pagehandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	GetThisPage(&filehandle, fileheader.rootPage, pagehandle);
	char *pagedata;
	GetData(pagehandle, &pagedata);
	IX_Node * IXnode = (IX_Node *)(pagedata+sizeof(IX_FileHeader));
	while(IXnode->is_leaf != 1){
		RID temprid;
		bool existence = false;
		if_existence(pagehandle, order, fileheader.attrType, attrlength, pData, &temprid, &existence);
		if(existence){
			GetThisPage(&filehandle, temprid.pageNum, pagehandle);
			GetData(pagehandle, &pagedata);
			IXnode = (IX_Node *)(pagedata+sizeof(IX_FileHeader));
		}
		else{
			return IX_EOF;
		}
	}
	int keynum = IXnode->keynum;
	char *pagekeys = pagedata + sizeof(IX_FileHeader) + sizeof(IX_Node);
	char *pagerids = pagekeys + order*attrlength;
	int flag = 0;
	int position = 0;
	float tempf;int tempdata;
	switch(fileheader.attrType){
	case chars:
		for(position = 0;position<keynum;position++){
			if(strcmp(pagekeys + position*attrlength, (char *)pData)>=0){
				if(strcmp(pagekeys + position*attrlength, (char *)pData)==0){
					RID *comrid = (RID *)(pagerids + position*sizeof(RID));
					if(rid->bValid==comrid->bValid && rid->pageNum==comrid->pageNum && rid->slotNum==comrid->slotNum){
						flag = 1;
						break;
					}
				}
				break;
			}
		}
		break;
	case ints:
		tempdata = *((int *)pData);
		for(position = 0; position<keynum;position++){
			int data = *((int *)pagekeys + position*attrlength);
			if(data >= tempdata){
				if(data==tempdata){
					RID *comrid = (RID *)(pagerids + position*sizeof(RID));
					if(rid->bValid==comrid->bValid && rid->pageNum==comrid->pageNum && rid->slotNum==comrid->slotNum){
						flag = 1;
						break;
					}
				}
				break;
			}
		}
		break;
	case floats:
		tempf = *((float *)pData);
		for(position = 0;position<keynum;position++){
			float fdata = *((float *)(pagekeys + position*attrlength));
			if(fdata >= tempf){
				if(fdata == tempf){
					RID *comrid = (RID *)(pagerids + position*sizeof(RID));
					if(rid->bValid==comrid->bValid && rid->pageNum==comrid->pageNum && rid->slotNum==comrid->slotNum){
						flag = 1;
						break;
					}
				}
				break;
			}
		}
		break;
	}
	if(flag==1){//�ҵ���Ӧ�Ĺؼ��ּ�¼������ɾ������
		memcpy(pagekeys + position*attrlength, pagekeys+(position+1)*attrlength,(keynum-position-1)*attrlength);//ɾ��key
		memcpy(pagerids + position*sizeof(RID), pagerids+(position+1)*sizeof(RID), (keynum-position-1)*sizeof(RID));//ɾ��rid
		keynum--;
		IXnode->keynum = keynum;
		MarkDirty(pagehandle);
		UnpinPage(pagehandle);
		PF_PageHandle *parentpagehandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
		while(IXnode->parent!=0){//��ǰ�ڵ㲻�Ǹ��ڵ�
			int status = 0;
			if(keynum < leastnum){//���ڷǸ��ڵ㣬keynum��������order/2ʱҪ���кϲ����㴦��
				lessthanleastnum(pagehandle, &filehandle, order, fileheader.attrType, attrlength, &status);
			}
			PF_PageHandle *parentpagehandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
			PageNum nodepagenum;
			GetThisPage(&filehandle, IXnode->parent, parentpagehandle);
			GetPageNum(pagehandle, &nodepagenum);

			if(status == 2){//����ڵ�����˺ϲ�
				//��parentpagehandle��Ӧ�Ľڵ㣨��ǰ�ڵ�ĸ��ڵ���ɾ����ǰ�ڵ��Ӧ������ֵ��
				deleteNode(parentpagehandle, &filehandle, order, fileheader.attrType, attrlength, nodepagenum, true, NULL);
				pagehandle = parentpagehandle;
				GetData(pagehandle, &pagedata);
				IXnode = (IX_Node *)(pagedata+sizeof(IX_FileHeader));
				pagekeys = pagedata+sizeof(IX_FileHeader)+sizeof(IX_Node);
				keynum = IXnode->keynum;
			}
			else if(status == 4){//���ҽڵ�����˺ϲ�
				if(position == 0){//����ɾ�����Ǹýڵ�ĵ�һ���ڵ�
					deleteNode(parentpagehandle, &filehandle, order, fileheader.attrType, attrlength, nodepagenum, false, pagekeys);
					position = 1;
				}
				GetThisPage(&filehandle, IXnode->brother, pagehandle);
				GetData(pagehandle, &pagedata);
				GetPageNum(pagehandle, &nodepagenum);
				IX_Node *temp =	IXnode;
				IXnode = (IX_Node *)(pagedata+sizeof(IX_FileHeader));
				GetThisPage(&filehandle, IXnode->parent, parentpagehandle);
				deleteNode(parentpagehandle,&filehandle, order, fileheader.attrType, attrlength, nodepagenum, true, NULL);
				temp->brother = IXnode->brother;
				pagehandle = parentpagehandle;
				GetData(pagehandle, &pagedata);
				IXnode = (IX_Node *)(pagedata + sizeof(IX_FileHeader));
				pagekeys = pagedata + sizeof(IX_FileHeader) + sizeof(IX_Node);
				keynum = IXnode->keynum;
			}
			else if(status == 1 || position == 0){
				deleteNode(parentpagehandle, &filehandle, order, fileheader.attrType, attrlength, nodepagenum, false, pagekeys);
				break;
			}
			//free(parentpagehandle);
		}
		if(IXnode->parent==0 && IXnode->keynum==1){
			RID * rootrid = (RID *)(IXnode->rids);
			PF_PageHandle * tempheadPH = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
			GetThisPage(&indexHandle->fileHandle,1,tempheadPH);
			char *temprootdata;
			GetData(tempheadPH, &temprootdata);
			IX_FileHeader * tempfileheader = (IX_FileHeader *)temprootdata;
			tempfileheader->rootPage = rootrid->pageNum;
			indexHandle->fileHeader.rootPage = rootrid->pageNum;
			MarkDirty(tempheadPH);
			UnpinPage(tempheadPH);
			free(tempheadPH);
		}
		update_parent(&(indexHandle->fileHandle), indexHandle->fileHeader);
		free(parentpagehandle);
	}
	else{
		return FAIL;
	}
	//free(pagehandle);
	return SUCCESS;
}

RC OpenIndexScan(IX_IndexScan *indexScan,IX_IndexHandle *indexHandle,CompOp compOp,char *value){
	PF_FileHandle filehandle = indexHandle->fileHandle;
	IX_FileHeader fileheader = indexHandle->fileHeader;

	indexScan->compOp = compOp;
	indexScan->value = value;
	indexScan->bOpen = true;
	indexScan->pIXIndexHandle = indexHandle;

	int ridIx = 0;
	PageNum startpage = 0;
	bool existence;
	switch(compOp){
	case EQual://����
		scan_thefirstequal(indexHandle, value, &startpage, &ridIx, &existence);
		if(existence == true){//�ҵ�����ȵ�ֵ����pnNext��Ϊ�ýڵ������ҳ
			indexScan->pnNext = startpage;
			indexScan->ridIx=ridIx;
		}
		else{//û�ҵ�
			indexScan->pnNext = 0;
		}
		break;
	case LEqual:
	case NEqual:
	case LessT:
	case NO_OP://<��<=��!=���ޱȽ�
		indexScan->pnNext = fileheader.first_leaf;
		indexScan->ridIx = 0;
		break;
	case GEqual:
		scan_thefirstequal(indexHandle, value, &startpage, &ridIx, &existence);
		indexScan->pnNext = startpage;
		indexScan->ridIx = ridIx;
		break;
	case GreatT:
		scan_lequal(indexHandle, value, &startpage, &ridIx);
		indexScan->pnNext = startpage;
		indexScan->ridIx = ridIx;
		break;
	default:
		break;
	}
	return SUCCESS;
}

RC IX_GetNextEntry(IX_IndexScan *indexScan,RID * rid){
	if(indexScan->bOpen){//�����Ѿ���
		PageNum pagenum = indexScan->pnNext;//���������ҳ���
		CompOp compop = indexScan->compOp;
		IX_IndexHandle *indexhandle = indexScan->pIXIndexHandle;
		char *value = indexScan->value;
		int ridIx = indexScan->ridIx;//�����������������

		if(pagenum == 0){
			rid->pageNum=0;
		}
		else{
			PF_FileHandle filehandle = indexhandle->fileHandle;
			IX_FileHeader fileheader = indexhandle->fileHeader;
			int order = fileheader.order;
			int attrlength = fileheader.attrLength;
			PF_PageHandle *pagehandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
			GetThisPage(&filehandle, pagenum, pagehandle);
			char *pagedata;
			GetData(pagehandle, &pagedata);
			IX_Node * IXnode = (IX_Node *)(pagedata + sizeof(IX_FileHeader));
			int keynum = IXnode->keynum;
			char *pagekeys = pagedata + sizeof(IX_FileHeader) + sizeof(IX_Node);
			char *pagerids = pagekeys + order*attrlength;
			if(ridIx == keynum){//ɨ���굱ǰҳ��
				if(IXnode->brother == 0){//�Ѿ����ļ���ĩβ
					rid->pageNum=0;
					return IX_EOF;
				}
				else{
					pagenum = IXnode->brother;
					indexScan->pnNext = pagenum;

					GetThisPage(&filehandle, pagenum, pagehandle);
					GetData(pagehandle, &pagedata);
					IXnode = (IX_Node *)(pagedata + sizeof(IX_FileHeader));
					pagekeys = pagedata + sizeof(IX_FileHeader) + sizeof(IX_Node);
					pagerids = pagekeys + order*attrlength;
					ridIx = 0;
				}
			}
			int comkey = comparekeys(pagekeys + ridIx*attrlength, value, fileheader.attrType);
			switch(compop){
			case EQual:
				if(comkey == 0){//��Ȼ��������
					memcpy(rid, pagerids + ridIx*sizeof(RID), sizeof(RID));
					ridIx++;
					indexScan->ridIx = ridIx;
					return SUCCESS;
				}
				else{
					rid->pageNum=0;
					return SUCCESS;
				}
				break;
			case NEqual:
				if(comkey != 0){
					memcpy(rid, pagerids + ridIx*sizeof(RID), sizeof(RID));
					ridIx++;
					indexScan->ridIx = ridIx;
					return SUCCESS;
				}
				else{
					rid->pageNum=0;
					return SUCCESS;
				}
				break;
			case LEqual:
				if(comkey <= 0){
					memcpy(rid, pagerids + ridIx*sizeof(RID), sizeof(RID));
					ridIx++;
					indexScan->ridIx = ridIx;
					return SUCCESS;
				}
				else{
					rid->pageNum=0;
					return SUCCESS;
				}
				break;
			case LessT:
				if(comkey < 0){
					memcpy(rid, pagerids + ridIx*sizeof(RID), sizeof(RID));
					ridIx++;
					indexScan->ridIx = ridIx;
					return SUCCESS;
				}
				else{
					rid->pageNum=0;
					return SUCCESS;
				}
				break;
			case GEqual://����ȫ������
				memcpy(rid, pagerids + ridIx*sizeof(RID), sizeof(RID));
				ridIx++;
				indexScan->ridIx = ridIx;
				break;
			case GreatT:
				memcpy(rid, pagerids + ridIx*sizeof(RID), sizeof(RID));
				ridIx++;
				indexScan->ridIx = ridIx;
				break;
			case NO_OP:
				memcpy(rid, pagerids + ridIx*sizeof(RID), sizeof(RID));
				ridIx++;
				indexScan->ridIx = ridIx;
				break;
			}
			free(pagehandle);
		}
		return SUCCESS;
	}
	else{
		rid->pageNum=0;
		return  SUCCESS;
	}
}

RC CloseIndexScan(IX_IndexScan *indexScan){
	free(indexScan);
	indexScan = NULL;
	return SUCCESS;
}

RC GetIndexTree(char *fileName, Tree *index){
	IX_IndexHandle *indexhandle = new IX_IndexHandle;
	if(openFile(fileName, &indexhandle->fileHandle))
		return FAIL;
	char *pagedata = NULL;;

	index->attrLength = indexhandle->fileHeader.attrLength;
	index->attrType = indexhandle->fileHeader.attrType;
	index->order = indexhandle->fileHeader.order;

	PF_PageHandle *pagehandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	GetThisPage(&indexhandle->fileHandle, indexhandle->fileHeader.rootPage, pagehandle);
	GetData(pagehandle, &pagedata);
	IX_Node * IXnode = (IX_Node *)(pagedata + sizeof(IX_FileHeader));
	char *pagekeys = pagedata + sizeof(IX_FileHeader) + sizeof(IX_Node);
	char *pagerids = pagekeys + indexhandle->fileHeader.order*indexhandle->fileHeader.attrLength;
	
	index->root->keyNum = IXnode->keynum;
	memcpy(index->root->keys, pagekeys, IXnode->keynum*indexhandle->fileHeader.attrLength);
	index->root->sibling = NULL;
	index->root->parent = NULL;
	free(pagehandle);
	return SUCCESS;
}

void searchpath(PF_PageHandle * pagehandle, int order, AttrType attrtype, int attrlength, void * pdata, RID * rid){
	char * nodedata = NULL;
	GetData(pagehandle, &nodedata);
	IX_Node * ixnode = (IX_Node *)(nodedata + sizeof(IX_FileHeader));
	char * nodekeys = nodedata + sizeof(IX_FileHeader) + sizeof(IX_Node);
	char * noderids = nodekeys + attrlength*order;
	int keynum = ixnode->keynum;
	int position = 0;
	float tempf;int tempdata;
	switch(attrtype){
	case chars:
		for(position = 0;position<keynum;position++){
			if(strcmp(nodekeys + position*attrlength, (char *)pdata)>0){
				break;
			}
		}
		break;
	case ints:
		tempdata = *((int *)pdata);
		for(position = 0;position<keynum;position++){
			int data = *((int *)(nodekeys + position*attrlength));
			if(data>tempdata){
				break;
			}
		}
		break;
	case floats:
		tempf = *((float *)pdata);
		for(position = 0;position<keynum;position++){
			float fdata = *((float *)(nodekeys + position*attrlength));
			if(fdata > tempf){
				break;
			}
		}
		break;
	}
	if(position == 0){
		memcpy(nodekeys, pdata, attrlength);
		memcpy(rid, noderids, sizeof(RID));
	}
	else{
		position--;
		memcpy(rid, noderids+position*sizeof(RID), sizeof(RID));
	}
}

void insertkeyandrid(PF_PageHandle * pagehandle, int order, AttrType attrtype, int attrlength, void * pdata, RID * rid){
	char * nodedata = NULL;
	GetData(pagehandle, &nodedata);
	IX_Node * ixnode = (IX_Node *)(nodedata + sizeof(IX_FileHeader));
	char * nodekeys = nodedata+sizeof(IX_FileHeader)+sizeof(IX_Node);
	char * noderids = nodedata+sizeof(IX_FileHeader)+sizeof(IX_Node)+order*attrlength;
	int keynum = ixnode->keynum;
	int position = 0;
	float tempf;int tempdata;
	switch(attrtype){
	case chars:
		for(position = 0;position<keynum;position++){
			if(strcmp(nodekeys + position*attrlength, (char *)pdata)>0){
				break;
			}
		}
		break;
	case ints:
		tempdata = *((int *)pdata);
		for(position = 0;position<keynum;position++){
			int data = *((int *)(nodekeys + position*attrlength));
			if(data>tempdata){
				break;
			}
		}
		break;
	case floats:
		tempf = *((float *)pdata);
		for(position = 0;position<keynum;position++){
			float fdata = *((float *)(nodekeys + position*attrlength));
			if(fdata > tempf){
				break;
			}
		}
		break;
	}
	memcpy(nodekeys+(position+1)*attrlength, nodekeys+position*attrlength, attrlength*(keynum-position));
	memcpy(nodekeys+position*attrlength, pdata, attrlength);
	memcpy(noderids+(position+1)*sizeof(RID), noderids+position*sizeof(RID), sizeof(RID)*(keynum-position));
	memcpy(noderids+position*sizeof(RID), rid, sizeof(RID));
	keynum++;
	ixnode->keynum=keynum;
	MarkDirty(pagehandle);
	UnpinPage(pagehandle);
}

void copykeyandrid(PF_PageHandle *pagehandle, void * keydata, int attrlength, int num, int order, void * riddata){
	char *pdata=NULL;
	GetData(pagehandle, &pdata);
	memcpy(pdata+sizeof(IX_FileHeader)+sizeof(IX_Node), keydata, num*attrlength);
	memcpy(pdata+sizeof(IX_FileHeader)+sizeof(IX_Node)+order*attrlength, riddata, num*sizeof(RID));
}


//�ҵ�pdata�ڸ�ҳ���ָ���������������򷵻�false��
void if_existence(PF_PageHandle * pagehandle, int order, AttrType attrtype, int attrlength, void *pdata, RID *rid, bool *existence){
	char *pagedata;
	GetData(pagehandle, &pagedata);
	IX_Node * IXnode = (IX_Node *)(pagedata+sizeof(IX_FileHeader));
	int keynum = IXnode->keynum;
	char *pagekeys = pagedata + sizeof(IX_FileHeader) + sizeof(IX_Node);
	char *pagerids = pagekeys + order*attrlength;
	int position = 0;
	int flag = 0;
	switch(attrtype){
	case chars:
		for(position = 0; position<keynum; position++){
			if(strcmp(pagekeys+position*attrlength, (char *)pdata)>=0){
				if(strcmp(pagekeys+position*attrlength, (char *)pdata)==0){
					flag = 1;
				}
				break;
			}
		}
		break;
	case ints:
		int tempdata;
		tempdata = *((int *)pdata);
		for(position = 0;position<keynum;position++){
			int data = *((int *)(pagekeys+position*attrlength));
			if(data >= tempdata){
				if(data == tempdata){
					flag = 1;
				}
				break;
			}
		}
		break;
	case floats:
		float tempf = *((float *)pdata);
		for(position = 0;position<keynum;position++){
			float fdata = *((float *)(pagekeys + position*attrlength));
			if(fdata >= tempf){
				if(fdata == tempf){
					flag = 1;
				}
				break;
			}
		}
		break;
	}
	if(flag==1){
		*existence = true;
		memcpy(rid, pagerids+position*sizeof(RID), sizeof(RID));
	}
	else{
		if(position==0){
			*existence = false;
		}
		else{
			*existence = true;
			position--;
			memcpy(rid, pagerids+sizeof(RID)*position, sizeof(RID));
		}
	}
}

//��ɾ���ڵ��ڵ��е�keynumС����Сֵ��ʱ��
void lessthanleastnum(PF_PageHandle *pagehandle, PF_FileHandle *filehandle, int order, AttrType attrtype, int attrlength, int *status){
	PageNum leftpagenum;
	leftbrother(pagehandle, filehandle, order, attrtype,attrlength, &leftpagenum);
	if(leftpagenum != 0){//�ýڵ������ֵ�
		PF_PageHandle * leftpagehandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
		GetThisPage(filehandle, leftpagenum, leftpagehandle);
		solvewithleft(pagehandle, leftpagehandle, order, attrtype, attrlength, status);
		free(leftpagehandle);
	}
	else{
		PF_PageHandle *rightpagehandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
		char *tempdata;
		GetData(pagehandle, &tempdata);
		IX_Node * tempnode = (IX_Node*)(tempdata+sizeof(IX_FileHeader));
		GetThisPage(filehandle, tempnode->brother, rightpagehandle);
		solvewithright(pagehandle, rightpagehandle, order, attrtype, attrlength, status);

		PF_PageHandle * parentpagehandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
		GetData(rightpagehandle, &tempdata);
		tempnode = (IX_Node*)(tempdata+sizeof(IX_FileHeader));
		GetThisPage(filehandle, tempnode->parent, parentpagehandle);
		PageNum nodepagenum;
		GetPageNum(rightpagehandle, &nodepagenum);
		char *tempkeys = tempdata+sizeof(IX_FileHeader)+sizeof(IX_Node);
		if (*status == 3)   //�����ֵܽ�һ���ؼ��֣�Ҫ�޸ĸ��ڵ��е�key
		{
			deleteNode(parentpagehandle, filehandle, order, attrtype, attrlength, nodepagenum, false, tempkeys);  //�ݹ��޸����ֵܽڵ�
		}
		free(rightpagehandle);
		free(parentpagehandle);
	}
	
	
	

}

//��ȡ�ڵ�����ֵܵ�pagenum
void leftbrother(PF_PageHandle *pagehandle, PF_FileHandle *filehandle, int order, AttrType attrtype, int attrlength, PageNum * leftbrothernum){
	char * pagedata;
	GetData(pagehandle, &pagedata);
	PageNum pagenum;
	GetPageNum(pagehandle, &pagenum);
	IX_Node *IXnode = (IX_Node *)(pagedata + sizeof(IX_FileHeader));
	PF_PageHandle *parentPH = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	GetThisPage(filehandle, IXnode->parent, parentPH);
	char *parentdata;
	GetData(parentPH, &parentdata);
	IX_Node *parentnode = (IX_Node *)(parentdata+sizeof(IX_FileHeader));
	char *parentkeys = parentdata + sizeof(IX_FileHeader) + sizeof(IX_Node);
	char *parentrids = parentkeys + order*attrlength;
	for(int i=0;i<parentnode->keynum;i++){
		RID *temprid = (RID *)parentrids+i*sizeof(RID);
		if(temprid->pageNum == pagenum){
			if(i!=0){
				i--;
				temprid = (RID *)parentrids + sizeof(RID)*i;
				*leftbrothernum = temprid->pageNum;
				return;
			}
			else{
				*leftbrothernum = 0;
				return;
			}
		}
	}
	*leftbrothernum = 0;
	free(parentPH);
	return;
}

void solvewithleft(PF_PageHandle *pagehandle, PF_PageHandle *lefthandle, int order, AttrType attrtype, int attrlength, int *status){
	char *pagedata;
	GetData(pagehandle, &pagedata);
	IX_Node * pagenode = (IX_Node *)(pagedata+sizeof(IX_FileHeader));
	int pagekeynum = pagenode->keynum;
	char *pagekeys = pagedata+sizeof(IX_FileHeader)+sizeof(IX_Node);
	char *pagerids = pagekeys+order*attrlength;
	char *leftdata;
	GetData(lefthandle, &leftdata);
	IX_Node *leftnode = (IX_Node *)(leftdata+sizeof(IX_FileHeader));
	int leftkeynum = leftnode->keynum;
	char *leftkeys = leftdata+sizeof(IX_FileHeader)+sizeof(IX_Node);
	char *leftrids = leftkeys+order*attrlength;
	if(leftkeynum > leastnum){//�����ֵܽ�һ���ڵ�
		memcpy(pagekeys+attrlength, pagekeys, pagekeynum*attrlength);
		memcpy(pagekeys, leftkeys+(leftkeynum - 1)*attrlength, attrlength);//�����ֵܵ����ֵ���뵽ԭ�ڵ����С��
		memcpy(pagerids+sizeof(RID), pagerids, pagekeynum*sizeof(RID));
		memcpy(pagerids, leftrids+(leftkeynum - 1)*sizeof(RID), sizeof(RID));
		pagekeynum++;
		pagenode->keynum = pagekeynum;
		leftkeynum--;
		leftnode->keynum = leftkeynum;
		*status = 1;
	}
	else{//�����ֵܺϲ�(��ԭ�ڵ��������ӵ����ֵ���)
		memcpy(leftkeys+leftkeynum*attrlength, pagekeys, pagekeynum*attrlength);
		memcpy(leftrids+leftkeynum*sizeof(RID),pagerids, pagekeynum*sizeof(RID));
		leftkeynum = leftkeynum + pagekeynum;
		leftnode->keynum = leftkeynum;
		pagekeynum = 0;
		pagenode->keynum = pagekeynum;
		leftnode->brother = pagenode->brother;
		*status = 2;
	}
	MarkDirty(pagehandle);
	UnpinPage(pagehandle);
	MarkDirty(lefthandle);
	UnpinPage(lefthandle);
}

void solvewithright(PF_PageHandle *pagehandle, PF_PageHandle *righthandle, int order, AttrType attrtype, int attrlength, int *status){
	char *pagedata;
	GetData(pagehandle, &pagedata);
	IX_Node * pagenode = (IX_Node *)(pagedata+sizeof(IX_FileHeader));
	int pagekeynum = pagenode->keynum;
	char *pagekeys = pagedata+sizeof(IX_FileHeader)+sizeof(IX_Node);
	char *pagerids = pagekeys+order*attrlength;
	char *rightdata;
	GetData(righthandle, &rightdata);
	IX_Node *rightnode = (IX_Node *)(rightdata+sizeof(IX_FileHeader));
	int rightkeynum = rightnode->keynum;
	char *rightkeys = rightdata+sizeof(IX_FileHeader)+sizeof(IX_Node);
	char *rightrids = rightkeys+order*attrlength;
	if(rightkeynum > leastnum){//�����ֵܽ�һ���ڵ�
		memcpy(pagekeys+pagekeynum*attrlength, rightkeys, attrlength);
		memcpy(rightkeys, rightkeys+attrlength, (rightkeynum-1)*attrlength);//�����ֵܵ����ֵ���뵽ԭ�ڵ����С��
		memcpy(pagerids+pagekeynum*sizeof(RID), rightrids, sizeof(RID));
		memcpy(rightrids, rightrids+sizeof(RID), (rightkeynum-1)*sizeof(RID));
		pagekeynum++;
		pagenode->keynum = pagekeynum;
		rightkeynum--;
		rightnode->keynum = rightkeynum;
		*status = 3;
	}
	else{//�����ֵܺϲ�(��ԭ�ڵ��������ӵ����ֵ���)
		memcpy(pagekeys+pagekeynum*attrlength, rightkeys, rightkeynum*attrlength);
		memcpy(pagerids+pagekeynum*sizeof(RID),rightrids, rightkeynum*sizeof(RID));
		pagekeynum = rightkeynum + pagekeynum;
		pagenode->keynum = pagekeynum;
		rightkeynum = 0;
		rightnode->keynum = rightkeynum;
		//pagenode->brother = rightnode->brother;
		*status = 4;
	}
	MarkDirty(pagehandle);
	UnpinPage(pagehandle);
	MarkDirty(righthandle);
	UnpinPage(righthandle);
}

//ɾ�����޸�ĳ���ڵ��е�����ֵ��pdata���������ǽ��ϲ������ڣ��޸ģ�������ֵΪ��С�����Ҫͬʱ�޸ĸ��ڵ��д洢������ֵ
void deleteNode(PF_PageHandle *parentpagehandle, PF_FileHandle *filehandle, int order, AttrType attrtype, int attrlength, PageNum nodepagenum, bool ifdelete, void *pdata){
	char *parentdata;
	char *parentkeys;
	char *parentrids;
	while(true){
		GetData(parentpagehandle,&parentdata);
		IX_Node * pagenode = (IX_Node *)(parentdata+sizeof(IX_FileHeader));
		int keynum = pagenode->keynum;
		parentkeys = parentdata+sizeof(IX_FileHeader)+sizeof(IX_Node);
		parentrids = parentkeys+order*attrlength;
		for(int i=0;i<pagenode->keynum;i++){
			RID *temprid = (RID *)(parentrids+i*sizeof(RID));
			if(temprid->pageNum == nodepagenum){
				if(ifdelete){//ɾ���ؼ���
					memcpy(parentkeys+i*attrlength, parentkeys+(i+1)*attrlength, (keynum-i-1)*attrlength);
					memcpy(parentrids+i*sizeof(RID), parentrids+(i+1)*sizeof(RID), (keynum-i-1)*sizeof(RID));
					keynum--;
					pagenode->keynum = keynum;
					return;
				}
				else{//�޸Ĺؼ���
					memcpy(parentkeys+i*attrlength, pdata,attrlength);
					if(i==0 && pagenode->parent!=0){//ͬʱҪ�޸ĸ��ڵ��еĹؼ���
						MarkDirty(parentpagehandle);
						UnpinPage(parentpagehandle);
						GetPageNum(parentpagehandle, &nodepagenum);
						GetThisPage(filehandle, pagenode->parent, parentpagehandle);
					}
					return;
				}
			}
		}
	}
	MarkDirty(parentpagehandle);
	UnpinPage(parentpagehandle);
}

//�ҵ�Ҷ�ӽڵ��е�һ�����ڻ����pdata��ֵ,startpagenum:��һ�����ڻ���ڵ�ҳ���
void scan_thefirstequal(IX_IndexHandle * indexhandle, void *pdata, PageNum *startpagenum, int *ridIx, bool *existence){
	PF_FileHandle filehandle = indexhandle->fileHandle;
	IX_FileHeader fileheader = indexhandle->fileHeader;
	int order = fileheader.order;
	int attrlength = fileheader.attrLength;
	AttrType attrtype = fileheader.attrType;
	PF_PageHandle *pagehandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	GetThisPage(&filehandle, fileheader.rootPage, pagehandle);
	char *pagedata;
	GetData(pagehandle, &pagedata);
	IX_Node * IXnode = (IX_Node *)(pagedata + sizeof(IX_FileHeader));
	int keynum = IXnode->keynum;
	int position;
	int flag;
	while(true){
		char *pagekeys = pagedata + sizeof(IX_FileHeader) + sizeof(IX_Node);
		char *pagerids = pagekeys + order*attrlength;
		for(position = 0, flag = 0;position<keynum;position++){
			int comkey = comparekeys(pagekeys+position*attrlength, pdata, attrtype);
			if(comkey >= 0){
				if(comkey == 0){
					flag = 1;
				}
				break;
			}
		}
		if(IXnode->is_leaf == 1){//����Ҷ��ҳ�棬ֱ���ж��Ƿ��ҵ�
			if(flag == 1){
				*existence = true;
			}
			else{
				*existence = false;
			}
			GetPageNum(pagehandle, startpagenum);
			*ridIx = position;
			return;
		}
		else if(flag==0 && IXnode->is_leaf == 0){//��������ҳ�棬Ҫ����������
			if(position>0){
				position--;
			}
		}
		MarkDirty(pagehandle);
		UnpinPage(pagehandle);
		RID *temprid = (RID *)(pagerids + position*sizeof(RID));
		GetThisPage(&filehandle, temprid->pageNum, pagehandle);
		GetData(pagehandle, &pagedata);
		IXnode = (IX_Node *)(pagedata + sizeof(IX_FileHeader));
		keynum = IXnode->keynum;
	}
	free(pagehandle);

}

//�Ƚ�������ֵ�Ĵ�С��data1>data2:1   data1==data2:0  data1<data2:-1
int comparekeys(void *data1, void *data2, AttrType attrtype){
	int res;
	int tempint1,tempint2;
	float tempf1, tempf2;
	switch(attrtype){
	case chars:
		res = strcmp((char *)data1, (char *)data2);
		break;
	case ints:
		tempint1 = *((int *)data1);
		tempint2 = *((int *)data2);
		if(tempint1>tempint2){
			res = 1;
		}
		if(tempint1==tempint2){
			res = 0;
		}
		if(tempint1<tempint2){
			res = -1;
		}
		break;
	case floats:
		tempf1 = *((float *)data1);
		tempf2 = *((float *)data2);
		if(tempf1>tempf2){
			res = 1;
		}
		if(tempf1==tempf2){
			res = 0;
		}
		if(tempf1<tempf2){
			res = -1;
		}
		break;
	}
	return res;
}

//�ҵ���һ������pdata��ֵ���ڵ�ҳ��
void scan_lequal(IX_IndexHandle *indexHandle, void *pdata, PageNum *startpagenum, int *ridIx){
	PF_FileHandle filehandle = indexHandle->fileHandle;
	IX_FileHeader fileheader = indexHandle->fileHeader;
	int order = fileheader.order;
	int attrlength = fileheader.attrLength;
	AttrType attrtype = fileheader.attrType;
	PF_PageHandle *pagehandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	GetThisPage(&filehandle, fileheader.rootPage, pagehandle);
	char *pagedata;
	GetData(pagehandle, &pagedata);
	IX_Node *IXnode = (IX_Node *)(pagedata + sizeof(IX_FileHeader));
	int keynum = IXnode->keynum;
	int position;
	int flag;
	while(true){
		char *pagekeys = pagedata + sizeof(IX_FileHeader) + sizeof(IX_Node);
		char *pagerids = pagekeys + order*attrlength;
		for(position=keynum-1,flag=0;position>=0;position--){
			int comkey = comparekeys(pagekeys+position*attrlength, pdata, attrtype);
			if(comkey<=0){
				break;
			}
		}
		if(IXnode->is_leaf==1){//Ҷ�ӽڵ�
			position++;
			*ridIx = position;
			GetPageNum(pagehandle, startpagenum);
			return;
		}
		if(position<0){
			position = 0;
		}
		MarkDirty(pagehandle);
		UnpinPage(pagehandle);
		RID *temprid = (RID *)(pagerids + position*sizeof(RID));
		GetThisPage(&filehandle, temprid->pageNum, pagehandle);
		GetData(pagehandle, &pagedata);
		IXnode = (IX_Node *)(pagedata + sizeof(IX_FileHeader));
		keynum = IXnode->keynum;
	}
	free(pagehandle);
}
//����/ɾ����ɺ󣬸���ÿ���ڵ��parent��Ϣ
void update_parent(PF_FileHandle *filehandle, IX_FileHeader fileheader){
	queue<PF_PageHandle *> que;
	char *pdata;
	char *rootparent;
	PageNum parent = 0;
	PF_PageHandle *pagehandle = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
	GetThisPage(filehandle,fileheader.rootPage,pagehandle);
	GetData(pagehandle,&rootparent);
	IX_Node *IXnodert = (IX_Node *)(rootparent+sizeof(IX_FileHeader));
	IXnodert->parent = 0;
	que.push(pagehandle);
	while(!que.empty()){
		PF_PageHandle * temppagehandle = que.front();
		que.pop();
		GetPageNum(temppagehandle, &parent);
		GetData(temppagehandle,&pdata);
		IX_Node *IXnode = (IX_Node *)(pdata+sizeof(IX_FileHeader));
		for(int i=0;i<IXnode->keynum;i++){
			char *pagekeys = (char *)IXnode+sizeof(IX_Node);;
			char *pagerids = pagekeys+fileheader.order*fileheader.attrLength;
			char *key = pagekeys+i*fileheader.attrLength;
			RID *rid = (RID *)(pagerids+i*sizeof(RID));
			if(IXnode->is_leaf!=1){
				PF_PageHandle *pagehandle2 = (PF_PageHandle *)malloc(sizeof(PF_PageHandle));
				GetThisPage(filehandle,rid->pageNum,pagehandle2);
				char *pdata2;
				GetData(pagehandle2,&pdata2);
				IX_Node *IXnode2 = (IX_Node *)(pdata2+sizeof(IX_FileHeader));
				IXnode2->parent = parent;
				que.push(pagehandle2);
				MarkDirty(pagehandle2);
				UnpinPage(pagehandle2);
			}
			//printf("%s ",key);
		}
		//printf("\n");
	}
	UnpinPage(pagehandle);
	free(pagehandle);
}