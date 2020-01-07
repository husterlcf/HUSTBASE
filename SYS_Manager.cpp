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
 
void ExecuteAndMessage(char * sql,CEditArea* editArea){//根据执行的语句类型在界面上显示执行结果。此函数需修改
	//AfxMessageBox(sql);
	std::string s_sql = sql;
	RC rc;
	if(s_sql.find("select") == 0){
		if(s_sql.find("SYSTABLES")<std::string::npos){
			RM_FileHandle *rm_table,*rm_reccol,*rm_data;
			RM_FileScan FileScan;
			RM_Record rectab,reccol,recdata;
			rm_table=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));//打开系统表文件
			rm_table->bOpen = false;
			if (RM_OpenFile("SYSTABLES",rm_table)!= SUCCESS)AfxMessageBox("SQL_SYNTAX");
			FileScan.bOpen=false;
			if(OpenScan(&FileScan,rm_table,0,NULL)!=SUCCESS)AfxMessageBox("SQL_SYNTAX");
			int s;

			char ** fields = new char *[2];//各字段名称
			char field[]="tablename";
			char field1[]="attrcount";
			fields[0] = new char[20];
			fields[1] = new char[20];
			memset(fields[0],'\0',20);
			memcpy(fields[0],field,20);
			memset(fields[1],'\0',20);
			memcpy(fields[1],field1,20);
			int i=0;
			char *** rows = new char**[100] ;//结果集
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
			rm_table=(RM_FileHandle*)malloc(sizeof(RM_FileHandle));//打开系统表文件
			rm_table->bOpen = false;
			if (RM_OpenFile("SYSCOLUMNS",rm_table)!= SUCCESS)AfxMessageBox("SQL_SYNTAX");
			FileScan.bOpen=false;
			if(OpenScan(&FileScan,rm_table,0,NULL)!=SUCCESS)AfxMessageBox("SQL_SYNTAX");
			int s;

			char ** fields = new char *[7];//各字段名称
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
			char *** rows = new char**[100] ;//结果集
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
		int col_num = res.col_num;//列
		int row_num = 0;//行
		
		SelResult *tmp=&res;
		while(tmp){//所有节点的记录数之和
			row_num+=tmp->row_num;
			tmp=tmp->next_res;
		}
		char ** fields = new char *[20];//各字段名称
		for(int i = 0;i<col_num;i++){
			fields[i] = new char[20];
			memset(fields[i],'\0',20);
			memcpy(fields[i],res.fields[i],20);
		}
	    tmp=&res;
		char *** rows = new char**[row_num];//结果集
		for(int i = 0;i<row_num;i++){
			rows[i] = new char*[col_num];//存放一条记录
			char *p=(tmp->res[i]);
			for (int j = 0; j <col_num; j++)
			{
				/*rows[i][j] = new char[20];//一条记录的一个字段
				memset(rows[i][j], '\0', 20);
				
				memcpy(rows[i][j],tmp->res[i][j],20);*/

				rows[i][j] = new char[20];//一条记录的一个字段
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
			if (i==99)tmp=tmp->next_res;//每个链表节点最多记录100条记录
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
		int row_num = 0;//行
		
		SelResult *tmp=&res;
		while(tmp){//所有节点的记录数之和
			row_num+=tmp->row_num;
			tmp=tmp->next_res;
		}
		char ** fields = new char *[20];//各字段名称

		tmp=&res;
	
		for(int i=0;i<tmp->col_num;i++){
			fields[i] = new char[20];
			memset(fields[i],'\0',20);
			memcpy(fields[i],tmp->fields[i],20);
			}
	
		char *** rows = new char**[row_num];

			for(int i = 0;i<row_num;i++){
			rows[i] = new char*[col_num];//存放一条记录
			char *p=(tmp->res[i]);
			for (int j = 0; j <col_num; j++)
			{
				/*rows[i][j] = new char[20];//一条记录的一个字段
				memset(rows[i][j], '\0', 20);
				
				memcpy(rows[i][j],tmp->res[i][j],20);*/

				rows[i][j] = new char[20];//一条记录的一个字段
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
			if (i==99)tmp=tmp->next_res;//每个链表节点最多记录100条记录
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
		messages[0] = "操作成功";
		editArea->ShowMessage(row_num,messages);
		delete[] messages;
		break;
	case SQL_SYNTAX:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "有语法错误";
		editArea->ShowMessage(row_num,messages);
		delete[] messages;
		break;
	
	default:
		row_num = 1;
		messages = new char*[row_num];
		messages[0] = "功能未实现";
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
  	rc = parse(sql, sql_str);//只有两种返回结果SUCCESS和SQL_SYNTAX
	//AfxMessageBox(sql_str->sstr.ins.relName);
	//AfxMessageBox(sql_str->sstr.ins.nValues);

	if (rc == SUCCESS)
	{
		int i = 0;
		switch (sql_str->flag)
		{
			//case 1:
			////判断SQL语句为select语句

			//break;

			case 2:
			//判断SQL语句为insert语句
				//AfxMessageBox("INSERT");
				rc=Insert(sql_str->sstr.ins.relName,sql_str->sstr.ins.nValues,sql_str->sstr.ins.values);
				return rc;
				break;

			case 3:	
			//判断SQL语句为update语句
				rc=Update(sql_str->sstr.upd.relName,sql_str->sstr.upd.attrName,&sql_str->sstr.upd.value,sql_str->sstr.upd.nConditions,sql_str->sstr.upd.conditions);
				return rc;
			break;

			case 4:					
			//判断SQL语句为delete语句
				rc=Delete(sql_str->sstr.del.relName,sql_str->sstr.del.nConditions,sql_str->sstr.del.conditions);
				return rc;
			break;

			case 5:
			//判断SQL语句为createTable
				rc=CreateTable(sql_str->sstr.cret.relName,sql_str->sstr.cret.attrCount,sql_str->sstr.cret.attributes);
				return rc;
			break;

			case 6:	
			//判断SQL语句为dropTable语句
				rc=DropTable(sql_str->sstr.drt.relName);
				return rc;
			break;

			case 7:
			//判断SQL语句为createIndex语句
				rc=CreateIndex(sql_str->sstr.crei.indexName,sql_str->sstr.crei.relName,sql_str->sstr.crei.attrName);
				return rc;
			break;
	
			case 8:	
			//判断SQL语句为dropIndex语句
				rc=DropIndex(sql_str->sstr.dri.indexName);
				return rc;
			break;
			
			case 9:
			//判断为help语句，可以给出帮助提示
			break;
		
			case 10: 
			//判断为exit语句，可以由此进行退出操作
			break;		
		}
	}else{
		AfxMessageBox(sql_str->sstr.errors);//弹出警告框，sql语句词法解析错误信息
		return rc;
	}
	return rc;

}

RC CreateDB(char *dbpath,char *dbname){  //在dbpath路径下面创建一个名为dbname的文件夹  文件夹中包含两个系统文件  SYSTABLES和SYSCOLUMNS

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
	BOOL isfinded=find.FindFile(operatepath);  //查找是否存在此文件夹
	while(isfinded){
		memset(operatepath,0,300);
		isfinded=find.FindNextFile();
		strcpy(operatepath,dbname);
		strcat(operatepath,"\\");
		strcat(operatepath,find.GetFileName().GetBuffer());//获取文件名称缓冲区的指针
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
	if(SetCurrentDirectory(dbname))return SUCCESS;//设置指定路径为工作路径
       else return SQL_SYNTAX;
	//if (access(dbname,0)!=0){
			//AfxMessageBox("Oops！打开了非数据库文件");
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
	recordsize=0;//建立对应的记录文件
	for (int i=0; i<attrCount;++i){
		recordsize +=attributes[i].attrLength;
	}
	RM_CreateFile(relName, recordsize);
	return SUCCESS;	


}
RC DropTable(char *relName){   //既要删除表文件，又要删除系统表中的记录

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
		
		//AfxMessageBox("没有此文件");
		return TABLE_NOT_EXIST;
	}
	CFile file;
	file.Remove((LPCTSTR)relName);//删除表文件
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
		if (strcmp(relName,tab.tablename)==0){//符合条件则删除该项
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
			if((reccol.pData+42+3*sizeof(int))=="1")   file.Remove((LPCTSTR)index);	//删除索引文件
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

	RM_FileHandle *sys_column,*rm_data;    //系统文件句柄
	sys_column = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	sys_column->bOpen = false;

	IX_IndexHandle *rm_index;			   //索引文件句柄
	RM_Record reccol;
	
	column col;


	if(RM_OpenFile("SYSCOLUMNS", sys_column)!=SUCCESS){
		AfxMessageBox("系统列文件打开失败");
		return SQL_SYNTAX;
	}

	RM_FileScan filescan;	  
	filescan.bOpen = false;
	if (OpenScan(&filescan, sys_column, 0, NULL)!=SUCCESS){
		AfxMessageBox("系统列文件扫描失败");
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
				


				rm_index = (IX_IndexHandle *)malloc(sizeof(IX_IndexHandle));//打开索引文件
	            rm_index->bOpen = false;
	            if(OpenIndex(indexName, rm_index)!=SUCCESS){      
		            AfxMessageBox("索引文件打开失败");
		            return SQL_SYNTAX;
	            }

				rm_data = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	            rm_data->bOpen = false;
	            if (RM_OpenFile(relName, rm_data)!=SUCCESS){
		           AfxMessageBox("记录文件打开失败");
		           return SQL_SYNTAX;
	            }

                filescan.bOpen = false;//打开表的记录文件进行扫描
	            if (OpenScan(&filescan, rm_data,0,NULL) != SUCCESS){
					AfxMessageBox("记录文件扫描失败");
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


	sys_column = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));//打开系统列文件
	sys_column->bOpen = false;
	if (RM_OpenFile("SYSCOLUMNS", sys_column)!=SUCCESS){
		AfxMessageBox("系统列文件打开失败");
		return SQL_SYNTAX;
	}


	filescan.bOpen = false;//打开系统列文件进行扫描，修改同名索引的记录项，flag变0。
	if (OpenScan(&filescan, sys_column, 0, NULL)!=SUCCESS){
		AfxMessageBox("系统列文件扫描失败");
		return SQL_SYNTAX;
	}


	while (GetNextRec(&filescan, &reccol) == SUCCESS){
		memcpy(index,reccol.pData+43+3*sizeof(int),21);
		if (strcmp(indexName,index)==0){
			if((reccol.pData+42+3*sizeof(int))=="0")return SQL_SYNTAX;//不存在索引则报错
			else{
				memcpy(reccol.pData+42+3*sizeof(int),"0",1);//存在  则标记位置0且删除索引文件
			    if (UpdateRec(sys_column,&reccol)!=SUCCESS)return SQL_SYNTAX;
				tmp.Remove((LPCTSTR)indexName);//删除索引文件
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
	int attrcount ;//属性的数量
	char *value=(char *)malloc(200);//读取数据表信息  ，所要插入的数据信息
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
		AfxMessageBox("系统表文件打开失败");
		return SQL_SYNTAX;
	}
	if(RM_OpenFile("SYSCOLUMNS", sys_column)!= SUCCESS){
		AfxMessageBox("系统列文件打开失败");
		return SQL_SYNTAX;
	}
	if(RM_OpenFile(relName, rm_data)!= SUCCESS){     //数据表
		//AfxMessageBox("数据文件打开失败");
		return TABLE_NOT_EXIST;
	}

	RM_FileScan filescan;
	filescan.bOpen=false;


	if(OpenScan(&filescan, sys_table,0,NULL)!= SUCCESS){
		AfxMessageBox("扫描系统表文件打开失败");
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
	//判定属性数量是否相等
	if (attrcount != nValues){
		//AfxMessageBox("属性个数不等，插入失败！");
		return FIELD_MISSING;
	}

	filescan.bOpen=false;
	if(OpenScan(&filescan, sys_column,0,NULL)!= SUCCESS){
		AfxMessageBox("扫描系统列文件打开失败");
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
	//关闭系统列文件扫描
	if(CloseScan(&filescan)!=SUCCESS)return SQL_SYNTAX;

	//打开系统列文件扫描
	filescan.bOpen = false;
	if (OpenScan(&filescan, sys_column, 0, NULL)!= SUCCESS){
		AfxMessageBox("系统列文件扫描失败");
		return SQL_SYNTAX;
	}


	char a;

	while (GetNextRec(&filescan, &reccol) == SUCCESS){
		if (strcmp(relName, reccol.pData) == 0){//找到表名为relName的第一个记录，依次读取attrcount个记录
			//for (int i = 0; i < attrcount; i++){
				memcpy(&a,reccol.pData+42+3*sizeof(int),1);
				if(a=='1'){//ix_flag为1，该属性上存在索引，需插入新的索引项
					IX_IndexHandle *rm_index;
					memcpy(index,reccol.pData+43+2*sizeof(int)+sizeof(AttrType),21);
					rm_index = (IX_IndexHandle *)malloc(sizeof(IX_IndexHandle));//打开索引文件
	                rm_index->bOpen = false;
	                if(OpenIndex(index, rm_index)!=SUCCESS){
		               AfxMessageBox("索引文件打开失败");
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
	//关闭文件
	if (RM_CloseFile(rm_data)!= SUCCESS){
		AfxMessageBox("记录文件关闭失败");
		return SQL_SYNTAX;
	}
	free(rm_data);
	if (RM_CloseFile(sys_table)!= SUCCESS){
		AfxMessageBox("系统表文件关闭失败");
		return SQL_SYNTAX;
	}
	free(sys_table);
	if (RM_CloseFile(sys_column)!= SUCCESS){
		AfxMessageBox("系统列文件关闭失败");
		return SQL_SYNTAX;
	}
	free(sys_column);
	return SUCCESS;
	


}


RC Delete(char *relName,int nConditions,Condition *conditions){    //只对单表进行删除，不涉及多余的表删除语句中只会出现一个表


	CFile tmp;
	RM_FileHandle *rm_data,*sys_table,*sys_column;
	RM_FileScan filescan;
	RM_Record recdata,rectab,reccol;
	column *Column, *ctmp,*ctmpleft,*ctmpright;
	Condition *contmp;
	Condition *cond =conditions;

	int i,torf;//是否符合删除条件
	int attrcount;//属性数量
	int intleft,intright; 
	char *charleft,*charright;
	float floatleft,floatright;//属性的值
	AttrType attrtype;
	char index[21],attr[21];

	//打开记录,系统表，系统列文件
	rm_data = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	rm_data->bOpen = false;
	if (RM_OpenFile(relName, rm_data)!= SUCCESS)return TABLE_NOT_EXIST;

	sys_table = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	sys_table->bOpen = false;
	if (RM_OpenFile("SYSTABLES", sys_table)!= SUCCESS)return SQL_SYNTAX;

	sys_column = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	sys_column->bOpen = false;
	if (RM_OpenFile("SYSCOLUMNS", sys_column)!= SUCCESS)return SQL_SYNTAX;

	//打开系统表文件扫描,然后找到对应表的属性数量,赋值给attrcount
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
	//关闭系统表文件扫描
	if(CloseScan(&filescan)!=SUCCESS)return SQL_SYNTAX;

	//打开系统列文件扫描
	filescan.bOpen = false;
	if (OpenScan(&filescan, sys_column, 0, NULL)!= SUCCESS)return SQL_SYNTAX;

	//读取属性信息，结果保存在Column  ,ctmp  中
	Column = (column *)malloc(attrcount*sizeof(column));
	ctmp = Column;
	column *col=Column;
	while (GetNextRec(&filescan, &reccol) == SUCCESS){
		if (strcmp(relName, reccol.pData) == 0){          //读取表名为relName的属性信息
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
		columnset.insert(str);  //插入成功返回1
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
	

	//关闭系统列文件扫描
	if(CloseScan(&filescan)!=SUCCESS)return SQL_SYNTAX;


	//打开记录文件扫描
	filescan.bOpen = false;
	if (OpenScan(&filescan, rm_data, 0, NULL)!= SUCCESS)return SQL_SYNTAX;

	while (GetNextRec(&filescan, &recdata) == SUCCESS){	//取每一条记录根据条件做判断

		for (i = 0, torf = 1,contmp = conditions;i < nConditions; i++, contmp++){//conditions条件逐一判断
			ctmpleft = ctmpright = Column;//每次循环都要将遍历整个系统列文件，找到各个条件对应的属性


			//左属性右值
			if (contmp->bLhsIsAttr == 1 && contmp->bRhsIsAttr == 0){
				for (int j = 0; j < attrcount; j++){              //对表中的所有属性进行比较判断
					if (contmp->lhsAttr.relName == NULL){
						contmp->lhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->lhsAttr.relName, relName);					}  //默认表名为relName;
					if ((strcmp(ctmpleft->tablename, contmp->lhsAttr.relName) == 0)
						&& (strcmp(ctmpleft->attrname, contmp->lhsAttr.attrName) == 0)){     //找到属性，跳出循环
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
			//右属性左值
			if (contmp->bLhsIsAttr == 0 && contmp->bRhsIsAttr == 1){
				for (int j = 0; j < attrcount; j++){        //对表中的所有属性进行比较判断
					if (contmp->rhsAttr.relName == NULL){
						contmp->rhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->rhsAttr.relName, relName);             //默认表名为relName;
					}
					if ((strcmp(ctmpright->tablename, contmp->rhsAttr.relName) == 0)
						&& (strcmp(ctmpright->attrname, contmp->rhsAttr.attrName) == 0)){       //找到属性，跳出循环
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
			//左右均属性
			else  if (contmp->bLhsIsAttr == 1 && contmp->bRhsIsAttr == 1){
				for (int j = 0; j < attrcount; j++){                   //对表中的所有属性进行比较判断
					if (contmp->lhsAttr.relName == NULL){
						contmp->lhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->lhsAttr.relName, relName);            //默认表名为relName;
					}
					if ((strcmp(ctmpleft->tablename, contmp->lhsAttr.relName) == 0)
						&& (strcmp(ctmpleft->attrname, contmp->lhsAttr.attrName) == 0)){//找到属性，跳出循环
						break;
					}
					ctmpleft++;
				}

				
				for (int j = 0; j < attrcount; j++){          //对表中的所有属性进行比较判断
					if (contmp->rhsAttr.relName == NULL){
						contmp->rhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->rhsAttr.relName, relName);//默认表名为relName;
					}
					if ((strcmp(ctmpright->tablename, contmp->rhsAttr.relName) == 0)
						&& (strcmp(ctmpright->attrname, contmp->rhsAttr.attrName) == 0)){//找到属性，跳出循环
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


	//关闭文件
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





RC Update(char *relName,char *attrName,Value *value,int nConditions,Condition *conditions){   //进行单列更新

	if(access(relName ,0)!=0) return TABLE_NOT_EXIST;

	RM_FileHandle *rm_data, *rm_table, *rm_column;
	RM_FileScan FileScan;
	RM_Record recdata, rectab, reccol;



	column *Column, *ctmp,*cupdate,*ctmpleft,*ctmpright;
	Condition *contmp;
	Condition *cond = conditions;
	int i, torf;//是否符合更新条件
	int attrcount;//属性数量

	int flag;

	int intleft,intright;
	char *charleft,*charright;
	float floatleft,floatright;//属性的值
	AttrType attrtype;


	//打开记录,系统表，系统列文件
	rm_data = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	rm_data->bOpen = false;
	if (RM_OpenFile(relName, rm_data)!= SUCCESS)return SQL_SYNTAX;
	rm_table = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	rm_table->bOpen = false;
	if (RM_OpenFile("SYSTABLES", rm_table)!= SUCCESS)return SQL_SYNTAX;
	rm_column = (RM_FileHandle *)malloc(sizeof(RM_FileHandle));
	rm_column->bOpen = false;
	if (RM_OpenFile("SYSCOLUMNS", rm_column)!= SUCCESS)return SQL_SYNTAX;
	//打开系统表文件扫描
	FileScan.bOpen = false;
	if (OpenScan(&FileScan, rm_table, 0, NULL)!= SUCCESS)return SQL_SYNTAX;
	
	while (GetNextRec(&FileScan, &rectab) == SUCCESS){
		if (strcmp(relName, rectab.pData) == 0){
			memcpy(&attrcount, rectab.pData + 21, sizeof(int));//该表的属性的数量
			break;
		}
	}
	//关闭系统表文件扫描
	if(CloseScan(&FileScan)!=SUCCESS)return SQL_SYNTAX;


	//打开系统列文件扫描
	FileScan.bOpen = false;
	if (OpenScan(&FileScan, rm_column, 0, NULL)!= SUCCESS)return SQL_SYNTAX;

	
	Column = (column *)malloc(attrcount*sizeof(column));//属性列的信息
	column *col=Column;
	cupdate = (column *)malloc(sizeof(column));//更新所对应的属性列
	ctmp = Column;


	while (GetNextRec(&FileScan, &reccol) == SUCCESS){
		if (strcmp(relName, reccol.pData) == 0){//找到表名为relName的第一个记录，依次读取attrcount个记录
			for (int i = 0; i < attrcount; i++,ctmp++){
				memcpy(ctmp->tablename, reccol.pData, 21);
				memcpy(ctmp->attrname, reccol.pData + 21, 21);
				memcpy(&(ctmp->attrtype), reccol.pData + 42, sizeof(AttrType));
				memcpy(&(ctmp->attrlength), reccol.pData + 42 + sizeof(AttrType), sizeof(int));
				memcpy(&(ctmp->attroffset), reccol.pData + 42 + sizeof(int)+sizeof(AttrType), sizeof(int));
				if ((strcmp(relName,ctmp->tablename) == 0) && (strcmp(attrName,ctmp->attrname) == 0)){
					cupdate = ctmp;//找到要更新的列的列的属性信息
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
		columnset.insert(str);  //插入成功返回1
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
	//关闭系统列文件扫描
	if(CloseScan(&FileScan)!=SUCCESS)return SQL_SYNTAX;



	//打开记录文件扫描
	FileScan.bOpen = false;
	if (OpenScan(&FileScan, rm_data, 0, NULL)!= SUCCESS)return SQL_SYNTAX;
	//循环查找表名为relName对应的数据表中的记录,并将记录信息保存于recdata中
	while (GetNextRec(&FileScan, &recdata) == SUCCESS){
		for (i = 0, torf = 1, contmp = conditions; i < nConditions; i++, contmp++){//conditions条件逐一判断
		                                                                           //都满足的时候才会进行更新操作
			ctmpleft = ctmpright = Column;//每次循环都要将遍历整个系统列文件，找到各个条件对应的属性
			//左属性右值
			if (contmp->bLhsIsAttr == 1 && contmp->bRhsIsAttr == 0){

				for (int j = 0; j < attrcount; j++){//attrcount个属性逐一判断
					if (contmp->lhsAttr.relName == NULL){//当条件中未指定表名时，默认为relName
						contmp->lhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->lhsAttr.relName, relName);
					}
					if ((strcmp(ctmpleft->tablename, contmp->lhsAttr.relName) == 0)
						&& (strcmp(ctmpleft->attrname, contmp->lhsAttr.attrName) == 0)){//根据表名属性名找到对应属性
						break;
					}
					ctmpleft++;
				}
				//对conditions的某一个条件进行判断
				if (ctmpleft->attrtype == ints){//判定属性的类型
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
			//右属性左值
			else  if (contmp->bLhsIsAttr == 0 && contmp->bRhsIsAttr == 1){
				for (int j = 0; j < attrcount; j++){//attrcount个属性逐一判断
					if (contmp->rhsAttr.relName == NULL){//当条件中未指定表名时，默认为relName
						contmp->rhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->rhsAttr.relName, relName);
					}
					if ((strcmp(ctmpright->tablename, contmp->rhsAttr.relName) == 0)
						&& (strcmp(ctmpright->attrname, contmp->rhsAttr.attrName) == 0)){//根据表名属性名找到对应属性
						break;
					}
					ctmpright++ ;
				}
				//对conditions的某一个条件进行判断
				if (ctmpright->attrtype == ints){//判定属性的类型
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
			//左右均属性
			else  if (contmp->bLhsIsAttr == 1 && contmp->bRhsIsAttr == 1){
				for (int j = 0; j < attrcount; j++){//attrcount个属性逐一判断
					if (contmp->lhsAttr.relName == NULL){//当条件中未指定表名时，默认为relName
						contmp->lhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->lhsAttr.relName, relName);
					}
					if ((strcmp(ctmpleft->tablename, contmp->lhsAttr.relName) == 0)
						&& (strcmp(ctmpleft->attrname, contmp->lhsAttr.attrName) == 0)){//根据表名属性名找到对应属性
						break;
					}
					ctmpleft++;
				}
				for (int j = 0; j < attrcount; j++){//attrcount个属性逐一判断
					if (contmp->rhsAttr.relName == NULL){//当条件中未指定表名时，默认为relName
						contmp->rhsAttr.relName = (char *)malloc(21);
						strcpy(contmp->rhsAttr.relName, relName);
					}
					if ((strcmp(ctmpright->tablename, contmp->rhsAttr.relName) == 0)
						&& (strcmp(ctmpright->attrname, contmp->rhsAttr.attrName) == 0)){//根据表名属性名找到对应属性
						break;
					}
					ctmpright++;
				}
				//对conditions的某一个条件进行判断
				if (ctmpright->attrtype == ints && ctmpleft->attrtype == ints){//判定属性的类型
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
	//关闭文件
	if (RM_CloseFile(rm_table)!= SUCCESS)return SQL_SYNTAX;
	free(rm_table);
    if (RM_CloseFile(rm_column)!= SUCCESS)return SQL_SYNTAX;
	free(rm_column);
	if (RM_CloseFile(rm_data)!= SUCCESS)return SQL_SYNTAX;
	free(rm_data);
	return SUCCESS;	


}


bool CanButtonClick(){//需要重新实现
	//如果当前有数据库已经打开
	return true;
	//如果当前没有数据库打开
	//return false;
}
