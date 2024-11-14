/*
This file is part of LightDM-KDE.

Copyright (C) 2024 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef NMSECRETAGENT_H
#define NMSECRETAGENT_H

#include <NetworkManagerQt/SecretAgent>

class NMSecretAgent: public NetworkManager::SecretAgent
{
    Q_OBJECT
public:
    explicit NMSecretAgent(const QString &connectionName, QObject *parent = nullptr);
    ~NMSecretAgent();

    void gotSecrets(const QVariantMap &secrets);
    void userCancel();

public Q_SLOTS:
    NMVariantMapMap GetSecrets(const NMVariantMapMap &connection,
                               const QDBusObjectPath &connectionPath,
                               const QString &settingName,
                               const QStringList &hints,
                               uint flags) override;
    void CancelGetSecrets(const QDBusObjectPath &connectionPath, const QString &settingName) override;
    void SaveSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connectionPath) override;
    void DeleteSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connectionPath) override;

Q_SIGNALS:
    void needSecrets(const QString &connectionName, const QList<QVariantMap> &secrets);

private:
    void sendSecrets(const NMStringMap &secretMap);
    void error(const QString &message, const QDBusMessage &request);

    QDBusMessage m_message;
    QString m_connectionName;
};

#endif // NMSECRETAGENT_H
