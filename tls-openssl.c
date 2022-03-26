/* iksemel (XML parser for Jabber)
** Copyright (C) 2010 Gurer Ozen
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#include "common.h"
#include "iksemel.h"

#ifdef HAVE_OPENSSL

#ifdef __MORPHOS__
#include <proto/socket.h>
#include <proto/exec.h>
#include <proto/dos.h>
extern struct Library *SysBase, *OpenSSL3Base, *SocketBase, *DOSBase;
#ifdef errno
#undef errno
#endif /* errno */
#define errno Errno()
#endif /* __MORPHOS__ */

#include <openssl/ssl.h>

struct ikstls_data
{
	SSL_CTX *ctx;
	SSL *ssl;
	ikstransport *trans;
	void *sock;
	int timeout;
};

static int tls_handshake(struct ikstls_data **datap, ikstransport *trans, void *sock)
{
	struct ikstls_data *data;
	int ssl_res;

	*datap = NULL;

	data = iks_malloc(sizeof(*data));
	if(!data)
		return IKS_NOMEM;

	data->trans = trans;
	data->sock = sock;
	data->timeout = -1;

	data->ctx = SSL_CTX_new(TLS_client_method());
	if(!data->ctx)
	{
		iks_free(data);
		return IKS_NOMEM;
	}

#ifdef __DEBUG__
	SSL_CTX_set_verify(data->ctx, SSL_VERIFY_NONE, NULL);
	tprintf("SSL SERVER CERTIFICATE VALIDATION DISABLED\n");
#endif /* __DEBUG__ */

	data->ssl = SSL_new(data->ctx);
	if(!data->ssl)
	{
		iks_free(data);
		SSL_CTX_free(data->ctx);
		return IKS_NOMEM;
	}

	SSL_set_fd(data->ssl, (int)data->sock);

	ssl_res = SSL_connect(data->ssl);
	if(ssl_res != 1)
	{
		int err = SSL_get_error(data->ssl, ssl_res);
		if(err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
		{
			*datap = data;
			return IKS_AGAIN;
		}
		else
		{
			SSL_free(data->ssl);
			SSL_CTX_free(data->ctx);
			iks_free(data);
			return IKS_NET_TLSFAIL;
		}
	}

	*datap = data;
	return IKS_OK;
}

static int tls_handshake_async(struct ikstls_data *data)
{
	int ssl_res;

	ssl_res = SSL_connect(data->ssl);
	if(ssl_res != 1)
	{
		int err = SSL_get_error(data->ssl, ssl_res);
		if(err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
			return IKS_AGAIN;

		return IKS_NET_TLSFAIL;
	}

	return IKS_OK;
}

static int tls_send(struct ikstls_data *data, const char *buf, size_t size)
{
	int total = 0, bytesleft = size;

	if(data->ssl == NULL || data->sock == (void*)-1)
		return IKS_NET_NOSOCK;

	if(!buf || size == 0)
		return IKS_NET_RWERR; /* nothing to send? */

	while(total < size)
	{
		int n = SSL_write(data->ssl, buf + total, bytesleft);

		if(n == -1)
		{
			int err = SSL_get_error(data->ssl, n);
			if(err == SSL_ERROR_SYSCALL && errno == EINTR)
				continue;
			if(err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
				return IKS_AGAIN;
		}

		total += n;
		bytesleft -= n;
	}

	return IKS_OK;
}

static int tls_recv(struct ikstls_data *data, char *buf, size_t size, int timeout)
{
	int n;

	while(1)
	{
		n = SSL_read(data->ssl, buf, size);

		if(n == -1)
		{
			int err = SSL_get_error(data->ssl, n);

			if(err == SSL_ERROR_SYSCALL && errno == EINTR)
				continue;
			if(err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ)
				return 0;
		}

		LEAVE();
		return n;
	}
}

static void tls_terminate(struct ikstls_data *data)
{
	SSL_shutdown(data->ssl);
	SSL_free(data->ssl);
	SSL_CTX_free(data->ctx);
	iks_free(data);
}

ikstls iks_default_tls =
{
	tls_handshake,
	tls_handshake_async,
	tls_send,
	tls_recv,
	tls_terminate
};

#ifdef __MORPHOS__
// libssl_shared references exit().
void exit(int)
{
	tprintf("exit() called\n");
	while(1)
		Wait(0);
}
#endif /* __MORPHOS__ */

#endif /* HAVE_OPENSSL */
