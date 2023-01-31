# Overview

## Get Started

* Download source code
    ```bash
         git clone --recursive https://github.com/open-telemetry/opentelemetry-cpp

         # executable file will be installed at /usr/local/bin
         # lib will be saved at /usr/local/lib
         # header files will be saved at /usr/local/include
    ```

* Install protobuf and protobuf-compiler
    ```bash
        # This command will install libprotobuf-dev(libprotobuf.a) and protobuf-compiler(protoc)
        # Note that protc is different protobuf, the former is compiler, the later is lib
        # Note that this libprotobuf-dev only contains some header files, link needs source file (implementation)
        sudo apt install -y protobuf-compiler

        # Check
        protoc --version
    ```
* Install protobuf source code
    ```bash
        #set -e
        [ -z "${PROTOBUF_VERSION}" ] && export PROTOBUF_VERSION="3.6.1"

        # Make sure that the PROTOBUF_VERSION must be same with protoc and libprotcbuf.a
        sudo ./ci/install_protobuf.sh
    ```

* Build examples
    ```bash
        cmake -DBUILD_TESTING=OFF -DWITH_EXAMPLES_HTTP=ON -DWITH_ZIPKIN=ON ..

        # Enable WITH_OTLP to let cmake execute find_package(Protobuf)
        sudo cmake -DBUILD_TESTING=OFF -DWITH_OTLP=ON -DWITH_OTLP_GRPC=ON ..

        cmake --build . --target example_zipkin 
        cmake --build . --target client
    ```

* Start a observable backend (here is zipkin)
    ```bash
        docker run -d -p 9411:9411 openzipkin/zipkin
    ```

## Q & A
* Protobuf compiler version 3.21.6 doesn't match library version 3.6.1
    ```bash
        Warning info:
            # CMake Warning at /usr/share/cmake-3.16/Modules/FindProtobuf.cmake:499 (message):
            # Protobuf compiler version 3.21.6 doesn't match library version 3.6.1
            # Call Stack (most recent call first):
            # CMakeLists.txt:307 (find_package)
            # -- Found Protobuf: /usr/local/lib/libprotobuf.so;-lpthread (found version "3.6.1") 

        # Find all paths containing libprotobuf.so
        find /usr -name libprotobuf.so 
        sudo apt-get remove protobuf-compiler
    ```
    * [ref](https://stackoverflow.com/questions/56704546/protobuf-compiler-version-doesnt-match-library-version-3-6-1-when-not-using-s)
    * [ref2](https://www.linuxquestions.org/questions/linux-newbie-8/protobuf-compiler-version-3-0-0-doesn%27t-match-library-version-2-6-1-aspera-transfer-sdk-4175684062/)
    * [Protocol Buffer Compiler Installation](https://grpc.io/docs/protoc-installation/)
        