#ifndef STATION_H
#define STATION_H

#include <QString>
#include <QObject>

class Station : public QObject
{
    Q_OBJECT

public:
    explicit Station(QObject *parent = nullptr);
    Station(int id, const QString &name, const QString &code, const QString &shortName, QObject *parent = nullptr);
    
    // Getters
    int getId() const { return m_id; }
    QString getName() const { return m_name; }
    QString getCode() const { return m_code; }
    QString getShortName() const { return m_shortName; }
    QString getTelecode() const { return m_telecode; }
    
    // Setters
    void setId(int id) { m_id = id; }
    void setName(const QString &name) { m_name = name; }
    void setCode(const QString &code) { m_code = code; }
    void setShortName(const QString &shortName) { m_shortName = shortName; }
    void setTelecode(const QString &telecode) { m_telecode = telecode; }
    
    // Statistics
    void addPassengerFlow(int passengers) { m_totalPassengers += passengers; }
    int getTotalPassengers() const { return m_totalPassengers; }
    void resetStatistics() { m_totalPassengers = 0; }

private:
    int m_id;
    QString m_name;
    QString m_code;
    QString m_shortName;
    QString m_telecode;
    int m_totalPassengers;
};

#endif // STATION_H 