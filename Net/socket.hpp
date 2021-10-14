#ifndef INCLUDE_SOCKET
#define INCLUDE_SOCKET

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <fcntl.h>

#include "sock_types.hpp"

inline bool socket_blocked() { return errno == EAGAIN || errno == EWOULDBLOCK; }

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

	sock_status bind_new_socket(const char *port) {
		return functions->bind_new_socket(this, port);
	}
	sock_status start_listen(int max_backlog) {
		return functions->start_listen(this, max_backlog);
	}
	void make_non_blocking(){
		return functions->make_non_blocking(this);
	}
	sock_status accept_new(Socket *out){
		return functions->accept_new(this, out);
	}
	ssize_t read(char *buffer, size_t buff_size){
		return functions->read(this, buffer, buff_size);
	}
	ssize_t write(const char *buffer, size_t buff_size){
		return functions->write(this, buffer, buff_size);
	}
	void erase() {
		return functions->erase(this);
	}

	inline int operator()() const {
		return functions->get_fd(this);
	}
	const AdressInfo &get_addr_info() const {
		return functions->get_addr_info(this);
	}
	bool is_listening() const {
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

    void construct(Socket *self) {
        SocketData *data = new SocketData;
        data->isListening = false;
        self->data = data;
    }

    void deconstruct(Socket *self) {
        SocketData &data = self->get_data<SocketData>();
        delete &data;
    }

    sock_status bind_new_socket(Socket *self, const char *port) {
        SocketData &data = self->get_data<SocketData>();
		struct addrinfo hints;

		memset(&hints, 0, sizeof hints);
	    hints.ai_family = AF_UNSPEC;
	    hints.ai_socktype = SOCK_STREAM;
	    hints.ai_flags = AI_PASSIVE;

		data.sock_fd = 0;

		struct addrinfo* res;

		if (getaddrinfo(NULL, port, &hints, &res)) {
			return sock_no_addr_info;
		}

		int one = 1;
		data.sock_fd = -1;

		for (addrinfo *i = res; i; i = i->ai_next) {
			if ((data.sock_fd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1) {
				continue;
			}

			if (setsockopt(data.sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
				self->erase();
				return sock_setsockopt_failed;
	        }

			if (bind(data.sock_fd, i->ai_addr, i->ai_addrlen) == -1) {
				self->erase();
				continue;
			}

			break;
		}

		freeaddrinfo(res);

		if (data.sock_fd == -1) return sock_not_bound;

		return sock_ok;
	}

	sock_status start_listen(Socket *self, int max_backlog) {
        SocketData &data = self->get_data<SocketData>();
		data.isListening = true;

		if (listen(data.sock_fd, max_backlog) == -1){
			return sock_listen_failed;
		}

		return sock_ok;
	}

	void make_non_blocking(Socket *self) {
        SocketData &data = self->get_data<SocketData>();
		fcntl(data.sock_fd, F_SETFL, O_NONBLOCK);
	}

	sock_status accept_new(Socket *self, Socket *out) {
        SocketData &data = self->get_data<SocketData>();
        SocketData &out_data = out->get_data<SocketData>();

		out_data.sock_fd = accept(data.sock_fd, &out_data.adress.peer_addr, &out_data.adress.peer_addr_len);

		if (out_data.sock_fd == -1) return sock_accept_failed;
		return sock_ok;
	}

	ssize_t read(Socket *self, char *buffer, size_t buff_size) {
        SocketData &data = self->get_data<SocketData>();
		return recv(data.sock_fd, buffer, buff_size, 0);
	}

	ssize_t write(Socket *self, const char *buffer, size_t buff_size) {
        SocketData &data = self->get_data<SocketData>();
		return send(data.sock_fd, buffer, buff_size, 0);
	}

	void erase(Socket *self){
        SocketData &data = self->get_data<SocketData>();
		close(data.sock_fd);
	}

	inline int get_fd(const Socket *self) {
        const SocketData &data = self->get_data<SocketData>();
		return data.sock_fd;
	}

    const AdressInfo &get_addr_info(const Socket *self) {
		const SocketData &data = self->get_data<SocketData>();
        return data.adress;
	}

	bool is_listening(const Socket *self) {
		const SocketData &data = self->get_data<SocketData>();
        return data.isListening;
	}

    SocketFunctions get(){
        SocketFunctions functions;

        functions.construct = construct;
        functions.deconstruct = deconstruct;
        functions.bind_new_socket = bind_new_socket;
        functions.start_listen = start_listen;
        functions.make_non_blocking = make_non_blocking;
        functions.accept_new = accept_new;
        functions.read = read;
        functions.write = write;
        functions.erase = erase;
        functions.get_fd = get_fd;
        functions.get_addr_info = get_addr_info;
        functions.is_listening = is_listening;

        return functions;
    }

	void erase(SocketFunctions &functions){

	}
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
		TLSv1_server_method,
		TLSv1_1_server_method,
		TLSv1_2_server_method
	};

	struct ConstSocketData {
		SSL_CTX *ctx;
	};

    struct SocketData {
        int sock_fd;
        bool isListening;

		SSL *ssl;

        AdressInfo adress;
    };

    void construct(Socket *self) {
        SocketData *data = new SocketData;
        data->isListening = false;
		data->ssl = nullptr;
        self->data = data;
    }

    void deconstruct(Socket *self) {
        SocketData &data = self->get_data<SocketData>();
        delete &data;
    }

    sock_status bind_new_socket(Socket *self, const char *port) {
        SocketData &data = self->get_data<SocketData>();
		struct addrinfo hints;

		memset(&hints, 0, sizeof hints);
	    hints.ai_family = AF_UNSPEC;
	    hints.ai_socktype = SOCK_STREAM;
	    hints.ai_flags = AI_PASSIVE;

		data.sock_fd = 0;

		struct addrinfo* res;

		if (getaddrinfo(NULL, port, &hints, &res)) {
			return sock_no_addr_info;
		}

		int one = 1;
		data.sock_fd = -1;

		for (addrinfo *i = res; i; i = i->ai_next) {
			if ((data.sock_fd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1) {
				continue;
			}

			if (setsockopt(data.sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
				self->erase();
				return sock_setsockopt_failed;
	        }

			if (bind(data.sock_fd, i->ai_addr, i->ai_addrlen) == -1) {
				self->erase();
				continue;
			}

			break;
		}

		freeaddrinfo(res);

		if (data.sock_fd == -1) return sock_not_bound;

		return sock_ok;
	}

	sock_status start_listen(Socket *self, int max_backlog) {
        SocketData &data = self->get_data<SocketData>();
		data.isListening = true;

		if (listen(data.sock_fd, max_backlog) == -1){
			return sock_listen_failed;
		}

		return sock_ok;
	}

	void make_non_blocking(Socket *self) {
        SocketData &data = self->get_data<SocketData>();
		fcntl(data.sock_fd, F_SETFL, O_NONBLOCK);
	}

	sock_status accept_new(Socket *self, Socket *out) {
        SocketData &data = self->get_data<SocketData>();
        SocketData &out_data = out->get_data<SocketData>();

		ConstSocketData &const_data = self->get_const_data<ConstSocketData>();

		out_data.sock_fd = accept(data.sock_fd, &out_data.adress.peer_addr, &out_data.adress.peer_addr_len);

		out_data.ssl = SSL_new(const_data.ctx);
        SSL_set_fd(out_data.ssl, client);

		if (SSL_accept(out_data.ssl) < 0) return sock_accept_failed;

		if (out_data.sock_fd == -1) return sock_accept_failed;
		return sock_ok;
	}

	ssize_t read(Socket *self, char *buffer, size_t buff_size) {
        SocketData &data = self->get_data<SocketData>();
		return SSL_read(data.ssl, buffer, buff_size);
	}

	ssize_t write(Socket *self, const char *buffer, size_t buff_size) {
        SocketData &data = self->get_data<SocketData>();
		return SSL_write(data.ssl, buffer, buff_size);
	}

	void erase(Socket *self){
        SocketData &data = self->get_data<SocketData>();
		close(data.sock_fd);
		if (data.ssl) {
			SSL_shutdown(data.ssl);
			SSL_free(data.ssl);
		}
	}

	inline int get_fd(const Socket *self) {
        const SocketData &data = self->get_data<SocketData>();
		return data.sock_fd;
	}

    const AdressInfo &get_addr_info(const Socket *self) {
		const SocketData &data = self->get_data<SocketData>();
        return data.adress;
	}

	bool is_listening(const Socket *self) {
		const SocketData &data = self->get_data<SocketData>();
        return data.isListening;
	}

	void init() {
		SSL_load_error_strings();	
		OpenSSL_add_ssl_algorithms();
	}

	SSL_CTX *create_context(ssl_version version) {
		const SSL_METHOD *method = ssl_method_funcs[version]();

		SSL_CTX *ctx = SSL_CTX_new(method);

		if (!ctx) {
			perror("Unable to create SSL context");
			ERR_print_errors_fp(stderr);
			exit(EXIT_FAILURE);
		}

		return ctx;
	}

	void configure_context(SSL_CTX *ctx, const char *cert_location, const char *private_key_location)
	{
		SSL_CTX_set_ecdh_auto(ctx, 1);

		/* Set the key and cert */
		if (SSL_CTX_use_certificate_file(ctx, cert_location, SSL_FILETYPE_PEM) <= 0) {
			ERR_print_errors_fp(stderr);
			exit(EXIT_FAILURE);
		}

		if (SSL_CTX_use_PrivateKey_file(ctx, private_key_location, SSL_FILETYPE_PEM) <= 0 ) {
			ERR_print_errors_fp(stderr);
			exit(EXIT_FAILURE);
		}
	}

    SocketFunctions get(ssl_version version, const char *cert_location, const char *private_key_location){
        SocketFunctions functions;

		ConstSocketData *data = new ConstSocketData;

		data->ctx = create_context(version);
		configure_context(data->ctx, cert_location, private_key_location);

		functions.sock_constants = data;

        functions.construct = construct;
        functions.deconstruct = deconstruct;
        functions.bind_new_socket = bind_new_socket;
        functions.start_listen = start_listen;
        functions.make_non_blocking = make_non_blocking;
        functions.accept_new = accept_new;
        functions.read = read;
        functions.write = write;
        functions.erase = erase;
        functions.get_fd = get_fd;
        functions.get_addr_info = get_addr_info;
        functions.is_listening = is_listening;

        return functions;
    }

	void erase(SocketFunctions &functions) {
		ConstSocketData *data = (ConstSocketData *)functions.sock_constants;
		SSL_CTX_free(data->ctx);
		delete data;
	}

	void erase_all() {
		EVP_cleanup();
	}
}

#endif