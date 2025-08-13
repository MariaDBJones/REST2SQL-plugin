# How to create an API user

stored proc : createUser

## arguments
- user
- password

## algorithm
1. check user does not exists
2. check password strength : between 10 and 20chars, 1 lowercase, 1 uppercase, 1 numeric & 1 special char amongst $_*-!/#@
3. create salt with cryptographic secure RANDOM_BYTES() (MRDB 10.10)
4. hash password + salt with safe for password hashing KDF() (MRDB11.3)
5. write user, hash & salt in table
6. creates role name "r$user"
7. allow APIUSER to assign role

## pseudocode
```
SET @valid = @password REGEXP '^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[$_\\*\\-!/#@])[A-Za-z0-9$_\\*\\-!/#@]{10,20}$';

-- Step 3: Only proceed if valid
IF @valid THEN

    -- Step 4: Generate salt
    SET @salt = RANDOM_BYTES(16);

    -- Step 5: Hash the password using KDF (e.g., PBKDF2-HMAC-SHA-256)
    SET @password_hash = KDF(
        @password,      -- the password
        @salt,          -- the salt
        'auth',         -- optional info string
        'pbkdf2_hmac',  -- algorithm
        256             -- output length in bits (256 = 32 bytes)
    );

    -- Step 6: Insert into table

ELSE
    SELECT 'Password does not meet security requirements.' AS error;
END IF;
```

## table structure

| userid | login | hashedPassword | salt | created_at | role | 
|----|----|----|----|----|----|
| uuid_short() INT UNSIGNED not null KEY | varchar(20) not null | varbinary(32) not null | varbinary(16) not null | datetime |   as (concat('r', user) virtual |
