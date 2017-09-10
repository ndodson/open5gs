#define TRACE_MODULE _esm_handler

#include "core_debug.h"

#include "nas_message.h"

#include "mme_context.h"
#include "nas_path.h"
#include "mme_gtp_path.h"

#include "esm_build.h"

void esm_handle_pdn_connectivity_request(mme_bearer_t *bearer, 
        nas_pdn_connectivity_request_t *pdn_connectivity_request)
{
    status_t rv;
    mme_ue_t *mme_ue = NULL;
    mme_sess_t *sess = NULL;

    d_assert(bearer, return, "Null param");
    sess = bearer->sess;
    d_assert(sess, return, "Null param");
    mme_ue = sess->mme_ue;
    d_assert(mme_ue, return, "Null param");

    d_assert(MME_UE_HAVE_IMSI(mme_ue), return,
        "No IMSI in PDN_CPNNECTIVITY_REQUEST");
    d_assert(SECURITY_CONTEXT_IS_VALID(mme_ue), return,
        "No Security Context in PDN_CPNNECTIVITY_REQUEST");

    if (pdn_connectivity_request->presencemask &
            NAS_PDN_CONNECTIVITY_REQUEST_ACCESS_POINT_NAME_PRESENT)
    {
        sess->pdn = mme_pdn_find_by_apn(mme_ue, 
                pdn_connectivity_request->access_point_name.apn);
        if (!sess->pdn)
        {
            if (FSM_CHECK(&mme_ue->sm, emm_state_attached))
            {
                nas_send_pdn_connectivity_reject(
                        sess, ESM_CAUSE_MISSING_OR_UNKNOWN_APN);
                FSM_TRAN(&bearer->sm, esm_state_session_exception);
                return;
            }
            else
            {
                d_assert(0, return, "Invalid EMM State");
            }
        }
    }

    if (pdn_connectivity_request->presencemask &
            NAS_PDN_CONNECTIVITY_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT)
    {
        nas_protocol_configuration_options_t *protocol_configuration_options = 
            &pdn_connectivity_request->protocol_configuration_options;

        NAS_STORE_DATA(&sess->ue_pco, protocol_configuration_options);
    }

    if (MME_UE_HAVE_APN(mme_ue))
    {
        if (FSM_CHECK(&mme_ue->sm, emm_state_attached))
        {
            rv = mme_gtp_send_create_session_request(sess);
            d_assert(rv == CORE_OK, return,
                    "mme_gtp_send_create_session_request failed");
        }
        else
        {
            if (MME_HAVE_SGW_S11_PATH(mme_ue))
            {
                rv = nas_send_attach_accept(mme_ue);
                d_assert(rv == CORE_OK, return,
                        "nas_send_attach_accept failed");
            }
            else
            {
                rv = mme_gtp_send_create_session_request(sess);
                d_assert(rv == CORE_OK, return,
                        "mme_gtp_send_create_session_request failed");
            }
        }
    }
    else
    {
        FSM_TRAN(&bearer->sm, esm_state_information);
    }
}

void esm_handle_information_response(mme_sess_t *sess, 
        nas_esm_information_response_t *esm_information_response)
{
    status_t rv;
    mme_ue_t *mme_ue = NULL;

    d_assert(sess, return, "Null param");
    mme_ue = sess->mme_ue;
    d_assert(mme_ue, return, "Null param");

    if (esm_information_response->presencemask &
            NAS_ESM_INFORMATION_RESPONSE_ACCESS_POINT_NAME_PRESENT)
    {
        sess->pdn = mme_pdn_find_by_apn(mme_ue, 
                esm_information_response->access_point_name.apn);
        d_assert(sess->pdn, return, "No PDN Context[APN:%s])", 
            esm_information_response->access_point_name.apn);
    }

    if (esm_information_response->presencemask &
            NAS_ESM_INFORMATION_RESPONSE_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT)
    {
        nas_protocol_configuration_options_t *protocol_configuration_options = 
            &esm_information_response->protocol_configuration_options;
        NAS_STORE_DATA(&sess->ue_pco, protocol_configuration_options);
    }

    rv = mme_gtp_send_create_session_request(sess);
    d_assert(rv == CORE_OK, return,
            "mme_gtp_send_create_session_request failed");
}

void esm_handle_activate_default_bearer_accept(mme_bearer_t *bearer)
{
    status_t rv;

    d_assert(bearer, return, "Null param");

    mme_bearer_t *dedicated_bearer = mme_bearer_next(bearer);
    while(dedicated_bearer)
    {
        rv = nas_send_activate_dedicated_bearer_context_request(
                dedicated_bearer);
        d_assert(rv == CORE_OK, return,
            "nas_send_activate_dedicated_bearer_context failed");

        dedicated_bearer = mme_bearer_next(dedicated_bearer);
    }
}
