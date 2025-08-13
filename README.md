# rest2sql-plugin 
### Creating a CRUD JSON REST API for remote consumption of MariaDB datas

The goal is to provide an easy way to map http requests to CRUD-like DML statements and eventually stored procedure calls with json objects in http as response. The scope of this project will stay "limited" to DML only.

## design decisions
* [discussion](https://github.com/SylvainA77/JSON2SQL-plugin/blob/main/doc/stack-n-architecture-decisions.md)
 
## Project Status

- **Release**: pre ALPHA
- **Current Version**: 0.1
- **dev version**: 0.2  
- **Status**: Active development
- **Last Updated**: 20250813
- **Actual development branch** : dev

## MariaDB version compatiblity

This project is based on MariaDB 11.4 (both CS or ES). Earlier versions are not compatible as it makes use of several functions introduced between 10.10 and 11.3.

## R(ead) Methods

* [GET → SELECT/DESC/LOGIN/STATUS](https://github.com/SylvainA77/JSON-API-plugin/blob/main/doc/handle_get_request.md)

## C(reate)U(pdate)D(elete) Methods

* [POST → INSERT](https://github.com/SylvainA77/JSON2SQL-plugin/blob/main/doc/handle_post_request.md)  

* [PATCH → UPDATE](https://github.com/SylvainA77/JSON2SQL-plugin/blob/main/doc/handle_patch_request.md)  

* [DELETE → DELETE](https://github.com/SylvainA77/JSON2SQL-plugin/blob/main/doc/handle_delete_request.md)

  
## compilation instructions

* [instructions](https://github.com/SylvainA77/JSON-API-plugin/blob/main/doc/compilation.md)

## upcoming features and releases

* [TODO](https://github.com/SylvainA77/JSON-API-plugin/blob/main/doc/TODO.md)

## Contributions

* [Awesome people](https://github.com/SylvainA77/JSON-API-plugin/blob/main/Contributions.md)
* If you want to contribute, please take a look at branches. The development branch is announced in the project status. There will be one per http method to handle with corresponding name.


