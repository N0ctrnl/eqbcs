# Use the Alpine base image
FROM alpine:latest AS build

# Install build tools
RUN apk add --no-cache g++ libstdc++ libgcc

# Set the working directory inside the container
WORKDIR /app

# Copy source files
COPY ./EQBCS.cpp /app
COPY ./BCCore.cpp /app
COPY ./EQBCS.h /app

# Compile eqbcs program
RUN g++ EQBCS.cpp BCCore.cpp -o eqbcs
RUN file="echo $(ls -lR /app)" && echo $file

# Stage 2: Release
FROM alpine:latest
RUN apk add --no-cache libstdc++ libgcc
WORKDIR /app

COPY --from=build /app/eqbcs /app/eqbcs

# Server default port is 2112
CMD ["/app/eqbcs"]
