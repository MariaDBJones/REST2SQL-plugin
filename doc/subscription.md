# How to subscribe to a resource/API endpoint

stored proc : subscribe

## arguments
- $user
- $password
- $method : GET/POST/PATCH/PUT/DELETE
- $URI : /version/resource/schema/table

Note : 
- wilcard symbol is * .
- wildcards allowed in method or resource or schema or table
  
## algorithm
1. check user & password exists and match
2. check user & role exists and match
3. check APIUSER can assign role "r$user"
4. method must be allowed
5. URI(resource/schema/table) must be allowed
6. method + URI(resource) must match
7. give role "r$user" permission according to method + URI(schema, table)

