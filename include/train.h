/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2025-07-05 21:59:50
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2025-07-07 15:01:30
 * @FilePath: \Railway\include\train.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef TRAIN_H
#define TRAIN_H

#include <QString>
#include <QObject>

class Train : public QObject
{
    Q_OBJECT

public:
    explicit Train(QObject *parent = nullptr);
    Train(const QString &code, const QString &trainCode, int capacity, QObject *parent = nullptr);
    
    // Getters
    QString getCode() const { return m_code; }
    QString getTrainCode() const { return m_trainCode; }
    int getCapacity() const { return m_capacity; }
    int getYearlyCapacity() const { return m_yearlyCapacity; }
    
    // Setters
    void setCode(const QString &code) { m_code = code; }
    void setTrainCode(const QString &trainCode) { m_trainCode = trainCode; }
    void setCapacity(int capacity) { m_capacity = capacity; }
    void setYearlyCapacity(int yearlyCapacity) { m_yearlyCapacity = yearlyCapacity; }
    
    // Statistics
    void addPassengerFlow(int passengers) { m_totalPassengers += passengers; }
    int getTotalPassengers() const { return m_totalPassengers; }
    double getUtilizationRate() const { 
        return m_capacity > 0 ? (double)m_totalPassengers / m_capacity : 0.0; 
    }
    void resetStatistics() { m_totalPassengers = 0; }

private:
    QString m_code;
    QString m_trainCode;
    int m_capacity;
    int m_yearlyCapacity;
    int m_totalPassengers;
};

#endif // TRAIN_H 