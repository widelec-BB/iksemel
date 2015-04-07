/* iksemel (XML parser for Jabber)
** Copyright (C) 2000-2003 Gurer Ozen
** tls-cyassl.c (non-blocking SSL connection via CyaSSL.library)
** Copyright (C) 2014-2015 Filip 'widelec-BB' Maryjanski
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#include <sys/fcntl.h>
#include <proto/socket.h>
#include <proto/cyassl.h>
#include "common.h"
#include "iksemel.h"

extern struct Library *SocketBase, *CyaSSLBase;

struct ikstls_data
{
	CYASSL_CTX *ctx;
	CYASSL *ssl;
	ikstransport *trans;
	void *sock;
	int timeout;
};

static int cyassl_io_handshake(struct ikstls_data **datap, ikstransport *trans, void *sock)
{
	struct ikstls_data *data;

	*datap = NULL;

	if((data = iks_malloc(sizeof(*data))))
	{
		memset(data, 0x00, sizeof(*data));

		data->trans = trans;
		data->sock = sock;
		data->timeout = -1;

		*datap = data;

		if(CyaSSL_Init() == SSL_SUCCESS)
		{
			CYASSL_METHOD *m;

			CyaSSL_Debugging_OFF();

			if((m = CyaSSLv23_client_method()))
			{
				if((data->ctx = CyaSSL_CTX_new(m)))
				{
					CyaSSL_CTX_set_verify(data->ctx, SSL_VERIFY_NONE, 0);

					if((data->ssl = CyaSSL_new(data->ctx)))
					{
						int ret;

						CyaSSL_set_socketbase(data->ssl, SocketBase);
						CyaSSL_set_fd(data->ssl, (int)sock);

						if(trans)
							CyaSSL_set_using_nonblock(data->ssl, 1);

						ret = CyaSSL_connect(data->ssl);

						if(ret != SSL_SUCCESS)
						{
							int r = CyaSSL_get_error(data->ssl, ret);

							if(r == SSL_ERROR_WANT_READ || r == SSL_ERROR_WANT_WRITE)
								return IKS_AGAIN;
						}
						else
							return IKS_OK;
					}
				}
			}
		}
	} 

	return IKS_NOMEM;
}

static int cyassl_io_send(struct ikstls_data *data, const char *buf, size_t len)
{
	LONG total = 0, bytesleft = len, n;

	do
	{
		n = CyaSSL_send(data->ssl, buf + total, bytesleft, 0);

		if(n == -1)
		{
			int r = CyaSSL_get_error(data->ssl, n);

			if(r != SSL_ERROR_WANT_WRITE)
				return -1;
		}
		total += n;
		bytesleft -= n;
	}while(total < len);

	return IKS_OK;
}

static int cyassl_io_recv(struct ikstls_data *data, char *buf, size_t len, int timeout)
{
	LONG result;

	if(!buf || len == 0) /* no buffer, error */
		return -1;

	while(TRUE)
	{
		result = CyaSSL_recv(data->ssl, ((UBYTE*)buf), len, 0);

		if(result == -1)
		{
			int r = CyaSSL_get_error(data->ssl, result);

			if(r == SSL_ERROR_ZERO_RETURN || r == SSL_ERROR_WANT_READ || r == SSL_ERROR_WANT_READ)
				return 0;
			else
				return -1;
		}

		return result;
	}
}

static void cyassl_io_terminate(struct ikstls_data *data)
{
	CyaSSL_free(data->ssl);
	CyaSSL_CTX_free(data->ctx);
	CyaSSL_Cleanup();
	iks_free(data);
}


ikstls iks_default_tls =
{
	cyassl_io_handshake,
	cyassl_io_send,
	cyassl_io_recv,
	cyassl_io_terminate
};
