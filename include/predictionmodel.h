#ifndef PREDICTIONMODEL_H
#define PREDICTIONMODEL_H

#include <QObject>
#include <QVector>
#include <QDate>
#include <QPair>
#include "analysisengine.h"

class PredictionModel : public QObject
{
    Q_OBJECT

public:
    explicit PredictionModel(AnalysisEngine *analysisEngine, QObject *parent = nullptr);
    
    // Prediction structures
    struct PredictionResult {
        QString label; // Can be date, station name, etc.
        QDate date;
        int predictedPassengers;
        double confidence;
        double lowerBound;
        double upperBound;
    };
    
    struct ModelParameters {
        int windowSize;
        double alpha;
        double beta;
        double gamma;
        int seasonality;
    };
    
    // Prediction methods
    QVector<PredictionResult> predictPassengerFlow(const QDate &startDate, int days, 
                                                  const ModelParameters &params = ModelParameters());
    QVector<PredictionResult> predictStationFlow(const QString &stationName, const QDate &startDate, 
                                                int days, const ModelParameters &params = ModelParameters());
    QVector<PredictionResult> predictTrainFlow(const QString &trainNumber, const QDate &startDate, 
                                              int days, const ModelParameters &params = ModelParameters());
    
    // Model evaluation
    struct ModelEvaluation {
        double mae;  // Mean Absolute Error
        double mse;  // Mean Squared Error
        double rmse; // Root Mean Squared Error
        double mape; // Mean Absolute Percentage Error
    };
    
    ModelEvaluation evaluateModel(const QVector<PredictionResult> &predictions, 
                                 const QVector<AnalysisEngine::TimeSeriesData> &actual) const;
    
    // Model training
    bool trainModel(const QVector<AnalysisEngine::TimeSeriesData> &trainingData);
    ModelParameters optimizeParameters(const QVector<AnalysisEngine::TimeSeriesData> &data);
    
    // Utility methods
    QVector<double> calculateMovingAverage(const QVector<double> &data, int window) const;
    QVector<double> calculateExponentialSmoothing(const QVector<double> &data, double alpha) const;
    double calculateTrend(const QVector<double> &data) const;
    QVector<double> calculateSeasonality(const QVector<double> &data, int period) const;
    double calculateBaseLevel(const QVector<double> &data) const;
    double calculateStandardDeviation(const QVector<double> &data) const;

private:
    AnalysisEngine *m_analysisEngine;
    ModelParameters m_currentParams;
    QVector<double> m_trainedCoefficients;
    
    // Helper methods
    QVector<double> extractTimeSeries(const QVector<AnalysisEngine::TimeSeriesData> &data) const;
    double calculateHoltWinters(const QVector<double> &data, int index, 
                               double alpha, double beta, double gamma, int seasonality) const;
    QVector<double> fitLinearRegression(const QVector<double> &x, const QVector<double> &y) const;
    double calculateConfidenceInterval(const QVector<double> &residuals, double confidence) const;
};

#endif // PREDICTIONMODEL_H