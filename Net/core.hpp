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

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <queue>
#include <unordered_map>

#include "socket.hpp"
#include "sock_types.hpp"

/*
sources:
	https://stackoverflow.com/questions/12982385/memory-handling-with-struct-epoll-event
	https://beej.us/guide/bgnet/pdf/bgnet_usl_c_1.pdf
	https://man7.org/linux/man-pages/man7/epoll.7.html (can also be seen with the command "man epoll")
    https://wiki.openssl.org/index.php/Simple_TLS_Server
*/

struct NetAction {
	net_action_t action;

	union {
		struct {
			char *buffer;
			size_t size;
			size_t progress;
		};

		int ping_data;
	};

	NetAction() : action(a_none), buffer(nullptr) {}

	void set_close() {
		action = a_close;
	}

	void set_send(char *buffer, size_t size) {
		action = a_send;
		this->buffer =  buffer;
		this->size = size;
		this->progress = 0;
	}

	void set_ping(int ping_data) {
		action = a_ping;
		this->ping_data = ping_data;
	}
};

struct Connection;
struct ConnectionSet;
struct ConnectionHandler;

struct ConnectionFunctions {
	void (*init)(ConnectionSet *all, ConnectionHandler *self);
	size_t (*get_receive_buffer)(ConnectionHandler *self, char **buffer);
	void (*on_receive)(ConnectionHandler *self, size_t received);
	void (*on_ping)(ConnectionHandler *self, int ping_data);
	void (*close)(ConnectionHandler *self);
};

struct ConnectionHandler {
	Connection *parent;
	void *data;

	void register_action(const NetAction &action);
	inline const AdressInfo &get_adress_info();

	const ConnectionFunctions *functions;

	inline ConnectionHandler(const ConnectionFunctions *functions) : functions(functions) {}
	inline ConnectionHandler() = default;

	template<typename T>
	inline T &get_data() const {
		return *((T *)data);
	}

	template<typename T>
	inline void get_data(T &p) const {
		p = *((T *)data);
	}
};

struct Connection {
	ConnectionHandler handler;
	Socket socket;

	bool issending;
	bool setsending;

	std::queue<NetAction> pending;

	inline Connection(const SocketFunctions *functions) : issending(false), setsending(false), socket(functions) {}
	Connection(const ConnectionHandler &other, const SocketFunctions *functions) : issending(false), setsending(false), socket(functions) {
		handler = other;
		handler.parent = this;
	}
	Connection(const Connection &other) : issending(false), setsending(false), socket(other.socket.functions) {
		handler = other.handler;
		handler.parent = this;
	}

	bool run_pending_actions(){
		issending = true;

		for (; !pending.empty();) {
			NetAction action = pending.front();

			
			if (action.action == a_send) {
				ssize_t sent = socket.write(action.buffer + action.progress, action.size - action.progress);

				if (sent == -1) return !socket_blocked();
				
				action.progress += sent;
				if (action.progress == action.size) pending.pop();
			} else  if (action.action == a_close) {
				return true;
			} else if (action.action == a_ping) {
				pending.pop();
				handler.functions->on_ping(&handler, action.ping_data);
			}
		}

		issending = false;
		return false;
	}
};

void ConnectionHandler::register_action(const NetAction &action) {
	parent->pending.push(action);
	if (action.action == a_send) {
		parent->setsending = true;
	}
}

inline const AdressInfo &ConnectionHandler::get_adress_info() {
	return parent->socket.get_addr_info();
}

struct Epoll {
	struct EpollEvent {
	private:
		epoll_event *ev;

	public:
		constexpr EpollEvent(epoll_event &ev) : ev(&ev) {}

		inline bool read() const {
			return ev->events & EPOLLIN;
		}

		inline bool write() const {
			return ev->events & EPOLLOUT;
		}

		inline int operator()() const {
			return ev->data.fd;
		}

		inline EpollEvent &operator=(EpollEvent other){
			ev = other.ev;
			return *this;
		}
	};

	int epoll_fd;
	epoll_event *event_list;
	int event_list_size;

	EpollEvent *simple_list;

	void init(int event_list_size){
		epoll_fd = epoll_create1(0);
		event_list = (epoll_event *)malloc(sizeof(epoll_event) * event_list_size);
		simple_list = (EpollEvent *)malloc(sizeof(EpollEvent) * event_list_size);
		for (int i = 0; i < event_list_size; i++) {
			epoll_event &ev = event_list[i];
			simple_list[i] = EpollEvent(ev);
		}
		this->event_list_size = event_list_size;
	}

	void add(int fd, int events) {
		epoll_event ev;
		ev.data.fd = fd;
		ev.events = events;

		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
	}

	void mod(int fd, int events) {
		epoll_event ev;
		ev.data.fd = fd;
		ev.events = events;

		epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
	}
	
	inline void remove(int fd) {
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	}

	inline int await() {
		return epoll_wait(epoll_fd, event_list, event_list_size, -1);
	}

	inline void erase(){
		close(epoll_fd);
		free(event_list);
	}

	inline int operator()(){
		return epoll_fd;
	}

	inline const EpollEvent &operator[](int index) {
		return simple_list[index];
	}
};

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
		Socket &socket = conn->socket;
		epoll.remove(socket());
		socket.erase();
		conn->handler.functions->close(&conn->handler);
		delete connections[socket()];
		connections.erase(socket());
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
		int event_count = epoll.await();
		if (event_count == -1) {
			return epoll_failed;
		}

		for (int i = 0; i < event_count; i++) {
			Connection *conn_i = connections[epoll[i]()]; 

			if (conn_i->socket.is_listening()) {
				if (connections.size() == max_connections) continue;

				Connection *new_conn = new Connection(*conn_i);
				if ((conn_i->socket.accept_new(&new_conn->socket))) {
					delete new_conn;
					continue;
				}

				new_conn->socket.make_non_blocking();
				new_conn->handler.functions->init(this, &new_conn->handler);

				add_connection(new_conn);
				set_write(new_conn);

				continue;
			}

			if (epoll[i].read()) {
				char *buffer;

				size_t max_size = conn_i->handler.functions->get_receive_buffer(&conn_i->handler, &buffer);
				ssize_t received = conn_i->socket.read(buffer, max_size);

				if (received == 0) close_connection(conn_i);
				else {
					conn_i->handler.functions->on_receive(&conn_i->handler, received);
					set_write(conn_i);
				}
			}

			if (epoll[i].write()) {
				if (conn_i->run_pending_actions()) close_connection(conn_i);
				else if (!conn_i->issending) epoll.mod(conn_i->socket(), EPOLLIN);
			}
		}


		return sock_ok;
	}

};

#endif