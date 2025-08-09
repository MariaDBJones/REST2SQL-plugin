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

3. **Signature** — the cryptographic signature created by signing the Base64Url-encoded header and payload with your secret key using the algorithm from the header.

***

### Pseudocode steps to compose a JWT after successful authentication:

```pseudo
// 1. Prepare the JWT header (typically a fixed JSON object)
header = {
  "typ": "JWT",
  "alg": "HS256"   // or another supported algorithm like RS256
}

// 2. Prepare the payload (claims) with user info and metadata
payload = {
  "sub": "user_id_or_username",          // user identifier from your auth step
  "iss": "your-api-name-or-url",         // issuer: your API or domain
  "aud": "your-api-audience",            // intended audience of the token
  "iat": current_unix_timestamp(),       // issued at (now)
  "exp": current_unix_timestamp() + 3600 // expiration time (e.g. 1 hour later)
  // additional claims like roles, permissions can also go here
}

// 3. Encode header and payload as Base64Url strings
encodedHeader = Base64UrlEncode(JSON.stringify(header))
encodedPayload = Base64UrlEncode(JSON.stringify(payload))

// 4. Create the signature input string
signingInput = encodedHeader + "." + encodedPayload

// 5. Generate the signature using your secret key with the algorithm specified
signature = HMAC_SHA256(secret_key, signingInput)  // if using HS256

// 6. Base64Url encode the signature
encodedSignature = Base64UrlEncode(signature)

// 7. Concatenate all parts with dots
jwtToken = encodedHeader + "." + encodedPayload + "." + encodedSignature

// 8. Return/send this JWT token to the client for use in Authorization header
```

***

### Usage on the client side
The client includes the JWT in API requests like so:

```
Authorization: Bearer 
```

***

Pseudocode steps to verify a JWT

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
function verifyJWT(token, secretOrPublicKey, expectedIssuer, expectedAudience):
    // 1. Split token into header, payload, signature
    header, payload, signature = decodeJWTParts(token)

    // 2. Verify signature with secret or public key using header.alg
    if not verifySignature(header, payload, signature, secretOrPublicKey, header.alg):
        return false

    // 3. Parse payload (JSON) claims
    claims = parseJSON(payload)

    // 4. Validate standard claims
    if claims.iss != expectedIssuer:
        return false
    if claims.aud != expectedAudience:
        return false
    currentTime = now()
    if claims.exp  currentTime:
        return false

    // 5. Additional checks if needed...

    return true, claims
```

***

### Important notes:

- Use a JWT library in your language (C or otherwise) to handle parsing and signature verification securely and correctly.
- For **HMAC (HS256)**, the secret key is shared between issuer and verifier.
- For **RSA/ECDSA (RS256, ES256)**, verify using the issuer's public key, often fetched from a JWKS endpoint.
- Always check the algorithm to avoid “alg=none” or downgrade attacks.
- Validate token expiration strictly to prevent reuse of old tokens.
- Keep secret keys and private keys secure; distribute only public keys as needed.

***

This approach complements your digest authentication by verifying the stateless JWT tokens that clients present after logging in.

If you want, I can provide example code or more detailed integration guidance for verifying JWT tokens in C or your preferred environment.

Sources:  
- [Cryptr JWT validation guide](https://www.cryptr.co/documentation/how-to-validate-jwt)[1]
- [Criipto JWT validation step-by-step](https://www.criipto.com/blog/jwt-validation-guide)[2]
- [Auth0 validate JWT](https://auth0.com/docs/secure/tokens/json-web-tokens/validate-json-web-tokens)[4]

[1] https://www.cryptr.co/documentation/how-to-validate-jwt
[2] https://www.criipto.com/blog/jwt-validation-guide
[3] https://www.cryptr.co/documentation/fr/how-to-validate-jwt
[4] https://auth0.com/docs/secure/tokens/json-web-tokens/validate-json-web-tokens
[5] https://docs.aws.amazon.com/cognito/latest/developerguide/amazon-cognito-user-pools-using-tokens-verifying-a-jwt.html
[6] https://jwt.io/introduction
[7] https://docs.kinde.com/build/tokens/verifying-json-web-tokens/
[8] https://jwt.io
[9] https://owasp.org/www-project-web-security-testing-guide/latest/4-Web_Application_Security_Testing/06-Session_Management_Testing/10-Testing_JSON_Web_Tokens
[10] https://www.ibm.com/docs/en/api-connect/10.0.8_lts?topic=tutorials-tutorial-validate-json-web-token-jwt

[1] https://jwt.io/introduction
[2] https://fr.wordpress.org/plugins/jwt-authentication-for-wp-rest-api/
[3] https://blog.logrocket.com/secure-rest-api-jwt-authentication/
[4] https://smart-tech.mg/authentification-sous-angular-en-utilisant-le-rest-api-et-jwt/
[5] https://blog.stephane-robert.info/docs/developper/programmation/python/connexion-4/
[6] https://auth0.com/fr/learn/json-web-tokens
[7] https://fusionauth.io/articles/tokens/jwt-components-explained
[8] https://www.youtube.com/watch?v=xfuIkI2C8-o
[9] https://www.ekino.fr/publications/introduction-aux-json-web-tokens/
[10] https://auth0.com/docs/secure/tokens/json-web-tokens/json-web-token-structure
