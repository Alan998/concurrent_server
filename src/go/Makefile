
EXECUTABLES = \
	simple-client \
	sequential-server

all: $(EXECUTABLES)

simple-client: simple-client.go
	go build $^

sequential-server: sequential-server.go
	go build $^

.PHONY: clean format
clean:
	-rm -f $(EXECUTABLES)

format:
	gofmt -w *.go
