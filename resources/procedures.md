# ??? PROCEDURES resource ???

A procedures resource represent a stored procedure in the database: STILL TBD

# Resource operations

## Execute a procedure
```
PUT /v1/procedures/:schema/:name/ HTTP/1.1 Host: example.com Content-Type: application/json Content-Length: xx {"arg1":"argval1",..,"argn":"argvaln"}
```

### Response

