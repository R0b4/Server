#ifndef INCLUDE_CORE
#define INCLUDE_CORE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <fcntl.h>

#include <queue>
#include <unordered_map>

#include "socket.hpp"
#include "epoll.hpp"
#include "sock_types.hpp"
#include "../utils.hpp"

#include <set>

/*
sources:
	https://stackoverflow.com/questions/12982385/memory-handling-with-struct-epoll-event
	https://beej.us/guide/bgnet/pdf/bgnet_usl_c_1.pdf
	https://man7.org/linux/man-pages/man7/epoll.7.html (can also be seen with the command "man epoll")
    https://wiki.openssl.org/index.php/Simple_TLS_Server
*/

struct Connection;
struct ConnectionSet;
struct ConnectionHandler;

struct ConnectionFunctions {
	void *handle_constants;

	void (*init)(ConnectionSet &all, ConnectionHandler &self);
	size_t (*get_receive_buffer)(ConnectionHandler &self, char **buffer);
	void (*on_receive)(ConnectionHandler &self, size_t received);
	void (*on_sent)(ConnectionHandler &self);
	void (*close)(ConnectionHandler &self);
};

struct ConnectionHandler {
	Connection *parent;
	void *data;

	const ConnectionFunctions *functions;

	inline ConnectionHandler(const ConnectionFunctions *functions) : functions(functions) {}
	inline ConnectionHandler() = default;

	void add_sent(string_view str);
	void add_close();

	inline const AdressInfo &get_adress_info();

	template<typename T>
	inline T &get_data() const {
		return *((T *)data);
	}

	template<typename T>
	inline void get_data(T &p) const {
		p = *((T *)data);
	}

	template<typename T>
    inline T &get_const_data() const {
        return *((T *)functions->handle_constants);
    }
};

struct Connection {
	ConnectionHandler handler;
	Socket socket;

	bool issending;
	bool setsending;

	bool isclosing;
	size_t progress;
	std::queue<string_view> pending;

	clock_t on_last_event;

	bool test_timeout(){
		clock_t now = clock();
		clock_t diff;
		if (now < on_last_event) {
			diff = on_last_event - now;
		} else {
			diff = now - on_last_event;
		}

		return diff > (3 * CLOCKS_PER_SEC);
	}

	void set_timeout(){
		on_last_event = clock();
	}

	inline Connection(const SocketFunctions *functions) : issending(false), setsending(false), socket(functions), progress(0), isclosing(false) {
		//pending.init();
		set_timeout();
	}
	Connection(const ConnectionHandler &other, const SocketFunctions *functions) : issending(false), setsending(false), socket(functions), progress(0), isclosing(false) {
		//pending.init();
		set_timeout();
		handler = other;
		handler.parent = this;
	}
	Connection(const Connection &other) : issending(false), setsending(false), socket(other.socket.functions), progress(0), isclosing(false) {
		//pending.init();
		set_timeout();
		handler = other.handler;
		handler.parent = this;
	}

	~Connection() {
		//pending.erase();
	}

	bool run_pending_actions(){
		issending = true;

		if (pending.empty()) {
			if (isclosing) return true;
			else {
				handler.functions->on_sent(handler);
				issending = !pending.empty();
				if (!issending) return false; 
			}
		}

		string_view front = pending.front() + progress;
		ssize_t sent = socket.write(front.str, front.size);

		if (sent == -1) {
			printf("connection blocked.\n");
			return !socket.isblocked();
		}
		
		if (sent == front.size) {
			progress = 0;
			pending.pop();
		} else {
			progress += sent;
		}

		return false;
	}
};

void ConnectionHandler::add_sent(string_view str) {
	parent->pending.push(str);
	parent->setsending = true;
}
void ConnectionHandler::add_close() {
	parent->isclosing = true;
}

inline const AdressInfo &ConnectionHandler::get_adress_info() {
	return parent->socket.get_addr_info();
}

struct ConnectionSet {
	std::unordered_map<int, Connection *> connections;

	Epoll epoll;
	int max_connections;

	ConnectionSet(int max_connections) : max_connections(max_connections){
		epoll.init(max_connections);
	}

	void add_connection(Connection *c){
		connections[c->socket()] = c;
		epoll.add(c->socket(), EPOLLIN);
	}

	void close_connection(Connection *conn) {		
		int fd = conn->socket();
		Socket &socket = conn->socket;
		epoll.remove(fd);
		socket.erase();
		conn->handler.functions->close(conn->handler);
		delete connections[fd];
		connections.erase(fd);
	}

	sock_status start_listener(const char *port, int max_backlog, ConnectionFunctions *handler, SocketFunctions *socket) {
		if (connections.size() == max_connections) return exceeded_max_connections;

		Connection *c = new Connection(ConnectionHandler(handler), socket);
		sock_status status;
		if ((status = c->socket.bind_new_socket(port))) {
			delete c;
			return status;
		}
		if ((status = c->socket.start_listen(max_backlog))){
			c->socket.erase();
			delete c;
			return status;
		}

		c->socket.make_non_blocking();

		add_connection(c);

		return sock_ok;
	}

	void set_write(Connection *conn) {
		if (!conn->setsending || conn->issending) return;

		conn->setsending = false;
		conn->issending = true;

		epoll.mod(conn->socket(), EPOLLIN | EPOLLOUT);
	}

	sock_status handle() {
		for (auto &conn : connections) {
			if (conn.second->socket.is_listening()) continue;

			if (conn.second->test_timeout()){
				close_connection(conn.second);
			}
		}

		int event_count = epoll.await();
		if (event_count == -1) {
			return epoll_failed;
		}

		for (int i = 0; i < event_count; i++) {
			Connection *conn_i = connections[epoll[i]()]; 
			conn_i->set_timeout();

			if (conn_i->socket.is_listening()) {
				if (connections.size() == max_connections) continue;

				Connection *new_conn = new Connection(*conn_i);
				if ((conn_i->socket.accept_new(&new_conn->socket))) {
					delete new_conn;
					continue;
				}

				new_conn->socket.make_non_blocking();
				new_conn->handler.functions->init(*this, new_conn->handler);

				add_connection(new_conn);
				set_write(new_conn);

				continue;
			}

			if (epoll[i].read()) {
				char *buffer;

				size_t max_size = conn_i->handler.functions->get_receive_buffer(conn_i->handler, &buffer);
				ssize_t received = conn_i->socket.read(buffer, max_size);

				if (received <= 0) close_connection(conn_i);
				else {
					conn_i->handler.functions->on_receive(conn_i->handler, received);
					set_write(conn_i);

					if (conn_i->run_pending_actions()) close_connection(conn_i);
					else if (!conn_i->issending) epoll.mod(conn_i->socket(), EPOLLIN);
				}
			}

			else if (epoll[i].write()) {
				if (conn_i->run_pending_actions()) close_connection(conn_i);
				else if (!conn_i->issending) epoll.mod(conn_i->socket(), EPOLLIN);
			}
		}


		return sock_ok;
	}

};

#endif