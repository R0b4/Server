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

#include <server/net/socket.hpp>
#include <server/net/epoll.hpp>
#include <server/net/sock_types.hpp>
#include <server/utils.hpp>

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
	bool issending;
	bool setsending;

	ConnectionHandler handler;
	Socket socket;

	size_t progress;
	bool isclosing;
	std::queue<string_view> pending;

	clock_t on_last_event;

	Connection(const SocketFunctions *functions);
	Connection(const ConnectionHandler &other, const SocketFunctions *functions);
	Connection(const Connection &other);

	bool test_timeout();
	void set_timeout();

	bool run_pending_actions();
};

inline const AdressInfo &ConnectionHandler::get_adress_info() {
	return parent->socket.get_addr_info();
}

struct ConnectionSet {
	std::unordered_map<int, Connection *> connections;

	Epoll epoll;
	int max_connections;

	ConnectionSet(int max_connections);

	void add_connection(Connection *c);
	void close_connection(Connection *conn);

	sock_status start_listener(const char *port, int max_backlog, ConnectionFunctions *handler, SocketFunctions *socket);
	void set_write(Connection *conn);

	sock_status handle();

};

#endif