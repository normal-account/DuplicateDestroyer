# docker build -t duplicatedestroyer .
# docker run -it -d duplicatedestroyer /bin/bash

FROM ubuntu:22.04
WORKDIR /DuplicateDestroyer
COPY CMakeListsDockerfile.txt ./CMakeLists.txt
COPY src/ src/
COPY single_include/ single_include/
RUN mkdir cmake-build-debug
RUN apt-get -y update
RUN TZ=Canada/Eastern
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone # Set the timezone so no packages ask for it during install
RUN apt-get -y install tesseract-ocr libtesseract-dev libboost-all-dev libopencv-dev libgmp3-dev cmake g++ git libssl-dev vim 
RUN git clone https://github.com/mysql/mysql-connector-cpp.git
WORKDIR mysql-connector-cpp/
RUN git checkout -b 8.0 987fada391411f2e8d71688812674c6203ef253f
RUN cmake .
RUN cmake --build . --target install --config release
WORKDIR ../cmake-build-debug
RUN cmake ..
RUN make
RUN apt -y install mysql-server
# From there on, you only have to create the database from within the container
