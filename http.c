/* iksemel (XML parser for Jabber)
** Copyright (C) 2000-2003 Gurer Ozen
** http.c (HTTP client)
** Copyright (C) 2015 Filip 'widelec-BB' Maryjanski
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#include <stdlib.h>
#include "common.h"
#include "iksemel.h"
#include "http.h"

int iks_https_post(char *server, char *url, char *data, int data_len, char **response)
{
	char buffer[100];
	long int *sfd;
	int res = -1;
	struct ikstls_data *tlsdata;

	if(data_len == -1)
	{
		if(data)
			data_len = iks_strlen(data);
		else
			data_len = 0;
	}
#ifndef USE_DEFAULT_IO
	return IKS_NET_NOTSUPP;
#endif

	if(iks_default_tls.handshake == NULL)
		return IKS_NET_NOTSUPP;

	if((res = iks_default_transport.connect(NULL, (void**)&sfd, server, 443)) == IKS_OK)
	{
		if(iks_default_tls.handshake(&tlsdata, NULL, (void*)sfd) == IKS_OK)
		{
			snprintf(buffer, sizeof(buffer), "POST /%s HTTP/1.1\n", url);

			if((res = iks_default_tls.send(tlsdata, buffer, iks_strlen(buffer))) == IKS_OK)
			{
				snprintf(buffer, sizeof(buffer), "Host: www.%s\n", server);

				if((res = iks_default_tls.send(tlsdata, buffer, iks_strlen(buffer))) == IKS_OK)
				{
					snprintf(buffer, sizeof(buffer), "Content-length: %d\n", data_len);

					if((res = iks_default_tls.send(tlsdata, buffer, iks_strlen(buffer))) == IKS_OK)
					{
						snprintf(buffer, sizeof(buffer), "Content-Type: application/x-www-form-urlencoded\n\n");

						if((res = iks_default_tls.send(tlsdata, buffer, iks_strlen(buffer))) == IKS_OK)
						{
							if(!data || (res = iks_default_tls.send(tlsdata, data, data_len)) == IKS_OK)
							{
								int res_len = -1, pos = sizeof(buffer), content = 0;
								char line[100];

								memset(buffer, 0x00, sizeof(buffer));

								do
								{
									int i;

									memcpy(buffer, buffer + pos, sizeof(buffer) - pos);

									memset(buffer + pos, 0x00, sizeof(buffer) - pos);

									res = iks_default_tls.recv(tlsdata, buffer + sizeof(buffer) - pos, sizeof(buffer) - (sizeof(buffer) - pos), 0);

									for(i = pos = 0; i < sizeof(buffer) && buffer[i] != 0x00; i++)
									{
										if(buffer[i] == '\n')
										{
											memcpy(line, buffer + pos, i - pos);
											line[i - pos] = 0x00;
											pos = i + 1;

											if(strncasecmp(line, "content-length", 14) == 0)
												res_len = atoi(line + 16);

											if(line[0] == '\r')
											{
												content = 1;
												break;
											}
										}
									}

									if(pos >= sizeof(buffer) || buffer[pos] == 0x00)
										pos = sizeof(buffer);

								}while(!content && res > 0);

								if(content && response)
								{
									if(res_len == -1)
										res_len = 1000;

									if((*response = iks_malloc(res_len + 1)))
									{
										if(pos < sizeof(buffer))
										{
											memcpy(*response, buffer + pos, sizeof(buffer) - pos);
											res = iks_default_tls.recv(tlsdata, *response + sizeof(buffer) - pos, res_len - (sizeof(buffer) - pos), 0);
										}
										else
											res = iks_default_tls.recv(tlsdata, *response, res_len, 0);

										(*response)[sizeof(buffer) - pos + res] = 0x00;
										res = IKS_OK;
									}
								}
							}
						}
					}
				}
			}
		}
		iks_default_tls.terminate(tlsdata);
		iks_default_transport.close((void*)sfd);
	}

	return res;
}
