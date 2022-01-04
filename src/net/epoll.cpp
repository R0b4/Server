#include <server/net/epoll.hpp>

void Epoll::init(int event_list_size){
    epoll_fd = epoll_create1(0);
    event_list = (epoll_event *)malloc(sizeof(epoll_event) * event_list_size);
    simple_list = (EpollEvent *)malloc(sizeof(EpollEvent) * event_list_size);
    for (int i = 0; i < event_list_size; i++) {
        epoll_event &ev = event_list[i];
        simple_list[i] = EpollEvent(ev);
    }
    this->event_list_size = event_list_size;
}

void Epoll::add(int fd, int events) {
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

void Epoll::mod(int fd, int events) {
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;

    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}