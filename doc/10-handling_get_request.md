 # Translate API GET requests into read statements  
  
* [DATA](../resources/data.md) GET /v1/data/$schema/$table/$colname/$colvalue → SELECT * FROM $schema.$table WHERE $colnmae = '$colvalue'

* [STRUCTURE](../resources/structure.md) GET /v1/structure/$schema/$table                 → show columns for $schema.$table

* [STATUS](../resources/status.md) GET /v1/status/                                  → SHOW GLOBAL STATUS

* [STATUS](../resources/status.md) GET /v1/status/$variable                         → SHOW GLOBAL STATUS like '$variable'
  
* [PING](../resources/ping.md) GET /v1/ping                                     → pong + now() 

* [LOGIN](../resources/login.md) GET /v1/login                                    → [authentication scheme](./06-digest-authentication.md)  

## Function declaration

[handle_get_request.h]() : static cJSON* response handle_get_request(const char *url)  

## Sources

[handle_get_request.c](../code/handle_get_request.c)


