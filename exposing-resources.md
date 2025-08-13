#

## double list system : blacklist + whitelist

default behaviour is  : 
- blacklist
- blacklist has precedence over whitelist

default behaviour parametrizable (is it worth the effort ?)

## resources

HTTP method + resource + individual objects can be blacklisted or whitelisted

| HTTP method | resource  | list (0 black, 1 white) |  
|------|------|------|  
| GET | /v1/status | 1 |  
| POST | /v1/data/myschema/\*/\* | 0 |   
| PUT | \* | 0 | 

