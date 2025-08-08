## To compile, install and launch your MariaDB plugin as an external library without compiling the entire MariaDB server, follow these steps:

* download [Dockerfile](https://github.com/SylvainA77/REST2SQL-plugin/blob/dev/docker/rockylinux/Dockerfile) 
* build once : docker build --no-cache -t json2sql:latest .   
  *  Container is built with all the tools and scripts
  *  check env variables in case you want to change the cloned branch  
* run many : docker run -it json2sql
  * make the command exeuctables : chmod +x /opt/*  
  *  To clone a branch in /app : clone
  *  To build in /app/plugin : build
  *  To deploy the lib from /app/plugin and start mariadb daemon : deploy
  *  To test ping : curl -i http://127.0.0.1:3000/v1/ping or simply  pingz
  *  To test status : curl -i http://127.0.0.1:3000/v1/status or simply status
  *  To cleanup for a new build : clean  

## Sources :  
[1] https://mariadb.org/installing-plugins-in-the-mariadb-docker-library-container/  
[2] https://docs.tiledb.com/mariadb/installation-from-source  
[3] https://github.com/pluots/sql-udf  
[4] https://mariadb.com/kb/en/generic-build-instructions/  
[5] https://mariadb.com/kb/en/compiling-mariadb-from-source/  
[6] https://stackoverflow.com/questions/61047894/building-mariadb-connector-c-library-as-a-submodule  
[7] https://mariadb.com/kb/en/development-writing-plugins-for-mariadb/  
[8] https://www.jetify.com/docs/devbox/devbox_examples/databases/mariadb/  
