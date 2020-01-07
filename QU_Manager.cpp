#include "StdAfx.h"
#include "QU_Manager.h"
#include "RM_Manager.h"
#include "SYS_Manager.h"

void Init_Result(SelResult * res){
	res->next_res = NULL;
}

void Destory_Result(SelResult * res){
	for(int i = 0;i<res->row_num;i++){
		free( res->res[i]);	
	}
	free(res->res);
	if(res->next_res != NULL){
		Destory_Result(res->next_res);
	}
}

RC Query(char * sql,SelResult * res){
	sqlstr *sql_str = NULL;
	RC rc;
	sql_str = get_sqlstr();
  	rc = parse(sql, sql_str);
	RC rc1;
	//SelResult *res;
	//if(rc==SUCCESS)
	rc1=Select (sql_str->sstr.sel.nSelAttrs,sql_str->sstr.sel.selAttrs,sql_str->sstr.sel.nRelations,sql_str->sstr.sel.relations,
		sql_str->sstr.sel.nConditions,sql_str->sstr.sel.conditions,res);
	if(rc1==SUCCESS)return SUCCESS;
	else return rc1;
}

RC Select(int nSelAttrs,RelAttr **selAttrs,int nRelations,char **relations,int nConditions,Condition *conditions,SelResult * res){


	if(access(*relations ,0)!=0) return TABLE_NOT_EXIST;
	RM_FileHandle *rm_table;
	RM_FileScan FileScan;
	RM_Record rectab;
	rm_table=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));//打开系统表文件
	rm_table->bOpen = false;
	if (RM_OpenFile("SYSTABLES",rm_table)!= SUCCESS)return SQL_SYNTAX;

	int flags=0;

	for (int i=0;i<nRelations;i++){
		FileScan.bOpen=false;
		if(OpenScan(&FileScan,rm_table,0,NULL)!=SUCCESS)return SQL_SYNTAX;
		while(GetNextRec(&FileScan, &rectab)==SUCCESS){
			//if(strcmp(relations[i],"SYSTABLES")){flags=1 ;break;}
			//if(strcmp(relations[i],"SYSCOLUMNS")){flags=2 ;break;}
			if(strcmp(relations[i], rectab.pData)==0)break;
			//if(GetNextRec(&FileScan, &rectab)!=SUCCESS){
			
				//return TABLE_NOT_EXIST;
			//}
		}
		if(CloseScan(&FileScan)!= SUCCESS)return SQL_SYNTAX;
	}
	//if(flags!=0){
		//single_nocon(nSelAttrs, selAttrs, 1, relations, 0, conditions, res);
		//return SUCCESS;

	//}
	
	if(nRelations==1&&nConditions==0)single_nocon(nSelAttrs, selAttrs, nRelations, relations, nConditions, conditions, res);//单表单属性，多属性 ，无条件查询

	else if(nRelations==1&&nConditions>0)single_con(nSelAttrs, selAttrs, nRelations, relations, nConditions, conditions, res);//单表条件查询
	else if(nRelations>1)multi(nSelAttrs, selAttrs, nRelations, relations, nConditions, conditions, res);//多表查询
	return SUCCESS;
}

//单表单属性，多属性 ，无条件查询
RC single_nocon(int nSelAttrs, RelAttr **selAttrs, int nRelations, char **relations, int nConditions, Condition *conditions, SelResult * res){

	//if(access(*relations,0)!=0) return TABLE_NOT_EXIST;
	SelResult *resHead = res;
	RM_FileHandle *rm_table,*rm_reccol,*rm_data;
	RM_FileScan FileScan;
	RM_Record rectab,reccol,recdata;
	rm_table=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));//打开系统表文件
	rm_table->bOpen = false;
	if (RM_OpenFile("SYSTABLES",rm_table)!= SUCCESS)return SQL_SYNTAX;
	FileScan.bOpen=false;
	if(OpenScan(&FileScan,rm_table,0,NULL)!=SUCCESS)return SQL_SYNTAX;
	while(GetNextRec(&FileScan, &rectab)==SUCCESS){
		if(strcmp(relations[0], rectab.pData)==0)memcpy(&(resHead->col_num), rectab.pData + 21, sizeof(int));//获取属性个数
	}
	if(CloseScan(&FileScan)!= SUCCESS)return SQL_SYNTAX;
	if(RM_CloseFile(rm_table)!= SUCCESS)return SQL_SYNTAX;
	rm_reccol=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));//打开系统列文件
	rm_reccol->bOpen = false;
	if (RM_OpenFile("SYSCOLUMNS",rm_reccol)!= SUCCESS)return SQL_SYNTAX;
	FileScan.bOpen=false;
	if(OpenScan(&FileScan,rm_reccol,0,NULL)!=SUCCESS)return SQL_SYNTAX;
	int i=0;
	while (GetNextRec(&FileScan, &reccol) == SUCCESS){
		if (strcmp(relations[0], reccol.pData) == 0){//获取各项属性信息
			//for (int i = 0; i <resHead->col_num; i++){
				memcpy(&resHead->type[i], reccol.pData+42, sizeof(AttrType));
				int s ;
				memcpy(&s,reccol.pData+46,4);
				memcpy(&resHead->fields[i], reccol.pData+21, s);
				memcpy(&resHead->offset[i], reccol.pData+42+sizeof(int)+sizeof(AttrType), sizeof(int));
				memcpy(&resHead->length[i], reccol.pData + 42 + sizeof(AttrType), sizeof(int));
			//}
			//break;
				i++;
		}
		
	}


	if(CloseScan(&FileScan)!= SUCCESS)return SQL_SYNTAX;

	if(RM_CloseFile(rm_reccol)!= SUCCESS)return SQL_SYNTAX;

	rm_data = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));

	rm_data->bOpen = false;
	if (RM_OpenFile(relations[0], rm_data)!= SUCCESS)return SQL_SYNTAX;
	FileScan.bOpen = false;
	if (OpenScan(&FileScan, rm_data, 0, NULL)!= SUCCESS)return SQL_SYNTAX;
	 i = 0;
	resHead->row_num = 0;
	int j=0;
	SelResult *curRes = resHead;  //尾插法向链表中插入新结点
	while (GetNextRec(&FileScan,&recdata) == SUCCESS){
		if (curRes->row_num >= 100){ //每个节点最多记录100条记录,当前结点已经保存100条记录时，新建结点
			curRes->next_res = (SelResult *)malloc(sizeof(SelResult));
			curRes->next_res->col_num = curRes->col_num;
			for (int j = 0; j < curRes->col_num; j++){
				strncpy(curRes->next_res->fields[j], curRes->fields[i], strlen(curRes->fields[i]));
				curRes->next_res->type[j] = curRes->type[i];
				curRes->next_res->offset[j] = curRes->offset[i];
			}
			curRes = curRes->next_res;
			curRes->next_res = NULL;
			curRes->row_num = 0;
		}
		int d=curRes->row_num;
		//curRes->res[curRes->row_num] = (char **)malloc(sizeof(char *));
		//(curRes->res[curRes->row_num++]) = &(recdata.pData);
		//curRes->res[curRes->row_num] = (char *)malloc(sizeof(char *));


		(curRes->res[curRes->row_num++]) = (recdata.pData);


		//int s;
		//char *p=(curRes->res[d]);
		//memcpy(&s,(p+20*j),4);
		//j++;
		//char str[5];
	   // sprintf(str,"%d", s);
		//AfxMessageBox(str);

	}





	res = curRes;
	if(strcmp((*selAttrs)->attrName ,"*")!=0){
	char filed[20][20];
	int offset[20];
	int length[20];
	int type[20];
	//(*selAttrs)--;

	//for(int i=0;i<nSelAttrs;i++,selAttrs++){
		//(*selAttrs)->attrName;
	//}//
	for(int i=0;i<nSelAttrs;i++,selAttrs++){
		for(int j=0;j<res->col_num;j++){
			if(strcmp((*selAttrs)->attrName ,res->fields[j])==0){
				strcpy(filed[i],res->fields[j]);
				offset[i]=res->offset[j];
				length[i]=res->length[j];
				type[i]=res->type[j];
				break;
			}
		}

	}

	res->col_num=nSelAttrs;

	for(int i=0,j=nSelAttrs-1;i<nSelAttrs;i++,j--){
		strcpy(res->fields[i],filed[j]);
		res->offset[i]=offset[j];
		res->length[i]=length[j];
		res->type[i] = (AttrType)type[j];

	}
	}

    if(CloseScan(&FileScan)!= SUCCESS)return SQL_SYNTAX;
	if(RM_CloseFile(rm_data)!= SUCCESS)return SQL_SYNTAX;
	free(rm_table);
	free(rm_reccol);
	free(rm_data);
	


	return SUCCESS;
}


//单表条件查询，，查询属性个数                   表的个数为一                      条件的个数
RC single_con(int nSelAttrs, RelAttr **selAttrs, int nRelations, char **relations, int nConditions, Condition *conditions, SelResult * res){

	SelResult *resHead = res;
	RM_FileHandle *rm_fileHandle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	RM_FileScan *rm_fileScan = (RM_FileScan *)malloc(sizeof(RM_FileScan));
	RM_Record *record = (RM_Record *)malloc(sizeof(RM_Record));
	rm_fileHandle->bOpen=false;
	

	char *in_flag;

	char indexname[21];

	Con cons[2];
	cons[0].attrType = chars;
	cons[0].bLhsIsAttr = 1;
	cons[0].LattrOffset = 0;
	cons[0].LattrLength = strlen(relations[0]) + 1;
	cons[0].compOp = EQual;
	cons[0].bRhsIsAttr = 0;
	cons[0].Rvalue = relations[0];
	int flag=0;
	if (!strcmp((*selAttrs)->attrName, "*")){//查询结果为所有属性

		if (RM_OpenFile("SYSTABLES", rm_fileHandle)!= SUCCESS) return SQL_SYNTAX;
		rm_fileScan->bOpen=false;
		OpenScan(rm_fileScan, rm_fileHandle, 1, &cons[0]);
		if (GetNextRec(rm_fileScan, record)!= SUCCESS) return SQL_SYNTAX;
		table *Table = (table *)record->pData;
		memcpy(&(resHead->col_num), record->pData + 21, sizeof(int));//获取属性个数
		in_flag=(char *)malloc((resHead->col_num+1)*sizeof(char));
		CloseScan(rm_fileScan);
		RM_CloseFile(rm_fileHandle);
		if (RM_OpenFile("SYSCOLUMNS", rm_fileHandle)!= SUCCESS) return SQL_SYNTAX;
		OpenScan(rm_fileScan, rm_fileHandle, 1, &cons[0]);
		for (int i = 0; i < resHead->col_num; i++){//获取属性信息
			if (GetNextRec(rm_fileScan, record)!= SUCCESS) return SQL_SYNTAX;
			char * column = record->pData;
			char attrname[21];
			memcpy(attrname,column+21,21);
			memcpy(&resHead->type[i], column + 42, sizeof(int));
			memcpy(&resHead->fields[i], column + 21, 21);
			memcpy(&resHead->offset[i], column + 50, sizeof(int));
			memcpy(&resHead->length[i], column + 46, sizeof(int));
			memcpy(&in_flag[i],column+54,sizeof(char));
			if(in_flag[i]=='1'&&strcmp(attrname,conditions[0].lhsAttr.attrName)==0){
				memcpy(indexname,column+55,21);
			    flag=1;
			}
			
		}
		CloseScan(rm_fileScan);
		RM_CloseFile(rm_fileHandle);
	}
	else{//查询结果为指定属性

		resHead->col_num = nSelAttrs; //属性个数为nSelAttrs
		in_flag=(char *)malloc((nSelAttrs+1)*sizeof(char));
		cons[1].attrType = chars;
		cons[1].bLhsIsAttr = 1;
		cons[1].LattrOffset = 21;
		cons[1].LattrLength = 21;
		cons[1].compOp = EQual;
		cons[1].bRhsIsAttr = 0;
		rm_fileHandle->bOpen=false;
		if (RM_OpenFile("SYSCOLUMNS", rm_fileHandle)!= SUCCESS) return SQL_SYNTAX;

		
		for (int i = 0; i < resHead->col_num; i++){
			cons[1].Rvalue = (selAttrs[resHead->col_num - i - 1])->attrName;
			rm_fileScan->bOpen=false;
			OpenScan(rm_fileScan, rm_fileHandle, 2, cons);
			if (GetNextRec(rm_fileScan, record)!= SUCCESS) return SQL_SYNTAX;
			char * column = record->pData;
			char attrname[21];
			memcpy(attrname,column+21,21);
			memcpy(&resHead->type[i], column + 42, sizeof(int));
			memcpy(&resHead->fields[i], column + 21, 21);
			memcpy(&resHead->offset[i], column + 50, sizeof(int));
			memcpy(&resHead->length[i], column + 46, sizeof(int));
			memcpy(&in_flag[i],column+54,sizeof(char));
			if(in_flag[i]=='1'&&strcmp(attrname,conditions[0].lhsAttr.attrName)==0){
				memcpy(indexname,column+55,21);
			    flag=1;
			}
			CloseScan(rm_fileScan);
		}
		RM_CloseFile(rm_fileHandle);
	}
	rm_fileHandle->bOpen=false;
	if (RM_OpenFile("SYSCOLUMNS", rm_fileHandle)!= SUCCESS) return SQL_SYNTAX;
	cons[1].attrType = chars;
	cons[1].bLhsIsAttr = 1;
	cons[1].LattrOffset = 21;
	cons[1].LattrLength = 21;
	cons[1].compOp = EQual;
	cons[1].bRhsIsAttr = 0;

	rm_fileScan->bOpen=false;
	//以条件查询的条件作为扫描条件
	Con *selectCons = (Con *)malloc(sizeof(Con) * nConditions);
	for (int i = 0; i < nConditions; i++) {//只需设置cons[1]->rValue,把条件里的属性设置成查询时的值
		if (conditions[i].bLhsIsAttr == 0 && conditions[i].bRhsIsAttr == 1){//左边是值，右边是属性
			cons[1].Rvalue = conditions[i].rhsAttr.attrName;
		}
		else if (conditions[i].bLhsIsAttr == 1 && conditions[i].bRhsIsAttr == 0){//左边是属性，右边是值
			cons[1].Rvalue = conditions[i].lhsAttr.attrName;
		}
		else{//两边都是属性或两边都是值，暂不考虑
		}
		OpenScan(rm_fileScan, rm_fileHandle, 2, cons);
		if (GetNextRec(rm_fileScan, record) != SUCCESS) return SQL_SYNTAX;
		selectCons[i].bLhsIsAttr = conditions[i].bLhsIsAttr;
		selectCons[i].bRhsIsAttr = conditions[i].bRhsIsAttr;
		selectCons[i].compOp = conditions[i].op;
		if (conditions[i].bLhsIsAttr == 1) //左边属性
		{ //设置属性长度和偏移量
			memcpy(&selectCons[i].LattrLength, record->pData + 46, 4);
			memcpy(&selectCons[i].LattrOffset, record->pData + 50, 4);
		}
		else {
			selectCons[i].attrType = conditions[i].lhsValue.type;
			selectCons[i].Lvalue = conditions[i].lhsValue.data;
		}

		if (conditions[i].bRhsIsAttr == 1) {
			memcpy(&selectCons[i].RattrLength, record->pData + 46, 4);
			memcpy(&selectCons[i].RattrOffset, record->pData + 50, 4);
		}
		else {
			selectCons[i].attrType = conditions[i].rhsValue.type;
			selectCons[i].Rvalue = conditions[i].rhsValue.data;
		}
		CloseScan(rm_fileScan);
	}


	RM_CloseFile(rm_fileHandle);

	rm_fileHandle->bOpen=false;
	rm_fileScan->bOpen=false;
	//扫描记录表，找出所有记录
	if (RM_OpenFile(*relations, rm_fileHandle)!= SUCCESS) return SQL_SYNTAX;
	
	int i = 0;
	resHead->row_num = 0;
	SelResult *curRes = resHead;  //尾插法向链表中插入新结点
	if((flag==1&&nConditions==1)){
		IX_IndexHandle *rm_index=(IX_IndexHandle *)malloc(sizeof(IX_IndexHandle));
		rm_index->bOpen=false;
		if(OpenIndex(indexname, rm_index)!=SUCCESS){      
		            AfxMessageBox("索引文件打开失败");
		            return SQL_SYNTAX;
	            }

		IX_IndexScan *indexScan=(IX_IndexScan *)malloc(sizeof(IX_IndexScan ));
		indexScan->bOpen=false;

		char str[10];
		
		memcpy(str,conditions[0].rhsValue.data,4);
		//sprintf(str,"%d",a );
		//str[4]='\0';
		OpenIndexScan(indexScan,rm_index,conditions[0].op,str);
		RID  *rid =(RID *)malloc(sizeof(RID));
		IX_GetNextEntry(indexScan,rid);
		//RM_Record record;
		while(rid->pageNum!=0){
			GetRec (rm_fileHandle, rid, record); 

			if (curRes->row_num >= 100) //每个节点最多记录100条记录
		{ //当前结点已经保存100条记录时，新建结点
			curRes->next_res = (SelResult *)malloc(sizeof(SelResult));
			curRes->next_res->col_num = curRes->col_num;
			for (int j = 0; j < curRes->col_num; j++)
			{
				strncpy(curRes->next_res->fields[i], curRes->fields[i], strlen(curRes->fields[i]));
				curRes->next_res->type[i] = curRes->type[i];
				curRes->next_res->offset[i] = curRes->offset[i];
			}
			curRes = curRes->next_res;
			curRes->next_res = NULL;
			curRes->row_num = 0;
		}
		curRes->res[curRes->row_num] = (char *)malloc(sizeof(char *));
		(curRes->res[curRes->row_num++]) = record->pData;
			IX_GetNextEntry(indexScan,rid);
		}


	}
	else{
		OpenScan(rm_fileScan, rm_fileHandle, nConditions, selectCons);
		while (GetNextRec(rm_fileScan, record) == SUCCESS)
	{
		if (curRes->row_num >= 100) //每个节点最多记录100条记录
		{ //当前结点已经保存100条记录时，新建结点
			curRes->next_res = (SelResult *)malloc(sizeof(SelResult));
			curRes->next_res->col_num = curRes->col_num;
			for (int j = 0; j < curRes->col_num; j++)
			{
				strncpy(curRes->next_res->fields[i], curRes->fields[i], strlen(curRes->fields[i]));
				curRes->next_res->type[i] = curRes->type[i];
				curRes->next_res->offset[i] = curRes->offset[i];
			}
			curRes = curRes->next_res;
			curRes->next_res = NULL;
			curRes->row_num = 0;
		}
		curRes->res[curRes->row_num] = (char *)malloc(sizeof(char *));
		(curRes->res[curRes->row_num++]) = record->pData;
	}
	}



	CloseScan(rm_fileScan);
	RM_CloseFile(rm_fileHandle);
	free(rm_fileHandle);
	free(rm_fileScan);
	free(record);
	res = resHead;
	return SUCCESS;

}



RC multi(int nSelAttrs, RelAttr **selAttrs, int nRelations, char **relations, int nConditions, Condition *conditions, SelResult * res) {

	SelResult *resHead = res;
	RM_FileHandle *rm_fileHandle = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	RM_FileScan *rm_fileScan = (RM_FileScan *)malloc(sizeof(RM_FileScan));
	RM_Record *record = (RM_Record *)malloc(sizeof(RM_Record));
	RM_Record recdata;
	RM_FileHandle *rm_data = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	
	//int *a=(int *)malloc(4*sizeof(int));
	//a[1]=0;
	SelResult *result=(SelResult*)malloc(nRelations*sizeof(SelResult));
	Con cons[2];
	//resHead->col_num=0;
	//int *amount=(int * )malloc(nRelations*sizeof(int));
	if (!strcmp((*selAttrs)->attrName, "*")){

		for( int j=0;j<nRelations;j++){

			rm_fileHandle->bOpen=false;
			cons[0].attrType = chars;
			cons[0].bLhsIsAttr = 1;
			cons[0].LattrOffset = 0;
			cons[0].LattrLength = strlen(relations[j]) + 1;
			cons[0].compOp = EQual;
			cons[0].bRhsIsAttr = 0;
			cons[0].Rvalue = relations[j];

			if (RM_OpenFile("SYSTABLES", rm_fileHandle)!= SUCCESS) return SQL_SYNTAX;
			rm_fileScan->bOpen=false;
			OpenScan(rm_fileScan, rm_fileHandle, 1, &cons[0]);
			if (GetNextRec(rm_fileScan, record)!= SUCCESS) return SQL_SYNTAX;
			table *Table = (table *)record->pData;
			//int count=0;
			memcpy(&result[j].col_num, record->pData + 21, sizeof(int));//获取属性个
			//amount[j]=count;
			CloseScan(rm_fileScan);
			RM_CloseFile(rm_fileHandle);
			if (RM_OpenFile("SYSCOLUMNS", rm_fileHandle)!= SUCCESS) return SQL_SYNTAX;
			OpenScan(rm_fileScan, rm_fileHandle, 1, &cons[0]);
			for (int i =0 ; i<result[j].col_num; i++){//获取属性信息
			if (GetNextRec(rm_fileScan, record)!= SUCCESS) return SQL_SYNTAX;
			char * column = record->pData;
			memcpy(&result[j].type[i], column + 42, sizeof(int));
			memcpy(result[j].fields[i], column + 21, 21);
			memcpy(&result[j].offset[i], column + 50, sizeof(int));
			memcpy(&result[j].length[i], column + 46, sizeof(int));
			
		    }
		CloseScan(rm_fileScan);
		RM_CloseFile(rm_fileHandle);
		}
		



		SelResult *curRes = res; 
		int i=0;
		int length=0;
			for( i=0;i<result[0].col_num;i++){
				memcpy(curRes->fields[i],result[0].fields[i],20);
				curRes->length[i]=result[0].length[i];
				curRes->offset[i]=result[0].offset[i];
				curRes->type[i]=result[0].type[i];
				length=length+result[0].length[i];
			}
			for(int j=0;j<result[1].col_num;j++){
				memcpy(curRes->fields[i],result[1].fields[j],20);
				curRes->length[i]=result[1].length[j];
				curRes->offset[i]=curRes->offset[i-1]+curRes->length[i-1];
				curRes->type[i]=result[1].type[j];
				length=length+result[1].length[j];
				i++;
			}
		curRes->row_num=0;
		curRes->col_num=result[0].col_num+result[1].col_num;
		//curRes->row_num=result[0].row_num*result[1].row_num;
			rm_data->bOpen = false;
			if (RM_OpenFile(relations[0], rm_data)!= SUCCESS)return SQL_SYNTAX;
			rm_fileScan->bOpen = false;
			if (OpenScan(rm_fileScan, rm_data, 0, NULL)!= SUCCESS)return SQL_SYNTAX;
			 //尾插法向链表中插入新结点
			while (GetNextRec(rm_fileScan,&recdata) == SUCCESS){
				/*if (curRes->row_num >= 100){ //每个节点最多记录100条记录,当前结点已经保存100条记录时，新建结点
				curRes->next_res = (SelResult *)malloc(sizeof(SelResult));
				curRes->next_res->col_num = curRes->col_num;
				for (int i = 0; i < curRes->col_num; i++){
					strncpy(curRes->next_res->fields[i], curRes->fields[i], strlen(curRes->fields[i]));
					curRes->next_res->type[i] = curRes->type[i];
					curRes->next_res->offset[i] = curRes->offset[i];
				}
				curRes = curRes->next_res;
				curRes->next_res = NULL;
				curRes->row_num = 0;
				}*/
				
				RM_FileHandle *rm_data1 = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	            RM_FileScan *rm_fileScan1 = (RM_FileScan *)malloc(sizeof(RM_FileScan));
				rm_data1->bOpen=false;
				rm_fileScan1->bOpen=false;
				if (RM_OpenFile(relations[1], rm_data1)!= SUCCESS)return SQL_SYNTAX;
			    if (OpenScan(rm_fileScan1, rm_data1, 0, NULL)!= SUCCESS)return SQL_SYNTAX;
				RM_Record recdata1;
				//curRes->res[curRes->row_num] = (char **)malloc(sizeof(char *));
				//(curRes->res[curRes->row_num++]) = &(recdata.pData);
				//curRes->res[curRes->row_num] = (char *)malloc(sizeof(char *)); 
				while (GetNextRec(rm_fileScan1,&recdata1) == SUCCESS){
				    //curRes->res1[curRes->row_num]=(char**) malloc((curRes->col_num)*sizeof(char*));
				    char *p=(char *)malloc(length*sizeof(char));
					curRes->res[curRes->row_num]=p;
					
					int i=0;
					for( i=0;i<result[0].col_num;i++){
						
						memcpy(curRes->res[curRes->row_num]+curRes->offset[i],recdata.pData+result[0].offset[i],result[0].length[i]);
					}
					for(int j=0;j<result[1].col_num;j++){
						memcpy(curRes->res[curRes->row_num]+curRes->offset[i],recdata1.pData+result[1].offset[j],result[0].length[j]);
						i++;
					}

					curRes->row_num++;
				}
				if(CloseScan(rm_fileScan1)!= SUCCESS)return SQL_SYNTAX;
		      if(RM_CloseFile(rm_data1)!= SUCCESS)return SQL_SYNTAX;

				//int s;
				//char *p=(curRes->res[d]);
				//memcpy(&s,(p+20*j),4);
				//j++;
				//char str[5];
	  			 // sprintf(str,"%d", s);
				//AfxMessageBox(str);

			}

			

		res = curRes;
		if(CloseScan(rm_fileScan)!= SUCCESS)return SQL_SYNTAX;
		if(RM_CloseFile(rm_data)!= SUCCESS)return SQL_SYNTAX;
		//free(rm_table);
		//free(rm_reccol);
		free(rm_data);

	}else{
		return SUCCESS;
	}


	return SUCCESS;



}
