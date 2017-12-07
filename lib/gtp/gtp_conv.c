#define TRACE_MODULE _gtp_conv

#include "core_debug.h"
#include "core_network.h"

#include "gtp_message.h"
#include "gtp_types.h"
#include "gtp_conv.h"

void gtp_bearers_in_create_indirect_tunnel_request(
        tlv_bearer_context_t *bearers[][GTP_MAX_NUM_OF_INDIRECT_TUNNEL],
        gtp_create_indirect_data_forwarding_tunnel_request_t *req)
{

    (*bearers)[0] = &req->bearer_context_0;
    (*bearers)[1] = &req->bearer_context_1;
    (*bearers)[2] = &req->bearer_context_2;
    (*bearers)[3] = &req->bearer_context_3;
    (*bearers)[4] = &req->bearer_context_4;
    (*bearers)[5] = &req->bearer_context_5;
    (*bearers)[6] = &req->bearer_context_6;
    (*bearers)[7] = &req->bearer_context_7;
    (*bearers)[8] = &req->bearer_context_8;
    (*bearers)[9] = &req->bearer_context_9;
    (*bearers)[10] = &req->bearer_context_10;
}

void gtp_bearers_in_create_indirect_tunnel_response(
        tlv_bearer_context_t *bearers[][GTP_MAX_NUM_OF_INDIRECT_TUNNEL],
        gtp_create_indirect_data_forwarding_tunnel_response_t *rsp)
{
    (*bearers)[0] = &rsp->bearer_context_0;
    (*bearers)[1] = &rsp->bearer_context_1;
    (*bearers)[2] = &rsp->bearer_context_2;
    (*bearers)[3] = &rsp->bearer_context_3;
    (*bearers)[4] = &rsp->bearer_context_4;
    (*bearers)[5] = &rsp->bearer_context_5;
    (*bearers)[6] = &rsp->bearer_context_6;
    (*bearers)[7] = &rsp->bearer_context_7;
    (*bearers)[8] = &rsp->bearer_context_8;
    (*bearers)[9] = &rsp->bearer_context_9;
    (*bearers)[10] = &rsp->bearer_context_10;
}

status_t gtp_f_teid_to_sockaddr(
    gtp_f_teid_t *f_teid, c_uint16_t port, c_sockaddr_t **list)
{
    c_sockaddr_t *addr = NULL, *addr6 = NULL;

    d_assert(f_teid, return CORE_ERROR,);
    d_assert(list, return CORE_ERROR,);

    addr = core_calloc(1, sizeof(c_sockaddr_t));
    d_assert(addr, return CORE_ERROR,);
    addr->c_sa_family = AF_INET;
    addr->c_sa_port = htons(port);

    addr6 = core_calloc(1, sizeof(c_sockaddr_t));
    d_assert(addr6, return CORE_ERROR,);
    addr6->c_sa_family = AF_INET6;
    addr6->c_sa_port = htons(port);

    if (f_teid->ipv4 && f_teid->ipv6)
    {
        addr->next = addr6;

        addr->sin.sin_addr.s_addr = f_teid->both.addr;
        memcpy(addr6->sin6.sin6_addr.s6_addr, f_teid->both.addr6, IPV6_LEN);

        *list = addr;
    }
    else if (f_teid->ipv4)
    {
        addr->sin.sin_addr.s_addr = f_teid->addr;
        core_free(addr6);

        *list = addr;
    }
    else if (f_teid->ipv6)
    {
        memcpy(addr6->sin6.sin6_addr.s6_addr, f_teid->addr6, IPV6_LEN);
        core_free(addr);

        *list = addr6;
    }
    else
    {
        core_free(addr);
        core_free(addr6);
        d_assert(0, return CORE_ERROR,);
    }

    return CORE_OK;
}

status_t gtp_sockaddr_to_f_teid(
    c_sockaddr_t *addr, c_sockaddr_t *addr6, gtp_f_teid_t *f_teid)
{
    d_assert(f_teid, return CORE_ERROR,);

    if (addr && addr6)
    {
        f_teid->ipv4 = 1;
        f_teid->both.addr = addr->sin.sin_addr.s_addr;
        f_teid->ipv6 = 1;
        memcpy(f_teid->both.addr6, addr6->sin6.sin6_addr.s6_addr, IPV6_LEN);
    }
    else if (addr)
    {
        f_teid->ipv4 = 1;
        f_teid->addr = addr->sin.sin_addr.s_addr;
    }
    else if (addr6)
    {
        f_teid->ipv6 = 1;
        memcpy(f_teid->addr6, addr6->sin6.sin6_addr.s6_addr, IPV6_LEN);
    }
    else
        d_assert(0, return CORE_ERROR,);

    return CORE_OK;
}

int gtp_f_teid_len(gtp_f_teid_t *f_teid)
{
    int len = 0;

    if (f_teid->ipv4 && f_teid->ipv6)
        len = GTP_F_TEID_IPV4_AND_IPV6_LEN;
    else if (f_teid->ipv4)
        len = GTP_F_TEID_IPV4_LEN;
    else if (f_teid->ipv6)
        len = GTP_F_TEID_IPV6_LEN;
    else
        d_assert(0, return 0,);

    return len;
}

gtp_f_teid_t *gtp_f_teid_copy(gtp_f_teid_t *dst, gtp_f_teid_t *src)
{
    d_assert(src, return NULL,);
    d_assert(dst, return NULL,);

    memset(dst, 0, sizeof(gtp_f_teid_t));
    dst->ipv4 = src->ipv4;
    dst->ipv6 = src->ipv6;
    dst->interface_type = 0;
    dst->teid = 0;

    if (dst->ipv4 && dst->ipv6)
    {
        dst->both.addr = src->both.addr;
        memcpy(dst->both.addr6, src->both.addr6, IPV6_LEN);
    }
    else if (dst->ipv4)
    {
        dst->addr = src->addr;
    }
    else if (dst->ipv6)
    {
        memcpy(dst->addr6, src->addr6, IPV6_LEN);
    }
    else
        d_assert(0, return NULL,);

    return dst;
}

