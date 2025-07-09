#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>
#include "station.h"
#include "train.h"
#include "passengerflow.h"

class DataManager : public QObject
{
    Q_OBJECT

public:
    explicit DataManager(QObject *parent = nullptr);
    
    // Data loading
    bool loadStations(const QString &filename);
    bool loadTrains(const QString &filename);
    bool loadPassengerFlow(const QString &filename);
    bool loadAllData();
    bool isDataLoaded() const;
    bool loadDataFromDirectory(const QString &path);

    // Data access
    QVector<Station*> getStations() const { return m_stations; }
    QVector<Train*> getTrains() const { return m_trains; }
    QVector<PassengerFlow*> getPassengerFlows() const { return m_passengerFlows; }
    
    Station* getStationById(int id) const;
    Station* getStationByName(const QString &name) const;
    Train* getTrainByCode(const QString &code) const;
    Train* getTrainByTrainCode(const QString &trainCode) const;
    int getStationIdByName(const QString &name) const;
    QStringList getStationNames() const;
    QStringList getTrainNumbers() const;
    
    // Statistics
    int getTotalPassengers() const;
    double getTotalRevenue() const;
    QMap<QString, int> getStationPassengerStats() const;
    QMap<QString, int> getTrainPassengerStats() const;
    QMap<int, int> getHourlyPassengerStats() const;
    QMap<int, int> getDailyPassengerStats() const;
    
    // Filtering
    QVector<PassengerFlow*> getPassengerFlowsByDate(const QDate &date) const;
    QVector<PassengerFlow*> getPassengerFlowsByStation(int stationId) const;
    QVector<PassengerFlow*> getPassengerFlowsByTrain(const QString &trainCode) const;
    QVector<PassengerFlow*> getPassengerFlowsByDateRange(const QDate &startDate, const QDate &endDate) const;
    
    // Data validation
    bool validateData() const;
    QString getDataSummary() const;

signals:
    void dataLoaded();
    void dataLoadError(const QString &error);

private:
    QVector<Station*> m_stations;
    QVector<Train*> m_trains;
    QVector<PassengerFlow*> m_passengerFlows;
    
    QMap<int, Station*> m_stationMap;
    QMap<QString, Train*> m_trainMap;
    
    void clearData();
    QTime parseTime(const QString &timeStr) const;
    QDate parseDate(const QString &dateStr) const;
};

#endif // DATAMANAGER_H