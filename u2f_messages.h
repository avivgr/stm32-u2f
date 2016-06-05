#ifndef __U2FMSG_H_INCLUDED__
#define __U2FMSG_H_INCLUDED__

#define VENDOR_U2F_NOMEM        0xCD01
#define VENDOR_U2F_VERSION "U2F_V2"

uint16_t u2f_register(U2F_REGISTER_REQ *req, U2F_REGISTER_RESP *resp, int flags, uint16_t *olen);
uint16_t u2f_authenticate(U2F_AUTHENTICATE_REQ *req, U2F_AUTHENTICATE_RESP *resp, int flags);

#endif