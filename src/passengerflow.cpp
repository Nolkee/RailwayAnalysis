#include "passengerflow.h"

PassengerFlow::PassengerFlow(QObject *parent)
    : QObject(parent)
    , m_stationId(0)
    , m_boardingPassengers(0)
    , m_alightingPassengers(0)
    , m_ticketPrice(0.0)
    , m_revenue(0.0)
{
}

PassengerFlow::PassengerFlow(const QString &lineCode, const QString &trainCode, int stationId,
                             const QDate &date, const QTime &arrivalTime, const QTime &departureTime,
                             int boardingPassengers, int alightingPassengers, const QString &ticketType,
                             double ticketPrice, double revenue, QObject *parent)
    : QObject(parent)
    , m_lineCode(lineCode)
    , m_trainCode(trainCode)
    , m_stationId(stationId)
    , m_date(date)
    , m_arrivalTime(arrivalTime)
    , m_departureTime(departureTime)
    , m_boardingPassengers(boardingPassengers)
    , m_alightingPassengers(alightingPassengers)
    , m_ticketType(ticketType)
    , m_ticketPrice(ticketPrice)
    , m_revenue(revenue)
{
} 