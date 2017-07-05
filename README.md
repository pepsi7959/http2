# HTTP/2
This library has been developed to transport GRPC packages or general data.
The main purpose is to gain performance by using minimum HTTP/2 feature. 

## Prerequisite
 * [protobuf-cpp](https://github.com/google/protobuf/releases/tag/v3.3.0)
 * [protobuf-c](https://github.com/protobuf-c/protobuf-c/releases/tag/v1.2.1)
 
## How To Install
 * Installing the protobuf-cpp
   - Download [protobuf-cpp-3.3.0.zip](https://github.com/google/protobuf/releases/download/v3.3.0/protobuf-cpp-3.3.0.zip)
   - Extract it 
     ```bash
     unzip protobuf-cpp-3.3.0.zip
     cd protobuf-3.3.0
     ```
   - Compile and Install
     ```bash
     ./autogen.sh
     ./configure
     make && make install
     ```
 * Installing the protobuf-c
   - set PKG_CONFIG_PATH (__DONOT__ forget this step)
      ```bash
      export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
      ```
   - Download [protobuf-c-1.2.1.tar.gz](https://github.com/protobuf-c/protobuf-c/releases/download/v1.2.1/protobuf-c-1.2.1.tar.gz)
   - Extract it
     ```bash
     tar -xzvf protobuf-c-1.2.1.tar.gz
     cd protobuf-c-1.2.1
     ```
   - Compile and install
     ```bash
     ./autogen.sh
     ./configure
     make && make install
     ```
   
## Example
 comming soon.
 
# GRPC
Besides http/2, The GRPC will be used for composing data. 

## Generate source files from .proto
  After installing the protobuf-cpp and protobuf-c, you can use `protoc-c` command.
  ```bash
  cd http2/proto
  protoc-c --c_out=. helloworld.proto
  ```
