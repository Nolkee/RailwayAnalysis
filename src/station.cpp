#include "station.h"

Station::Station(QObject *parent)
    : QObject(parent)
    , m_id(0)
    , m_totalPassengers(0)
{
}

Station::Station(int id, const QString &name, const QString &code, const QString &shortName, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_name(name)
    , m_code(code)
    , m_shortName(shortName)
    , m_totalPassengers(0)
{
} 