# Translate API PUT requests into CALL statements  
  
* PUT /v1/procedures/:schema/:name {JSON BODY REQUEST} → USE :schema;CALL :name(argv1,...argvn)

## expected body request format

{  
"argv": "argv1,...argvn"  
}  

## RESULT  

