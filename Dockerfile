FROM ubuntu:24.04

# Install dependencies
RUN apt update && apt install -y g++ cmake make libmysqlclient-dev curl git

WORKDIR /app

COPY . .

RUN cmake -S . -B build && cmake --build build

EXPOSE 8080

CMD ["./build/kv_server"]