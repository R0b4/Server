#ifndef INCLUDE_SOCK_TYPES
#define INCLUDE_SOCK_TYPES

enum sock_type {
	client, server
};

enum sock_status {
	sock_ok = 0,
	sock_no_addr_info,
	sock_setsockopt_failed,
	sock_not_bound,
	sock_non_blocking_failed,
	sock_listen_failed,
	sock_select_failed,
	sock_accept_failed,
	epoll_failed,
	exceeded_max_connections
};

constexpr int amount_action_types = 4;
enum net_action_t {
	a_none = 0, a_close, a_send, a_ping
};

enum sock_t {
	s_standard, s_ssl
};

#endif