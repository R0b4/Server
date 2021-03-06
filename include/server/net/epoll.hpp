#ifndef INCLUDE_CORE_EPOLL
#define INCLUDE_CORE_EPOLL

#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>

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

	void init(int event_list_size);
	void add(int fd, int events);
	void mod(int fd, int events);
	
	inline void remove(int fd) {
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	}

	inline int await() {
		return epoll_wait(epoll_fd, event_list, event_list_size, -1);
	}

	inline void erase(){
		close(epoll_fd);
		free(event_list);
		free(simple_list);
	}

	inline int operator()(){
		return epoll_fd;
	}

	inline const EpollEvent &operator[](int index) {
		return simple_list[index];
	}
};

#endif