/*
 * Copyright (C) 2013 Apple Inc.  All rights reserved.
 * Copyright (C) 2017 Sony Interactive Entertainment Inc.
 * Copyright (C) 2017 NAVER Corp.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
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
 */

#pragma once

#include "CurlContext.h"
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/Lock.h>
#include <wtf/Noncopyable.h>
#include <wtf/Threading.h>

namespace WebCore {

class CurlJobList;
using CurlJobTicket = void*;

class CurlJobClient {
public:
    virtual void retain() = 0;
    virtual void release() = 0;

    virtual void setupRequest() = 0;
    virtual void notifyFinish() = 0;
    virtual void notifyFail() = 0;
};

class CurlJobManager {
    WTF_MAKE_NONCOPYABLE(CurlJobManager);
public:
    static CurlJobManager& singleton()
    {
        static CurlJobManager shared;
        return shared;
    }

    CurlJobManager() = default;
    ~CurlJobManager() { stopThread(); }

    CurlJobTicket add(CurlHandle&, CurlJobClient&);
    void cancel(CurlJobTicket);

    void callOnJobThread(WTF::Function<void()>&&);

private:
    void startThreadIfNeeded();
    void stopThreadIfNoMoreJobRunning();
    void stopThread();

    void updateJobList(CurlJobList&);

    void workerThread();

    RefPtr<Thread> m_thread;
    HashMap<CurlJobTicket, CurlJobClient*> m_pendingJobs;
    HashSet<CurlJobTicket> m_cancelledTickets;
    HashSet<CurlJobTicket> m_finishedTickets;
    HashSet<CurlJobTicket> m_failedTickets;
    Vector<WTF::Function<void()>> m_taskQueue;
    mutable Lock m_mutex;
    bool m_runThread { false };
};

}
