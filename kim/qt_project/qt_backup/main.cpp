#include "main.h"
#include <QApplication>
#include <QFont>
#include <QHostAddress>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

// TcpServerWorker Implementation
TcpServerWorker::TcpServerWorker(QObject *parent) : QThread(parent), tcpServer(new QTcpServer(this)), clientSocket(nullptr) {}

TcpServerWorker::~TcpServerWorker() {
    tcpServer->close();
}

void TcpServerWorker::run() {
    connect(tcpServer, &QTcpServer::newConnection, this, &TcpServerWorker::handleNewConnection);

    if (!tcpServer->listen(QHostAddress::Any, 8080)) {
        qWarning() << "Server failed to start on port 8080";
        return;
    }

    qInfo() << "Server is listening on port 8080...";
    exec(); // Start the thread event loop
}

void TcpServerWorker::handleNewConnection() {
    clientSocket = tcpServer->nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead, this, &TcpServerWorker::handleClientData);
    connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);

    qInfo() << "New client connected.";
}

void TcpServerWorker::handleClientData() {
    while (clientSocket && clientSocket->bytesAvailable() >= static_cast<qint64>(sizeof(sensor_dto))) {
        sensor_dto sensorData;
        clientSocket->read(reinterpret_cast<char *>(&sensorData), sizeof(sensor_dto));
        emit dataReceived(sensorData);
    }
}

// FullScreenWindow Implementation
FullScreenWindow::FullScreenWindow() {
    setWindowTitle("Sensor Dashboard");

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QGridLayout *gridLayout = new QGridLayout(centralWidget);

    // Create dashboard labels
    motorSpeedLabel = createStyledLabel("Motor Speed");
    precipitationLabel = createStyledLabel("Precipitation");
    tunnelDistanceLabel = createStyledLabel("Tunnel Distance");
    airConditionLabel = createStyledLabel("Air Condition");

    gridLayout->addWidget(motorSpeedLabel, 0, 0);
    gridLayout->addWidget(precipitationLabel, 0, 1);
    gridLayout->addWidget(tunnelDistanceLabel, 1, 0);
    gridLayout->addWidget(airConditionLabel, 1, 1);

    serverWorker = new TcpServerWorker(this);
    connect(serverWorker, &TcpServerWorker::dataReceived, this, &FullScreenWindow::onDataReceived);

    serverWorker->start();

    updateDisplay({0, 0, 0, 0}); // Initialize with empty data
}

void FullScreenWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        if (isFullScreenMode) {
            this->showNormal();
            isFullScreenMode = false;
        } else {
            this->showFullScreen();
            isFullScreenMode = true;
        }
    }
}

void FullScreenWindow::onDataReceived(sensor_dto data) {
    updateDisplay(data);
}

QLabel *FullScreenWindow::createStyledLabel(const QString &title) {
    QLabel *label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(
        "QLabel {"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   color: #333333;"
        "   border: 2px solid #CCCCCC;"
        "   padding: 15px;"
        "   border-radius: 10px;"
        "   background-color: #E0F7FA;"
        "}");
    label->setText(title + "\n---");
    return label;
}

void FullScreenWindow::updateDisplay(const sensor_dto &data) {
    motorSpeedLabel->setText(QString("Motor Speed\n%1").arg(data.motor_speed));
    precipitationLabel->setText(QString("Precipitation\n%1").arg(data.precipitation));
    tunnelDistanceLabel->setText(QString("Tunnel Distance\n%1").arg(data.tunnel_distance));
    airConditionLabel->setText(QString("Air Condition\n%1").arg(data.air_condition));
}

// Main Function
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    FullScreenWindow mainWindow;
    mainWindow.resize(600, 400);
    mainWindow.show();

    return app.exec();
}
