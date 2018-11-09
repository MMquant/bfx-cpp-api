# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Stage 1 - Installing essential and utility pkgs.
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

FROM alpine:edge as runtime

# Environment variables
ENV ESSENTIAL_PACKAGES="cmake clang curl curl-dev gcc g++ git gzip make mlocate openssh py-pip tar supervisor" \
    UTILITY_PACKAGES="nano vim ca-certificates"

# Configure essential and utility packages
RUN apk update && \
    apk --no-cache --progress add $ESSENTIAL_PACKAGES $UTILITY_PACKAGES && \
    pip install --upgrade pip && \
    pip install supervisor-stdout

# compiling https://github.com/weidai11/cryptopp.git
RUN cd /tmp && \
    git clone https://github.com/weidai11/cryptopp.git && \
    cd cryptopp && make && make install

RUN mkdir -p /home/bfx-cpp-api

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Stage 2 - Applying needed configurations.
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

COPY ./docker/etc /etc

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Stage 3 - Adding project files into VM.
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# Adding project folder and needed files
ADD ./app /home/bfx-cpp-api/app
RUN chmod -R a+w /home/bfx-cpp-api/app

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Stage 4 - Adding entry point.
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# Adding entry point
ADD ./docker/entrypoint.sh /sbin/entrypoint.sh
