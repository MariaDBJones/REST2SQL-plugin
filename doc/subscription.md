# How to subscribe to a resource/API endpoint

stored proc : subscribe

## arguments
- user
- password
- method : GET/POST/PATCH/PUT/DELETE
- URI : /version/resource/schema/table

Note : wilcard symbol is * . wildcards allowed in method or resource or schema or table
  

## algorithm
1. check user & password exists and match
2. check role with name "ruser" exists
3. method must be allowed
4. URI(resource/schema/table) must be allowed
5. method + URI(resource) must match
7. give role "ruser" permission according to method + URI(schema, table)
8. allow APIUSER to assign itself the role

