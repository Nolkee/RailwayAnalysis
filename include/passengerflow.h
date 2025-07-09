#ifndef PASSENGERFLOW_H
#define PASSENGERFLOW_H

#include <QString>
#include <QDateTime>
#include <QObject>

class PassengerFlow : public QObject
{
    Q_OBJECT

public:
    explicit PassengerFlow(QObject *parent = nullptr);
    PassengerFlow(const QString &lineCode, const QString &trainCode, int stationId,
                  const QDate &date, const QTime &arrivalTime, const QTime &departureTime,
                  int boardingPassengers, int alightingPassengers, const QString &ticketType,
                  double ticketPrice, double revenue, QObject *parent = nullptr);
    
    // Getters
    QString getLineCode() const { return m_lineCode; }
    QString getTrainCode() const { return m_trainCode; }
    int getStationId() const { return m_stationId; }
    QDate getDate() const { return m_date; }
    QTime getArrivalTime() const { return m_arrivalTime; }
    QTime getDepartureTime() const { return m_departureTime; }
    int getBoardingPassengers() const { return m_boardingPassengers; }
    int getAlightingPassengers() const { return m_alightingPassengers; }
    int getTotalPassengers() const { return m_boardingPassengers + m_alightingPassengers; }
    QString getTicketType() const { return m_ticketType; }
    double getTicketPrice() const { return m_ticketPrice; }
    double getRevenue() const { return m_revenue; }
    QString getStartStation() const { return m_startStation; }
    QString getEndStation() const { return m_endStation; }
    
    // Setters
    void setLineCode(const QString &lineCode) { m_lineCode = lineCode; }
    void setTrainCode(const QString &trainCode) { m_trainCode = trainCode; }
    void setStationId(int stationId) { m_stationId = stationId; }
    void setDate(const QDate &date) { m_date = date; }
    void setArrivalTime(const QTime &arrivalTime) { m_arrivalTime = arrivalTime; }
    void setDepartureTime(const QTime &departureTime) { m_departureTime = departureTime; }
    void setBoardingPassengers(int passengers) { m_boardingPassengers = passengers; }
    void setAlightingPassengers(int passengers) { m_alightingPassengers = passengers; }
    void setTicketType(const QString &ticketType) { m_ticketType = ticketType; }
    void setTicketPrice(double price) { m_ticketPrice = price; }
    void setRevenue(double revenue) { m_revenue = revenue; }
    void setStartStation(const QString &station) { m_startStation = station; }
    void setEndStation(const QString &station) { m_endStation = station; }
    
    // Time analysis
    int getHour() const { return m_departureTime.hour(); }
    int getDayOfWeek() const { return m_date.dayOfWeek(); }
    bool isWeekend() const { return m_date.dayOfWeek() > 5; }
    bool isPeakHour() const { 
        int hour = m_departureTime.hour();
        return (hour >= 7 && hour <= 9) || (hour >= 17 && hour <= 19);
    }

private:
    QString m_lineCode;
    QString m_trainCode;
    int m_stationId;
    QDate m_date;
    QTime m_arrivalTime;
    QTime m_departureTime;
    int m_boardingPassengers;
    int m_alightingPassengers;
    QString m_ticketType;
    double m_ticketPrice;
    double m_revenue;
    QString m_startStation;
    QString m_endStation;
};

#endif // PASSENGERFLOW_H 