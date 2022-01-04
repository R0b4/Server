#ifndef INCLUDE_SOCKET
#define INCLUDE_SOCKET

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <server/net/sock_types.hpp>

struct AdressInfo {
	sockaddr peer_addr;
	socklen_t peer_addr_len;
};

struct Socket;

struct SocketFunctions {
	void *sock_constants;

	void (*construct)(Socket *self);
    void (*deconstruct)(Socket *self);

	sock_status (*bind_new_socket)(Socket *self, const char *port);
	sock_status (*start_listen)(Socket *self, int max_backlog);
	void (*make_non_blocking)(Socket *self);
	sock_status (*accept_new)(Socket *self, Socket *out);
	ssize_t (*read)(Socket *self, char *buffer, size_t buff_size);
	ssize_t (*write)(Socket *self, const char *buffer, size_t buff_size);
	bool (*isblocked)(Socket *self);
	void (*erase)(Socket *self);

	int (*get_fd)(const Socket *self);
	const AdressInfo &(*get_addr_info)(const Socket *self);
	bool (*is_listening)(const Socket *self);
};

struct Socket {
	const SocketFunctions *functions;
	sock_t type;

	void *data;

	Socket(const SocketFunctions *functions) : functions(functions) {
		functions->construct(this);
	}

	~Socket() {
		functions->deconstruct(this);
	}

	inline sock_status bind_new_socket(const char *port) {
		return functions->bind_new_socket(this, port);
	}
	inline sock_status start_listen(int max_backlog) {
		return functions->start_listen(this, max_backlog);
	}
	inline void make_non_blocking(){
		return functions->make_non_blocking(this);
	}
	inline sock_status accept_new(Socket *out){
		return functions->accept_new(this, out);
	}
	inline ssize_t read(char *buffer, size_t buff_size){
		return functions->read(this, buffer, buff_size);
	}
	inline ssize_t write(const char *buffer, size_t buff_size){
		return functions->write(this, buffer, buff_size);
	}
	inline bool isblocked() {
		return functions->isblocked(this);
	}
	inline void erase() {
		return functions->erase(this);
	}
	inline void deconstruct() {
		return functions->deconstruct(this);
	}

	inline int operator()() const {
		return functions->get_fd(this);
	}
	inline const AdressInfo &get_addr_info() const {
		return functions->get_addr_info(this);
	}
	inline bool is_listening() const {
		return functions->is_listening(this);
	}

	template<typename T>
    inline T &get_data() const {
        return *((T *)data);
    }

	template<typename T>
    inline T &get_const_data() const {
        return *((T *)functions->sock_constants);
    }
};

namespace StandardSocket
{
    struct SocketData {
        int sock_fd;
        bool isListening;

        AdressInfo adress;
    };

    void construct(Socket *self);
    void deconstruct(Socket *self);

    sock_status bind_new_socket(Socket *self, const char *port);
	sock_status start_listen(Socket *self, int max_backlog);
	void make_non_blocking(Socket *self);

	sock_status accept_new(Socket *self, Socket *out);

	ssize_t read(Socket *self, char *buffer, size_t buff_size);
	ssize_t write(Socket *self, const char *buffer, size_t buff_size);

	void erase(Socket *self);

	inline int get_fd(const Socket *self);
    const AdressInfo &get_addr_info(const Socket *self);
	bool is_listening(const Socket *self);
	bool isblocked(Socket *self);

    SocketFunctions get();
}

namespace SSLSocket
{
	//https://www.openssl.org/docs/manmaster/man3/TLSv1_2_server_method.html
	//https://www.openssl.org/docs/manmaster/man3/
	//https://wiki.openssl.org/index.php/Simple_TLS_Server

	enum ssl_version {
		ssl23, ssl3, tls, tls1, tls11, tls12
	};

	constexpr const SSL_METHOD *(*ssl_method_funcs[])() = {
		SSLv23_server_method,
		SSLv23_server_method,
		TLS_server_method,
		TLS_server_method,
		TLS_server_method,
		TLS_server_method
	};

	struct ConstSocketData {
		SSL_CTX *ctx;
	};

    struct SocketData {
        int sock_fd;
        bool isListening;

		SSL *ssl;
		int last_ret;

        AdressInfo adress;
    };

    void construct(Socket *self);
    void deconstruct(Socket *self);

    sock_status bind_new_socket(Socket *self, const char *port);
	sock_status start_listen(Socket *self, int max_backlog);
	void make_non_blocking(Socket *self);

	sock_status accept_new(Socket *self, Socket *out);

	ssize_t read(Socket *self, char *buffer, size_t buff_size);
	ssize_t write(Socket *self, const char *buffer, size_t buff_size);

	void erase(Socket *self);

	inline int get_fd(const Socket *self);
    const AdressInfo &get_addr_info(const Socket *self);
	bool is_listening(const Socket *self);

	SSL_CTX *create_context(ssl_version version);
	void configure_context(SSL_CTX *ctx, const char *cert_location, const char *private_key_location);
	
	bool isblocked(Socket *self);

	void erase(SocketFunctions &functions);

    SocketFunctions get(ssl_version version, const char *cert_location, const char *private_key_location);

	void init();
	void erase_all();
}

#endif