FROM alpine:3.20 

RUN apk add --no-cache \
    build-base \
    make

WORKDIR /app

COPY . .

RUN make

CMD ["./app"]

RUN make clean