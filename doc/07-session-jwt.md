# session management

a session JWT is issued as part of the response to a succesfull digest login

## issuing the JWT (server side)

1. **Header** — JSON object specifying token type and signing algorithm, e.g.:
   ```json
   {
     "typ": "JWT",
     "alg": "HS256"
   }
   ```
   This means the token type is JWT and the signing algorithm is HMAC SHA-256 (or whichever you choose).

2. **Payload (Claims)** — a JSON object containing information about the user and token metadata, such as:
   - `sub`: Subject (user identifier)
   - `iss`: Issuer (your API or authentication server)
   - `aud`: Audience (who the token is for)
   - `exp`: Expiration timestamp
   - Any other custom user or session claims (e.g., roles, permissions)

Summary Table
| Claim	| Purpose	| Your Usage Example	| Notes | 
|---|---|---|---|
|sub	|User identifier (login or ID)	|"sub": "johndoe"	|Unique user ID or username|
|iss	|Token issuer (the API or auth server)	|"iss": "rest2sql.hostname.com"	|Validates authority|
|aud	|Intended recipient(s) (API/service name)	|"aud": "rest2sql.hostname.com"	|Distinguishes token target|
|iat	|Token issue time (epoch timestamp)	|"iat": 1692057240	|Marks token creation time|
|exp	|Token expiration time (epoch timestamp)	|"exp": 1692060840	|Token expiry enforcement|
|scope	|User’s assigned roles or permissions	|"roles": ["reader", "editor"]	|Used for authorization checks|

3. **Signature** — the cryptographic signature created by signing the Base64Url-encoded header and payload with your secret key using the algorithm from the header.

***

### Pseudocode steps to compose a JWT after successful authentication:

```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "jwt.h"    // from jwt-c
#include "cJSON.h"  // from cJSON

// Configurable values:
static const char *jwt_secret = "your-256-bit-secret"; // Replace with generated secret
static const char *jwt_issuer = "rest2sql.server";
static const char *jwt_audience = "rest2sql.server";
static const int jwt_ttl_seconds = 3600; // 1 hour

cJSON* issue_jwt(const char *login) {
    time_t now = time(NULL);

    // 1. Build cJSON payload object
    cJSON *payload_obj = cJSON_CreateObject();
    cJSON_AddStringToObject(payload_obj, "sub", login);
    cJSON_AddStringToObject(payload_obj, "iss", jwt_issuer);
    cJSON_AddStringToObject(payload_obj, "aud", jwt_audience);
    cJSON_AddNumberToObject(payload_obj, "iat", (double)now);
    cJSON_AddNumberToObject(payload_obj, "exp", (double)(now + jwt_ttl_seconds));

    // Optional custom claims
    cJSON *roles_arr = cJSON_CreateArray();
    cJSON_AddItemToArray(roles_arr, cJSON_CreateString("admin"));
    cJSON_AddItemToObject(payload_obj, "roles", roles_arr);

    // 2. Convert payload JSON to string
    char *payload_str = cJSON_PrintUnformatted(payload_obj);

    // 3. Encode (sign) JWT with HS256
    char token[1024];
    if (jwt_encode_hmac_sha256(payload_str, jwt_secret, token, sizeof(token)) != 0) {
        fprintf(stderr, "Failed to create JWT\n");
        free(payload_str);
        cJSON_Delete(payload_obj);
        return NULL;
    }

    free(payload_str);

    // 4. Create return object: { "token": "<jwt>" }
    cJSON *result_obj = cJSON_CreateObject();
    cJSON_AddStringToObject(result_obj, "token", token);

    // The payload_obj isn't returned; only the token JSON goes out
    cJSON_Delete(payload_obj);

    return result_obj; // Caller must cJSON_Delete() when done
}

```

***

## Using thee JWT (client side)
The client includes the JWT in API requests like so:

```
GET /v1/status HTTP/1.1
Host: api.example.com
Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiJqb2huZG9lIiwiaXNzIjoicmVzdDIxc3FsLmV4YW1wbGUuY29tIiwiYXVkIjoicmVzdDIxc3FsLmV4YW1wbGUuY29tIiwiaWF0IjoxNjk4MDEyMzAwLCJleHAiOjE2OTgwMTU5MDB9.XmFgLz1PiJwZpj0fJF29nNdk1PHCGw3s-3t4h6TGvbs
User-Agent: MyApiClient/1.0
Accept: application/json

```

***

## verifying a JWT (server side)


1. **Decode the JWT** into its three parts: header, payload, and signature.

2. **Verify the signature**:  
   - Use the signing algorithm specified in the JWT header (e.g., HS256, RS256).
   - For symmetric algorithms (like HS256), verify the signature using the shared secret key.
   - For asymmetric algorithms (like RS256), verify using the public key from your key set.
   Signature verification ensures the token was issued by a trusted source and was not tampered with.

3. **Validate claims** in the payload, such as:
   - `iss` (issuer): Confirm the token was issued by your trusted issuer.
   - `aud` (audience): Check the token is intended for your API.
   - `exp` (expiration): Reject the token if expired.
   - `nbf` (not before), `iat` (issued at): Optionally check token activation time constraints.
   - Any other application-specific claims.

4. **Reject the token** if:
   - The signature is invalid.
   - Any claim validation fails (expired, wrong issuer, wrong audience, etc.).

5. **If all checks pass**, accept the JWT as valid and use the payload claims (e.g., user ID, roles) for authentication/authorization decisions.

***

### Summary pseudocode for JWT verification:

```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "jwt.h"    // jwt-c
#include "cJSON.h"  // cJSON

// Configured secret used for signing and verifying JWTs (set accordingly)
static const char *jwt_secret = "your-256-bit-secret";

int verifyJWT(cJSON* jwt_json) {
    if (jwt_json == NULL) {
        fprintf(stderr, "verifyJWT: input cJSON object is NULL\n");
        return 0;
    }

    // Extract the JWT token string from the JSON object
    const cJSON *token_item = cJSON_GetObjectItem(jwt_json, "token");
    if (!cJSON_IsString(token_item) || token_item->valuestring == NULL) {
        fprintf(stderr, "verifyJWT: No 'token' string found in cJSON object\n");
        return 0;
    }
    const char *token = token_item->valuestring;

    // Buffer to hold decoded payload (JSON string)
    char payload_str[2048];  // Adjust size as needed

    // Verify the JWT signature with HS256 and extract the payload string
    if (jwt_decode_hmac_sha256(token, jwt_secret, payload_str, sizeof(payload_str)) != 0) {
        fprintf(stderr, "verifyJWT: JWT signature verification failed\n");
        return 0;
    }

    // Parse the payload JSON string
    cJSON *payload_json = cJSON_Parse(payload_str);
    if (payload_json == NULL) {
        fprintf(stderr, "verifyJWT: Failed to parse JWT payload JSON\n");
        return 0;
    }

    // Check 'exp' claim to ensure token is not expired
    const cJSON *exp_item = cJSON_GetObjectItem(payload_json, "exp");
    if (!cJSON_IsNumber(exp_item)) {
        fprintf(stderr, "verifyJWT: 'exp' claim missing or invalid\n");
        cJSON_Delete(payload_json);
        return 0;
    }

    time_t now = time(NULL);
    if ((time_t)exp_item->valuedouble <= now) {
        fprintf(stderr, "verifyJWT: Token expired\n");
        cJSON_Delete(payload_json);
        return 0;
    }

    // Optionally, verify other claims here (iss, aud, nbf, etc.)

    // Cleanup
    cJSON_Delete(payload_json);

    // Token verified and not
    return 1;
```

***

### Important notes:

- Use a JWT library in your language (C or otherwise) to handle parsing and signature verification securely and correctly.
- For **HMAC (HS256)**, the secret key is shared between issuer and verifier.
- For **RSA/ECDSA (RS256, ES256)**, verify using the issuer's public key, often fetched from a JWKS endpoint.
- Always check the algorithm to avoid “alg=none” or downgrade attacks.
- Validate token expiration strictly to prevent reuse of old tokens.
- Keep secret keys and private keys secure; distribute only public keys as needed.

[1] https://www.cryptr.co/documentation/how-to-validate-jwt
[2] https://www.criipto.com/blog/jwt-validation-guide
[3] https://www.cryptr.co/documentation/fr/how-to-validate-jwt
[4] https://auth0.com/docs/secure/tokens/json-web-tokens/validate-json-web-tokens
[5] https://docs.aws.amazon.com/cognito/latest/developerguide/amazon-cognito-user-pools-using-tokens-verifying-a-jwt.html
[7] https://docs.kinde.com/build/tokens/verifying-json-web-tokens/
[9] https://owasp.org/www-project-web-security-testing-guide/latest/4-Web_Application_Security_Testing/06-Session_Management_Testing/10-Testing_JSON_Web_Tokens
[10] https://www.ibm.com/docs/en/api-connect/10.0.8_lts?topic=tutorials-tutorial-validate-json-web-token-jwt

[1] https://jwt.io/introduction
[2] https://fr.wordpress.org/plugins/jwt-authentication-for-wp-rest-api/
[3] https://blog.logrocket.com/secure-rest-api-jwt-authentication/
[4] https://smart-tech.mg/authentification-sous-angular-en-utilisant-le-rest-api-et-jwt/
[5] https://blog.stephane-robert.info/docs/developper/programmation/python/connexion-4/
[6] https://auth0.com/fr/learn/json-web-tokens
[7] https://fusionauth.io/articles/tokens/jwt-components-explained
[9] https://www.ekino.fr/publications/introduction-aux-json-web-tokens/
[10] https://auth0.com/docs/secure/tokens/json-web-tokens/json-web-token-structure
