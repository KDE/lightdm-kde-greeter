/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2025 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "helper.h"

#include <QDebug>
#include <KConfig>
#include <KConfigGroup>

#include <QDBusUnixFileDescriptor>
#include <QDir>
#include <QFile>
#include <QUrl>

#include <grp.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>

#include "config.h"

using namespace Qt::StringLiterals;

static const char *s_greeterUserName = "_ldm";

struct Helper::ParsedKey {

    static ParsedKey parse(const QString &key, QString &errorMessage)
    {
        ParsedKey result;
        QStringList lst = key.split(QLatin1Char('/'));
        if (lst.size() != 3)
        {
            errorMessage = u"Invalid key format: %1"_s.arg(key);
            return result;
        }
        result.fileName = lst[0];
        result.groupName = lst[1];
        result.keyName = lst[2];
        return result;
    }

    QString fileName;
    QString groupName;
    QString keyName;
};

class Helper::Configs {

public:
    Configs() {
        m_coreConfig = openConfig(CoreConfig);
        m_greeterConfig = openConfig(GreeterConfig);
    };

    QSharedPointer<KConfig> getByFilename(const QString &fileName, QString &errorMessage)
    {
        if (fileName == u"core"_s) {
            return m_coreConfig;
        } else if (fileName == u"greeter"_s) {
            return m_greeterConfig;
        } else {
            errorMessage = u"Unknown config file: %1"_s.arg(fileName);
            return {};
        }
    }

    void sync()
    {
        m_coreConfig->sync();
        m_greeterConfig->sync();
    }

private:
    enum WhichConfig
    {
        CoreConfig,
        GreeterConfig
    };

    QSharedPointer<KConfig> openConfig(WhichConfig which)
    {
        QString name = QStringLiteral("%1/%2")
            .arg(QStringLiteral(LIGHTDM_CONFIG_DIR))
            .arg(which == CoreConfig ? QStringLiteral("lightdm.conf") : QStringLiteral("lightdm-kde-greeter.conf"))
            ;

        QFile file(name);
        if (!file.exists())
        {
            // If we are creating the config file, ensure it is world-readable: if
            // we don't do that, KConfig will create a file which is only readable
            // by root
            file.open(QIODevice::WriteOnly);
            file.close();
            file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);
        }

        return QSharedPointer<KConfig>(new KConfig(file.fileName(), KConfig::SimpleConfig));
    }

    QSharedPointer<KConfig> m_coreConfig;
    QSharedPointer<KConfig> m_greeterConfig;
};

class Helper::Privileges {

public:
    bool drop(const char *username)
    {
        auto pw = getpwnam(username);
        if (!pw) {
            qWarning() << "Failed to retrieve the user database entry";
            return false;
        }

        if (setegid(pw->pw_gid)) {
            qWarning() << "Failed to set gid:" << strerror(errno);
            return false;
        }

        m_groupListSize = getgroups(sizeof(m_groupList) / sizeof(m_groupList[0]), m_groupList);

        if (m_groupListSize < 0) {
            qWarning() << "Failed to get groups:" << strerror(errno);
            return false;
        }

        if (m_groupListSize > 0) {
            // drop any supplementary group membership
            if (setgroups(0, nullptr)) {
                qWarning() << "Failed to unset groups:" << strerror(errno);
                return false;
            }
        }

        if (seteuid(pw->pw_uid)) {
            qWarning() << "Failed to set uid:" << strerror(errno);
            return false;
        }

        return true;
    }

    bool reset()
    {
        if (setegid(0)) {
            qCritical() << "Failed to reset gid:" << strerror(errno);
            return false;
        }

        if (seteuid(0)) {
            qCritical() << "Failed to reset uid:" << strerror(errno);
            return false;
        }

        if (m_groupListSize > 0) {
            // get back supplementary group membership
            if (setgroups(m_groupListSize, m_groupList)) {
                qWarning() << "Failed to get back groups:" << strerror(errno);
                return false;
            }
        }

        return true;
    }

private:
    int m_groupListSize{};
    gid_t m_groupList[NGROUPS_MAX]{};
};

static bool check_fd_sanity(int fd)
{
    struct stat s;
    if (fstat(fd, &s)) {
        qWarning() << "Failed to get file params:" << strerror(errno);
        return false;
    }

    if (!S_ISREG(s.st_mode)) {
        qWarning() << "Provided file is not regular";
        return false;
    };

    int flags = fcntl(fd, F_GETFL);
    if ((flags & (O_PATH|O_DIRECTORY)) != 0) {
        qWarning() << "Provided file is a directory or an O_PATH file descriptor";
        return false;
    }

    return true;
}

KAuth::ActionReply Helper::save(const QVariantMap &args)
{
    const QString optionsKey = u"options"_s;

    KAuth::ActionReply errorReply = KAuth::ActionReply::HelperErrorReply();
    Configs configs;
    QString errorMessage;

    auto options = args[optionsKey].value<QVariantMap>();

    auto setReplyError = [&errorReply](const QString &errorMessage) {
        errorReply.setErrorDescription(errorMessage);
        return errorReply;
    };

    for (auto i = options.constBegin(); i != options.constEnd(); ++i)
    {
        auto p = ParsedKey::parse(i.key(), errorMessage);
        if (!errorMessage.isEmpty()) return setReplyError(errorMessage);

        auto config = configs.getByFilename(p.fileName, errorMessage);
        if (!errorMessage.isEmpty()) return setReplyError(errorMessage);

        config->group(p.groupName).writeEntry(p.keyName, i.value());
    }

    // all keys except "options" carry in their value file descriptors of files
    // that need to be copied to the home directory of the greeter user
    Privileges privileges;
    if (!privileges.drop(s_greeterUserName)) {
        return setReplyError(u"Can't drop privileges to greeter user (%1)"_s.arg(s_greeterUserName));
    }

    for (auto i = args.constBegin() ; i != args.constEnd() ; ++i)
    {
        if (i.key() == optionsKey) continue;

        auto p = ParsedKey::parse(i.key(), errorMessage);
        if (!errorMessage.isEmpty()) return setReplyError(errorMessage);

        auto config = configs.getByFilename(p.fileName, errorMessage);
        if (!errorMessage.isEmpty()) return setReplyError(errorMessage);

        if (i.value().isNull()) {

            errorMessage = removeImage(p.groupName, p.keyName);
            if (!errorMessage.isEmpty()) return setReplyError(errorMessage);

            // erase the value of this option
            // do not delete the key so that the default value is not used
            config->group(p.groupName).writeEntry(p.keyName, u""_s);
            continue;
        }

        auto dbusFD = i.value().value<QDBusUnixFileDescriptor>();
        if (!dbusFD.isValid()) {
            return setReplyError(u"Invalid file descriptor for key: %1"_s.arg(i.key()));
        }

        if (!check_fd_sanity(dbusFD.fileDescriptor())) {
            return setReplyError(u"Invalid file for key: %1"_s.arg(i.key()));
        }

        QString ldmPath{};
        errorMessage = copyImage(dbusFD.fileDescriptor(), p.groupName, p.keyName, ldmPath);
        if (!errorMessage.isEmpty()) return setReplyError(errorMessage);

        config->group(p.groupName).writeEntry(p.keyName, ldmPath);
    }

    if (!privileges.reset()) {
        // If resetting privileges fails then there is a bigger issue at hand
        // and the helper should actually kill itself, because it can no longer
        // fulfill its purpose.
        exit(1);
    }

    configs.sync();

    return KAuth::ActionReply::SuccessReply();
}

static QString safe_theme_and_name(QString &theme, QString &name)
{
    // in case anyone put some path instead of name
    theme = QFileInfo(theme).fileName().trimmed();
    if (theme.isEmpty()) return u"Theme is empty"_s;
    if (theme == u".."_s) return u"Theme is equal to '..'"_s;

    name = QFileInfo(name).fileName().trimmed();
    if (name.isEmpty()) return u"Name is empty"_s;
    if (name == u".."_s) return u"Name is equal to '..'"_s;
    return QString{};
}

static QString construct_filename(const QString &theme, const QString &name)
{
    return QStringLiteral("%1/%2/%3").arg(QStringLiteral(GREETER_IMAGES_DIR)).arg(theme).arg(name);
}

/**
 * Remove the image from the home directory of the greeter.
 *
 * @param theme theme name, in which the image will be used
 * @param name filename, must not contain subdirectories, they are ignored
 * @return errorMessage reference to string, to write an error in it
 */
QString Helper::removeImage(QString theme, QString name)
{
    QString errorMessage = safe_theme_and_name(theme, name);
    if (!errorMessage.isEmpty()) return errorMessage;

    QFile dest{ construct_filename(theme, name) };

    if (!dest.exists()) return QString{};
    if (!dest.remove()) return u"Can't remove file: %1"_s.arg(dest.fileName());

    return QString{};
}

/**
 * Copy the image to the home directory of the greeter.
 *
 * @param sourceFD file descriptor
 * @param theme theme name, in which the image will be used
 * @param name filename, must not contain subdirectories, they are ignored
 * @param out argument, path to the resulting file
 * @return errorMessage reference to string, to write an error in it
 */
QString Helper::copyImage(int sourceFD, QString theme, QString name, QString &ldmPath)
{
    QString errorMessage = safe_theme_and_name(theme, name);
    if (!errorMessage.isEmpty()) return errorMessage;

    QFile source;
    if (!source.open(sourceFD, QFile::ReadOnly)) {
        return u"Can't open file from FD: %1"_s.arg(sourceFD);
    }

    // that should be enough
    constexpr qint64 maxImageFileSize = 1024 * 1024 * 50;

    if (source.size() > maxImageFileSize) {
        return u"Image size is too large: %1 max size %2"_s.arg(source.size()).arg(maxImageFileSize);
    }

    QFile dest{ construct_filename(theme, name) };

    if (dest.exists()) {
        dest.remove();
    } else {
        QDir dir{ QFileInfo{ dest }.absolutePath() };
        if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
            return u"Can't create target directory: %1"_s.arg(dir.absolutePath());
        }
    }

    if (!dest.open(QFile::WriteOnly)) {
        return u"Can't open destination file for writing: %1"_s.arg(dest.fileName());
    }

    auto mappedSource = source.map(0, source.size());
    if (!mappedSource) {
        return u"Can't map source file: %1"_s.arg(source.fileName());
    }

    qint64 written = dest.write((char*)mappedSource, source.size());
    if (written != source.size()) {
        return u"Not the whole image is copied, copied %1 from %2"_s.arg(written).arg(source.fileName());
    }

    ldmPath = dest.fileName();
    return {};
}

KAUTH_HELPER_MAIN("org.kde.kcontrol.kcmlightdm", Helper)

#include "moc_helper.cpp"
