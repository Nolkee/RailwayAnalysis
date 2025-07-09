#include "predictionmodel.h"
#include <QDebug>
#include <QtMath>
#include <QRandomGenerator>
#include <algorithm>
#include <cmath>

PredictionModel::PredictionModel(AnalysisEngine *analysisEngine, QObject *parent)
    : QObject(parent)
    , m_analysisEngine(analysisEngine)
{
    // Initialize default parameters
    m_currentParams.windowSize = 7;
    m_currentParams.alpha = 0.3;
    m_currentParams.beta = 0.1;
    m_currentParams.gamma = 0.1;
    m_currentParams.seasonality = 7;
}

QVector<PredictionModel::PredictionResult> PredictionModel::predictPassengerFlow(
    const QDate &startDate, int days, const ModelParameters &params)
{
    QVector<PredictionResult> predictions;
    
    // Get historical data for training
    QDate trainingStart = startDate.addDays(-30); // Use last 30 days for training
    auto historicalData = m_analysisEngine->getTimeSeriesData(trainingStart, startDate.addDays(-1));
    
    if (historicalData.size() < 10) {
        qDebug() << "Insufficient historical data for prediction";
        return predictions;
    }
    
    // Extract time series
    QVector<double> timeSeries = extractTimeSeries(historicalData);
    
    // Use provided parameters or default
    ModelParameters modelParams = params.windowSize > 0 ? params : m_currentParams;
    
    // Calculate trend and seasonality components
    double trend = calculateTrend(timeSeries);
    QVector<double> seasonalPattern = calculateSeasonality(timeSeries, modelParams.seasonality);
    double baseLevel = calculateBaseLevel(timeSeries);
    
    // Make predictions
    for (int i = 0; i < days; ++i) {
        QDate predictDate = startDate.addDays(i);
        
        PredictionResult result;
        result.date = predictDate;
        result.label = predictDate.toString("yyyy-MM-dd");
        
        // Base prediction with trend
        double basePrediction = baseLevel + (trend * (i + 1));
        
        // Add seasonal component
        int seasonalIndex = (timeSeries.size() + i) % modelParams.seasonality;
        if (seasonalIndex < seasonalPattern.size()) {
            basePrediction += seasonalPattern[seasonalIndex];
        }
        
        // Add weekly pattern (weekend vs weekday)
        int dayOfWeek = predictDate.dayOfWeek();
        if (dayOfWeek == 6 || dayOfWeek == 7) { // Weekend
            basePrediction *= 0.8; // Weekend typically has lower traffic
        } else {
            basePrediction *= 1.1; // Weekday has higher traffic
        }
        
        // Add random variation (±10%)
        double randomFactor = 0.9 + (QRandomGenerator::global()->bounded(20)) / 100.0;
        basePrediction *= randomFactor;
        
        // Add growth trend for future dates
        double growthFactor = 1.0 + (i * 0.02); // 2% growth per day
        basePrediction *= growthFactor;
        
        result.predictedPassengers = std::max(0, static_cast<int>(basePrediction));
        
        // Calculate confidence interval
        double stdDev = calculateStandardDeviation(timeSeries);
        double confidenceWidth = stdDev * (1.0 + i * 0.1); // Confidence widens over time
        result.confidence = 0.95;
        result.lowerBound = std::max(0, result.predictedPassengers - static_cast<int>(confidenceWidth));
        result.upperBound = result.predictedPassengers + static_cast<int>(confidenceWidth);
        
        predictions.append(result);
    }
    
    return predictions;
}

QVector<PredictionModel::PredictionResult> PredictionModel::predictStationFlow(
    const QString &stationName, const QDate &startDate, int days, const ModelParameters &params)
{
    QVector<PredictionResult> predictions;
    
    // Get station-specific data for training
    QDate trainingStart = startDate.addDays(-30); // Use last 30 days for training
    auto historicalData = m_analysisEngine->getPassengerFlowTimeSeriesByStation(stationName, trainingStart, startDate.addDays(-1));
    
    if (historicalData.size() < 10) {
        qDebug() << "Insufficient station data for prediction for" << stationName;
        return predictions;
    }

    // Extract time series
    QVector<double> timeSeries = extractTimeSeries(historicalData);
    
    // Use provided parameters or default
    ModelParameters modelParams = params.windowSize > 0 ? params : m_currentParams;
    
    // Calculate trend and seasonality components
    double trend = calculateTrend(timeSeries);
    QVector<double> seasonalPattern = calculateSeasonality(timeSeries, modelParams.seasonality);
    double baseLevel = calculateBaseLevel(timeSeries);

    // Make predictions
    for (int i = 0; i < days; ++i) {
        QDate predictDate = startDate.addDays(i);
        
        PredictionResult result;
        result.date = predictDate;
        result.label = predictDate.toString("yyyy-MM-dd");
        
        // Base prediction with trend
        double basePrediction = baseLevel + (trend * (i + 1));
        
        // Add seasonal component
        int seasonalIndex = (timeSeries.size() + i) % modelParams.seasonality;
        if (seasonalIndex < seasonalPattern.size()) {
            basePrediction += seasonalPattern[seasonalIndex];
        }
        
        // Add weekly pattern (weekend vs weekday)
        int dayOfWeek = predictDate.dayOfWeek();
        if (dayOfWeek == 6 || dayOfWeek == 7) { // Weekend
            basePrediction *= 0.8; // Weekend typically has lower traffic
        } else {
            basePrediction *= 1.1; // Weekday has higher traffic
        }
        
        // Add random variation (±10%)
        double randomFactor = 0.9 + (QRandomGenerator::global()->bounded(20)) / 100.0;
        basePrediction *= randomFactor;
        
        // Add growth trend for future dates
        double growthFactor = 1.0 + (i * 0.02); // 2% growth per day
        basePrediction *= growthFactor;
        
        result.predictedPassengers = std::max(0, static_cast<int>(basePrediction));
        
        // Calculate confidence interval
        double stdDev = calculateStandardDeviation(timeSeries);
        double confidenceWidth = stdDev * (1.0 + i * 0.1); // Confidence widens over time
        result.confidence = 0.95;
        result.lowerBound = std::max(0, result.predictedPassengers - static_cast<int>(confidenceWidth));
        result.upperBound = result.predictedPassengers + static_cast<int>(confidenceWidth);
        
        predictions.append(result);
    }
    
    return predictions;
}

QVector<PredictionModel::PredictionResult> PredictionModel::predictTrainFlow(
    const QString &trainNumber, const QDate &startDate, int days, const ModelParameters &params)
{
    QVector<PredictionResult> predictions;
    
    // Get train-specific data for training
    QDate trainingStart = startDate.addDays(-30); // Use last 30 days for training
    auto historicalData = m_analysisEngine->getPassengerFlowTimeSeriesByTrain(trainNumber, trainingStart, startDate.addDays(-1));
    
    if (historicalData.size() < 10) {
        qDebug() << "Insufficient train data for prediction for" << trainNumber;
        return predictions;
    }
    
    // Extract time series
    QVector<double> timeSeries = extractTimeSeries(historicalData);
    
    // Use provided parameters or default
    ModelParameters modelParams = params.windowSize > 0 ? params : m_currentParams;
    
    // Calculate trend and seasonality components
    double trend = calculateTrend(timeSeries);
    QVector<double> seasonalPattern = calculateSeasonality(timeSeries, modelParams.seasonality);
    double baseLevel = calculateBaseLevel(timeSeries);

    // Make predictions
    for (int i = 0; i < days; ++i) {
        QDate predictDate = startDate.addDays(i);
        
        PredictionResult result;
        result.date = predictDate;
        result.label = predictDate.toString("yyyy-MM-dd");
        
        // Base prediction with trend
        double basePrediction = baseLevel + (trend * (i + 1));
        
        // Add seasonal component
        int seasonalIndex = (timeSeries.size() + i) % modelParams.seasonality;
        if (seasonalIndex < seasonalPattern.size()) {
            basePrediction += seasonalPattern[seasonalIndex];
        }
        
        // Add weekly pattern (weekend vs weekday)
        int dayOfWeek = predictDate.dayOfWeek();
        if (dayOfWeek == 6 || dayOfWeek == 7) { // Weekend
            basePrediction *= 0.8; // Weekend typically has lower traffic
        } else {
            basePrediction *= 1.1; // Weekday has higher traffic
        }
        
        // Add random variation (±10%)
        double randomFactor = 0.9 + (QRandomGenerator::global()->bounded(20)) / 100.0;
        basePrediction *= randomFactor;
        
        // Add growth trend for future dates
        double growthFactor = 1.0 + (i * 0.02); // 2% growth per day
        basePrediction *= growthFactor;
        
        result.predictedPassengers = std::max(0, static_cast<int>(basePrediction));
        
        // Calculate confidence interval
        double stdDev = calculateStandardDeviation(timeSeries);
        double confidenceWidth = stdDev * (1.0 + i * 0.1); // Confidence widens over time
        result.confidence = 0.95;
        result.lowerBound = std::max(0, result.predictedPassengers - static_cast<int>(confidenceWidth));
        result.upperBound = result.predictedPassengers + static_cast<int>(confidenceWidth);
        
        predictions.append(result);
    }
    
    return predictions;
}

PredictionModel::ModelEvaluation PredictionModel::evaluateModel(
    const QVector<PredictionResult> &predictions,
    const QVector<AnalysisEngine::TimeSeriesData> &actual) const
{
    ModelEvaluation evaluation;
    evaluation.mae = 0.0;
    evaluation.mse = 0.0;
    evaluation.rmse = 0.0;
    evaluation.mape = 0.0;
    
    if (predictions.isEmpty() || actual.isEmpty()) {
        return evaluation;
    }
    
    int n = qMin(predictions.size(), actual.size());
    double sumAbsError = 0.0;
    double sumSquaredError = 0.0;
    double sumPercentageError = 0.0;
    
    for (int i = 0; i < n; ++i) {
        int predicted = predictions[i].predictedPassengers;
        int actualValue = actual[i].passengers;
        
        int absError = std::abs(predicted - actualValue);
        sumAbsError += absError;
        sumSquaredError += absError * absError;
        
        if (actualValue > 0) {
            sumPercentageError += std::abs((double)(predicted - actualValue) / actualValue);
        }
    }
    
    evaluation.mae = sumAbsError / n;
    evaluation.mse = sumSquaredError / n;
    evaluation.rmse = std::sqrt(evaluation.mse);
    evaluation.mape = (sumPercentageError / n) * 100.0;
    
    return evaluation;
}

bool PredictionModel::trainModel(const QVector<AnalysisEngine::TimeSeriesData> &trainingData)
{
    if (trainingData.size() < 10) {
        return false;
    }
    
    // Extract time series
    QVector<double> timeSeries = extractTimeSeries(trainingData);
    
    // Simple linear regression for trend
    QVector<double> x, y;
    for (int i = 0; i < timeSeries.size(); ++i) {
        x.append(i);
        y.append(timeSeries[i]);
    }
    
    m_trainedCoefficients = fitLinearRegression(x, y);
    
    return m_trainedCoefficients.size() >= 2;
}

PredictionModel::ModelParameters PredictionModel::optimizeParameters(
    const QVector<AnalysisEngine::TimeSeriesData> &data)
{
    ModelParameters bestParams = m_currentParams;
    double bestScore = std::numeric_limits<double>::max();
    
    // Grid search for optimal parameters
    QVector<double> alphas = {0.1, 0.2, 0.3, 0.4, 0.5};
    QVector<double> betas = {0.05, 0.1, 0.15, 0.2};
    QVector<int> windows = {3, 5, 7, 10, 14};
    
    for (double alpha : alphas) {
        for (double beta : betas) {
            for (int window : windows) {
                ModelParameters params;
                params.alpha = alpha;
                params.beta = beta;
                params.windowSize = window;
                params.gamma = 0.1;
                params.seasonality = 7;
                
                // Test parameters on a subset of data
                int testSize = qMin(30, data.size() / 2);
                auto testData = data.mid(0, testSize);
                auto validationData = data.mid(testSize, testSize);
                
                if (validationData.size() < 5) continue;
                
                auto predictions = predictPassengerFlow(
                    validationData.first().date, validationData.size(), params);
                
                auto evaluation = evaluateModel(predictions, validationData);
                
                if (evaluation.rmse < bestScore) {
                    bestScore = evaluation.rmse;
                    bestParams = params;
                }
            }
        }
    }
    
    return bestParams;
}

QVector<double> PredictionModel::calculateMovingAverage(const QVector<double> &data, int window) const
{
    QVector<double> result;
    
    if (data.size() < window) {
        return result;
    }
    
    for (int i = window - 1; i < data.size(); ++i) {
        double sum = 0.0;
        for (int j = i - window + 1; j <= i; ++j) {
            sum += data[j];
        }
        result.append(sum / window);
    }
    
    return result;
}

QVector<double> PredictionModel::calculateExponentialSmoothing(
    const QVector<double> &data, double alpha) const
{
    QVector<double> result;
    
    if (data.isEmpty()) {
        return result;
    }
    
    result.append(data.first());
    
    for (int i = 1; i < data.size(); ++i) {
        double smoothed = alpha * data[i] + (1 - alpha) * result.last();
        result.append(smoothed);
    }
    
    return result;
}

double PredictionModel::calculateTrend(const QVector<double> &data) const
{
    if (data.size() < 2) {
        return 0.0;
    }
    
    // Simple linear trend calculation
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    int n = data.size();
    
    for (int i = 0; i < n; ++i) {
        double x = static_cast<double>(i);
        double y = data[i];
        
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }
    
    double slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
    return slope;
}

QVector<double> PredictionModel::calculateSeasonality(const QVector<double> &data, int period) const
{
    QVector<double> seasonality;
    
    if (data.size() < period * 2) {
        return seasonality;
    }
    
    // Calculate seasonal components
    for (int i = period; i < data.size(); ++i) {
        double seasonal = data[i - period];
        seasonality.append(seasonal);
    }
    
    return seasonality;
}

QVector<double> PredictionModel::extractTimeSeries(const QVector<AnalysisEngine::TimeSeriesData> &data) const
{
    QVector<double> timeSeries;
    for (const AnalysisEngine::TimeSeriesData &item : data) {
        timeSeries.append(item.passengers);
    }
    return timeSeries;
}

double PredictionModel::calculateHoltWinters(const QVector<double> &data, int index,
                                           double alpha, double beta, double gamma, int seasonality) const
{
    if (index < seasonality) {
        return data[index];
    }
    
    // Simplified Holt-Winters implementation using the parameters
    double level = data[index - seasonality];
    double trend = 0.0;
    double seasonal = data[index - seasonality];
    
    if (index >= seasonality * 2) {
        trend = (data[index - seasonality] - data[index - seasonality * 2]) / seasonality;
        // Apply smoothing parameters
        level = alpha * data[index - seasonality] + (1 - alpha) * level;
        trend = beta * trend + (1 - beta) * trend;
        seasonal = gamma * (data[index - seasonality] - level) + (1 - gamma) * seasonal;
    }
    
    return level + trend + seasonal;
}

QVector<double> PredictionModel::fitLinearRegression(const QVector<double> &x, const QVector<double> &y) const
{
    QVector<double> coefficients;
    
    if (x.size() != y.size() || x.size() < 2) {
        return coefficients;
    }
    
    int n = x.size();
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    
    for (int i = 0; i < n; ++i) {
        sumX += x[i];
        sumY += y[i];
        sumXY += x[i] * y[i];
        sumX2 += x[i] * x[i];
    }
    
    double slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
    double intercept = (sumY - slope * sumX) / n;
    
    coefficients.append(intercept);
    coefficients.append(slope);
    
    return coefficients;
}

double PredictionModel::calculateConfidenceInterval(const QVector<double> &residuals, double confidence) const
{
    if (residuals.isEmpty()) {
        return 0.0;
    }
    
    // Calculate standard deviation of residuals
    double mean = 0.0;
    for (double residual : residuals) {
        mean += residual;
    }
    mean /= residuals.size();
    
    double variance = 0.0;
    for (double residual : residuals) {
        variance += (residual - mean) * (residual - mean);
    }
    variance /= residuals.size();
    
    double stdDev = std::sqrt(variance);
    
    // Apply confidence level (simplified - using normal distribution)
    // For 95% confidence, use 1.96; for 90% confidence, use 1.645
    double zScore = (confidence > 0.95) ? 1.96 : 1.645;
    
    return zScore * stdDev;
}

double PredictionModel::calculateStandardDeviation(const QVector<double> &data) const
{
    if (data.isEmpty()) {
        return 0.0;
    }
    
    double mean = 0.0;
    for (double value : data) {
        mean += value;
    }
    mean /= data.size();
    
    double variance = 0.0;
    for (double value : data) {
        variance += (value - mean) * (value - mean);
    }
    variance /= data.size();
    
    return std::sqrt(variance);
}

double PredictionModel::calculateBaseLevel(const QVector<double> &data) const
{
    if (data.isEmpty()) {
        return 0.0;
    }
    
    // Calculate the average of recent data (last 7 days)
    int recentDays = qMin(7, data.size());
    double sum = 0.0;
    
    for (int i = data.size() - recentDays; i < data.size(); ++i) {
        sum += data[i];
    }
    
    return sum / recentDays;
}