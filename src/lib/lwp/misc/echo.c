/* 
 * server.c
 *
 * lwp (echo) connection handler server
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SP_ACCEPT	3
#define SP_READER	3

#include "lwp.h"

struct context {
	struct lwpProc *us;
	struct sockaddr_in addr;
	int	fd;
};

/*ARGSUSED*/
int readConn(argc, argv, ud)
	int	argc;
	char	**argv;
	void	*ud;
{
	struct	context *ctx = (struct context *) ud;
	char	buf[1024];
	int	n;

	while (1) {
		printf("sleeping\n");
		lwpSleepFd(ctx->fd, LWP_FD_READ);
		printf("waiting to read\n");
		if ((n = read(ctx->fd, buf, sizeof(buf))) <= 0)
			break;
		printf("got %d char\n", n);
		lwpSleepFd(ctx->fd, LWP_FD_WRITE);
		printf("waiting to write\n");
		if (write(ctx->fd, buf, n) < 0)
			break;
		printf("wrote %d char\n", n);
	}
	printf("process/fd %d exiting\n", ctx->fd);
	close(ctx->fd);
	lwpExit();
	/*NOTREACHED*/
}

int acceptConn(argc, argv)
	int	argc;
	char	**argv;
{
	struct	sockaddr_in sin;
	int	s;
	int	ns;
	int	len;
	int	maxfd;
	struct context *ctx;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s port\n", *argv);
		exit(-1);
	}
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("inet socket");
		exit(-1);
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons(atoi(argv[1]));
	sin.sin_addr.s_addr = 0;
	if (bind(s, &sin, sizeof(sin)) < 0) {
		perror("inet socket bind");
		exit(-1);
	}
	if (listen(s, LISTENMAXCONN) < 0) {
		perror("inet socket listen");
		exit(-1);
	}
	maxfd = getdtablesize() - 1;
	while (1) {
		lwpSleepFd(s, LWP_FD_READ);
		len = sizeof(sin);
		ns = accept(s, &sin, &len);
		if (ns < 0) {
			perror("accept");
			exit(-1);
		}
		if (ns == maxfd) {
			fprintf(stderr, "no more connections");
			close(ns);
		}
		printf("got connection from %s\n", inet_ntoa(sin.sin_addr));
		ctx = (struct context *) malloc(sizeof(*ctx));
		ctx->addr = sin;
		ctx->fd = ns;
		ctx->us = lwpCreate(SP_READER, readConn, 8192, 0, 0, ctx);
	}
	/*NOTREACHED*/
}

int main(argc, argv)
	int	argc;
	char	**argv;
{
	lwpInitSystem(1);
	lwpCreate(SP_ACCEPT, acceptConn, 8192, argc, argv, 0);
	lwpReschedule();
	/*NOTREACHED*/
}
