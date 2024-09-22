/*	Simple client used to interact with servers
*
*	Launches N concurrent client connections, each executing a pre-set sequence
*	of sends to the server, and logs what was received back.
 */
package main

import (
	"flag"
	"fmt"
	"net"
	"sync"
	"time"
)

func check_end_connection(count *int, buf_data string) bool {
	const end_conn_len = 4
	for _, val := range buf_data {
		if val == '1' {
			*count++
			if *count == end_conn_len {
				return true
			}
		} else {
			*count = 0
		}
	}
	return false
}

func recv(client_name string, conn net.Conn, connect_wg *sync.WaitGroup) {
	defer connect_wg.Done()
	count := 0
	empty_buf := make([]byte, 1024)
	for {
		buf := empty_buf
		data_len, err := conn.Read(buf)
		if err != nil {
			fmt.Println(err)
			return
		}

		// Print data received
		buf_data := fmt.Sprintf("%s", buf[:data_len])
		fmt.Printf("%v received %v\n", client_name, buf_data)

		// if fullbuf contains '1111', it means the server will close the connection
		if check_end_connection(&count, buf_data) {
			break
		}
	}
}

func send(client_name string, send_data string, conn net.Conn, sleep_time time.Duration) {
	// send data to server
	send_data_byte := []byte(send_data)
	fmt.Printf("%v sending %v\n", client_name, send_data)
	_, err := conn.Write(send_data_byte)
	if err != nil {
		fmt.Println(err)
		return
	}
	time.Sleep(sleep_time)
}

func make_connection(client_name string, server_name string, main_wg *sync.WaitGroup) {
	defer main_wg.Done()

	// create a single socket connection to server
	// server_name is host:port
	conn, err := net.Dial("tcp", server_name)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer conn.Close()

	// server will first send a '*' before client can start sending
	// wait for server to response to start
	initial := make([]byte, 1)
	if _, err := conn.Read(initial); err != nil || initial[0] != '*' {
		fmt.Printf("%s: something is wrong! Did not receive *", client_name)
		return
	}

	// client receive socket wait group
	var connect_wg sync.WaitGroup
	connect_wg.Add(1)
	go recv(client_name, conn, &connect_wg)

	fmt.Printf("%v connected...\n", client_name)

	send_data := "^abc$de^abte$f"
	sleep_time := time.Second * 1
	send(client_name, send_data, conn, sleep_time)
	send_data = "xyz^123"
	send(client_name, send_data, conn, sleep_time)

	// The 0000 sent to the server here will result in an echo of 1111
	// which is a sign for the reading thread to terminate
	sleep_time = time.Millisecond * 200
	send_data = "25$^ab0000$abab"
	send(client_name, send_data, conn, sleep_time)

	// wait for client socket to end connection
	connect_wg.Wait()
	fmt.Printf("%v disconnecting...\n", client_name)
}

func main() {
	// parse flags
	n_connection := flag.Uint("n", 1, "launch n client connections")
	port := flag.Int("port", 8080, "connection port used")
	host := flag.String("host", "localhost", "Server host name")
	flag.Parse()
	fmt.Printf("launching %v connections connecting to %v:%v\n", *n_connection,
		*host, *port)

	var main_wg sync.WaitGroup

	// start timer
	start_time := time.Now()

	// launch n connections
	for i := uint(0); i < *n_connection; i++ {
		main_wg.Add(1)
		client_name := fmt.Sprintf("client_%v", i)
		server_name := fmt.Sprintf("%v:%v", *host, *port)
		go make_connection(client_name, server_name, &main_wg)
	}
	main_wg.Wait()

	// stop timer
	stop_time := time.Now()
	elapsed_time := stop_time.Sub(start_time)

	fmt.Println("Elapsed time: ", elapsed_time)
}
