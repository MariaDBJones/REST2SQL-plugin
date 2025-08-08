# DATA resource

A data resource represent the content of a table in the database

# Resource operations

## Read a set of lines
```
GET /v1/data/:schema/:name/:colname/:colvalue
```

### Response


## Insert a new set of rows
```
POST /v1/data/:schema/:name {"colname1":"colvalue1",...,"colnamen":"colvaluen"}
```

### Response


## Update a set of rows
```
PATCH /v1/data/:schema/:name:colname/:colvalue {"colname1":"colvalue1",...,"colnamen":"colvaluen"}
```

### Response


## Delete a set of rows
```
DELETE /v1/data/:schema/:name/:colname/:colvalue
```

### Response
