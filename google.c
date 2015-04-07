/* iksemel (XML parser for Jabber)
** Copyright (C) 2000-2003 Gurer Ozen
** google.c (Google Jabber/XMPP extensions)
** Copyright (C) 2015 Filip 'widelec-BB' Maryjanski
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/
#include "common.h"
#include "iksemel.h"
#include "http.h"
#include "urlencode.h"
#include "google.h"

int iks_google_fetch_token(const char *code, const char *client_id, const char *client_secret, char **access_token, char **refresh_token)
{
	int result = -1;
	int data_len = 0;
	char *data;

	data_len += iks_strlen("code=");
	data_len += iks_urlencode_len(code);
	data_len += iks_strlen("&client_id=");
	data_len += iks_urlencode_len(client_id);
	data_len += iks_strlen("&client_secret=");
	data_len += iks_urlencode_len(client_secret);
	data_len += iks_strlen("&redirect_uri=urn%3Aietf%3Awg%3Aoauth%3A2.0%3Aoob%3Aauto&grant_type=authorization_code");

	if((data = iks_malloc(data_len + 1)))
	{
		int res;
		char *response = NULL, *t;

		t = strcopy("code=", data);
		t = iks_urlencode_copy(code, t);
		t = strcopy("&client_id=", t);
		t = iks_urlencode_copy(client_id, t);
		t = strcopy("&client_secret=", t);
		t = iks_urlencode_copy(client_secret, t);
		t = strcopy("&redirect_uri=urn%3Aietf%3Awg%3Aoauth%3A2.0%3Aoob%3Aauto&grant_type=authorization_code", t);

		*t = 0x00;

		if((res = iks_https_post("googleapis.com", "oauth2/v3/token", data, data_len, &response)) == IKS_OK)
		{
			if(response)
			{
				if(!strstr(response, "error"))
				{
					char *token, *end;

					result = 0;

					if(access_token && (token = strstr(response, "\"access_token\":")))
					{
						token += 15;

						while(*token && *token++ != '"');

						if(*token)
						{
							end = token;

							while(*end && *end++ != '"');

							if(*end--)
							{
								*end = 0x00;
								*access_token = iks_strdup(token);
								*end = '"';
							}
						}
					}

					if(refresh_token && (token = strstr(response, "\"refresh_token\":")))
					{
						token += 16;

						while(*token && *token++ != '"');

						if(*token)
						{
							end = token;

							while(*end && *end++ != '"');

							if(*end--)
							{
								*end = 0x00;
								*refresh_token = iks_strdup(token);
								*end = '"';
							}
						}
					}
				}
				iks_free(response);
			}
		}
		iks_free(data);
	}
	return result;
}

int iks_google_renew_token(const char *refresh_token, const char *client_id, const char *client_secret, char **access_token)
{
	int result = -1;
	int data_len = 0;
	char *data;

	data_len += iks_strlen("client_id=");
	data_len += iks_urlencode_len(client_id);
	data_len += iks_strlen("&client_secret=");
	data_len += iks_urlencode_len(client_secret);
	data_len += iks_strlen("&refresh_token=");
	data_len += iks_urlencode_len(refresh_token);
	data_len += iks_strlen("&grant_type=refresh_token");

	if((data = iks_malloc(data_len + 1)))
	{
		int res;
		char *response = NULL, *t;

		t = strcopy("client_id=", data);
		t = iks_urlencode_copy(client_id, t);
		t = strcopy("&client_secret=", t);
		t = iks_urlencode_copy(client_secret, t);
		t = strcopy("&refresh_token=", t);
		t = iks_urlencode_copy(refresh_token, t);
		t = strcopy("&grant_type=refresh_token", t);

		*t = 0x00;

		if((res = iks_https_post("googleapis.com", "oauth2/v3/token", data, data_len, &response)) == IKS_OK)
		{
			if(response)
			{
				if(!strstr(response, "error"))
				{
					char *token, *end;

					result = 0;

					if(access_token && (token = strstr(response, "\"access_token\":")))
					{
						token += 15;

						while(*token && *token++ != '"');

						if(*token)
						{
							end = token;

							while(*end && *end++ != '"');

							if(*end--)
							{
								*end = 0x00;
								*access_token = iks_strdup(token);
								*end = '"';
							}
						}
					}
				}
				iks_free(response);
			}
		}

		iks_free(data);
	}

	return result;
}

iks *iks_google_make_mail_request(iksid *id, const char *newer_time, const char *newer_tid)
{
	iks *res;

	if((res = iks_make_iq(IKS_TYPE_GET, "google:mail:notify")))
	{
		iks_insert_attrib(res, "from", id->full);
		iks_insert_attrib(res, "to", id->partial);
		iks_insert_attrib(res, "id", "mail-request-1");

		if(newer_time || newer_tid)
		{
			iks *query;

			if((query = iks_child(res)))
			{
				if(newer_time)
					iks_insert_attrib(query, "newer-than-time", newer_time);
				if(newer_tid)
					iks_insert_attrib(query, "newer-than-tid", newer_tid);
			}
		}
	}

	return res;
}

iks *iks_google_make_mail_response(iksid *id)
{
	iks *res;

	if((res = iks_make_iq(IKS_TYPE_RESULT, "mail-request-2")))
	{
		iks_insert_attrib(res, "from", id->partial);
		iks_insert_attrib(res, "to", id->full);
	}

	return res;
}
