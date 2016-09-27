/*
 * popcorn (c) 2016 Michael Franzl
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QList>
#include <QVariantList>
#include <QSqlError>
#include <QDebug>
#include <QSettings>
#include "math.h"

Database::Database(QString label, QObject *parent) :
    QObject(parent)
{
    qDebug() << "Level0 [Database::Database] initialized";
    m_is_setup = false;
    m_label = label;
}

Database::~Database() {
    qDebug() << "Level0 [Database::~Database] Called";
    close();
    qDebug() << "Level0 [Database::~Database] Done";
}

void Database::setup(QString dbpath) {
    if (m_is_setup) {
        qDebug() << "Level0 [Database::setup] already set up" << dbpath;
        return;
    }

    qDebug() << "Level0 [Database::setup] setting up" << dbpath;
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbpath);
    m_db.setHostName("localhost");

    m_query = QSqlQuery(m_db);
    m_query.setForwardOnly(true);

    m_is_setup = true;
}


void Database::close() {
    m_db.close();
}

QVariantMap Database::open() {
    QVariantMap errors;
    bool success = m_db.open();
    if (!success) {
        errors.insert("db", m_db.lastError().databaseText());
        errors.insert("driver", m_db.lastError().driverText());
    }
    qDebug() << "Level0 [Database::open]" << errors;
    return errors;
}

bool Database::isOpen() {
    return m_db.isOpen();
}

bool Database::hasFeature(int feature) {
    return m_query.driver()->hasFeature((QSqlDriver::DriverFeature)feature);
}

QVariantMap Database::run(QString querystring) {
    qDebug() << "[Database::run] start" << querystring;
    QVariantMap result;
    QVariantMap errors;
    QVariantList view;

    m_query.clear();
    bool success = m_query.exec(querystring);

    if(success) {
        while (m_query.next()) {
            QSqlRecord record= m_query.record();
            QVariantMap map;
            for(int index = 0; index < record.count(); ++index) {
                QString key = record.fieldName(index);
                QVariant value = record.value(index);
                qulonglong val = value.toULongLong();
                if (val > pow(2, 53)) {
                    // return as string. javascript doesn't support more than 2^53
                    QString string;
                    string.setNum(val);
                    value = string;
                }
                map.insert(key, value);
            }
            view.append(map);
        }
        result.insert("lastInsertID", m_query.lastInsertId());
    } else {
        errors.insert("driver", m_query.lastError().driverText());
        errors.insert("db", m_query.lastError().databaseText());
    }
    result.insert("success", success);
    result.insert("view", view);
    result.insert("errors", errors);
    result.insert("query", querystring);
    result.insert("numRowsAffected", m_query.numRowsAffected());
    return result;
}
