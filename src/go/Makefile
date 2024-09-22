
EXECUTABLES = \
	simple-client

all: $(EXECUTABLES)

simple-client: simple-client.go
	go build $^

.PHONY: clean format
clean:
	-rm -f $(EXECUTABLES)

format:
	gofmt -w *.go
