#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDateTime>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_timer(new QTimer(this))
    , m_serverUrl("http://localhost:5000/api/data")
{
    setWindowTitle("Weather Station Client");
    setMinimumSize(800, 600);
    
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &MainWindow::onDataReceived);
    
    connect(m_timer, &QTimer::timeout, this, &MainWindow::requestData);
    m_timer->start(2000); // Update every 2 seconds
    
    setupUI();
    requestData();
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Title
    QLabel *titleLabel = new QLabel("🌤️ Weather Station Monitor", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Last update label
    m_lastUpdateLabel = new QLabel("Last update: -", this);
    m_lastUpdateLabel->setAlignment(Qt::AlignRight);
    mainLayout->addWidget(m_lastUpdateLabel);
    
    // Arduino sensors group
    QGroupBox *arduinoGroup = new QGroupBox("📡 Arduino Sensors", this);
    QVBoxLayout *arduinoLayout = new QVBoxLayout(arduinoGroup);
    m_arduinoLabel = new QLabel("Loading...", this);
    m_arduinoLabel->setWordWrap(true);
    arduinoLayout->addWidget(m_arduinoLabel);
    mainLayout->addWidget(arduinoGroup);
    
    // Elbear sensors group
    QGroupBox *elbearGroup = new QGroupBox("🤖 Elbear Sensors", this);
    QVBoxLayout *elbearLayout = new QVBoxLayout(elbearGroup);
    m_elbearLabel = new QLabel("Loading...", this);
    m_elbearLabel->setWordWrap(true);
    elbearLayout->addWidget(m_elbearLabel);
    mainLayout->addWidget(elbearGroup);
    
    setCentralWidget(centralWidget);
}

void MainWindow::requestData()
{
    QNetworkRequest request{QUrl(m_serverUrl)};
    m_networkManager->get(request);
}

void MainWindow::onDataReceived(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObj = jsonDoc.object();
        
        updateDisplay(jsonObj);
    } else {
        m_arduinoLabel->setText("Error: " + reply->errorString());
        m_elbearLabel->setText("Check server connection");
    }
    
    reply->deleteLater();
}

void MainWindow::updateDisplay(const QJsonObject &data)
{
    QJsonObject arduino = data["arduino"].toObject();
    QJsonObject elbear = data["elbear"].toObject();
    QString lastUpdate = data["last_update"].toString();
    
    // Update Arduino display
    QString arduinoText = QString(
        "LM75A Temperature: %1 °C\n"
        "DHT22 Temperature: %2 °C\n"
        "DHT22 Humidity: %3 %\n"
        "BMP280 Temperature: %4 °C\n"
        "BMP280 Pressure: %5 mmHg\n"
        "Timestamp: %6"
    ).arg(arduino["lm75a_temp"].isNull() ? "N/A" : QString::number(arduino["lm75a_temp"].toDouble(), 'f', 1))
     .arg(arduino["dht_temp"].isNull() ? "N/A" : QString::number(arduino["dht_temp"].toDouble(), 'f', 1))
     .arg(arduino["dht_humidity"].isNull() ? "N/A" : QString::number(arduino["dht_humidity"].toDouble(), 'f', 1))
     .arg(arduino["bmp_temp"].isNull() ? "N/A" : QString::number(arduino["bmp_temp"].toDouble(), 'f', 1))
     .arg(arduino["bmp_pressure"].isNull() ? "N/A" : QString::number(arduino["bmp_pressure"].toDouble(), 'f', 1))
     .arg(arduino["timestamp"].toString());
    
    m_arduinoLabel->setText(arduinoText);
    
    // Update Elbear display
    QString elbearText = QString(
        "MGS-THP80 Temperature: %1 °C\n"
        "MGS-THP80 Humidity: %2 %\n"
        "MGS-THP80 Pressure: %3 mmHg\n"
        "Wind Speed: %4 m/s\n"
        "Wind Direction: %5°\n"
        "Rainfall: %6 mm\n"
        "Radiation: %7 μSv/h\n"
        "Light Intensity: %8 lux\n"
        "Sound Level: %9 dB\n"
        "Gas Level: %10 ppm\n"
        "CO Level: %11 ppm\n"
        "Distance: %12 cm\n"
        "Timestamp: %13"
    ).arg(elbear["thp_temp"].isNull() ? "N/A" : QString::number(elbear["thp_temp"].toDouble(), 'f', 1))
     .arg(elbear["thp_humidity"].isNull() ? "N/A" : QString::number(elbear["thp_humidity"].toDouble(), 'f', 1))
     .arg(elbear["thp_pressure"].isNull() ? "N/A" : QString::number(elbear["thp_pressure"].toDouble(), 'f', 1))
     .arg(elbear["wind_speed"].isNull() ? "N/A" : QString::number(elbear["wind_speed"].toDouble(), 'f', 1))
     .arg(elbear["wind_direction"].isNull() ? "N/A" : QString::number(elbear["wind_direction"].toDouble(), 'f', 1))
     .arg(elbear["rainfall"].isNull() ? "N/A" : QString::number(elbear["rainfall"].toDouble(), 'f', 1))
     .arg(elbear["radiation"].isNull() ? "N/A" : QString::number(elbear["radiation"].toDouble(), 'f', 2))
     .arg(elbear["light_intensity"].isNull() ? "N/A" : QString::number(elbear["light_intensity"].toDouble(), 'f', 1))
     .arg(elbear["sound_level"].isNull() ? "N/A" : QString::number(elbear["sound_level"].toDouble(), 'f', 1))
     .arg(elbear["gas_level"].isNull() ? "N/A" : QString::number(elbear["gas_level"].toDouble(), 'f', 2))
     .arg(elbear["co_level"].isNull() ? "N/A" : QString::number(elbear["co_level"].toDouble(), 'f', 2))
     .arg(elbear["distance"].isNull() ? "N/A" : QString::number(elbear["distance"].toDouble(), 'f', 1))
     .arg(elbear["timestamp"].toString());
    
    m_elbearLabel->setText(elbearText);
    
    // Update last update time
    m_lastUpdateLabel->setText("Last update: " + lastUpdate);
}

MainWindow::~MainWindow()
{
}
