# How to create an API user

stored proc : createUser

## arguments
- user
- password

## algorithm
1. check user does not exists
2. check password strength : between 10 and 20chars, 1 lowercase, 1 uppercase, 1 numeric & 1 special char amongst $_*-!/#@
3. write user & password in table
4. creates role name "r$user"
5. allow APIUSER to assign role
