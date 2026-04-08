# Use Rocky Linux as the base image
# change this to comile on your favored linux flavour
FROM rockylinux/rockylinux:8 AS build

# Add a label for the image author
LABEL maintainer="sylvain@arbaudie.it"

# Install necessary tools and libraries
RUN dnf update -y && yum install -y epel-release
RUN dnf install -y git gcc gcc-c++ cmake curl net-tools cjson-devel procps-ng
RUN dnf --enablerepo=devel install -y libmicrohttpd-devel

#housekeeping time
RUN yum clean all

# Add MariaDB 11.4 repository and install it + devel headers + housekeeping
RUN curl -sSL https://downloads.mariadb.com/MariaDB/mariadb_repo_setup | bash -s -- --mariadb-server-version="mariadb-11.4" && \
    yum install -y MariaDB-server MariaDB-client MariaDB-devel && yum clean all

# Set the working directory to /opt
WORKDIR /opt

# Add /opt to the $PATH for easy calling from inside the container
ENV PATH="${PATH}:/opt"

# setting the branch name we want to clone
ENV GITBRANCH=dev

# now creating basic config file & utility scripts for easy dev & test

# minimal config
RUN printf "[mariadb]\nplugin-load-add=librest2sql\nplugin_maturity=unknown\nskip-name-resolve \n [rest2sql] \n rest2sql_address=127.0.0.1 \n rest2sql_port=3000 \n" > /etc/rest2sql.cnf

# utilities
RUN printf "#!/bin/bash \n rm -rf /app && mkdir -p /app \n cd /app \n git clone -b \"${1:-$GITBRANCH}\" --single-branch --depth 1 https://github.com/MariaDBJones/REST2SQL-plugin.git . \n mkdir build \n cd build \n cmake .. \n cd /app/build \n cmake --build . --verbose \n" > build
RUN printf "#!/bin/bash \n cp -f /app/plugin/librest2sql.so /usr/lib64/mysql/plugin/ \n mariadbd-safe --defaults-extra-file=/etc/rest2sql.cnf & \n sleep 3 \n grep -i plugin /var/lib/mysql/*.err \n" > deploy
RUN printf "#!/bin/bash \n mariadb -e 'shutdown;' \n rm -f /usr/lib64/mysql/plugin/librest2sql.so \n rm -rf /app \n mkdir -p /app" > clean

# test scripts
RUN printf "#!/bin/bash \n curl -i -X GET http://127.0.0.1:3000/v1/status" > status
RUN printf "#!/bin/bash \n curl -i -X GET http://127.0.0.1:3000/v1/ping" > pingz

# make scripts executables
RUN chmod -R +x /opt

# Set the working directory to /app
WORKDIR /app

# Expose the default rest2sql port
EXPOSE 3000

# Start command for the container (MariaDB will not auto-start)
CMD ["/bin/bash"]
