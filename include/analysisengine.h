#ifndef ANALYSISENGINE_H
#define ANALYSISENGINE_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>
#include "datamanager.h"

class AnalysisEngine : public QObject
{
    Q_OBJECT

public:
    explicit AnalysisEngine(DataManager *dataManager, QObject *parent = nullptr);
    
    // Basic statistics
    struct StationStatistics {
        QString stationName;
        int totalPassengers;
        int boardingPassengers;
        int alightingPassengers;
        double averageTicketPrice;
        double totalRevenue;
        int peakHour;
        int peakDay;
    };
    
    struct TrainStatistics {
        QString trainCode;
        int totalPassengers;
        double utilizationRate;
        double averageTicketPrice;
        double totalRevenue;
        int totalTrips;
    };
    
    struct TimeSeriesData {
        QDate date;
        int passengers;
        double revenue;
    };
    
    struct TicketTypeAnalysis {
        QString ticketType;
        int totalCount;
        int totalPassengers;
        double totalRevenue;
        double averagePrice;
    };
    
    // Analysis methods
    QVector<StationStatistics> getStationStatistics() const;
    QVector<TrainStatistics> getTrainStatistics() const;
    QVector<TimeSeriesData> getTimeSeriesData(const QDate &startDate, const QDate &endDate) const;
    
    // Peak analysis
    QMap<int, int> getHourlyPeakAnalysis() const;
    QMap<int, int> getDailyPeakAnalysis() const;
    QMap<QString, int> getStationPeakAnalysis() const;
    
    // Correlation analysis
    QVector<QPair<QString, QString>> getStationCorrelations() const;
    QVector<QPair<QString, QString>> getTrainCorrelations() const;
    
    // Revenue analysis
    QMap<QString, double> getStationRevenueAnalysis() const;
    QMap<QString, double> getTrainRevenueAnalysis() const;
    double getAverageTicketPrice() const;

    QMap<QString, double> getStationFlowByDateRange(const QDate &startDate, const QDate &endDate) const;
    QMap<QString, double> getTrainFlowByDateRange(const QDate &startDate, const QDate &endDate) const;
    QVector<TimeSeriesData> getTotalPassengerFlowTimeSeries(const QDate &startDate, const QDate &endDate) const;
    QVector<TimeSeriesData> getPassengerFlowTimeSeriesByStation(const QString &stationName, const QDate &startDate, const QDate &endDate) const;
    QVector<TimeSeriesData> getPassengerFlowTimeSeriesByTrain(const QString &trainNumber, const QDate &startDate, const QDate &endDate) const;
    QVector<QPointF> getFlowAndTrainCountCorrelation(const QDate &startDate, const QDate &endDate) const;

    // Efficiency analysis
    QVector<QPair<QString, double>> getStationEfficiency() const;
    QVector<QPair<QString, double>> getTrainEfficiency() const;
    
    // Pattern analysis
    QMap<QString, QMap<int, int>> getStationHourlyPatterns() const;
    QMap<QString, QMap<int, int>> getStationDailyPatterns() const;
    
    // Summary statistics
    QString getAnalysisSummary() const;
    
    // Ticket analysis
    QVector<TicketTypeAnalysis> getTicketTypeAnalysis(const QDate &startDate, const QDate &endDate) const;
    QMap<double, int> getTicketPriceDistribution() const;
    QMap<QString, QMap<double, int>> getTicketTypeAndPriceAnalysis() const;

public:
    // Helper methods
    QVector<PassengerFlow*> getFilteredData() const;

private:
    DataManager *m_dataManager;
    
    // Helper methods
    double calculateCorrelation(const QVector<int> &x, const QVector<int> &y) const;
    QMap<int, int> aggregateByHour(const QVector<PassengerFlow*> &data) const;
    QMap<int, int> aggregateByDay(const QVector<PassengerFlow*> &data) const;
};

#endif // ANALYSISENGINE_H