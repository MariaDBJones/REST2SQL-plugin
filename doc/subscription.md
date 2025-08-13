# How to subscribe to a resource/API endpoint

stored proc : subscribe

## arguments
- user
- method : GET/POST/PATCH/PUT/DELETE
- URI : /version/resource/schema/table

Note : wilcard symbol is * . wildcards allowed in method or resource or schema or table
  
## algorithm
1. check user & role exists and match
2. method must be allowed
3. URI(resource/schema/table) must be allowed
4. method + URI(resource) must match
5. give role "r$user" permission according to method + URI(schema, table)
6. allow APIUSER to assign itself the role

