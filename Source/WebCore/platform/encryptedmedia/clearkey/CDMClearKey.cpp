/*
 * Copyright (C) 2016 Metrological Group B.V.
 * Copyright (C) 2016 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "CDMClearKey.h"

#if ENABLE(ENCRYPTED_MEDIA)

#include "CDMKeySystemConfiguration.h"
#include "CDMRestrictions.h"
#include "CDMSessionType.h"
#include "SharedBuffer.h"
#include <inspector/InspectorValues.h>
#include <wtf/MainThread.h>
#include <wtf/text/Base64.h>

using namespace Inspector;

namespace WebCore {

class ClearKeyState {
    using KeyStore = HashMap<String, Vector<CDMInstanceClearKey::Key>>;

public:
    static ClearKeyState& singleton();

    KeyStore& keys() { return m_keys; }
    HashSet<String>& persistentSessions() { return m_persistentSessions; }

private:
    ClearKeyState();
    KeyStore m_keys;
    HashSet<String> m_persistentSessions;
};

ClearKeyState& ClearKeyState::singleton()
{
    static ClearKeyState s_state;
    return s_state;
}

ClearKeyState::ClearKeyState() = default;

RefPtr<InspectorObject> parseJSONObject(const SharedBuffer& buffer)
{
    // Fail on large buffers whose size doesn't fit into a 32-bit unsigned integer.
    size_t size = buffer.size();
    if (size > std::numeric_limits<unsigned>::max())
        return nullptr;

    // Parse the buffer contents as JSON, returning the root object (if any).
    String json { buffer.data(), static_cast<unsigned>(size) };
    RefPtr<InspectorValue> value;
    RefPtr<InspectorObject> object;
    if (!InspectorValue::parseJSON(json, value) || !value->asObject(object))
        return nullptr;

    return object;
}

std::optional<Vector<CDMInstanceClearKey::Key>> parseLicenseFormat(const InspectorObject& root)
{
    // If the 'keys' key is present in the root object, parse the JSON further
    // according to the specified 'license' format.
    auto it = root.find("keys");
    if (it == root.end())
        return std::nullopt;

    // Retrieve the keys array.
    RefPtr<InspectorArray> keysArray;
    if (!it->value->asArray(keysArray))
        return std::nullopt;

    Vector<CDMInstanceClearKey::Key> decodedKeys;
    bool validFormat = std::all_of(keysArray->begin(), keysArray->end(),
        [&decodedKeys] (const auto& value) {
            RefPtr<InspectorObject> keyObject;
            if (!value->asObject(keyObject))
                return false;

            String keyType;
            if (!keyObject->getString("kty", keyType) || !equalLettersIgnoringASCIICase(keyType, "oct"))
                return false;

            String keyID, keyValue;
            if (!keyObject->getString("kid", keyID) || !keyObject->getString("k", keyValue))
                return false;

            Vector<char> keyIDData, keyValueData;
            if (!WTF::base64URLDecode(keyID, { keyIDData }) || !WTF::base64URLDecode(keyValue, { keyValueData }))
                return false;

            decodedKeys.append({ CDMInstanceClearKey::KeyStatus::Usable, SharedBuffer::create(WTFMove(keyIDData)), SharedBuffer::create(WTFMove(keyValueData)) });
            return true;
        });
    if (!validFormat)
        return std::nullopt;

    return decodedKeys;
}

bool parseLicenseReleaseAcknowledgementFormat(const InspectorObject& root)
{
    // If the 'kids' key is present in the root object, parse the JSON further
    // according to the specified 'license release acknowledgement' format.
    auto it = root.find("kids");
    if (it == root.end())
        return false;

    // Retrieve the kids array.
    RefPtr<InspectorArray> kidsArray;
    if (!it->value->asArray(kidsArray))
        return false;

    // FIXME: Return the key IDs and validate them.
    return true;
}

CDMFactoryClearKey& CDMFactoryClearKey::singleton()
{
    static CDMFactoryClearKey s_factory;
    return s_factory;
}

CDMFactoryClearKey::CDMFactoryClearKey() = default;
CDMFactoryClearKey::~CDMFactoryClearKey() = default;

std::unique_ptr<CDMPrivate> CDMFactoryClearKey::createCDM(const String& keySystem)
{
#ifdef NDEBUG
    UNUSED_PARAM(keySystem);
#else
    ASSERT(supportsKeySystem(keySystem));
#endif
    return std::unique_ptr<CDMPrivate>(new CDMPrivateClearKey);
}

bool CDMFactoryClearKey::supportsKeySystem(const String& keySystem)
{
    // `org.w3.clearkey` is the only supported key system.
    return equalLettersIgnoringASCIICase(keySystem, "org.w3.clearkey");
}

CDMPrivateClearKey::CDMPrivateClearKey() = default;
CDMPrivateClearKey::~CDMPrivateClearKey() = default;

bool CDMPrivateClearKey::supportsInitDataType(const AtomicString& initDataType) const
{
    // `keyids` is the only supported init data type.
    return equalLettersIgnoringASCIICase(initDataType, "keyids");
}

bool containsPersistentLicenseType(const Vector<CDMSessionType>& types)
{
    return std::any_of(types.begin(), types.end(),
        [] (auto& sessionType) { return sessionType == CDMSessionType::PersistentLicense; });
}

bool CDMPrivateClearKey::supportsConfiguration(const CDMKeySystemConfiguration& configuration) const
{
    // Reject any configuration that marks distinctive identifier as required.
    if (configuration.distinctiveIdentifier == CDMRequirement::Required)
        return false;

    // Reject any configuration that marks persistent state as required, unless
    // the 'persistent-license' session type has to be supported.
    if (configuration.persistentState == CDMRequirement::Required && !containsPersistentLicenseType(configuration.sessionTypes))
        return false;

    return true;
}

bool CDMPrivateClearKey::supportsConfigurationWithRestrictions(const CDMKeySystemConfiguration& configuration, const CDMRestrictions& restrictions) const
{
    // Reject any configuration that marks distincitive identifier as required, or that marks
    // distinctive identifier as optional even when restrictions mark it as denied.
    if ((configuration.distinctiveIdentifier == CDMRequirement::Optional && restrictions.distinctiveIdentifierDenied)
        || configuration.distinctiveIdentifier == CDMRequirement::Required)
        return false;

    // Reject any configuration that marks persistent state as optional even when
    // restrictions mark it as denied.
    if (configuration.persistentState == CDMRequirement::Optional && restrictions.persistentStateDenied)
        return false;

    // Reject any configuration that marks persistent state as required, unless
    // the 'persistent-license' session type has to be supported.
    if (configuration.persistentState == CDMRequirement::Required && !containsPersistentLicenseType(configuration.sessionTypes))
        return false;

    return true;
}

bool CDMPrivateClearKey::supportsSessionTypeWithConfiguration(CDMSessionType& sessionType, const CDMKeySystemConfiguration& configuration) const
{
    // Only support the 'temporary' and 'persistent-license' session types.
    if (sessionType != CDMSessionType::Temporary && sessionType != CDMSessionType::PersistentLicense)
        return false;
    return supportsConfiguration(configuration);
}

bool CDMPrivateClearKey::supportsRobustness(const String& robustness) const
{
    // Only empty `robustness` string is supported.
    return robustness.isEmpty();
}

CDMRequirement CDMPrivateClearKey::distinctiveIdentifiersRequirement(const CDMKeySystemConfiguration&, const CDMRestrictions& restrictions) const
{
    // Distinctive identifier is not allowed if it's been denied, otherwise it's optional.
    if (restrictions.distinctiveIdentifierDenied)
        return CDMRequirement::NotAllowed;
    return CDMRequirement::Optional;
}

CDMRequirement CDMPrivateClearKey::persistentStateRequirement(const CDMKeySystemConfiguration&, const CDMRestrictions& restrictions) const
{
    // Persistent state is not allowed if it's been denied, otherwise it's optional.
    if (restrictions.persistentStateDenied)
        return CDMRequirement::NotAllowed;
    return CDMRequirement::Optional;
}

bool CDMPrivateClearKey::distinctiveIdentifiersAreUniquePerOriginAndClearable(const CDMKeySystemConfiguration&) const
{
    return false;
}

RefPtr<CDMInstance> CDMPrivateClearKey::createInstance()
{
    return adoptRef(new CDMInstanceClearKey);
}

void CDMPrivateClearKey::loadAndInitialize()
{
    // No-op.
}

bool CDMPrivateClearKey::supportsServerCertificates() const
{
    // Server certificates are not supported.
    return false;
}

bool CDMPrivateClearKey::supportsSessions() const
{
    // Sessions are supported.
    return true;
}

bool CDMPrivateClearKey::supportsInitData(const AtomicString& initDataType, const SharedBuffer& initData) const
{
    // Fail for init data types other than 'keyids'.
    if (!equalLettersIgnoringASCIICase(initDataType, "keyids"))
        return false;

    // Validate the initData buffer as an JSON object.
    if (!parseJSONObject(initData))
        return false;

    return true;
}

RefPtr<SharedBuffer> CDMPrivateClearKey::sanitizeResponse(const SharedBuffer& response) const
{
    // Validate the response buffer as an JSON object.
    if (!parseJSONObject(response))
        return nullptr;

    return response.copy();
}

std::optional<String> CDMPrivateClearKey::sanitizeSessionId(const String& sessionId) const
{
    // Validate the session ID string as an 32-bit integer.
    bool ok;
    sessionId.toUIntStrict(&ok);
    if (!ok)
        return std::nullopt;

    return sessionId;
}

CDMInstanceClearKey::CDMInstanceClearKey()
{
}

CDMInstanceClearKey::~CDMInstanceClearKey() = default;

CDMInstance::SuccessValue CDMInstanceClearKey::initializeWithConfiguration(const CDMKeySystemConfiguration&)
{
    // No-op.
    return Succeeded;
}

CDMInstance::SuccessValue CDMInstanceClearKey::setDistinctiveIdentifiersAllowed(bool allowed)
{
    // Reject setting distinctive identifiers as allowed.
    return !allowed ? Succeeded : Failed;
}

CDMInstance::SuccessValue CDMInstanceClearKey::setPersistentStateAllowed(bool allowed)
{
    // Reject setting persistent state as allowed.
    return !allowed ? Succeeded : Failed;
}

CDMInstance::SuccessValue CDMInstanceClearKey::setServerCertificate(Ref<SharedBuffer>&&)
{
    // Reject setting any server certificate.
    return Failed;
}

void CDMInstanceClearKey::requestLicense(LicenseType, const AtomicString&, Ref<SharedBuffer>&& initData, LicenseCallback callback)
{
    static uint32_t s_sessionIdValue = 0;
    ++s_sessionIdValue;

    callOnMainThread(
        [weakThis = m_weakPtrFactory.createWeakPtr(*this), callback = WTFMove(callback), initData = WTFMove(initData), sessionIdValue = s_sessionIdValue]() mutable {
            if (!weakThis)
                return;

            callback(WTFMove(initData), String::number(sessionIdValue), false, Succeeded);
        });
}

void CDMInstanceClearKey::updateLicense(const String& sessionId, LicenseType, const SharedBuffer& response, LicenseUpdateCallback callback)
{
    // Use a helper functor that schedules the callback dispatch, avoiding
    // duplicated callOnMainThread() calls.
    auto dispatchCallback =
        [this, &callback](bool sessionWasClosed, std::optional<KeyStatusVector>&& changedKeys, SuccessValue succeeded) {
            callOnMainThread(
                [weakThis = m_weakPtrFactory.createWeakPtr(*this), callback = WTFMove(callback), sessionWasClosed, changedKeys = WTFMove(changedKeys), succeeded] () mutable {
                    if (!weakThis)
                        return;

                    callback(sessionWasClosed, WTFMove(changedKeys), std::nullopt, std::nullopt, succeeded);
                });
        };

    // Parse the response buffer as an JSON object.
    RefPtr<InspectorObject> root = parseJSONObject(response);
    if (!root) {
        dispatchCallback(false, std::nullopt, SuccessValue::Failed);
        return;
    }

    // Parse the response using 'license' formatting, if possible.
    if (auto decodedKeys = parseLicenseFormat(*root)) {
        // Retrieve the target Vector of Key objects for this session.
        auto& keyVector = ClearKeyState::singleton().keys().ensure(sessionId, [] { return Vector<Key> { }; }).iterator->value;

        // For each decoded key, find an existing item for the decoded key's ID. If none exist,
        // the key is decoded. Otherwise, the key is updated in case there's a mismatch between
        // the size or data of the existing and proposed key.
        bool keysChanged = false;
        for (auto& key : *decodedKeys) {
            auto it = std::find_if(keyVector.begin(), keyVector.end(),
                [&key] (const Key& containedKey) {
                    return containedKey.keyIDData->size() == key.keyIDData->size()
                        && !std::memcmp(containedKey.keyIDData->data(), key.keyIDData->data(), containedKey.keyIDData->size());
                });
            if (it != keyVector.end()) {
                auto& existingKey = it->keyValueData;
                auto& proposedKey = key.keyValueData;

                // Update the existing Key if it differs from the proposed key in key value.
                if (existingKey->size() != proposedKey->size() || std::memcmp(existingKey->data(), proposedKey->data(), existingKey->size())) {
                    *it = WTFMove(key);
                    keysChanged = true;
                }
            } else {
                // In case a Key for this key ID doesn't exist yet, append the new one to keyVector.
                keyVector.append(WTFMove(key));
                keysChanged = true;
            }
        }

        // In case of changed keys, we have to provide a KeyStatusVector of all the keys for
        // this session.
        std::optional<KeyStatusVector> changedKeys;
        if (keysChanged) {
            // First a helper Vector is constructed, cotaining pairs of SharedBuffer RefPtrs
            // representint key ID data, and the corresponding key statuses.
            // We can't use KeyStatusVector here because this Vector has to be sorted, which
            // is not possible to do on Ref<> objects.
            Vector<std::pair<RefPtr<SharedBuffer>, KeyStatus>> keys;
            keys.reserveInitialCapacity(keyVector.size());
            for (auto& it : keyVector)
                keys.uncheckedAppend(std::pair<RefPtr<SharedBuffer>, KeyStatus> { it.keyIDData, it.status });

            // Sort first by size, second by data.
            std::sort(keys.begin(), keys.end(),
                [] (const auto& a, const auto& b) {
                    if (a.first->size() != b.first->size())
                        return a.first->size() < b.first->size();

                    return std::memcmp(a.first->data(), b.first->data(), a.first->size()) < 0;
                });

            // Finally construct the mirroring KeyStatusVector object and move it into the
            // std::optional<> object that will be passed to the callback.
            KeyStatusVector keyStatusVector;
            keyStatusVector.reserveInitialCapacity(keys.size());
            for (auto& it : keys)
                keyStatusVector.uncheckedAppend(std::pair<Ref<SharedBuffer>, KeyStatus> { *it.first, it.second });

            changedKeys = WTFMove(keyStatusVector);
        }

        // Cache the key information Vector on CDMInstance for easier access from the pipeline.
        m_keys = keyVector;

        dispatchCallback(false, WTFMove(changedKeys), SuccessValue::Succeeded);
        return;
    }

    // Parse the response using 'license release acknowledgement' formatting, if possible.
    if (parseLicenseReleaseAcknowledgementFormat(*root)) {
        // FIXME: Retrieve the key ID information and use it to validate the keys for this sessionId.
        ClearKeyState::singleton().keys().remove(sessionId);
        m_keys.clear();
        dispatchCallback(true, std::nullopt, SuccessValue::Succeeded);
        return;
    }

    // Bail in case no format was recognized.
    dispatchCallback(false, std::nullopt, SuccessValue::Failed);
}

void CDMInstanceClearKey::loadSession(LicenseType, const String& sessionId, const String&, LoadSessionCallback callback)
{
    // Use a helper functor that schedules the callback dispatch, avoiding duplicated callOnMainThread() calls.
    auto dispatchCallback =
        [this, &callback](std::optional<KeyStatusVector>&& existingKeys, SuccessValue success, SessionLoadFailure loadFailure) {
            callOnMainThread(
                [weakThis = m_weakPtrFactory.createWeakPtr(*this), callback = WTFMove(callback), existingKeys = WTFMove(existingKeys), success, loadFailure]() mutable {
                    if (!weakThis)
                        return;

                    callback(WTFMove(existingKeys), std::nullopt, std::nullopt, success, loadFailure);
                });
        };

    // Construct the KeyStatusVector object, representing all the known keys for this session.
    KeyStatusVector keyStatusVector;
    {
        auto& keys = ClearKeyState::singleton().keys();
        auto it = keys.find(sessionId);
        if (it == keys.end()) {
            dispatchCallback(std::nullopt, Failed, SessionLoadFailure::NoSessionData);
            return;
        }

        auto& keyVector = it->value;
        keyStatusVector.reserveInitialCapacity(keyVector.size());
        for (auto& key : keyVector)
            keyStatusVector.uncheckedAppend(std::pair<Ref<SharedBuffer>, KeyStatus> { *key.keyIDData, key.status });
    }

    dispatchCallback(WTFMove(keyStatusVector), Succeeded, SessionLoadFailure::None);
}

void CDMInstanceClearKey::closeSession(const String&, CloseSessionCallback callback)
{
    callOnMainThread(
        [weakThis = m_weakPtrFactory.createWeakPtr(*this), callback = WTFMove(callback)] {
            if (!weakThis)
                return;

            callback();
        });
}

void CDMInstanceClearKey::removeSessionData(const String& sessionId, LicenseType, RemoveSessionDataCallback callback)
{
    // Use a helper functor that schedules the callback dispatch, avoiding duplicated callOnMainThread() calls.
    auto dispatchCallback =
        [this, &callback](KeyStatusVector&& keyStatusVector, std::optional<Ref<SharedBuffer>>&& message, SuccessValue success) {
            callOnMainThread(
                [weakThis = m_weakPtrFactory.createWeakPtr(*this), callback = WTFMove(callback), keyStatusVector = WTFMove(keyStatusVector), message = WTFMove(message), success]() mutable {
                    if (!weakThis)
                        return;

                    callback(WTFMove(keyStatusVector), WTFMove(message), success);
                });
        };

    // Construct the KeyStatusVector object, representing released keys, and the message in the
    // 'license release' format.
    KeyStatusVector keyStatusVector;
    RefPtr<SharedBuffer> message;
    {
        // Retrieve information for the given session ID, bailing if none is found.
        auto& keys = ClearKeyState::singleton().keys();
        auto it = keys.find(sessionId);
        if (it == keys.end()) {
            dispatchCallback(KeyStatusVector { }, std::nullopt, SuccessValue::Failed);
            return;
        }

        // Retrieve the Key vector, containing all the keys for this session, and
        // then remove the key map entry for this session.
        auto keyVector = WTFMove(it->value);
        keys.remove(it);

        // Construct the KeyStatusVector object, pairing key IDs with the 'released' status.
        keyStatusVector.reserveInitialCapacity(keyVector.size());
        for (auto& key : keyVector)
            keyStatusVector.uncheckedAppend(std::pair<Ref<SharedBuffer>, KeyStatus> { *key.keyIDData, KeyStatus::Released });

        // Construct JSON that represents the 'license release' format, creating a 'kids' array
        // of base64URL-encoded key IDs for all keys that were associated with this session.
        auto rootObject = InspectorObject::create();
        {
            auto array = InspectorArray::create();
            for (auto& key : keyVector) {
                ASSERT(key.keyIDData->size() <= std::numeric_limits<unsigned>::max());
                array->pushString(WTF::base64URLEncode(key.keyIDData->data(), static_cast<unsigned>(key.keyIDData->size())));
            }
            rootObject->setArray("kids", WTFMove(array));
        }

        // Copy the JSON data into a SharedBuffer object.
        String messageString = rootObject->toJSONString();
        CString messageCString = messageString.utf8();
        message = SharedBuffer::create(messageCString.data(), messageCString.length());
    }

    dispatchCallback(WTFMove(keyStatusVector), Ref<SharedBuffer>(*message), SuccessValue::Succeeded);
}

void CDMInstanceClearKey::storeRecordOfKeyUsage(const String&)
{
}

const String& CDMInstanceClearKey::keySystem() const
{
    static const String s_keySystem("org.w3.clearkey");

    return s_keySystem;
}

} // namespace WebCore

#endif // ENABLE(ENCRYPTED_MEDIA)
