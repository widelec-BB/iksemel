#ifndef __GOOGLE_H__
#define __GOOGLE_H__

#include "iksemel.h"

int iks_google_fetch_token(const char *code, const char *client_id, const char *client_secret, char **access_token, char **refresh_token);
int iks_google_renew_token(const char *refresh_token, const char *client_id, const char *client_secret, char **access_token);
iks *iks_google_make_mail_request(iksid *id, const char *newer_time, const char *newer_tid);
iks *iks_google_make_mail_response(iksid *id);

#endif /* __GOOGLE_H__ */
