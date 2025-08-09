# Translate API POST requests into INSERT statements  
  
* POST /v1/tables/SCHEMA/TABLE/ {JSON REQUEST BODY} → INSERT INTO SCHEMA.TABLE (COLNAME1,...,COLNAMEn) VALUES (COLVALUE1,...,COLVALUEn)  

## expected request body
{  
 "upsert": "yes/no",  
 "columns": "col1,col2,..,coln",  
 "rows": m,  
 "values": [  
  "1": "val41,val2,...,valn",  
  ...  
  "m": "val41,val2,...,valn"  
 ]  
}   

## RESULT  
* If failure  
 {  
  "status": "ERR_MSG",  
  "mariadbcode": "MYSQL_ERROR_CODE",  
  "httpcode": 400/500,  
  "rows": 0  
} 

* If OK  
{  
  "status": "OK",  
  "mariadbcode": 0,  
  "httpcode": 200,  
  "rows": n  
}  
