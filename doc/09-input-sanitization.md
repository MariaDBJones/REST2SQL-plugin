# Existing input sanitization rules, techniques, and libraries for C:

***

## Input Sanitization Libraries

- **Libinjection**: A small, widely-used C library (MIT-licensed) for detecting SQL Injection and Cross-site Scripting (XSS). It analyzes strings for injection patterns and can be integrated into C applications for automated checking against these attacks.[1]
- **StringSan**: A library focused on sanitizing strings, helping prevent buffer overflows and removing potentially malicious content. It provides robust protection in scenarios where handling untrusted string data is critical.[1]
- **libtidy**: For sanitizing and cleaning up HTML input, especially important if your API deals with user-supplied markup content.[1]

***

## Classic C Techniques and Example Functions

- **Whitelisting**: Only allow known safe characters (e.g., alphanumeric, specified symbols). Reject or replace anything not matching your pattern. This is recommended over blacklisting for security.[2][4][5]
- **Input Length Limits**: Always place bounds on input length to prevent buffer overflows.[2]
- **Escaping and Encoding**: For database and shell interactions, escape or encode special characters to prevent command injection or SQL injection.[3][5]
- **Regex Validation**: Use regular expressions (with libraries like PCRE) for complex input pattern enforcement.[2]
- **Trimming and Normalization**: Remove leading/trailing whitespace and normalize encoding to defeat common bypasses.[2]

### Example: Simple Whitelist Sanitization
```c
#include 
#include 
void sanitize_input(char *input) {
    // Remove unwanted characters, keep only alphanumeric and _
    for (int i = 0; input[i]; i++) {
        if (!(isalnum((unsigned char)input[i]) || input[i] == '_')) {
            input[i] = '_'; // Replace bad char
        }
    }
}
```

***

## References and Usage

- For general input sanitization patterns, see guidance from secure coding standards like SEI CERT C and best practices summarized in tutorials and guides.[4][6][8][2]
- When using MariaDB/MySQL in C, rely on escaping functions like `mysql_real_escape_string()` for SQL parameters and always combine this with other validations.
- only allow for MariaDB identifiers using valid characters
- For web APIs, always validate input against expected types and lengths before using or passing to subsystems.

***

## Allowed characters for MariaDB identifiers
By default (when not quoted), MariaDB identifiers must start with a letter (A-Z, a-z) or an underscore (_), and can be followed by:

- Letters (A–Z, a–z)
- Digits (0–9)
- Underscores (_)
- Dollar signs ($)

## References

[1] https://aospinsight.com/secure-development-sanitizing-user-inputs/  
[2] https://blog.heycoach.in/data-sanitization-in-c-programs/  
[3] https://www.reddit.com/r/C_Programming/comments/kdmkan/sanitization_functions_written_in_c/  
[4] https://wiki.sei.cmu.edu/confluence/display/c/STR02-C.+Sanitize+data+passed+to+complex+subsystems  
[5] https://stackoverflow.com/questions/28413986/input-sanitisation-in-c  
[6] https://labex.io/tutorials/c-how-to-sanitize-user-input-safely-420440  
[7] https://cyber.gouv.fr/sites/default/files/2022/04/anssi-guide-rules_for_secure_c_language_software_development-v1.4.pdf  
[8] https://labex.io/tutorials/wireshark-how-to-implement-input-validation-and-sanitization-in-cybersecurity-417883  


