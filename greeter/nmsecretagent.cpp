/*
This file is part of LightDM-KDE.

Copyright (C) 2024 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "nmsecretagent.h"

#include "NetworkManagerQt/Setting"
#define TRANSLATION_DOMAIN "lightdm_kde_greeter"
#include <KLocalizedString>
#include <QDBusConnection>
#include <QLoggingCategory>

#include "nm-dbus-interface.h"
#include "connectionsmodel.h"

static const QLoggingCategory lc("NMSecretAgent");

using namespace Qt::StringLiterals;

NMSecretAgent::NMSecretAgent(const QString &connectionName, QObject *parent) :
    NetworkManager::SecretAgent(u"org.altlinux.LightDMKdeGreeter.nm-agent"_s, NetworkManager::SecretAgent::Capability::VpnHints, parent),
    m_connectionName(connectionName)
{
    qCInfo(lc,) << "Starting for connection" << m_connectionName;
}

NMSecretAgent::~NMSecretAgent()
{
    qCInfo(lc,) << "Stopping for connection" << m_connectionName;
}

NMVariantMapMap NMSecretAgent::GetSecrets(const NMVariantMapMap &connection,
                                   const QDBusObjectPath &connectionPath,
                                   const QString &settingName,
                                   const QStringList &hints,
                                   uint flags)
{
    Q_UNUSED(connectionPath);
    Q_UNUSED(settingName);

    if (!(flags & NetworkManager::SecretAgent::AllowInteraction)) {
        return {};
    }

    auto connectionName = connection[u"connection"_s][u"id"_s].toString();
    if (m_connectionName != connectionName) {
        qCWarning(lc,) << "Received a request for an unexpected connection:" << connectionName << "expected:" << m_connectionName;
        return {};
    }

    auto dataRaw = connection[u"vpn"_s][u"data"_s];
    const auto arg = dataRaw.value<QDBusArgument>();

    QMap<QString, QString> data;
    int maxElem = 100;

    arg.beginArray();

    while (!arg.atEnd() && --maxElem > 0) {

        arg.beginStructure();
        QString s1, s2;
        arg >> s1 >> s2;
        arg.endStructure();

        data.insert(s1, s2);
    }

    if (maxElem == 0) {
        error(u"Unexpected data structure."_s, message());
        return {};
    }

    const QList<std::pair<QString, QString>> secretNames{
        { u"password"_s           , ki18n("Password").toString()             },
        { u"cert-pass"_s          , ki18n("Certificate password").toString() },
        { u"http-proxy-password"_s, ki18n("HTTP proxy password").toString()  }
    };

    auto secretNamesContainsHint = [secretNames](const QString &hint) {
        for (auto [name, localized] : secretNames) {
            if (name == hint) return true;
        }
        return false;
    };

    // check for unsupported hints
    if (!hints.isEmpty()) {
        for (auto hint : hints) {
            if (secretNamesContainsHint(hint)) continue;
            if (hint.startsWith(u"x-vpn-message:"_s)) continue;
            error(u"Unsupported request: '%1'"_s.arg(hint), message());
            return {};
        }
    }

    QList<QVariantMap> neededSecrets;

    auto addNeededSecret = [&neededSecrets] (const QString &name, const QString &localized) {
        QVariantMap item;
        item[u"secretName"_s] = name;
        item[u"secretLocalized"_s] = localized;
        neededSecrets.push_back(item);
    };

    for (auto [name, localized] : secretNames) {

        // if there are hints, we take only them
        if (!hints.isEmpty()) {
            if (hints.contains(name)) {
                addNeededSecret(name, localized);
            }
            continue;
        }

        QString flagsKey = name + u"-flags"_s;
        auto iter = data.find(flagsKey);
        if (iter == data.end()) continue;

        int flags = iter->toInt();
        if (!(flags & NetworkManager::Setting::AgentOwned || flags & NetworkManager::Setting::NotSaved)) continue;

        addNeededSecret(name, localized);
    }

    if (neededSecrets.size() == 0) return {};

    setDelayedReply(true);
    m_message = message();

    Q_EMIT needSecrets(m_connectionName, neededSecrets);

    return {};
}

void NMSecretAgent::CancelGetSecrets(const QDBusObjectPath &connectionPath, const QString &settingName)
{
    qCInfo(lc,) << "CancelGetSecrets" << "connection path:" << connectionPath.path() << "setting name:" << settingName;
    sendError(SecretAgent::AgentCanceled, u"Agent canceled the password dialog"_s, m_message);
}

void NMSecretAgent::SaveSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connectionPath)
{
    qCWarning(lc,) << "unexpected SaveSecrets" << "connection:" << connection << "path:" << connectionPath.path();
}

void NMSecretAgent::DeleteSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connectionPath)
{
    qCWarning(lc,) << "unexpected DeleteSecrets" << "connection:" << connection << "path:" << connectionPath.path();
}

void NMSecretAgent::gotSecrets(const QVariantMap &secrets)
{
    NMStringMap secretMap;
    for (auto i = secrets.begin(); i != secrets.end(); ++i) {
        secretMap[i.key()] = i.value().toString();
    }
    sendSecrets(secretMap);
}

void NMSecretAgent::sendSecrets(const NMStringMap &secretMap)
{
    QVariantMap secretsEntry;
    secretsEntry.insert(u"secrets"_s, QVariant::fromValue(secretMap));

    NMVariantMapMap result;
    result.insert(u"vpn"_s, secretsEntry);

    QDBusMessage reply;
    reply = m_message.createReply(QVariant::fromValue(result));
    if (!QDBusConnection::systemBus().send(reply)) {
        qCWarning(lc,) << "Failed put the secret into the queue";
    }
}

void NMSecretAgent::userCancel()
{
    sendError(SecretAgent::UserCanceled, u"User canceled the password dialog"_s, m_message);
}

void NMSecretAgent::error(const QString &msg, const QDBusMessage &request)
{
    qCWarning(lc, "%s", qPrintable(msg));
    sendError(SecretAgent::InternalError, msg, request);
}
