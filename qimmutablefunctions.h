#ifndef QSYNCABLEFUNCTIONS_H
#define QSYNCABLEFUNCTIONS_H

#include <QObject>
#include <QJSValue>
#include <QVariant>
#include <string.h>
#include <QMetaProperty>
#include <QDebug>

namespace QImmutable {

    /// Assign properties from source object to the destination object.
    void assign(QVariantMap& dest, const QObject*source);

    void assign(QObject* dest, const QVariantMap& source);

    void assign(QObject* dest, const QJSValue& source);

    template <typename T>
    void assignOnGadget(T& dest, const QVariantMap& source) {
        const QMetaObject meta = T::staticMetaObject;

        QMap<QString,QVariant>::const_iterator iter = source.begin();
        while (iter != source.end()) {
            QByteArray key = iter.key().toLocal8Bit();

            int index = meta.indexOfProperty(key.constData());
            if (index < 0) {
                qWarning() << QString("QImmutable::assign:assign a non-existed property: %1").arg(iter.key());
                iter++;
                continue;
            }
            QMetaProperty prop = meta.property(index);

            QVariant orig = prop.readOnGadget(&dest);
            QVariant value = source[iter.key()];

            if (orig.canConvert<QObject*>()) {
                if (value.type() != QVariant::Map) {
                    qWarning() << QString("QImmutable::assign:expect a QVariantMap property but it is not: %1");
                } else {
                    assign(orig.value<QObject*>(), value.toMap());
                }
            } else if (orig != value) {
                prop.writeOnGadget(&dest, value);
            }

            iter++;
        }
    }

    template <typename T>
    void assignOnGadget(QVariantMap& dest, const T& source) {

        const QMetaObject meta = T::staticMetaObject;

        for (int i = 0 ; i < meta.propertyCount(); i++) {
            const QMetaProperty property = meta.property(i);
            QString p = property.name();

            QVariant value = property.readOnGadget(&source);

            if (value.canConvert<QObject*>()) {
                QVariantMap map;
                assign(map, value.value<QObject*>()); // nested properties is not supported yet
                value = map;
            }

            dest[p] = value;
        }

    }

    /// Gets the value at path of object. If the path is not found, the defaultValue is returned.
    /*
     Example:

     get(object, "a.b.c");

    */
    QVariant get(const QObject* source, const QString& path, const QVariant& defaultValue = QVariant());

    QVariant get(const QObject* source, const QStringList& path, const QVariant& defaultValue = QVariant());

    QVariant get(const QVariantMap& source, const QString& path, const QVariant& defaultValue = QVariant());

    QVariant get(const QVariantMap& source, const QStringList& path, const QVariant& defaultValue = QVariant());

    /// Sets the value at path of object. If a portion of path doesn't exist, it's created.
    /*
     Example:

     set(data, "a.b", 3); // data["a"] will be a QVariantMap that contains a key of "b".

     */
    void set(QVariantMap& dest, const QString& path, const QVariant& value);

    void set(QVariantMap& dest, const QStringList& path, const QVariant& value);

    /// Creates an QVariantMap composed of the picked object properties at paths.
    /*
     Example:

         pick(object, QStringList() << "a" << "b.c");

     If a property contains QObject pointer, it will be converted to QVariantMap.

     In case you need to obtain a QObject pointer, please use get().

     */

    QVariantMap pick(QObject* source, const QStringList& paths);

    QVariantMap pick(QVariantMap source, const QStringList& paths);

    QVariantMap pick(QVariantMap source, const QVariantMap& paths);

    /// The opposite of pick(), this method creates an QVariantMap composed of the own properties that are not omitted.
    /*
     If a property contains QObject pointer, it will be converted to QVariantMap.

     In case you need to obtain a QObject pointer, please use get().
     */
    QVariantMap omit(const QVariantMap& source, const QVariantMap& properties);

    /// Compare two variant in a fast way. If it is a immutable type, it will simply compare the instance
    bool fastCompare(QVariant v1, QVariant v2);

    /// Find out the diff between QVariantMap

    QVariantMap diff(const QVariantMap& v1, const QVariantMap& v2);

    template <typename T>
    bool isShared(const T& v1, const T& v2) {
        return memcmp(&v1, &v2 , sizeof(T)) == 0;
    }

    template <typename T>
    QVariantMap toMap(const T& t) {
        QVariantMap res;
        assignOnGadget(res, t);
        return res;
    }
}

#endif // QSYNCABLEFUNCTIONS_H
