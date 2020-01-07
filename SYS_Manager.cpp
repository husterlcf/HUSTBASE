#include "stdafx.h"
#include "EditArea.h"
#include "SYS_Manager.h"
#include "QU_Manager.h"
#include"IX_Manager.h"
#include <iostream>
#include<vector>
#include <string>
#include<set>
#include<stdio.h>
//#include"RC.h"
std::vector<RM_FileHandle *> vec;
using namespace std;

//std::string;
 
void ExecuteAndMessage(char * sql,CEditArea* editArea){//����ִ�е���������ڽ�������ʾִ�н�����˺������޸�
	//AfxMessageBox(sql);
	std::string s_sql = sql;
	RC rc;
	if(s_sql.find("select") == 0){
		if(s_sql.find("SYSTABLES")<std::string::npos){
			RM_FileHandle *rm_table,*rm_reccol,*rm_data;
			RM_FileScan FileScan;
			RM_Record rectab,reccol,recdata;
			rm_table=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));//��ϵͳ���ļ�
			rm_table->bOpen = false;
			if (RM_OpenFile("SYSTABLES",rm_table)!= SUCCESS)AfxMessageBox("SQL_SYNTAX");
			FileScan.bOpen=false;
			if(OpenScan(&FileScan,rm_table,0,NULL)!=SUCCESS)AfxMessageBox("SQL_SYNTAX");
			int s;

			char ** fields = new char *[2];//���ֶ�����
			char field[]="tablename";
			char field1[]="attrcount";
			fields[0] = new char[20];
			fields[1] = new char[20];
			memset(fields[0],'\0',20);
			memcpy(fields[0],field,20);
			memset(fields[1],'\0',20);
			memcpy(fields[1],field1,20);
			int i=0;
			char *** rows = new char**[100] ;//�����
			while(GetNextRec(&FileScan, &rectab)==SUCCESS){
				rows[i] = new char*[2];
				rows[i][0] = new char[20];
				rows[i][1] = new char[20];
				memcpy(rows[i][0],rectab.pData,21);

				int s;
				memcpy(&s,rectab.pData+21,sizeof(int));
				char str[5];
				sprintf(str,"%d",s);
				memcpy(rows[i][1],str,5);
				i++;
			}
			editArea->ShowSelResult(2,i,fields,rows);
			for(int i = 0;i<2;i++){
			delete[] fields[i];
			}
			delete[] fields;
			
			if(CloseScan(&FileScan)!= SUCCESS)AfxMessageBox("SQL_SYNTAX");
			if(RM_CloseFile(rm_table)!= SUCCESS)AfxMessageBox("SQL_SYNTAX");
			return ;
		}

		if(s_sql.find("SYSCOLUMNS")<std::string::npos){

			RM_FileHandle *rm_table,*rm_reccol,*rm_data;
			RM_FileScan FileScan;
			RM_Record rectab,reccol,recdata;
			rm_table=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));//��ϵͳ���ļ�
			rm_table->bOpen = false;
			if (RM_OpenFile("SYSCOLUMNS",rm_table)!= SUCCESS)AfxMessageBox("SQL_SYNTAX");
			FileScan.bOpen=false;
			if(OpenScan(&FileScan,rm_table,0,NULL)!=SUCCESS)AfxMessageBox("SQL_SYNTAX");
			int s;

			char ** fields = new char *[7];//���ֶ�����
			char **field=new char*[7];
			 field[0]="tablename";
			 field[1]="attrname";
			 field[2]="attrtype";
			 field[3]="attrlength";
			 field[4]="attroffset";
			 field[5]="ix_flag";
			 field[6]="indexname";
			for(int i=0;i<7;i++){
				fields[i] = new char[20];
				memset(fields[i],'\0',20);
				memcpy(fields[i],field[i],20);
			}
			int i=0;
			char *** rows = new char**[100] ;//�����
			while(GetNextRec(&FileScan, &rectab)==SUCCESS){
				rows[i] = new char*[7];
				rows[i][0] = new char[21];
				rows[i][1] = new char[21];
				rows[i][2] = new char[5];
				rows[i][3] = new char[5];
				rows[i][4] = new char[5];
				rows[i][5] = new char[1];
				rows[i][6] = new char[21];
				memset(rows[i][6],'\0',21);
			
				memcpy(rows[i][0],rectab.pData,21);
				memcpy(rows[i][1],rectab.pData+21,21);
				int type;
				memcpy(&type,rectab.pData+42,sizeof(int));
				char str[5];
				sprintf(str,"%d",type);
				memcpy(rows[i][2],str,5);
				int length;
				memcpy(&length,rectab.pData+46,sizeof(int));
				str[5];
				sprintf(str,"%d",length);
				memcpy(rows[i][3],str,5);
				int offset;
				memcpy(&offset,rectab.pData+50,sizeof(int));
				 str[5];
				sprintf(str,"%d",offset);
				memcpy(rows[i][4],str,5);
				memcpy(rows[i][5],rectab.pData+54,1);
				if(rows[i][5][0]=='1')
				memcpy(rows[i][6],rectab.pData+55,21);
				
				i++;
			}
			editArea->ShowSelResult(7,i,fields,rows);
			for(int i = 0;i<7;i++){
			delete[] fields[i];
			}
			delete[] fields;
			
			if(CloseScan(&FileScan)!= SUCCESS)AfxMessageBox("SQL_SYNTAX");
			if(RM_CloseFile(rm_table)!= SUCCESS)AfxMessageBox("SQL_SYNTAX");
			return ;
		}
		SelResult res;
		SelResult *pres=&res;
		Init_Result(&res);
		rc = Query(sql,&res);
		if(rc!=SUCCESS)return;
		sqlstr *sql_str = NULL;
	    RC rcc;
	    sql_str = get_sqlstr();
  	    rcc = parse(sql, sql_str);
		int nrelations=sql_str->sstr.sel.nRelations;
	if(nrelations==1){
		int col_num = res.col_num;//��
		int row_num = 0;//��
		
		SelResult *tmp=&res;
		while(tmp){//���нڵ�ļ�¼��֮��
			row_num+=tmp->row_num;
			tmp=tmp->next_res;
		}
		char ** fields = new char *[20];//���ֶ�����
		for(int i = 0;i<col_num;i++){
			fields[i] = new char[20];
			memset(fields[i],'\0',20);
			memcpy(fields[i],res.fields[i],20);
		}
	    tmp=&res;
		char *** rows = new char**[row_num];//�����
		for(int i = 0;i<row_num;i++){
			rows[i] = new char*[col_num];//���һ����¼
			char *p=(tmp->res[i]);
			for (int j = 0; j <col_num; j++)
			{
				/*rows[i][j] = new char[20];//һ����¼��һ���ֶ�
				memset(rows[i][j], '\0', 20);
				
				memcpy(rows[i][j],tmp->res[i][j],20);*/

				rows[i][j] = new char[20];//һ����¼��һ���ֶ�
				//memset(rows[i][j], '\0', 20);

				if(tmp->type[j]==ints){
				int a;
				//char **p1=tmp->res[i];
			
				
				memcpy(&a,(p+tmp->offset[j]),tmp->length[j]);
				char str[5];
				sprintf(str,"%d",a);

				//AfxMessageBox(str);
				memcpy(rows[i][j],str,5);}

				else if(tmp->type[j]==chars){


					memcpy(rows[i][j],p+tmp->offset[j],tmp->length[j]);
				}
				else{
				}

			}
			if (i==99)tmp=tmp->next_res;//ÿ������ڵ�����¼100����¼
		}
		editArea->ShowSelResult(col_num,row_num,fields,rows);
		for(int i = 0;i<col_num;i++){
			delete[] fields[i];
		}
		delete[] fields;
		}else {
			SelResult *tmps=&res;
		int col_num=0;
	    
        col_num = (tmps->col_num);
		int row_num = 0;//��
		
		SelResult *tmp=&res;
		while(tmp){//���нڵ�ļ�¼��֮��
			row_num+=tmp->row_num;
			tmp=tmp->next_res;
		}
		char ** fields = new char *[20];//���ֶ�����

		tmp=&res;
	
		for(int i=0;i<tmp->col_num;i++){
			fields[i] = new char[20];
			memset(fields[i],'\0',20);
			memcpy(fields[i],tmp->fields[i],20);
			}
	
		char *** rows = new char**[row_num];

			for(int i = 0;i<row_num;i++){
			rows[i] = new char*[col_num];//���һ����¼
			char *p=(tmp->res[i]);
			for (int j = 0; j <col_num; j++)
			{
				/*rows[i][j] = new char[20];//һ����¼��һ���ֶ�
				memset(rows[i][j], '\0', 20);
				
				memcpy(rows[i][j],tmp->res[i][j],20);*/

				rows[i][j] = new char[20];//һ����¼��һ���ֶ�
				//memset(rows[i][j], '\0', 20);

				if(tmp->type[j]==ints){
				int a;
				//char **p1=tmp->res[i];
			
				
				memcpy(&a,(p+tmp->offset[j]),tmp->length[j]);
				char str[5];
				sprintf(str,"%d",a);

				//AfxMessageBox(str);
				memcpy(rows[i][j],str,5);}

				else if(tmp->type[j]==chars){


					memcpy(rows[i][j],p+tmp->offset[j],tmp->length[j]);
				}
				else{
				}

			}
			if (i==99)tmp=tmp->next_res;//ÿ������ڵ�����¼100����¼
		}

		editArea->ShowSelResult(col_num,row_num,fields,rows);


		for(int i = 0;i<col_num;i++){
			delete[] fields[i];
		}
		delete[] fields;
		}
		
	
		

		/*for(int i = 0;i<row_num;i++){
			
			for (int j = 0; j <col_num; j++)
			{
			   delete []rows[i][j];

			}
			delete [] rows[i];
		}

		delete [] rows;*/
		




		//Destory_Result(&res);
		return;
	}


	 rc = execute(sql); 

	int row_num = 0;
	char**messages;
	switch(rc){
	case SUCCESS:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "�����ɹ�";
		editArea->ShowMessage(row_num,messages);
		delete[] messages;
		break;
	case SQL_SYNTAX:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "���﷨����";
		editArea->ShowMessage(row_num,messages);
		delete[] messages;
		break;
	
	default:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "����δʵ��";
		editArea->ShowMessage(row_num,messages);
	delete[] messages;
		break;
	}
}

RC execute(char * sql){


	sqlstr *sql_str = NULL;
	RC rc;
	sql_str = get_sqlstr();

	//AfxMessageBox("SSSSSSELE");
  	rc = parse(sql, sql_str);//ֻ�����ַ��ؽ��SUCCESS��SQL_SYNTAX
	//AfxMessageBox(sql_str->sstr.ins.relName);
	//AfxMessageBox(sql_str->sstr.ins.nValues);

	if (rc == SUCCESS)
	{
		int i = 0;
		switch (sql_str->flag)
		{
			//case 1:
			////�ж�SQL���Ϊselect���

			//break;

			case 2:
			//�ж�SQL���Ϊinsert���
				//AfxMessageBox("INSERT");
				rc=Insert(sql_str->sstr.ins.relName,sql_str->sstr.ins.nValues,sql_str->sstr.ins.values);
				return rc;
				break;

			case 3:	
			//�ж�SQL���Ϊupdate���
				rc=Update(sql_str->sstr.upd.relName,sql_str->sstr.upd.attrName,&sql_str->sstr.upd.value,sql_str->sstr.upd.nConditions,sql_str->sstr.upd.conditions);
				return rc;
			break;

			case 4:					
			//�ж�SQL���Ϊdelete���
				rc=Delete(sql_str->sstr.del.relName,sql_str->sstr.del.nConditions,sql_str->sstr.del.conditions);
				return rc;
			break;

			case 5:
			//�ж�SQL���ΪcreateTable
				rc=CreateTable(sql_str->sstr.cret.relName,sql_str->sstr.cret.attrCount,sql_str->sstr.cret.attributes);
				return rc;
			break;

			case 6:	
			//�ж�SQL���ΪdropTable���
				rc=DropTable(sql_str->sstr.drt.relName);
				return rc;
			break;

			case 7:
			//�ж�SQL���ΪcreateIndex���
				rc=CreateIndex(sql_str->sstr.crei.indexName,sql_str->sstr.crei.relName,sql_str->sstr.crei.attrName);
				return rc;
			break;
	
			case 8:	
			//�ж�SQL���ΪdropIndex���
				rc=DropIndex(sql_str->sstr.dri.indexName);
				return rc;
			break;
			
			case 9:
			//�ж�Ϊhelp��䣬���Ը���������ʾ
			break;
		
			case 10: 
			//�ж�Ϊexit��䣬�����ɴ˽����˳�����
			break;		
		}
	}else{
		AfxMessageBox(sql_str->sstr.errors);//���������sql���ʷ�����������Ϣ
		return rc;
	}
	return rc;

}

RC CreateDB(char *dbpath,char *dbname){  //��dbpath·�����洴��һ����Ϊdbname���ļ���  �ļ����а�������ϵͳ�ļ�  SYSTABLES��SYSCOLUMNS

	char finalpath[300];
	strcpy(finalpath,dbpath);
	strcat(finalpath,"\\");
	strcat(finalpath,dbname);
	if(CreateDirectory(finalpath,NULL)){
		SetCurrentDirectory(finalpath);
		if(RM_CreateFile("SYSTABLES",25)==SUCCESS&&RM_CreateFile("SYSCOLUMNS",76)==SUCCESS){
			
			//execute("")
			SetCurrentDirectory(dbpath);
			return SUCCESS;
		}
	}
	return SQL_SYNTAX;
}

RC DropDB(char *dbname){

	
	CFileFind find;
	char operatepath[300];
	strcpy(operatepath,dbname);
	strcat(operatepath,"\\*.*");
	int flag=0;
	BOOL isfinded=find.FindFile(operatepath);  //�����Ƿ���ڴ��ļ���
	while(isfinded){
		memset(operatepath,0,300);
		isfinded=find.FindNextFile();
		strcpy(operatepath,dbname);
		strcat(operatepath,"\\");
		strcat(operatepath,find.GetFileName().GetBuffer());//��ȡ�ļ����ƻ�������ָ��
		if(strcmp("SYSCOLUMNS",find.GetFileName())==0||strcmp(find.GetFileName(),"SYSTABLES")==0)
			flag++;
		DeleteFile(operatepath);

		

	}


	find.Close();
	if(flag!=2) return DB_NOT_EXIST;
	if(RemoveDirectory(dbname)){
		return SUCCESS;
	}
	else return SUCCESS;
	
	return SUCCESS;
}

RC OpenDB(char *dbname){
	if(SetCurrentDirectory(dbname))return SUCCESS;//����ָ��·��Ϊ����·��
       else return SQL_SYNTAX;
	//if (access(dbname,0)!=0){
			//AfxMessageBox("Oops�����˷����ݿ��ļ�");
			//return DB_NOT_EXIST;
		//}
	return SUCCESS;
}


RC CloseDB(){

	for(std::vector<RM_FileHandle *>::size_type i = 0; i < vec.size(); i++){
		if(vec[i] != NULL)
			RM_CloseFile(vec[i]);
	}
	vec.clear();
	return SUCCESS;

}


RC CreateTable(char *relName,int attrCount,AttrInfo *attributes){

	if(strlen(relName)>20) return TABLE_NAME_ILLEGAL;
	int i=access(relName,0);
	if(access(relName,0)==0)return TABLE_EXIST;
	RM_FileHandle *sys_table, *sys_column;
	int recordsize;
	sys_table = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	sys_column=(RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	sys_table->bOpen=false;
	sys_column->bOpen=false;
	if(RM_OpenFile("SYSTABLES", sys_table)!= SUCCESS)return SQL_SYNTAX;
	if(RM_OpenFile("SYSCOLUMNS", sys_column)!= SUCCESS)return SQL_SYNTAX;
	char *pData;
	pData=(char *)malloc(sizeof(table));
	
	memcpy(pData,relName,21);


	memcpy(pData+21,&attrCount,sizeof(int));
	RID *rid = (RID *)malloc(sizeof(RID));
	rid->bValid=false;
    RC rc=InsertRec (sys_table, pData, rid); 


	if(rc!=SUCCESS) return  SQL_SYNTAX;
	RC rc1=RM_CloseFile (sys_table);
	if(rc1!=SUCCESS) return  SQL_SYNTAX;
	free(sys_table);
	free(pData);
	free(rid);


	set<string> columnset;
	typedef set<string>::iterator IT;
	pair<IT,bool> result;


	AttrInfo *attrtmp=attributes;
	for(int i=0 ,offset=0;i<attrCount;i++){
		pData=(char*)malloc(sizeof(column));
		memcpy(pData, relName,21);
		memcpy(pData+21, attributes[i].attrName,21);
		string str(attributes[i].attrName);
		result=columnset.insert(str);
		if(!result.second) return TABLE_COLUMN_ERROR;
 		memcpy(pData+42, &( attributes[i].attrType),sizeof(int));
		memcpy(pData+42+sizeof(int),&(attributes[i].attrLength),sizeof(int));
		memcpy(pData+42+2*sizeof(int),&offset,sizeof(int));
		memcpy(pData+42+3*sizeof(int),"0",sizeof(char));
		rid = (RID *)malloc(sizeof(RID)); 
		rid->bValid = false;
		if (InsertRec(sys_column, pData, rid) != SUCCESS)return SQL_SYNTAX;
		free(pData);
		free(rid);
		offset +=attributes[i].attrLength;

	}
	if (RM_CloseFile(sys_column) != SUCCESS)return SQL_SYNTAX;
	free(sys_column);
	recordsize=0;//������Ӧ�ļ�¼�ļ�
	for (int i=0; i<attrCount;++i){
		recordsize +=attributes[i].attrLength;
	}
	RM_CreateFile(relName, recordsize);
	return SUCCESS;	


}
RC DropTable(char *relName){   //��Ҫɾ�����ļ�����Ҫɾ��ϵͳ���еļ�¼

	CFileFind find;
	char operatepath[300],operatepath1[200];
	GetCurrentDirectory(200,operatepath);
	GetCurrentDirectory(200,operatepath1);
	strcat(operatepath,"\\");
	strcat(operatepath,relName);
	strcat(operatepath,"*.*");
	BOOL  isfind=find.FindFile(operatepath);
	//AfxMessageBox(operatepath);
	if(!isfind){
		
		//AfxMessageBox("û�д��ļ�");
		return TABLE_NOT_EXIST;
	}
	CFile file;
	file.Remove((LPCTSTR)relName);//ɾ�����ļ�
	RM_FileHandle *sys_table, *sys_column;
	sys_table = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	sys_column=(RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	sys_table->bOpen=false;
	sys_column->bOpen=false;
	RM_FileScan  filescan;
	//AfxMessageBox("SSSSSSSSSSSSSSSS");
	SetCurrentDirectory(operatepath1);
	
	if(RM_OpenFile("SYSTABLES", sys_table)!= SUCCESS)return SQL_SYNTAX;
	//AfxMessageBox("SSSSSSSSSSSSSSSS1");
	if(RM_OpenFile("SYSCOLUMNS", sys_column)!= SUCCESS)return SQL_SYNTAX;
	//AfxMessageBox("SSSSSSSSSSSSSSSS2");

	filescan.bOpen=false;
	if(OpenScan(&filescan,sys_table,0,NULL)!= SUCCESS)return SQL_SYNTAX;
	//AfxMessageBox("SSSSSSSSSSSSSSSS3");
	RM_Record rectab,reccol;
	table tab;
	column col;
	while(GetNextRec(&filescan, &rectab)==SUCCESS){
		memcpy(tab.tablename,rectab.pData,21);
		//AfxMessageBox(tab.tablename);
		if (strcmp(relName,tab.tablename)==0){//����������ɾ������
			DeleteRec(sys_table,&(rectab.rid));
			break;
		}
	}
	char index[21];
	filescan.bOpen=false;
	if(OpenScan(&filescan,sys_column,0,NULL)!= SUCCESS)return SQL_SYNTAX;
	while(GetNextRec(&filescan, &reccol)==SUCCESS){
		memcpy(col.tablename,reccol.pData,21);
		if (strcmp(relName,col.tablename) == 0){
			memcpy(index,reccol.pData+43+3*sizeof(int),21);
			if((reccol.pData+42+3*sizeof(int))=="1")   file.Remove((LPCTSTR)index);	//ɾ�������ļ�
			DeleteRec(sys_column, &(reccol.rid));
		}

	}
	if(CloseScan(&filescan)!=SUCCESS)return SQL_SYNTAX;
	if (RM_CloseFile(sys_table)!=SUCCESS)return SQL_SYNTAX;
	free(sys_table);
	if (RM_CloseFile(sys_column)!=SUCCESS)return SQL_SYNTAX;
	free(sys_column);
	return SUCCESS;

}


RC CreateIndex(char *indexName,char *relName,char *attrName){

	RM_FileHandle *sys_column,*rm_data;    //ϵͳ�ļ����
	sys_column = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	sys_column->bOpen = false;

	IX_IndexHandle *rm_index;			   //�����ļ����
	RM_Record reccol;
	
	column col;


	if(RM_OpenFile("SYSCOLUMNS", sys_column)!=SUCCESS){
		AfxMessageBox("ϵͳ���ļ���ʧ��");
		return SQL_SYNTAX;
	}

	RM_FileScan filescan;	  
	filescan.bOpen = false;
	if (OpenScan(&filescan, sys_column, 0, NULL)!=SUCCESS){
		AfxMessageBox("ϵͳ���ļ�ɨ��ʧ��");
		return SQL_SYNTAX;
	}
	while(GetNextRec(&filescan, &reccol) == SUCCESS){
		memcpy(col.tablename,reccol.pData,21);
		memcpy(col.attrname,reccol.pData+21,21);
		memcpy(&col.attrtype,reccol.pData+42 ,4);
		memcpy(&col.attrlength,reccol.pData+46 ,4);
		memcpy(&col.attroffset,reccol.pData+50 ,4);
		memcpy(&col.ix_flag,reccol.pData+54 ,1);
		if(strcmp(relName,col.tablename)==0&&strcmp(attrName,col.attrname)==0){
			if((reccol.pData+42+3*sizeof(int))=="1")return SQL_SYNTAX;
			else{

				if(CloseScan(&filescan)!=SUCCESS)return SQL_SYNTAX;
				IXCreateIndex(indexName,col.attrtype,col.attrlength);
				memcpy(reccol.pData+42+3*sizeof(int),"1",1);
				memcpy(reccol.pData+43+3*sizeof(int),indexName,21);
				UpdateRec(sys_column,&reccol);
				


				rm_index = (IX_IndexHandle *)malloc(sizeof(IX_IndexHandle));//�������ļ�
	            rm_index->bOpen = false;
	            if(OpenIndex(indexName, rm_index)!=SUCCESS){      
		            AfxMessageBox("�����ļ���ʧ��");
		            return SQL_SYNTAX;
	            }

				rm_data = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	            rm_data->bOpen = false;
	            if (RM_OpenFile(relName, rm_data)!=SUCCESS){
		           AfxMessageBox("��¼�ļ���ʧ��");
		           return SQL_SYNTAX;
	            }

                filescan.bOpen = false;//�򿪱�ļ�¼�ļ�����ɨ��
	            if (OpenScan(&filescan, rm_data,0,NULL) != SUCCESS){
					AfxMessageBox("��¼�ļ�ɨ��ʧ��");
					return SQL_SYNTAX;
				}
	            while (GetNextRec(&filescan, &reccol) == SUCCESS){
		            char *data = (char *)malloc(col.attrlength);
		            memcpy(data, reccol.pData + col.attroffset, col.attrlength);


		            InsertEntry(rm_index, data, &(reccol.rid));
	            }
	            if(CloseScan(&filescan)!=SUCCESS)return SQL_SYNTAX;
				if(RM_CloseFile(rm_data)!=SUCCESS)return SQL_SYNTAX;
				if(CloseIndex(rm_index)!=SUCCESS)return SQL_SYNTAX;
				if(RM_CloseFile(sys_column)!=SUCCESS)return SQL_SYNTAX;
				break;


			}

		}


	}








	return SUCCESS;
}

RC DropIndex(char *indexName){

	CFile tmp;
	RM_FileHandle *sys_column;
	RM_FileScan filescan;
	RM_Record reccol;	
	char index[21];


	sys_column = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));//��ϵͳ���ļ�
	sys_column->bOpen = false;
	if (RM_OpenFile("SYSCOLUMNS", sys_column)!=SUCCESS){
		AfxMessageBox("ϵͳ���ļ���ʧ��");
		return SQL_SYNTAX;
	}


	filescan.bOpen = false;//��ϵͳ���ļ�����ɨ�裬�޸�ͬ�������ļ�¼�flag��0��
	if (OpenScan(&filescan, sys_column, 0, NULL)!=SUCCESS){
		AfxMessageBox("ϵͳ���ļ�ɨ��ʧ��");
		return SQL_SYNTAX;
	}


	while (GetNextRec(&filescan, &reccol) == SUCCESS){
		memcpy(index,reccol.pData+43+3*sizeof(int),21);
		if (strcmp(indexName,index)==0){
			if((reccol.pData+42+3*sizeof(int))=="0")return SQL_SYNTAX;//�����������򱨴�
			else{
				memcpy(reccol.pData+42+3*sizeof(int),"0",1);//����  ����λ��0��ɾ�������ļ�
			    if (UpdateRec(sys_column,&reccol)!=SUCCESS)return SQL_SYNTAX;
				tmp.Remove((LPCTSTR)indexName);//ɾ�������ļ�
			}
		}
	}

	if(CloseScan(&filescan)!=SUCCESS)return SQL_SYNTAX;

	if (RM_CloseFile(sys_column)!= SUCCESS)return SQL_SYNTAX;
	return SUCCESS;

	
}

RC Insert(char *relName,int nValues,Value * values){

	RM_FileHandle *sys_table,*sys_column,*rm_data;
	RM_Record rectab, reccol;
	int attrcount ;//���Ե�����
	char *value=(char *)malloc(200);//��ȡ���ݱ���Ϣ  ����Ҫ�����������Ϣ
	RID *rid;
	char index[21];
	memset(value,0,200);
	//AfxMessageBox("INSERT INTO");
	sys_table = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	sys_table->bOpen=false;
	sys_column = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	sys_column->bOpen=false;
	rm_data = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	rm_data->bOpen=false;
	if(RM_OpenFile("SYSTABLES", sys_table)!= SUCCESS){
		AfxMessageBox("ϵͳ���ļ���ʧ��");
		return SQL_SYNTAX;
	}
	if(RM_OpenFile("SYSCOLUMNS", sys_column)!= SUCCESS){
		AfxMessageBox("ϵͳ���ļ���ʧ��");
		return SQL_SYNTAX;
	}
	if(RM_OpenFile(relName, rm_data)!= SUCCESS){     //���ݱ�
		//AfxMessageBox("�����ļ���ʧ��");
		return TABLE_NOT_EXIST;
	}

	RM_FileScan filescan;
	filescan.bOpen=false;


	if(OpenScan(&filescan, sys_table,0,NULL)!= SUCCESS){
		AfxMessageBox("ɨ��ϵͳ���ļ���ʧ��");
		return SQL_SYNTAX;
	}
	int flag=0;
	while(GetNextRec(&filescan, &rectab)==SUCCESS){
		char str[21];
		memcpy(str,rectab.pData,21);
		if (strcmp(relName,str)==0){
			memcpy(&attrcount,rectab.pData+21,sizeof(int));
			flag=1;
			break;
		}
	}

	if(flag==0) return TABLE_NOT_EXIST;

	if(CloseScan(&filescan)!=SUCCESS)return SQL_SYNTAX;
	//�ж����������Ƿ����
	if (attrcount != nValues){
		//AfxMessageBox("���Ը������ȣ�����ʧ�ܣ�");
		return FIELD_MISSING;
	}

	filescan.bOpen=false;
	if(OpenScan(&filescan, sys_column,0,NULL)!= SUCCESS){
		AfxMessageBox("ɨ��ϵͳ���ļ���ʧ��");
		return SQL_SYNTAX;
	}

	column *col1,*col2;
	col1=(column*)malloc(attrcount*sizeof(column));
	col2=col1;
	while(GetNextRec(&filescan, &reccol)==SUCCESS){
		char str[21];
		//AfxMessageBox(relName);
		memcpy(str,reccol.pData,21);
		//AfxMessageBox(str);

		if (strcmp(relName,str)==0){
			for(int i=0;i<nValues;i++,col2++){
				memcpy(col2->tablename, reccol.pData, 21);
				memcpy(col2->attrname, reccol.pData + 21, 21);
				memcpy(&(col2->attrtype), reccol.pData + 42, sizeof(AttrType));
				memcpy(&(col2->attrlength), reccol.pData + 42 + sizeof(AttrType), sizeof(int));
				memcpy(&(col2->attroffset), reccol.pData + 42 + sizeof(int)+sizeof(AttrType), sizeof(int));
				if (GetNextRec(&filescan, &reccol)!= SUCCESS)break;
		}
			break;

		}
	}
	col2=col1;
	values = values + nValues -1;
	for (int i = 0; i < nValues; i++, values--,col2++){
		//char str[5];
	    //sprintf(str,"%d", col2->attroffset);
		//AfxMessageBox(str);
		
	    //sprintf(str,"%d", col2->attrlength);
		//AfxMessageBox(str);

		if(values->type!=col2->attrtype) return  FIELD_TYPE_MISMATCH;


		memcpy(value + col2->attroffset, values->data, col2->attrlength);
	}

	rid = (RID*)malloc(sizeof(RID));
	rid->bValid = false;
	InsertRec(rm_data, value, rid);
	//�ر�ϵͳ���ļ�ɨ��
	if(CloseScan(&filescan)!=SUCCESS)return SQL_SYNTAX;

	//��ϵͳ���ļ�ɨ��
	filescan.bOpen = false;
	if (OpenScan(&filescan, sys_column, 0, NULL)!= SUCCESS){
		AfxMessageBox("ϵͳ���ļ�ɨ��ʧ��");
		return SQL_SYNTAX;
	}


	char a;

	while (GetNextRec(&filescan, &reccol) == SUCCESS){
		if (strcmp(relName, reccol.pData) == 0){//�ҵ�����ΪrelName�ĵ�һ����¼�����ζ�ȡattrcount����¼
			//for (int i = 0; i < attrcount; i++){
				memcpy(&a,reccol.pData+42+3*sizeof(int),1);
				if(a=='1'){//ix_flagΪ1���������ϴ���������������µ�������
					IX_IndexHandle *rm_index;
					memcpy(index,reccol.pData+43+2*sizeof(int)+sizeof(AttrType),21);
					rm_index = (IX_IndexHandle *)malloc(sizeof(IX_IndexHandle));//�������ļ�
	                rm_index->bOpen = false;
	                if(OpenIndex(index, rm_index)!=SUCCESS){
		               AfxMessageBox("�����ļ���ʧ��");
		               return SQL_SYNTAX;
	                }


					int length,offset;
					//length=(char *)malloc(sizeof(int));
					//offset=(char *)malloc(sizeof(int));
					memcpy(&length,reccol.pData+42+sizeof(int),sizeof(int));
		            memcpy(&offset,reccol.pData+42+2*sizeof(int),sizeof(int));
					char *data = (char *)malloc(length);
		            memcpy(data,value+offset,length);


		            InsertEntry(rm_index,(void*)data,rid);
					if(CloseIndex(rm_index)!=SUCCESS)return SQL_SYNTAX;
					free(rm_index);
					break;
				}  
				//GetNextRec(&filescan, &reccol);
			//}
		}
	}

	free(value);
	free(rid);
	free(col1);
	if(CloseScan(&filescan)!=SUCCESS)return SQL_SYNTAX;
	//�ر��ļ�
	if (RM_CloseFile(rm_data)!= SUCCESS){
		AfxMessageBox("��¼�ļ��ر�ʧ��");
		return SQL_SYNTAX;
	}
	free(rm_data);
	if (RM_CloseFile(sys_table)!= SUCCESS){
		AfxMessageBox("ϵͳ���ļ��ر�ʧ��");
		return SQL_SYNTAX;
	}
	free(sys_table);
	if (RM_CloseFile(sys_column)!= SUCCESS){
		AfxMessageBox("ϵͳ���ļ��ر�ʧ��");
		return SQL_SYNTAX;
	}
	free(sys_column);
	return SUCCESS;
	


}


RC Delete(char *relName,int nConditions,Condition *conditions){    //ֻ�Ե������ɾ�������漰����ı�ɾ�������ֻ�����һ����


	CFile tmp;
	RM_FileHandle *rm_data,*sys_table,*sys_column;
	RM_FileScan filescan;
	RM_Record recdata,rectab,reccol;
	column *Column, *ctmp,*ctmpleft,*ctmpright;
	Condition *contmp;
	Condition *cond =conditions;

	int i,torf;//�Ƿ����ɾ������
	int attrcount;//��������
	int intleft,intright; 
	char *charleft,*charright;
	float floatleft,floatright;//���Ե�ֵ
	AttrType attrtype;
	char index[21],attr[21];

	//�򿪼�¼,ϵͳ��ϵͳ���ļ�
	rm_data = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	rm_data->bOpen = false;
	if (RM_OpenFile(relName, rm_data)!= SUCCESS)return TABLE_NOT_EXIST;

	sys_table = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	sys_table->bOpen = false;
	if (RM_OpenFile("SYSTABLES", sys_table)!= SUCCESS)return SQL_SYNTAX;

	sys_column = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	sys_column->bOpen = false;
	if (RM_OpenFile("SYSCOLUMNS", sys_column)!= SUCCESS)return SQL_SYNTAX;

	//��ϵͳ���ļ�ɨ��,Ȼ���ҵ���Ӧ�����������,��ֵ��attrcount
	filescan.bOpen = false;
	if (OpenScan(&filescan, sys_table, 0, NULL)!= SUCCESS)return SQL_SYNTAX;

	int flag=0;
	while (GetNextRec(&filescan, &rectab) == SUCCESS){
		if (strcmp(relName, rectab.pData) == 0){
			flag=1;
			memcpy(&attrcount, rectab.pData + 21 , sizeof(int));
			break;
		}
	}
	if(flag==0) return TABLE_NOT_EXIST;
	//�ر�ϵͳ���ļ�ɨ��
	if(CloseScan(&filescan)!=SUCCESS)return SQL_SYNTAX;

	//��ϵͳ���ļ�ɨ��
	filescan.bOpen = false;
	if (OpenScan(&filescan, sys_column, 0, NULL)!= SUCCESS)return SQL_SYNTAX;

	//��ȡ������Ϣ�����������Column  ,ctmp  ��
	Column = (column *)malloc(attrcount*sizeof(column));
	ctmp = Column;
	column *col=Column;
	while (GetNextRec(&filescan, &reccol) == SUCCESS){
		if (strcmp(relName, reccol.pData) == 0){          //��ȡ����ΪrelName��������Ϣ
			for (int i = 0; i < attrcount; i++,ctmp++){
				memcpy(ctmp->tablename, reccol.pData, 21);
				memcpy(ctmp->attrname, reccol.pData + 21, 21);
				memcpy(&(ctmp->attrtype), reccol.pData + 42, sizeof(AttrType));
				memcpy(&(ctmp->attrlength), reccol.pData + 42 + sizeof(AttrType), sizeof(int));
				memcpy(&(ctmp->attroffset), reccol.pData + 42 + sizeof(int)+sizeof(AttrType), sizeof(int));				
				if(GetNextRec(&filescan, &reccol)!= SUCCESS)break;
			}
			break;
		}
	}


	set<string> columnset;
	typedef set<string>::iterator IT;
	pair<IT,bool> result,result1;
	
	for(int i=0;i<attrcount;i++,col++){
		string str(col->attrname);
		columnset.insert(str);  //����ɹ�����1
	}
	for(int i=0;i<nConditions;i++,cond++){
		if(cond->bLhsIsAttr==1&&cond->bRhsIsAttr==0){
			string str(cond->lhsAttr.attrName);
			result=columnset.insert(str);
			if(result.second==1)  return TABLE_COLUMN_ERROR;
		}else if(cond->bLhsIsAttr==0&&cond->bRhsIsAttr==1){
			string str(cond->rhsAttr.attrName);
			result=columnset.insert(str);
			if(result.second==1)  return TABLE_COLUMN_ERROR;
		}
		else if(cond->bLhsIsAttr==1&&cond->bRhsIsAttr==1){
			string str(cond->rhsAttr.attrName);
			string str1(cond->lhsAttr.attrName);
			result=columnset.insert(str);
			result1=columnset.insert(str1);
			if(result.second==1||result1.second==1) return TABLE_COLUMN_ERROR;
			
		}
		else;
	}
	

	//�ر�ϵͳ���ļ�ɨ��
	if(CloseScan(&filescan)!=SUCCESS)return SQL_SYNTAX;


	//�򿪼�¼�ļ�ɨ��
	filescan.bOpen = false;
	if (OpenScan(&filescan, rm_data, 0, NULL)!= SUCCESS)return SQL_SYNTAX;

	while (GetNextRec(&filescan, &recdata) == SUCCESS){	//ȡÿһ����¼�����������ж�

		for (i = 0, torf = 1,contmp = conditions;i < nConditions; i++, contmp++){//conditions������һ�ж�
			ctmpleft = ctmpright = Column;//ÿ��ѭ����Ҫ����������ϵͳ���ļ����ҵ�����������Ӧ������


			//��������ֵ
			if (contmp->bLhsIsAttr == 1 && contmp->bRhsIsAttr == 0){
				for (int j = 0; j < attrcount; j++){              //�Ա��е��������Խ��бȽ��ж�
					if (contmp->lhsAttr.relName == NULL){
						contmp->lhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->lhsAttr.relName, relName);					}  //Ĭ�ϱ���ΪrelName;
					if ((strcmp(ctmpleft->tablename, contmp->lhsAttr.relName) == 0)
						&& (strcmp(ctmpleft->attrname, contmp->lhsAttr.attrName) == 0)){     //�ҵ����ԣ�����ѭ��
						break;
					}
					ctmpleft++;
				}
				
				switch (ctmpleft->attrtype){
					case ints:
						attrtype = ints;
						memcpy(&intleft, recdata.pData + ctmpleft->attroffset, sizeof(int));
						memcpy(&intright, contmp->rhsValue.data, sizeof(int));
						break;
					case chars:
						attrtype = chars;
						charleft = (char *)malloc(ctmpleft->attrlength);
						memcpy(charleft, recdata.pData + ctmpleft->attroffset, ctmpleft->attrlength);
						charright = (char *)malloc(ctmpleft->attrlength);
						memcpy(charright, contmp->rhsValue.data, ctmpleft->attrlength);
						break;
					case floats:
						attrtype = floats;
						memcpy(&floatleft, recdata.pData + ctmpleft->attroffset, sizeof(float));
						memcpy(&floatright, contmp->rhsValue.data, sizeof(float));
						break;
				}
			}
			//��������ֵ
			if (contmp->bLhsIsAttr == 0 && contmp->bRhsIsAttr == 1){
				for (int j = 0; j < attrcount; j++){        //�Ա��е��������Խ��бȽ��ж�
					if (contmp->rhsAttr.relName == NULL){
						contmp->rhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->rhsAttr.relName, relName);             //Ĭ�ϱ���ΪrelName;
					}
					if ((strcmp(ctmpright->tablename, contmp->rhsAttr.relName) == 0)
						&& (strcmp(ctmpright->attrname, contmp->rhsAttr.attrName) == 0)){       //�ҵ����ԣ�����ѭ��
						break;
					}
					ctmpright++;
				}
				
				switch (ctmpright->attrtype){
				case ints:
					attrtype = ints;
					memcpy(&intleft, contmp->lhsValue.data, sizeof(int));
					memcpy(&intright, recdata.pData + ctmpright->attroffset, sizeof(int));
					break;
				case chars:
					attrtype = chars;
					charleft = (char *)malloc(ctmpright->attrlength);
					memcpy(charleft, contmp->lhsValue.data, ctmpright->attrlength);
					charright = (char *)malloc(ctmpright->attrlength);
					memcpy(charright, recdata.pData + ctmpright->attroffset, ctmpright->attrlength);
					break;
				case floats:
					attrtype = floats;
					memcpy(&floatleft, contmp->lhsValue.data, sizeof(float));
					memcpy(&floatright, recdata.pData + ctmpright->attroffset, sizeof(float));
					break;
				}
			}
			//���Ҿ�����
			else  if (contmp->bLhsIsAttr == 1 && contmp->bRhsIsAttr == 1){
				for (int j = 0; j < attrcount; j++){                   //�Ա��е��������Խ��бȽ��ж�
					if (contmp->lhsAttr.relName == NULL){
						contmp->lhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->lhsAttr.relName, relName);            //Ĭ�ϱ���ΪrelName;
					}
					if ((strcmp(ctmpleft->tablename, contmp->lhsAttr.relName) == 0)
						&& (strcmp(ctmpleft->attrname, contmp->lhsAttr.attrName) == 0)){//�ҵ����ԣ�����ѭ��
						break;
					}
					ctmpleft++;
				}

				
				for (int j = 0; j < attrcount; j++){          //�Ա��е��������Խ��бȽ��ж�
					if (contmp->rhsAttr.relName == NULL){
						contmp->rhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->rhsAttr.relName, relName);//Ĭ�ϱ���ΪrelName;
					}
					if ((strcmp(ctmpright->tablename, contmp->rhsAttr.relName) == 0)
						&& (strcmp(ctmpright->attrname, contmp->rhsAttr.attrName) == 0)){//�ҵ����ԣ�����ѭ��
						break;
					}
					ctmpright++;
				}
				
				switch (ctmpright->attrtype){
					case ints:
						attrtype = ints;
						memcpy(&intleft, recdata.pData + ctmpleft->attroffset, sizeof(int));
						memcpy(&intright, recdata.pData + ctmpright->attroffset, sizeof(int));
						break;
					case chars:
						attrtype = chars;
						charleft = (char *)malloc(ctmpright->attrlength);
						memcpy(charleft, recdata.pData + ctmpleft->attroffset, ctmpright->attrlength);
						charright = (char *)malloc(ctmpright->attrlength);
						memcpy(charright, recdata.pData + ctmpright->attroffset, ctmpright->attrlength);
						break;
					case floats:
						attrtype = floats;
						memcpy(&floatleft, recdata.pData + ctmpleft->attroffset, sizeof(float));
						memcpy(&floatright, recdata.pData + ctmpright->attroffset, sizeof(float));
						break;
				}
			}
			if (attrtype == ints){
				if ((intleft == intright && contmp->op == EQual) ||
					(intleft > intright && contmp->op == GreatT) ||
					(intleft >= intright && contmp->op == GEqual) ||
					(intleft < intright && contmp->op == LessT) ||
					(intleft <= intright && contmp->op == LEqual) ||
					(intleft != intright && contmp->op == NEqual))
					torf &= 1;
				else
					torf &= 0;
			}
			else if (attrtype == chars){
				if ((strcmp(charleft, charright) == 0 && contmp->op == EQual) ||
					(strcmp(charleft, charright) > 0 && contmp->op == GreatT) ||
					((strcmp(charleft, charright) > 0 || strcmp(charleft, charright) == 0) && contmp->op == GEqual) ||
					(strcmp(charleft, charright) < 0 && contmp->op == LessT) ||
					((strcmp(charleft, charright) < 0 || strcmp(charleft, charright) == 0) && contmp->op == LEqual) ||
					(strcmp(charleft, charright) != 0 && contmp->op == NEqual))
					torf &= 1;
				else
					torf &= 0;
				free(charleft);
				free(charright);
			}
			else if (attrtype == floats){
				if ((floatleft == floatright && contmp->op == EQual) ||
					(floatleft > floatright && contmp->op == GreatT) ||
					(floatleft >= floatright && contmp->op == GEqual) ||
					(floatleft < floatright && contmp->op == LessT) ||
					(floatleft <= floatright && contmp->op == LEqual) ||
					(floatleft != floatright && contmp->op == NEqual))
					torf &= 1;
				else
					torf &= 0;
			}
			else
				torf &= 0;
		}

		if (torf == 1){
			DeleteRec(rm_data, &(recdata.rid));
		}
	}
	free(Column);


	//�ر��ļ�
	if(CloseScan(&filescan)!=SUCCESS)return SQL_SYNTAX;
	if (RM_CloseFile(sys_table)!= SUCCESS)return SQL_SYNTAX;
	free(sys_table);
	if (RM_CloseFile(sys_column)!= SUCCESS)return SQL_SYNTAX;
	free(sys_column);
	if (RM_CloseFile(rm_data)!= SUCCESS)return SQL_SYNTAX;
	free(rm_data);
	return SUCCESS;

	 

	return SUCCESS;
}





RC Update(char *relName,char *attrName,Value *value,int nConditions,Condition *conditions){   //���е��и���

	if(access(relName ,0)!=0) return TABLE_NOT_EXIST;

	RM_FileHandle *rm_data, *rm_table, *rm_column;
	RM_FileScan FileScan;
	RM_Record recdata, rectab, reccol;



	column *Column, *ctmp,*cupdate,*ctmpleft,*ctmpright;
	Condition *contmp;
	Condition *cond = conditions;
	int i, torf;//�Ƿ���ϸ�������
	int attrcount;//��������

	int flag;

	int intleft,intright;
	char *charleft,*charright;
	float floatleft,floatright;//���Ե�ֵ
	AttrType attrtype;


	//�򿪼�¼,ϵͳ��ϵͳ���ļ�
	rm_data = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	rm_data->bOpen = false;
	if (RM_OpenFile(relName, rm_data)!= SUCCESS)return SQL_SYNTAX;
	rm_table = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	rm_table->bOpen = false;
	if (RM_OpenFile("SYSTABLES", rm_table)!= SUCCESS)return SQL_SYNTAX;
	rm_column = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	rm_column->bOpen = false;
	if (RM_OpenFile("SYSCOLUMNS", rm_column)!= SUCCESS)return SQL_SYNTAX;
	//��ϵͳ���ļ�ɨ��
	FileScan.bOpen = false;
	if (OpenScan(&FileScan, rm_table, 0, NULL)!= SUCCESS)return SQL_SYNTAX;
	
	while (GetNextRec(&FileScan, &rectab) == SUCCESS){
		if (strcmp(relName, rectab.pData) == 0){
			memcpy(&attrcount, rectab.pData + 21, sizeof(int));//�ñ�����Ե�����
			break;
		}
	}
	//�ر�ϵͳ���ļ�ɨ��
	if(CloseScan(&FileScan)!=SUCCESS)return SQL_SYNTAX;


	//��ϵͳ���ļ�ɨ��
	FileScan.bOpen = false;
	if (OpenScan(&FileScan, rm_column, 0, NULL)!= SUCCESS)return SQL_SYNTAX;

	
	Column = (column *)malloc(attrcount*sizeof(column));//�����е���Ϣ
	column *col=Column;
	cupdate = (column *)malloc(sizeof(column));//��������Ӧ��������
	ctmp = Column;


	while (GetNextRec(&FileScan, &reccol) == SUCCESS){
		if (strcmp(relName, reccol.pData) == 0){//�ҵ�����ΪrelName�ĵ�һ����¼�����ζ�ȡattrcount����¼
			for (int i = 0; i < attrcount; i++,ctmp++){
				memcpy(ctmp->tablename, reccol.pData, 21);
				memcpy(ctmp->attrname, reccol.pData + 21, 21);
				memcpy(&(ctmp->attrtype), reccol.pData + 42, sizeof(AttrType));
				memcpy(&(ctmp->attrlength), reccol.pData + 42 + sizeof(AttrType), sizeof(int));
				memcpy(&(ctmp->attroffset), reccol.pData + 42 + sizeof(int)+sizeof(AttrType), sizeof(int));
				if ((strcmp(relName,ctmp->tablename) == 0) && (strcmp(attrName,ctmp->attrname) == 0)){
					cupdate = ctmp;//�ҵ�Ҫ���µ��е��е�������Ϣ
				}
				if (GetNextRec(&FileScan, &reccol)!= SUCCESS)break;
			}
			break;
		}
	}


	set<string> columnset;
	typedef set<string>::iterator IT;
	pair<IT,bool> result,result1 ,result2;
	
	for(int i=0;i<attrcount;i++,col++){
		string str(col->attrname);
		columnset.insert(str);  //����ɹ�����1
	}
	string st(attrName);
	result2=columnset.insert(st);
	if(result2.second==1)  return TABLE_COLUMN_ERROR;
	for(int i=0;i<nConditions;i++,cond++){
		if(cond->bLhsIsAttr==1&&cond->bRhsIsAttr==0){
			string str(cond->lhsAttr.attrName);
			result=columnset.insert(str);
			if(result.second==1)  return TABLE_COLUMN_ERROR;
		}else if(cond->bLhsIsAttr==0&&cond->bRhsIsAttr==1){
			string str(cond->rhsAttr.attrName);
			result=columnset.insert(str);
			if(result.second==1)  return TABLE_COLUMN_ERROR;
		}
		else if(cond->bLhsIsAttr==1&&cond->bRhsIsAttr==1){
			string str(cond->rhsAttr.attrName);
			string str1(cond->lhsAttr.attrName);
			result=columnset.insert(str);
			result1=columnset.insert(str1);
			if(result.second==1||result1.second==1) return TABLE_COLUMN_ERROR;
			
		}
		else;
	}
	//�ر�ϵͳ���ļ�ɨ��
	if(CloseScan(&FileScan)!=SUCCESS)return SQL_SYNTAX;



	//�򿪼�¼�ļ�ɨ��
	FileScan.bOpen = false;
	if (OpenScan(&FileScan, rm_data, 0, NULL)!= SUCCESS)return SQL_SYNTAX;
	//ѭ�����ұ���ΪrelName��Ӧ�����ݱ��еļ�¼,������¼��Ϣ������recdata��
	while (GetNextRec(&FileScan, &recdata) == SUCCESS){
		for (i = 0, torf = 1, contmp = conditions; i < nConditions; i++, contmp++){//conditions������һ�ж�
		                                                                           //�������ʱ��Ż���и��²���
			ctmpleft = ctmpright = Column;//ÿ��ѭ����Ҫ����������ϵͳ���ļ����ҵ�����������Ӧ������
			//��������ֵ
			if (contmp->bLhsIsAttr == 1 && contmp->bRhsIsAttr == 0){

				for (int j = 0; j < attrcount; j++){//attrcount��������һ�ж�
					if (contmp->lhsAttr.relName == NULL){//��������δָ������ʱ��Ĭ��ΪrelName
						contmp->lhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->lhsAttr.relName, relName);
					}
					if ((strcmp(ctmpleft->tablename, contmp->lhsAttr.relName) == 0)
						&& (strcmp(ctmpleft->attrname, contmp->lhsAttr.attrName) == 0)){//���ݱ����������ҵ���Ӧ����
						break;
					}
					ctmpleft++;
				}
				//��conditions��ĳһ�����������ж�
				if (ctmpleft->attrtype == ints){//�ж����Ե�����
					attrtype = ints;
					memcpy(&intleft, recdata.pData + ctmpleft->attroffset, sizeof(int));
					memcpy(&intright, contmp->rhsValue.data, sizeof(int));
				}
				else if (ctmpleft->attrtype == chars){
					attrtype = chars;
					charleft = (char *)malloc(ctmpleft->attrlength);
					memcpy(charleft, recdata.pData + ctmpleft->attroffset, ctmpleft->attrlength);
					charright = (char *)malloc(ctmpleft->attrlength);
					memcpy(charright, contmp->rhsValue.data, ctmpleft->attrlength);
				}
				else if (ctmpleft->attrtype == floats){ 
					attrtype = floats;
					memcpy(&floatleft, recdata.pData + ctmpleft->attroffset, sizeof(float));
					memcpy(&floatright, contmp->rhsValue.data, sizeof(float));
				}
				else
					torf &= 0;
			}
			//��������ֵ
			else  if (contmp->bLhsIsAttr == 0 && contmp->bRhsIsAttr == 1){
				for (int j = 0; j < attrcount; j++){//attrcount��������һ�ж�
					if (contmp->rhsAttr.relName == NULL){//��������δָ������ʱ��Ĭ��ΪrelName
						contmp->rhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->rhsAttr.relName, relName);
					}
					if ((strcmp(ctmpright->tablename, contmp->rhsAttr.relName) == 0)
						&& (strcmp(ctmpright->attrname, contmp->rhsAttr.attrName) == 0)){//���ݱ����������ҵ���Ӧ����
						break;
					}
					ctmpright++ ;
				}
				//��conditions��ĳһ�����������ж�
				if (ctmpright->attrtype == ints){//�ж����Ե�����
					attrtype = ints;
					memcpy(&intleft, contmp->lhsValue.data, sizeof(int));
					memcpy(&intright, recdata.pData + ctmpright->attroffset, sizeof(int));
				}
				else if (ctmpright->attrtype == chars){
					attrtype = chars;
					charleft = (char *)malloc(ctmpright->attrlength);
					memcpy(charleft, contmp->lhsValue.data, ctmpright->attrlength);
					charright = (char *)malloc(ctmpright->attrlength);
					memcpy(charright, recdata.pData + ctmpright->attroffset, ctmpright->attrlength);
				}
				else if (ctmpright->attrtype == floats){
					attrtype = floats;
					memcpy(&floatleft, contmp->lhsValue.data, sizeof(float));
					memcpy(&floatright, recdata.pData + ctmpright->attroffset, sizeof(float));
				}
				else
					torf &= 0;
			}
			//���Ҿ�����
			else  if (contmp->bLhsIsAttr == 1 && contmp->bRhsIsAttr == 1){
				for (int j = 0; j < attrcount; j++){//attrcount��������һ�ж�
					if (contmp->lhsAttr.relName == NULL){//��������δָ������ʱ��Ĭ��ΪrelName
						contmp->lhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->lhsAttr.relName, relName);
					}
					if ((strcmp(ctmpleft->tablename, contmp->lhsAttr.relName) == 0)
						&& (strcmp(ctmpleft->attrname, contmp->lhsAttr.attrName) == 0)){//���ݱ����������ҵ���Ӧ����
						break;
					}
					ctmpleft++;
				}
				for (int j = 0; j < attrcount; j++){//attrcount��������һ�ж�
					if (contmp->rhsAttr.relName == NULL){//��������δָ������ʱ��Ĭ��ΪrelName
						contmp->rhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->rhsAttr.relName, relName);
					}
					if ((strcmp(ctmpright->tablename, contmp->rhsAttr.relName) == 0)
						&& (strcmp(ctmpright->attrname, contmp->rhsAttr.attrName) == 0)){//���ݱ����������ҵ���Ӧ����
						break;
					}
					ctmpright++;
				}
				//��conditions��ĳһ�����������ж�
				if (ctmpright->attrtype == ints && ctmpleft->attrtype == ints){//�ж����Ե�����
					attrtype = ints;
					memcpy(&intleft, recdata.pData + ctmpleft->attroffset, sizeof(int));
					memcpy(&intright, recdata.pData + ctmpright->attroffset, sizeof(int));
				}
				else if (ctmpright->attrtype == chars &&ctmpleft->attrtype == chars){
					attrtype = chars;
					charleft = (char *)malloc(ctmpright->attrlength);
					memcpy(charleft, recdata.pData + ctmpleft->attroffset, ctmpright->attrlength);
					charright = (char *)malloc(ctmpright->attrlength);
					memcpy(charright, recdata.pData + ctmpright->attroffset, ctmpright->attrlength);
				}
				else if (ctmpright->attrtype == floats &&ctmpleft->attrtype == floats){
					attrtype = floats;
					memcpy(&floatleft, recdata.pData + ctmpleft->attroffset, sizeof(float));
					memcpy(&floatright, recdata.pData + ctmpright->attroffset, sizeof(float));
				}
				else
					torf &= 0;
			}

			if (attrtype == ints){
				if ((intleft == intright && contmp->op == EQual) ||
					(intleft > intright && contmp->op == GreatT) ||
					(intleft >= intright && contmp->op == GEqual) ||
					(intleft < intright && contmp->op == LessT) ||
					(intleft <= intright && contmp->op == LEqual) ||
					(intleft != intright && contmp->op == NEqual))
					torf &= 1;
				else
					torf &= 0;
			}
			else if (attrtype == chars){
				if ((strcmp(charleft, charright) == 0 && contmp->op == EQual) ||
					(strcmp(charleft, charright) > 0 && contmp->op == GreatT) ||
					((strcmp(charleft, charright) > 0 || strcmp(charleft, charright) == 0) && contmp->op == GEqual) ||
					(strcmp(charleft, charright) < 0 && contmp->op == LessT) ||
					((strcmp(charleft, charright) < 0 || strcmp(charleft, charright) == 0) && contmp->op == LEqual) ||
					(strcmp(charleft, charright) != 0 && contmp->op == NEqual))
					torf &= 1;
				else
					torf &= 0;
				free(charleft);
				free(charright);
			}
			else if (attrtype == floats){
				if ((floatleft == floatright && contmp->op == EQual) ||
					(floatleft > floatright && contmp->op == GreatT) ||
					(floatleft >= floatright && contmp->op == GEqual) ||
					(floatleft < floatright && contmp->op == LessT) ||
					(floatleft <= floatright && contmp->op == LEqual) ||
					(floatleft != floatright && contmp->op == NEqual))
					torf &= 1;
				else
					torf &= 0;
			}
			else
				torf &= 0;
		}
		if (torf == 1){
			memcpy(recdata.pData + cupdate->attroffset,value->data,cupdate->attrlength);
			UpdateRec(rm_data, &recdata);
		}
	}
	free(Column);

	if(CloseScan(&FileScan)!=SUCCESS)return SQL_SYNTAX;
	//�ر��ļ�
	if (RM_CloseFile(rm_table)!= SUCCESS)return SQL_SYNTAX;
	free(rm_table);
    if (RM_CloseFile(rm_column)!= SUCCESS)return SQL_SYNTAX;
	free(rm_column);
	if (RM_CloseFile(rm_data)!= SUCCESS)return SQL_SYNTAX;
	free(rm_data);
	return SUCCESS;	


}


bool CanButtonClick(){//��Ҫ����ʵ��
	//�����ǰ�����ݿ��Ѿ���
	return true;
	//�����ǰû�����ݿ��
	//return false;
}
