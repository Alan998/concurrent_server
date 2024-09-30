
EXECUTABLES = \
	simple-client \
	sequential-server \
	threaded-server \
	pool-server

all: $(EXECUTABLES)

simple-client: simple-client.go
	go build $^

sequential-server: sequential-server.go
	go build $^

threaded-server: threaded-server.go
	go build $^

pool-server: threadpool-server/threadpool.go threadpool-server/threadpool-server.go
	cd threadpool-server && go build -o threadpool-server threadpool.go threadpool-server.go

.PHONY: clean format
clean:
	-rm -f $(EXECUTABLES)
	-rm -f threadpool-server/threadpool-server

format:
	gofmt -w *.go
