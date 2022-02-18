#include <server/net/socket.hpp>

namespace StandardSocket
{
    void construct(Socket *self) {
        SocketData *data = new SocketData;
        data->isListening = false;
        self->data = data;
    }

    void deconstruct(Socket *self) {
        SocketData &data = self->get_data<SocketData>();
        free(&data);
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
				freeaddrinfo(res);
				return sock_setsockopt_failed;
	        }

			if (bind(data.sock_fd, i->ai_addr, i->ai_addrlen) == -1) {
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

	bool isblocked(Socket *self) {
		return errno == EAGAIN || errno == EWOULDBLOCK;
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
		functions.isblocked = isblocked;
        functions.get_fd = get_fd;
        functions.get_addr_info = get_addr_info;
        functions.is_listening = is_listening;

        return functions;
    }
}

namespace SSLSocket
{
	//https://www.openssl.org/docs/manmaster/man3/TLSv1_2_server_method.html
	//https://www.openssl.org/docs/manmaster/man3/
	//https://wiki.openssl.org/index.php/Simple_TLS_Server

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
        out_data.last_ret = SSL_set_fd(out_data.ssl, out_data.sock_fd);

		out_data.last_ret = SSL_accept(out_data.ssl);
		if (out_data.last_ret < 0) return sock_accept_failed;

		if (out_data.sock_fd == -1) return sock_accept_failed;
		return sock_ok;
	}

	ssize_t read(Socket *self, char *buffer, size_t buff_size) {
        SocketData &data = self->get_data<SocketData>();
		data.last_ret = SSL_read(data.ssl, buffer, buff_size);

		if (data.last_ret < 0) {
			int err = SSL_get_error(data.ssl, data.last_ret);

			if (err == SSL_ERROR_ZERO_RETURN) return 0;
			else return -1;
		}

		return data.last_ret;
	}

	ssize_t write(Socket *self, const char *buffer, size_t buff_size) {
        SocketData &data = self->get_data<SocketData>();
		return data.last_ret = SSL_write(data.ssl, buffer, buff_size);
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

	bool isblocked(Socket *self) {
		const SocketData &data = self->get_data<SocketData>();
		
		int err = SSL_get_error(data.ssl, data.last_ret);

		if (err == SSL_ERROR_ZERO_RETURN || err == SSL_ERROR_NONE ||
			err == SSL_ERROR_SYSCALL || err == SSL_ERROR_SSL) return false;
		
		return true;
	}

	void erase(SocketFunctions &functions) {
		ConstSocketData *data = (ConstSocketData *)functions.sock_constants;
		SSL_CTX_free(data->ctx);
		delete data;
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
		functions.isblocked = isblocked;
        functions.get_fd = get_fd;
        functions.get_addr_info = get_addr_info;
        functions.is_listening = is_listening;

        return functions;
    }

	void init() {
		SSL_load_error_strings();	
		OpenSSL_add_ssl_algorithms();
	}

	void erase_all() {
		EVP_cleanup();
	}
}