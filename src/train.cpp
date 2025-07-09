#include "train.h"

Train::Train(QObject *parent)
    : QObject(parent)
    , m_capacity(0)
    , m_yearlyCapacity(0)
    , m_totalPassengers(0)
{
}

Train::Train(const QString &code, const QString &trainCode, int capacity, QObject *parent)
    : QObject(parent)
    , m_code(code)
    , m_trainCode(trainCode)
    , m_capacity(capacity)
    , m_yearlyCapacity(0)
    , m_totalPassengers(0)
{
} 