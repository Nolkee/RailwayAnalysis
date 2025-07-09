#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTabWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QProgressBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QApplication>
#include "datamanager.h"
#include "analysisengine.h"
#include "predictionmodel.h"
#include "tablewidget.h"
#include "chartwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // Data management
    void onLoadData();
    void onExportData();
    void onExportChart();
    
    // Analysis actions
    void onAnalyzeStations();
    void onAnalyzeTrains();
    void onAnalyzeTimeSeries();
    void onAnalyzeCorrelations();
    void onAnalyze();
    
    // Prediction actions
    void onPredictPassengerFlow();
    void onPredictStationFlow();
    void onPredictTrainFlow();
    void onEvaluateModel();
    void onPredict();
    
    // Filter actions
    void onFilterByDate();
    void onFilterByStation();
    void onFilterByTrain();
    void onClearFilters();
    
    // Chart actions
    void onRefreshChart();
    void onSaveChart();
    
    // Data events
    void onDataLoaded();
    void onDataLoadError(const QString &error);
    
    // Utility actions
    void onSettings();
    void onAbout();
    
    // Progress and status
    void updateStatus(const QString &message);
    void updateProgress(int value);
    void updateControlStates();
    void onShowStationAnalysis();
    void onShowTrainAnalysis();
    void onShowTimeSeriesAnalysis();
    void onShowCorrelationAnalysis();
    void onShowTicketTypeAnalysis(); // 新增车票类型分析

private:
    // UI setup methods
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupCentralWidget();
    void setupControlPanel();
    void setupStatusBar();
    void setupConnections();
    
    // Data management
    void loadSettings();
    void saveSettings();
    void populateComboBoxes();
    bool validateDataLoaded();
    bool validateStationSelected();
    bool validateTrainSelected();
    
    // Utility methods
    void showInfo(const QString &title, const QString &message);
    void showError(const QString &title, const QString &message);
    void showAnalysisResults();
    void showPredictionResults();

private:
    // Core components
    DataManager *m_dataManager;
    AnalysisEngine *m_analysisEngine;
    PredictionModel *m_predictionModel;
    TableWidget *m_tableWidget;
    ChartWidget *m_chartWidget;
    
    // UI components
    QSplitter *m_mainSplitter;
    QTabWidget *m_tabWidget;
    QWidget *m_controlPanel;
    QStatusBar *m_statusBar;
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;
    
    // Control buttons
    QPushButton *m_analyzeButton;
    QPushButton *m_exportButton;
    
    // Analysis controls
    QComboBox *m_analysisTypeCombo;
    
    // Filter controls
    QDateEdit *m_startDateEdit;
    QDateEdit *m_endDateEdit;
    QComboBox *m_stationCombo;
    QComboBox *m_trainCombo;

    QComboBox *m_predictionTargetCombo;
    QSpinBox *m_predictionDaysSpin;
    QPushButton *m_predictButton;

    QSettings *m_settings;
};

#endif // MAINWINDOW_H