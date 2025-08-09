# Translate PATCH requests into UPDATE statements
* PATCH /v1/tables/:SCHEMA/:TABLE/ {JSON BODY REQUEST"COLNAME":COLVALUE, "set":["COLNAME1":COLVALUE1, ... "COLNAMEn":COLVALUEn}  → UPDATE SCHEMA.TABLE SET COLs = VALs, ... WHERE COLNAME = COLVALUE

## expected body request format
{  
"column": "value",  
"cols": m,  
"set": [  
"col1": "val1",  
...  
"colm": "valm"  
]  
}  

## RESULT  
* If failure
{
  "request":"PUT",
  "source": "tables",
  "schema": "schema_name",
  "table": "table_name",
  "status": "MYSQL_ERROR_CODE",
  "rows": 0
} 

* If OK  
{
  "request":"PUT",
  "source": "tables",
  "schema": "schema_name",
  "table": "table_name",
  "status": "OK"
}
