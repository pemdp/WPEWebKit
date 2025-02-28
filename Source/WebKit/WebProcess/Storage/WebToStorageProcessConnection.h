/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include "Connection.h"
#include "MessageSender.h"
#include "WebIDBConnectionToServer.h"
#include "WebSWClientConnection.h"
#include <WebCore/SWServer.h>
#include <pal/SessionID.h>
#include <wtf/RefCounted.h>

namespace PAL {
class SessionID;
}

namespace WebKit {

class WebToStorageProcessConnection : public RefCounted<WebToStorageProcessConnection>, public IPC::Connection::Client, public IPC::MessageSender {
public:
    static Ref<WebToStorageProcessConnection> create(IPC::Connection::Identifier connectionIdentifier)
    {
        return adoptRef(*new WebToStorageProcessConnection(connectionIdentifier));
    }
    ~WebToStorageProcessConnection();
    
    IPC::Connection& connection() { return m_connection.get(); }

#if ENABLE(INDEXED_DATABASE)
    WebIDBConnectionToServer& idbConnectionToServerForSession(const PAL::SessionID&);
#endif
#if ENABLE(SERVICE_WORKER)
    WebSWClientConnection& serviceWorkerConnectionForSession(const PAL::SessionID&);
#endif

private:
    WebToStorageProcessConnection(IPC::Connection::Identifier);

    // IPC::Connection::Client
    void didReceiveMessage(IPC::Connection&, IPC::Decoder&) override;
    void didClose(IPC::Connection&) override;
    void didReceiveInvalidMessage(IPC::Connection&, IPC::StringReference messageReceiverName, IPC::StringReference messageName) override;

    // IPC::MessageSender
    IPC::Connection* messageSenderConnection() override { return m_connection.ptr(); }
    uint64_t messageSenderDestinationID() override { return 0; }

    Ref<IPC::Connection> m_connection;

#if ENABLE(INDEXED_DATABASE)
    HashMap<PAL::SessionID, RefPtr<WebIDBConnectionToServer>> m_webIDBConnectionsBySession;
    HashMap<uint64_t, RefPtr<WebIDBConnectionToServer>> m_webIDBConnectionsByIdentifier;
#endif

#if ENABLE(SERVICE_WORKER)
    HashMap<PAL::SessionID, std::unique_ptr<WebSWClientConnection>> m_swConnectionsBySession;
    HashMap<uint64_t, WebSWClientConnection*> m_swConnectionsByIdentifier;
#endif
};

} // namespace WebKit
