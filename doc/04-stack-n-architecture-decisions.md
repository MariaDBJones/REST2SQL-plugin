Here we discuss the various choices made during implementation of this API

## MariaDB plugin vs Maxscale plugin vs external app
### MariaDB Plugin
* Design Philosophy: A MariaDB plugin allows for direct integration with the database server, enabling custom functionality to be executed within the database environment.
* Features:
  * Direct Access: Provides direct access to the database's internals, allowing for efficient execution of SQL commands based on JSON input.
  * Performance: Since it runs within the MariaDB server, it can leverage existing resources and minimize latency associated with external calls.
  * Security: Benefits from the security context of the database server, allowing for controlled access to sensitive operations.
  * Complexity: Requires knowledge of MariaDB's plugin architecture and may involve more complex development and maintenance processes.
* Use Case: Ideal for scenarios where tight integration with the database is necessary, such as custom data processing or when needing to handle complex SQL operations directly.
### MaxScale
* Design Philosophy: MaxScale is a database proxy that provides advanced routing, load balancing, and monitoring capabilities between client applications and MariaDB servers.
* Features:
  * Routing Capabilities: Can intelligently route requests based on defined rules, making it suitable for handling JSON requests and forwarding them to the appropriate backend databases.
  * Load Balancing: Supports load balancing across multiple database servers, enhancing performance and availability.
  * Plugin Architecture: Offers a flexible plugin architecture that can be extended with custom filters or routers to process JSON data before sending SQL commands to the database.
  * Latency: Introduces some latency due to being an intermediary between clients and the database, which may impact performance in high-throughput scenarios.
* Use Case: Best suited for environments where multiple databases are in use and where intelligent routing and load balancing are required. It is also useful when you want to decouple application logic from database interactions.
### External Application
* Design Philosophy: An external application can be developed separately from the database server to handle JSON input and convert it into SQL commands.
* Features:
  * Flexibility: Allows for greater flexibility in design and implementation; developers can choose any programming language or framework best suited for their needs.
  * Separation of Concerns: Keeps the database server focused solely on data storage and retrieval while offloading processing logic to a dedicated application.
  * Scalability: Can be scaled independently of the database server, allowing for better resource management in high-load situations.
  * Complexity in Integration: Requires additional work to manage connections and ensure secure communication between the application and the database.
* Use Case: Ideal for applications that require complex business logic not easily implemented as a plugin or when integrating with other services (e.g., web applications) that need to handle JSON.
### Decision : 
* MariaDB plugin
  * Why : it has been designed as an easy & simple way to consume datas in a single database. so integrating tightly with thze database, allows for the best performance

## C vs Go vs RUST vs js
### C
* Design Philosophy: C is a low-level, procedural programming language that provides direct access to memory and system resources. It is designed for efficiency and control, making it suitable for systems programming.
* Features:
  * Performance: Offers high performance due to minimal runtime overhead and direct memory manipulation.
  * Portability: Code written in C can be compiled on various platforms with minimal changes.
  * Memory Management: Requires manual memory management, giving developers full control but increasing the risk of memory leaks and buffer overflows.
  * Low-Level Access: Provides low-level access to hardware and system resources, making it ideal for operating systems and embedded systems.
  * Learning Curve: The learning curve can be steep for beginners due to complex concepts like pointers and manual memory management.
* Use Case: Best suited for system-level programming, embedded systems, and performance-critical applications.
### Rust
* Design Philosophy: Rust is a systems programming language focused on safety and performance. It aims to provide memory safety without a garbage collector through its ownership model.
* Features:
  * Memory Safety: Enforces memory safety at compile time through ownership and borrowing rules, reducing runtime errors.
  * Performance: Comparable to C/C++ in terms of speed, with optimizations that allow for high-performance applications.
  * Concurrency: Supports safe concurrency through its ownership model, enabling developers to write concurrent code without data races.
  * Modern Syntax: Offers a modern syntax with features like pattern matching and type inference that enhance code readability.
  * Learning Curve: Has a steeper learning curve compared to Go due to its complex memory management concepts.
* Use Case: Ideal for performance-critical applications, systems programming, and scenarios requiring safe concurrency.
### Go
* Design Philosophy: Go (or Golang) is designed by Google for simplicity and efficiency in building scalable applications. It emphasizes ease of use and built-in support for concurrency.
* Features:
  * Simplicity: Features a simple syntax that promotes quick learning and development speed.
  * Concurrency Model: Built-in support for concurrency through goroutines and channels, allowing efficient handling of multiple tasks simultaneously.
  * Garbage Collection: Relies on automatic garbage collection for memory management, simplifying development but potentially impacting performance in some cases.
  * Fast Compilation: Known for fast compilation times, making it suitable for large codebases with frequent changes.
  * Learning Curve: Generally easier to learn than Rust due to its straightforward syntax and concepts.
* Use Case: Best suited for cloud-native applications, web services, and projects requiring high concurrency.
### JavaScript (Node.js)
* Design Philosophy: JavaScript is primarily known as a client-side scripting language but has gained popularity on the server side through Node.js. It focuses on asynchronous programming and event-driven architecture.
* Features:
  * Asynchronous I/O: Utilizes an event-driven, non-blocking I/O model that makes it efficient for handling I/O-heavy applications like web servers.
  * Cross-Platform: Runs on various platforms thanks to the V8 engine, which allows JavaScript to be executed outside the browser environment.
  * Rich Ecosystem: Offers a vast ecosystem of libraries and frameworks (e.g., Express.js) that facilitate rapid development.
  * Dynamic Typing: Uses dynamic typing, which can lead to flexibility but may introduce runtime errors if not managed carefully.
  * Learning Curve: Generally easy to learn for beginners due to its widespread use in web development and extensive resources available online.
* Use Case: Ideal for web development (both frontend and backend), real-time applications (e.g., chat apps), and rapid prototyping.
### Decision : 
* C
  * Why : MariaDB plugins are written in C also i knew the language already
   
## HTTPD
### libmicrohttpd
* Design Philosophy: libmicrohttpd is designed to be a small, fast, and flexible HTTP server library that can be easily integrated into applications. It aims to provide a simple API while maintaining compliance with HTTP standards.
* Features:
  * Lightweight: The library is compact and has a low memory footprint, making it ideal for embedded systems or applications with limited resources.
  * HTTP/1.1 Compliance: Fully compliant with the HTTP/1.1 specification, ensuring reliable communication.
  * Threading Modes: Supports multiple threading models, including thread-per-connection and thread pools, allowing developers to choose the best model for their application.
  * Polling Modes: Offers various socket polling modes (select(), poll(), epoll) to optimize performance based on the platform.
  * Incremental POST Data Processing: Supports incremental processing of POST data, which can be beneficial for handling large uploads.
* Use Case: Best suited for applications that require a lightweight HTTP server with basic functionality and flexibility in threading and polling.
### CivetWeb
* Design Philosophy: CivetWeb is designed as an easy-to-use web server that can be embedded into applications. It focuses on providing a simple API while supporting modern web standards.
* Features:
  * Embedded Web Server: Easily embeddable into C/C++ applications, allowing for quick integration of web server capabilities.
  * WebSocket Support: Provides support for WebSockets, making it suitable for real-time applications.
  * SSL/TLS Support: Includes built-in support for secure connections using SSL/TLS.
  * Cross-Platform: Works across various platforms, including Windows, Linux, and macOS.
  * Simple Configuration: Offers a straightforward configuration process with minimal setup required.
* Use Case: Ideal for applications that need an embedded web server with support for modern web features like WebSockets and secure connections.
### mongoose
* Design Philosophy: Mongoose is a versatile web server library that aims to provide a comprehensive solution for building web applications in C/C++. It focuses on ease of use while offering extensive features.
* Features:
  * Full-Featured HTTP Server: Supports both HTTP and HTTPS protocols, allowing developers to build secure web applications easily.
  * RESTful API Support: Facilitates the creation of RESTful APIs with built-in support for JSON handling.
  * Asynchronous I/O: Utilizes asynchronous I/O operations to handle multiple connections efficiently without blocking.
  * File Uploads and Downloads: Provides built-in support for handling file uploads and downloads seamlessly.
  * Extensive Documentation: Well-documented with examples that simplify the integration process.
* Use Case: Suitable for developers looking to create robust web applications or services that require extensive features and ease of integration.
### Decision : 
* libmicrohttpd
  * Why : mhd waqs the most lightweight library for a mariadb plugin, with reasonable ease of use but it does not natively support https so change might be incoming
  
## JSON
### cJSON
* Design Philosophy: cJSON is designed to be simple and lightweight. It is a single-file library that aims to provide a straightforward API for JSON parsing and generation.
* Features:
  * Minimalistic and easy to integrate into projects.
  * Supports basic JSON operations without enforcing strict data structures.
  * Does not perform extensive error checking, which can lead to issues if invalid JSON is processed.
* Memory Management: Users are responsible for managing memory, including deleting objects when they are no longer needed.
* Use Case: Best suited for projects where simplicity and low overhead are prioritized over extensive features.
### Jansson
* Design Philosophy: Jansson focuses on providing a more feature-rich API while maintaining ease of use. It is designed to be user-friendly with comprehensive documentation.
* Features:
  * Full Unicode support (UTF-8).
  * A more structured API that includes error handling and type safety.
  * Supports serialization and deserialization of JSON objects seamlessly.
  * Offers an extensive test suite for reliability.
  * Memory Management: Jansson handles memory management automatically, making it easier for developers to avoid memory leaks.
* Use Case: Ideal for applications requiring robust JSON handling with a focus on performance and reliability.
### PicoJSON
* Design Philosophy: PicoJSON is a header-only library that emphasizes minimalism and simplicity while being C++ oriented. It is designed for ease of use in C++ projects but can be used in C as well.
* Features:
  * Lightweight and easy to integrate; it consists solely of a single header file.
  * Focuses on parsing JSON with minimal overhead.
  * Lacks some advanced features found in other libraries, such as built-in support for UTF-8 or complex data structures.
  * Memory Management: Since it's header-only, memory management is straightforward but less flexible compared to Jansson.
* Use Case: Best suited for lightweight applications or embedded systems where simplicity and low memory footprint are critical.
### Decision :
* cJSON
  * Why : it was initally choosen for its lightweightness. might be replaced by Jansson for both JSON & JWT handdling

## JWT
### Jansson
* Design Philosophy: Jansson is primarily a JSON manipulation library but includes support for JWT creation and verification. It is designed to be simple and user-friendly.
* Features:
  * JSON Support: Provides robust functionality for handling JSON data, which is essential for creating and parsing JWTs.
  * Ease of Use: Offers a straightforward API that makes it easy to create and manipulate JWTs.
  * Claims Handling: Supports basic claim management, allowing users to add standard claims easily.
  * Memory Management: Automatic memory management helps prevent memory leaks, making it easier for developers.
* Use Case: Best suited for applications that require both JSON handling and JWT functionality without needing extensive features.
### jwt-c
* Design Philosophy: jwt-c is a lightweight library specifically focused on creating and verifying JWTs in C. It aims to provide a simple interface while maintaining security.
* Features:
  * Token Creation and Verification: Provides functions for generating and validating JWTs with various signing algorithms (e.g., HMAC, RSA).
  * Claims Support: Allows for adding standard claims easily, such as iss, exp, and sub.
  * Security Focus: Emphasizes secure token handling, making it suitable for production environments.
  * Learning Curve: Relatively easy to use, with clear documentation that facilitates quick integration.
* Use Case: Ideal for projects that need a dedicated JWT library without the overhead of additional features.
### libjwt
* Design Philosophy: libjwt is another dedicated library for working with JWTs in C. It focuses on providing a comprehensive set of features while remaining lightweight and efficient.
* Features:
  * Comprehensive JWT Support: Supports various signing algorithms (HMAC, RSA, ECDSA) and allows for easy token creation and verification.
  * Claims Management: Offers extensive claim handling capabilities, including custom claims.
  * Flexible API: Designed to be flexible, enabling developers to customize their JWT handling as needed.
  * Documentation and Community Support: Well-documented with examples, facilitating ease of use.
* Use Case: Suitable for applications that require robust JWT handling with flexibility in implementation.
### Decision
* jwt-c
  * Why : rest2sql is the only issuer & verifier, so no need for private/public keypair and ECDSA-like asymmetric algorithms. also server key is reclaculated at every restart, so lightweight + HMAC256 fels like a great choice.
