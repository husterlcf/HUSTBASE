#ifndef STR_H_H
#define STR_H_H
#define MAX_NUM 20
#define MAX_REL_NAME 20
#define MAX_ATTR_NAME 20
#define MAX_ERROR_MESSAGE 20
#define MAX_DATA 50

/*
文件: str.h
内容: the definition of sqls' structs
*/
#include<stdlib.h>
#include<string.h>

//属性结构体   属性名和表名
typedef struct  {
  char *relName;     // relation name (may be NULL) 表名
  char *attrName;    // attribute name              属性名
}RelAttr;          

typedef enum {
	EQual,			//"="			0
	LEqual,			//"<="          1         
	NEqual,			//"<>"			2       
	LessT,			//"<"			3            
	GEqual,			//">="			4         
	GreatT,			//">"           5    
	NO_OP
}CompOp;

//属性值类型
typedef enum {
		chars,
		ints,
		floats
}AttrType;
//属性值  属性类型与值
typedef struct _Value Value;
struct _Value{
	AttrType type;     // type of value               
	void     *data;    // value    
};

typedef struct  _Condition Condition;   //比较条件
struct  _Condition{
  int     bLhsIsAttr;   // TRUE if left-hand side is an attribute 1时，操作符左边是属性，0时，是值
  Value   lhsValue;		// left-hand side value if bLhsIsAttr = FALSE 当是属性的时候，没有值，但不是属性时候，里面存储的是值的大小
  RelAttr lhsAttr;      // left-hand side attribute    


  CompOp  op;           // comparison operator   

  int     bRhsIsAttr;   // TRUE if right-hand side is an attribute 1时，操作符右边是属性，0时，是值
                        //   and not a value
  RelAttr rhsAttr;      // right-hand side attribute if bRhsIsAttr = TRUE 右边的属性
  Value   rhsValue;     // right-hand side value if bRhsIsAttr = FALSE 

};

//struct of select
typedef struct {
	int nSelAttrs;              	//Length of attrs in Select clause      所选属性数量   
	RelAttr *selAttrs[MAX_NUM];			//attrs in Select clause   包含所选属性的数组
	int nRelations;								//Length of relations in Fro clause  所选表的数量
	char *relations[MAX_NUM];			//relations in From clause  包含所选表
	int nConditions;							//Length of conditions in Where clause  //条件数量 
	Condition conditions[MAX_NUM];//conditions in Where clause
}selects;




//struct of insert 
typedef struct {
	char *relName;				//Relation to insert into 
	int nValues;					//Length of values
	Value values[MAX_NUM];//values to insert
}inserts;

//struct of delete 
typedef struct {
	char *relName;										//Relation to delete from
	int nConditions;									//Length of conditions in Where clause 
	  Condition conditions[MAX_NUM];	//conditions in Where clause
}deletes;

//struct of update 
typedef struct {
	char *relName;					//Relation to update
	char *attrName;					//Attribute to update
	Value value;						//update value
	int nConditions;				//Length of conditions in Where clause 
	 Condition conditions[MAX_NUM];	// conditions in Where clause
}updates;

//struct of AttrInfo  
typedef struct _AttrInfo AttrInfo;
struct _AttrInfo{
	char *attrName;		//Attribute name
	AttrType attrType;		//Type of attribute
	int attrLength;		//Length of attribute
};
//struct of craete_table
typedef struct {
	char *relName;		//Relation name
	int attrCount;		//Length of attribute 
	AttrInfo attributes[MAX_NUM];	//attributes 
}createTable;

//struct of drop_table  
typedef struct {
	char *relName;		//Relation name
}dropTable;

//struct of create_index  
typedef struct {
	char *indexName;	// Index name
	char *relName;		// Relation name
	char *attrName;		// Attribute name
}createIndex;

//struct of  drop_index  
typedef struct {
	char *indexName;		// Index name

}dropIndex;

//union of sql_structs
union sqls{
	 selects sel;
	 inserts ins;
	 deletes del;
	 updates upd;
	 createTable cret;
	 dropTable drt;
	 createIndex crei;
	 dropIndex dri;
	 char *errors;
};


// struct of flag and sql_struct
typedef struct {
	int flag;	/*match to the sqls 0--error;1--select;2--insert;3--update;4--delete;5--create table;6--drop table;7--create index;8--drop index;9--help;10--exit;*/
 	union sqls sstr;
}sqlstr;

#ifndef RC_HH
#define RC_HH
typedef enum{
	SUCCESS,
	SQL_SYNTAX,
	PF_EXIST,
	PF_FILEERR,
	PF_INVALIDNAME,
	PF_WINDOWS,
	PF_FHCLOSED,
	PF_FHOPEN,
	PF_PHCLOSED,
	PF_PHOPEN,
	PF_NOBUF,
	PF_EOF,
	PF_INVALIDPAGENUM,
	PF_NOTINBUF,
	PF_PAGEPINNED,
	RM_FHCLOSED,
	RM_FHOPENNED,
	RM_INVALIDRECSIZE,
	RM_INVALIDRID,
	RM_FSCLOSED,
	RM_NOMORERECINMEM,
	RM_FSOPEN,
	IX_IHOPENNED,
	IX_IHCLOSED,
	IX_INVALIDKEY,
	IX_NOMEM,
	RM_NOMOREIDXINMEM,
	IX_EOF,
	IX_SCANCLOSED,
	IX_ISCLOSED,
	IX_NOMOREIDXINMEM,
	IX_SCANOPENNED,
	FAIL,

	DB_EXIST,
	DB_NOT_EXIST,
	NO_DB_OPENED,

	TABLE_NOT_EXIST,
	TABLE_EXIST,
	TABLE_NAME_ILLEGAL,

	FLIED_NOT_EXIST,//在不存在的字段上增加索引
	FIELD_NAME_ILLEGAL,
	FIELD_MISSING,//插入的时候字段不足
	FIELD_REDUNDAN,//插入的时候字段太多
	FIELD_TYPE_MISMATCH,//字段类型有误

	RECORD_NOT_EXIST,//对一条不存在的记录进行删改时

	INDEX_NAME_REPEAT,
	INDEX_EXIST,//在指定字段上，已经存在索引了
	INDEX_NOT_EXIST,
	INDEX_CREATE_FAILED,
	INDEX_DELETE_FAILED,

	INCORRECT_QUERY_RESULT,    //查询结果错误
	ABNORMAL_EXIT,  //异常退出

	TABLE_CREATE_REPEAT,  //表重复创建
	TABLE_CREATE_FAILED,  //创建表失败
	TABLE_COLUMN_ERROR,    //列数/列名/列类型/列长度错误
	TABLE_ROW_ERROR,
	TABLE_DELETE_FAILED,   //表删除失败

	DATABASE_FAILED //数据库创建或删除失败
}RC;
#endif
#ifdef __cplusplus
extern "C"{
	sqlstr * get_sqlstr();
	RC parse(char* st,sqlstr* sqln);
};
#endif
#endif
