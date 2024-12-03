#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QKeyEvent>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMutex>

struct sensor_dto {
    int motor_speed;
    int precipitation;
    int tunnel_distance;
    int air_condition;
};

class BackgroundWidget : public QWidget {
    Q_OBJECT
private:
    QTcpServer *server;
    QTcpSocket *clientSocket;
    sensor_dto sensorData;
    QMutex mutex;

public:
    explicit BackgroundWidget(QWidget *parent = nullptr) : QWidget(parent), server(nullptr), clientSocket(nullptr) {
        // 서버 설정
        server = new QTcpServer(this);
        connect(server, &QTcpServer::newConnection, this, &BackgroundWidget::onNewConnection);

        if (!server->listen(QHostAddress::Any, 8080)) { // 포트 12345에서 대기
            qWarning("Server failed to start!");
        } else {
            qDebug("Server started on port 12345.");
        }
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);

        QPainter painter(this);
        QPixmap background("car_dashboard.jpg");
        painter.drawPixmap(0, 0, width(), height(), background);

        // 텍스트 색상 설정
        painter.setPen(Qt::white);

        // 데이터 표시
        mutex.lock();
        int motorSpeed = sensorData.motor_speed;
        int precipitation = sensorData.precipitation;
        int tunnelDistance = sensorData.tunnel_distance;
        int airCondition = sensorData.air_condition;
        mutex.unlock();

        // **Motor Speed 텍스트 크기 설정 (더 크게)**
        QFont largeFont("Arial", 50, QFont::Bold); // 글꼴 크기를 24로 설정하고, 굵게 표시
        painter.setFont(largeFont);
        painter.drawText(160, 330, QString::number(motorSpeed));

        // **나머지 텍스트는 기본 크기로 설정**
        QFont normalFont("Arial", 16);
        painter.setFont(normalFont);
        painter.drawText(20, 80, "Precipitation: " + QString::number(precipitation));
        painter.drawText(20, 110, "Tunnel Distance: " + QString::number(tunnelDistance));
        painter.drawText(20, 140, "Air Condition: " + QString::number(airCondition));
    }

    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Escape) {
            if (isFullScreen()) {
                showNormal();
            } else {
                showFullScreen();
            }
        } else {
            QWidget::keyPressEvent(event);
        }
    }

    void timerEvent(QTimerEvent *event) override {
        Q_UNUSED(event);
        update(); // 화면 갱신
    }

private slots:
    void onNewConnection() {
        if (clientSocket) {
            clientSocket->disconnect();
            clientSocket->deleteLater();
        }

        clientSocket = server->nextPendingConnection();
        connect(clientSocket, &QTcpSocket::readyRead, this, &BackgroundWidget::onDataReceived);
        connect(clientSocket, &QTcpSocket::disconnected, this, &BackgroundWidget::onClientDisconnected);
        qDebug("Client connected!");
    }

    void onDataReceived() {
        if (!clientSocket) return;

        while (clientSocket->bytesAvailable() >= static_cast<qint64>(sizeof(sensor_dto))) {
            sensor_dto tempData;
            clientSocket->read(reinterpret_cast<char *>(&tempData), sizeof(sensor_dto));

            mutex.lock();
            sensorData = tempData;
            mutex.unlock();

            qDebug("Data received: Motor Speed=%d, Precipitation=%d, Tunnel Distance=%d, Air Condition=%d",
                   sensorData.motor_speed,
                   sensorData.precipitation,
                   sensorData.tunnel_distance,
                   sensorData.air_condition);
        }
    }

    void onClientDisconnected() {
        qDebug("Client disconnected.");
        if (clientSocket) {
            clientSocket->deleteLater();
            clientSocket = nullptr;
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    BackgroundWidget window;
    window.setWindowTitle("Sensor Data Viewer");
    window.resize(800, 600);
    window.startTimer(100); // 화면 갱신 주기 설정
    window.show();

    return app.exec();
}

#include "main.moc"
