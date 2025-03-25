# Use the Alpine base image
FROM alpine:latest AS build

# Install build tools
RUN apk add --no-cache g++ libstdc++ libgcc git

# Set the working directory inside the container
WORKDIR /app

# Copy the entire 'app' directory from the host into the container
#COPY ./app /
RUN git clone https://github.com/n0ctrnl/eqbcs
COPY eqbcs/EQBCS.cpp /app
COPY eqbcs/BCCore.cpp /app
COPY eqbcs/EQBCS.h /app

# Compile the C++ program
RUN g++ EQBCS.cpp BCCore.cpp -o eqbcs
RUN file="echo $(ls -lR /app)" && echo $file
RUN ldd /app/eqbcs

# Stage 2: Release
FROM alpine:latest
RUN apk add --no-cache libstdc++ libgcc
WORKDIR /app

COPY --from=build /app/eqbcs /app/eqbcs

# Set the default command to verify the compiled program
CMD ["/app/eqbcs"]
#CMD tail -f /dev/null

