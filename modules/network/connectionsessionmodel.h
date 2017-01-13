#ifndef CONNECTIONSESSIONMODEL_H
#define CONNECTIONSESSIONMODEL_H

#include <QObject>
#include <QMap>
#include <QJsonObject>

#include <types/networkerrors.h>

namespace dcc {

namespace network {

class ConnectionSessionModel : public QObject
{
    Q_OBJECT

    friend class ConnectionSessionWorker;

public:
    explicit ConnectionSessionModel(QObject *parent = 0);

    const QString connectionPath() const { return m_connPath; }
    const NetworkErrors errors() const { return m_errors; }
    const QStringList sections() const { return m_sections; }
    const QList<QJsonObject> sectionKeys(const QString &section) const { return m_visibleItems[section]; }
    const QJsonObject keysInfo(const QString &section, const QString &vKey) const;
    const QString sectionName(const QString &section) const { return m_sectionName[section]; }
    const QString virtualSectionName(const QString &realSectionName) const { return m_virtualSectionName[realSectionName]; }
    const QMap<QString, QMap<QString, QJsonObject>> vkList() const { return m_vks; }
    const QJsonObject vkInfo(const QString &section, const QString &vKey) const { return m_vks[section][vKey]; }

signals:
    void connectionPathChanged(const QString &connPath) const;
    void connectionNameChanged(const QString &connName) const;
    void visibleItemsChanged(const QMap<QString, QList<QJsonObject>> &vkList) const;
    void keysChanged(const QMap<QString, QMap<QString, QJsonObject>> &vkList) const;
    void errorsChanged(const NetworkErrors &errors) const;
    void saveFinished(const bool ret) const;

private slots:
//    void setVisibleKeys(const QMap<QString, QStringList> &keys);
    void setConnPath(const QDBusObjectPath &connPath);
    void setErrors(const NetworkErrors &errors);
    void setAvailableItems(const QString &items);
    void setAllKeys(const QString &allKeys);

private:
    QString m_connPath;
    QList<QString> m_sections;
    NetworkErrors m_errors;
    QMap<QString, QString> m_sectionName;
    QMap<QString, QString> m_virtualSectionName;
    QMap<QString, QList<QJsonObject>> m_visibleItems;
    QMap<QString, QMap<QString, QJsonObject>> m_vks;
};

}

}

#endif // CONNECTIONSESSIONMODEL_H
