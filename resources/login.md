# LOGIN resource

An unauthenticated authentication resource 

# Resource operations

## Initiate login procedure
```
GET /v1/login
```

## Answering the challenge
```
GET /v1/login
Authorization: Digest username="bob",
               realm="MyAPI",
               nonce="abc123xyz",
               uri="/v1/login",
               qop=auth,
               nc=00000001,
               cnonce="xyz987",
               response="2f9d2c8f......",
               algorithm=SHA-256,
               opaque="opaqueToken123"
```
