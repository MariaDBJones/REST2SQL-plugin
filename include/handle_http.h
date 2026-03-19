#ifndef HANDLE_HTTP_H
#define HANDLE_HTTP_H

#include "common.h"
#include <microhttpd.h>

/* ============================================================
 *  DEBUG timestamp macro
 *  Usage : HTTP_DEBUG_STAMP(json_obj, "label")
 * ============================================================ */
#if DEBUG == 1
#define HTTP_DEBUG_STAMP(obj, label)                                        \
    do {                                                                    \
        struct timeval _tv;                                                 \
        char _ts[30], _uts[40];                                             \
        gettimeofday(&_tv, NULL);                                           \
        struct tm *_loc = localtime(&_tv.tv_sec);                           \
        strftime(_ts, sizeof(_ts), "%Y-%m-%d %H:%M:%S", _loc);             \
        snprintf(_uts, sizeof(_uts), "%s.%06ld", _ts, _tv.tv_usec);        \
        cJSON_AddStringToObject((obj), (label), _uts);                      \
    } while (0)
#else
#define HTTP_DEBUG_STAMP(obj, label)  ((void)0)
#endif

/* ============================================================
 *  Constantes
 * ============================================================ */
#define URL_MAX_LEN   506

/* ============================================================
 *  Validation
 * ============================================================ */

/**
 * Vérifie que la chaîne ne contient que [A-Za-z0-9_].
 * Utilisé pour valider les identifiants SQL (schema, table, colonne)
 * qui ne peuvent pas être des paramètres préparés.
 *
 * Retourne 1 si valide, 0 sinon.
 */
int http_is_valid_sql_ident(const char *s);

/**
 * Valide une URL entrante :
 *  - non NULL
 *  - longueur <= URL_MAX_LEN
 *
 * En cas d'erreur, remplit json_response et retourne 0.
 * Retourne 1 si valide.
 */
int http_validate_url(const char *url, cJSON *json_response);

/* ============================================================
 *  Construction de réponses JSON
 * ============================================================ */

/**
 * Ajoute une erreur + httpcode dans json_response.
 * Factorise le pattern répété dans tous les handlers.
 */
void http_set_error(cJSON *json_response, const char *error_msg, int httpcode);

/**
 * Sérialise json_response et envoie la réponse HTTP via libmicrohttpd.
 * Extrait le httpcode depuis le champ "httpcode" du JSON.
 * Retourne MHD_YES ou MHD_NO.
 */
int http_send_json_response(struct MHD_Connection *connection,
                            cJSON *json_response);

#endif /* HANDLE_HTTP_H */
