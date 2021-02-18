FROM nvcr.io/nvidia/deepstream:5.0.1-20.09-triton

RUN apt-get update \
  && apt-get install -y ssh

RUN apt-get install -y locate cmake net-tools git curl libcurl3-dev libcpprest-dev libjson-glib-dev libopenblas-dev protobuf-compiler libprotobuf-dev protobuf-compiler ffmpeg libzmq3-dev

RUN pip3 install pyzmq

RUN useradd -m user && yes password | passwd user

CMD ["echo", "Hello World...!"]

