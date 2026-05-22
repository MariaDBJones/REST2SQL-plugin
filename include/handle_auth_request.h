#ifndef HANDLE_AUTH_REQUEST_H
#define HANDLE_AUTH_REQUEST_H

#include "common.h"
#include "handle_http_request.h"

/* ============================================================
 *  Ouverture de la connexion MariaDB par requête HTTP.
 *  La connexion EST le marqueur d'authentification :
 *    conn != NULL → authentifié
 *    conn == NULL → échec (→ 401)
 *  MVP  : socket Unix, utilisateur OS, pas de credentials.
 *  Beta : extraire user/pass du header Authorization: Basic ...
 * ============================================================ */
MYSQL *open_db_connection(struct MHD_Connection *connection);

/* Auth endpoints — stubs MVP, retournent 405 */
cJSON *handle_session_request(const char *url,
                              const char *upload_data,
                              size_t     *upload_data_size);

/* MHD connection lifecycle — mysql_thread_init/end */
void connection_start(void *cls,
                      struct MHD_Connection *connection,
                      void **socket_context);

void connection_finish(void *cls,
                       struct MHD_Connection *connection,
                       void **socket_context,
                       enum MHD_RequestTerminationCode toe);

#endif // HANDLE_AUTH_REQUEST_H
