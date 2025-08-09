# rest2sql-plugin 
### Creating a CRUD JSON REST API for remote consumption of MariaDB datas

The goal is to provide an easy way to map http requests to CRUD-like DML statements and eventually stored procedure calls with json objects in http as response. The scope of this project will stay "limited" to DML only.

## design decisions
* [discussion](https://github.com/SylvainA77/JSON2SQL-plugin/blob/main/doc/stack-n-architecture-decisions.md)
 
## Project Status

- **Release**: pre ALPHA
- **Current release**: 0.1
- **Current Version**: 0.2
- **Status**: Active development
- **Last Updated**: 20250809
- **Actual development branch** : dev


## Direct tables access

* [GET → SELECT](https://github.com/SylvainA77/JSON-API-plugin/blob/main/doc/handle_get_request.md)  

* [POST → INSERT](https://github.com/SylvainA77/JSON2SQL-plugin/blob/main/doc/handle_post_request.md)  

* [PATCH → UPDATE](https://github.com/SylvainA77/JSON2SQL-plugin/blob/main/doc/handle_patch_request.md)  

* [DELETE → DELETE](https://github.com/SylvainA77/JSON2SQL-plugin/blob/main/doc/handle_delete_request.md)


## Stored procedure calls (TBD)

* [PUT → CALL](https://github.com/SylvainA77/JSON2SQL-plugin/blob/main/doc/handle_put_request.md)
  
## compilation instructions

* [instructions](https://github.com/SylvainA77/JSON-API-plugin/blob/main/doc/compilation.md)

## upcoming features

* [TODO](https://github.com/SylvainA77/JSON-API-plugin/blob/main/doc/TODO.md)

## Contributions

* [Awesome people](https://github.com/SylvainA77/JSON-API-plugin/blob/main/Contributions.md)
* If you want to contribute, please take a look at branches. The development branch is announced in the project status. There will be one per http method to handle with corresponding name.


