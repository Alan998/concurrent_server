// one thread per client server
// it is a trivial change from sequential-server, just call the serve_connection
// function using a goroutine
package main

import (
	"flag"
	"fmt"
	"net"
)

const BUFF_SIZE = 1024

// enum type Process_State
type Process_State int8

const (
	WAIT_FOR_MSG Process_State = iota
	IN_MSG
)

func serve_connection(conn net.Conn) {
	// close connection after job finishes
	defer conn.Close()

	// send start symbol '*' to client and start receiving data from client
	_, err := conn.Write([]byte("*"))
	if err != nil {
		fmt.Println(err)
		return
	}

	process_state := WAIT_FOR_MSG
	buf := make([]byte, BUFF_SIZE)
	for {
		// read incoming data
		data_len, err := conn.Read(buf)
		if data_len == 0 {
			// end connection
			fmt.Println("peer done")
			break
		}
		if err != nil {
			fmt.Println(err)
			return
		}

		// start data symbol '^', end data symbol '$'
		for _, ch := range buf[:data_len] {
			if process_state == WAIT_FOR_MSG {
				if ch == '^' {
					process_state = IN_MSG
				}
			} else {
				if ch == '$' {
					process_state = WAIT_FOR_MSG
					continue
				}

				// add 1 to each character received and send it back to client
				_, err := conn.Write([]byte{ch + 1})
				if err != nil {
					fmt.Println(err)
					return
				}
			}
		}
	}
}

func handle_connection(server_name string) {
	// listen for incoming connections on host:port
	server, err := net.Listen("tcp", server_name)
	if err != nil {
		fmt.Println(err)
		return
	}

	// accept incoming connections
	for {
		conn, err := server.Accept()
		if err != nil {
			fmt.Println(err)
			continue
		}

		// get the client's address
		client_addr := conn.RemoteAddr().String()
		fmt.Println("client connected from: ", client_addr)
		go serve_connection(conn)
	}
}

func main() {
	host := flag.String("host", "localhost", "host name")
	port := flag.String("port", "8080", "connection port used")
	flag.Parse()

	fmt.Println("Press <Ctrl + c> to quit")
	fmt.Println("Serving on port: ", *port)

	server_name := fmt.Sprintf("%v:%v", *host, *port)
	handle_connection(server_name)
	return
}
