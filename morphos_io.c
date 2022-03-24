/* iksemel (XML parser for Jabber)
** Copyright (C) 2000-2003 Gurer Ozen
** morphos_io.c (non-blocking socket connection for MorphOS)
** Copyright (C) 2014-2015 Filip 'widelec-BB' Maryjanski
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#include <proto/socket.h>

#include "common.h"
#include "iksemel.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <proto/socket.h>
#include <sys/filio.h>
#include <sys/ioctl.h>
#include <errno.h>

#ifdef errno
#undef errno
#endif /* errno */

#define errno Errno()
#define bzero(b, l) memset(b, 0x00, l)
#define UNUSED __attribute__((unused))

extern struct Library *SocketBase;

static int morphos_io_connect(iksparser *prs, void **socketptr, const char *server, int port)
{
	struct sockaddr_in addrname = {0};
	LONG soc; 
	struct hostent *he = (struct hostent *)gethostbyname((const UBYTE*)server);

	*socketptr = (void*) -1;

	if((soc = socket(AF_INET, SOCK_STREAM, 0)) >= 0)
	{
		addrname.sin_port = htons(port);
		addrname.sin_family = AF_INET;
		addrname.sin_addr = *((struct in_addr *) he->h_addr);

		if(connect(soc, (struct sockaddr*)&addrname, sizeof(addrname)) != -1)
		{
			*socketptr = (void*)soc;
			return IKS_OK;
		}
		else
			return IKS_NET_UNKNOWN;
	}
	return IKS_NET_UNKNOWN;
}

static int morphos_io_send(void *soc, const char *buf, size_t len)
{
	LONG total = 0, bytesleft = len, n;

	if((LONG)soc == -1)
		return IKS_NET_NOSOCK;
	
	if(!buf || len == 0)
		return IKS_NET_RWERR; /* nothing to send? */

	while(total < len)
	{
		n = send((LONG)soc, ((const UBYTE*)buf) + total, bytesleft, 0);

		if(n == -1 && Errno() != EINTR)
			return -1;

		total += n;
		bytesleft -= n;
	}

	return IKS_OK;
}

static int morphos_io_recv(void *soc, char *buf, size_t len, UNUSED int timeout)
{
	LONG result;

	if((LONG)soc == -1)
		return -1;

	if(!buf || len == 0) /* no buffer, error */
		return -1;

	while(TRUE)
	{
		result = recv((LONG)soc, ((UBYTE*)buf), len, 0);

		if(result == -1 && Errno() == EINTR)
			continue;

		if(result == -1 && Errno() == EWOULDBLOCK)
			return 0;

		return result;
	}
}

static void morphos_io_close(void *soc)
{
	if((LONG)soc != -1)
		CloseSocket((LONG)soc);
}

static int morphos_io_connect_async(iksparser *prs, void **socketptr, const char *server, const char *server_name, int port, UNUSED void *notify_data, UNUSED iksAsyncNotify *notify_func)
{
	*socketptr = (void*) -1;

	if(server)
	{
		LONG soc;

		if((soc = socket(AF_INET, SOCK_STREAM, 0)) != -1)
		{
			struct sockaddr_in my_addr = {0};

			my_addr.sin_family = AF_INET;

			if(bind(soc, (struct sockaddr *)&my_addr, sizeof(my_addr)) != -1)
			{
				LONG non_block = 1;

				if(IoctlSocket(soc, FIONBIO, (caddr_t)&non_block) != -1)
				{
					struct sockaddr_in addrname = {0};
					struct hostent *he;

					if((he = gethostbyname((const UBYTE*)server)))
					{
						LONG res;

						addrname.sin_port = htons(port);
						addrname.sin_family = he->h_addrtype;
						addrname.sin_addr = *((struct in_addr*) he->h_addr);

						res = connect(soc, (struct sockaddr*)&addrname, sizeof(struct sockaddr_in));

						if(res == 0 || Errno() == EINPROGRESS)
						{
							*socketptr = (void*)soc;
							return IKS_OK;
						}
						return IKS_NET_NOCONN;
					}
					return IKS_NET_NODNS;
				}
				return IKS_NET_NOTSUPP;
			}
			CloseSocket(soc);
			return IKS_NET_DROPPED;
		}
		return IKS_NET_NOSOCK;
	}
	return IKS_NET_UNKNOWN;
}

ikstransport iks_default_transport =
{
	IKS_TRANSPORT_V1,
	morphos_io_connect,
	morphos_io_send,
	morphos_io_recv,
	morphos_io_close,
	morphos_io_connect_async
};

