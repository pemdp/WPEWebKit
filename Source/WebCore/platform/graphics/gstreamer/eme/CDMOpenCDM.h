/* GStreamer OpenCDM decryptor
 *
 * Copyright (C) 2016-2017 TATA ELXSI
 * Copyright (C) 2016-2017 Metrological
 * Copyright (C) 2016-2017 Igalia S.L
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */

#pragma once

#if ENABLE(ENCRYPTED_MEDIA) && USE(OPENCDM)

#include "CDM.h"
#include "CDMFactory.h"
#include "CDMInstance.h"
#include "MediaKeyStatus.h"
#include <map>
#include <open_cdm.h>
#include <wtf/HashMap.h>

namespace WebCore {

class CDMFactoryOpenCDM : public CDMFactory {
private:
    CDMFactoryOpenCDM() = default;
    CDMFactoryOpenCDM(const CDMFactoryOpenCDM&) = delete;
    CDMFactoryOpenCDM& operator=(const CDMFactoryOpenCDM&) = delete;

public:
    static CDMFactoryOpenCDM& singleton();

    virtual ~CDMFactoryOpenCDM() = default;

    virtual std::unique_ptr<CDMPrivate> createCDM(const String&) final;
    virtual bool supportsKeySystem(const String&) final;
};

class CDMInstanceOpenCDM final : public CDMInstance {
private:
    CDMInstanceOpenCDM() = delete;
    CDMInstanceOpenCDM(const CDMInstanceOpenCDM&) = delete;
    CDMInstanceOpenCDM& operator=(const CDMInstanceOpenCDM&) = delete;

    class Session {
    private:
        Session() = delete;
        Session(const Session&) = delete;
        Session& operator=(const Session&) = delete;

    public:
        Session(const media::OpenCdm& source, Ref<WebCore::SharedBuffer>&& initData);
        ~Session() = default;

    public:
        bool isValid() const { return m_url.empty() == false; }
        const std::string& url() const { return m_url; }
        const std::string& message() const { return m_message; }
        bool needsIndividualization() const { return m_needsIndividualization; }
        Ref<WebCore::SharedBuffer>& initData() { return m_initData; }
        media::OpenCdm::KeyStatus update(const uint8_t* data, const uint16_t length, std::string& response) { m_lastStatus = m_session.Update(data, length, response); return m_lastStatus; }
        int load(std::string& response) { return m_session.Load(response); }
        int remove(std::string& response) { return m_session.Remove(response); }
        int close() { return m_session.Close(); }
        media::OpenCdm::KeyStatus lastStatus() const { return m_lastStatus; }
        bool operator==(const String& initData) const { return m_initData->size() == initData.sizeInBytes() && !memcmp(m_initData->data(), initData.latin1().data(), m_initData->size()); }
        bool operator!=(const String& initData) const { return !operator==(initData); }

    private:
        media::OpenCdm m_session;
        std::string m_message;
        std::string m_url;
        bool m_needsIndividualization { false };
        Ref<WebCore::SharedBuffer> m_initData;
        media::OpenCdm::KeyStatus m_lastStatus { media::OpenCdm::KeyStatus::StatusPending };
    };

public:
    CDMInstanceOpenCDM(media::OpenCdm&, const String&);
    virtual ~CDMInstanceOpenCDM() = default;

    // Metadata getters, just for some DRM characteristics.
    const String& keySystem() const final { return m_keySystem; }
    ImplementationType implementationType() const final { return ImplementationType::OpenCDM; }
    SuccessValue initializeWithConfiguration(const MediaKeySystemConfiguration&) final { return Succeeded; }
    SuccessValue setDistinctiveIdentifiersAllowed(bool) final { return Succeeded; }
    SuccessValue setPersistentStateAllowed(bool) final { return Succeeded; }

    // Operations on the DRM system.
    SuccessValue setServerCertificate(Ref<SharedBuffer>&&) final;

    // Request License will automagically create a Session. The session is later on referred to with its session id.
    void requestLicense(LicenseType, const AtomicString&, Ref<SharedBuffer>&&, LicenseCallback) final;

    // Operations on the DRM system -> Session.
    void updateLicense(const String&, LicenseType, const SharedBuffer&, LicenseUpdateCallback) final;
    void loadSession(LicenseType, const String&, const String&, LoadSessionCallback) final;
    void closeSession(const String&, CloseSessionCallback) final;
    void removeSessionData(const String&, LicenseType, RemoveSessionDataCallback) final;
    void storeRecordOfKeyUsage(const String&) final { }

    // The init data, is the only way to find a proper session id.
    String sessionIdByInitData(const String&, bool firstInLine) const;
    bool isSessionIdUsable(const String&) const;

private:
    media::OpenCdm m_openCDM;
    const char* m_mimeType;
    std::map<std::string, Session> m_sessionsMap;
    String m_keySystem;
};

} // namespace WebCore

SPECIALIZE_TYPE_TRAITS_CDM_INSTANCE(WebCore::CDMInstanceOpenCDM, WebCore::CDMInstance::ImplementationType::OpenCDM);

#endif // ENABLE(ENCRYPTED_MEDIA) && USE(OPENCDM)
