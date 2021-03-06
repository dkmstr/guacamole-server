/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "config.h"

#include "auth.h"
#include "vnc.h"

#include <guacamole/client.h>
#include <rfb/rfbclient.h>
#include <rfb/rfbproto.h>

char* guac_vnc_get_password(rfbClient* client) {
    guac_client* gc = rfbClientGetClientData(client, GUAC_VNC_CLIENT_KEY);
    return ((guac_vnc_client*) gc->data)->settings->password;
}

rfbCredential* guac_vnc_get_credentials(rfbClient* client, int credentialType) {
    guac_client* gc = rfbClientGetClientData(client, GUAC_VNC_CLIENT_KEY);
    guac_vnc_settings* settings = ((guac_vnc_client*) gc->data)->settings;
    
    if (credentialType == rfbCredentialTypeUser) {
        rfbCredential *creds = malloc(sizeof(rfbCredential));
        creds->userCredential.username = settings->username;
        creds->userCredential.password = settings->password;
        return creds;
    }

    guac_client_abort(gc, GUAC_PROTOCOL_STATUS_SERVER_ERROR,
            "Unsupported credential type requested.");
    guac_client_log(gc, GUAC_LOG_DEBUG,
            "Unable to provide requested type of credential: %d.",
            credentialType);
    return NULL;
    
}
