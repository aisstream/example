#include <QCoreApplication>
#include <QObject>
#include <QWebSocket>
#include <QUrl>
#include <QWebSocket>

class AisStreamClient : public QObject
{
    Q_OBJECT
public:
    explicit AisStreamClient(QObject* parent = nullptr);

// private Q_SLOTS:
    void onConnected();
    void onTextMessageReceived(QString message);
    void onBinaryMessageReceived(const QByteArray& message);
    void onSslErrors(const QList<QSslError>& errors);
    void onError(QAbstractSocket::SocketError e);
    void onPong(quint64 elapsedTime, const QByteArray &payload);


    void timerEvent(QTimerEvent *event) override;

  private:
    QWebSocket m_webSocket;

};

#include "qt.moc"

AisStreamClient::AisStreamClient(QObject* parent)
    : QObject(parent)
{
    connect(&m_webSocket, &QWebSocket::connected, this, &AisStreamClient::onConnected);
    connect(&m_webSocket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors), this, &AisStreamClient::onSslErrors);
    connect(&m_webSocket, &QWebSocket::pong, this, &AisStreamClient::onPong);
    QSslConfiguration sslConfiguration;
    m_webSocket.setSslConfiguration(sslConfiguration);
    // NOTE: aisstream.io requires compression to serve full message bandwidth, and
    // QWebSocket does not implement permessage-deflate, so this client cannot
    // negotiate it. See https://bugreports.qt.io/browse/QTBUG-44552 — the Boost.Beast
    // example in this directory does support compression.
    m_webSocket.open(QUrl("wss://stream.aisstream.io/v0/stream"));
}

void AisStreamClient::onPong(quint64 , const QByteArray &)
{
    qDebug() << "pong ";
}

void AisStreamClient::timerEvent(QTimerEvent *)
{
    qDebug() << "ping";
    m_webSocket.ping();
}

void AisStreamClient::onConnected()
{
    qDebug() << "WebSocket connected";
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &AisStreamClient::onTextMessageReceived);
    connect(&m_webSocket,
            static_cast<void (QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::errorOccurred), this,
            &AisStreamClient::onError);
    connect(&m_webSocket, &QWebSocket::binaryMessageReceived, this, &AisStreamClient::onBinaryMessageReceived);
    m_webSocket.sendTextMessage(QStringLiteral("{ \"APIKey\": \"<YOUR API KEY>\", \"FilterMessageTypes\": [\"PositionReport\"] ,\"BoundingBoxes\": [[[65, 17], [53, 23]]]}"));

    // Need to ping pong to have the connection alive
    startTimer(1000* 60 );
}

void AisStreamClient::onError(QAbstractSocket::SocketError e)
{
    qDebug() << "Error " << e;
}


void AisStreamClient::onTextMessageReceived(QString message)
{
    qDebug() << "Message received:" << message;
}

void AisStreamClient::onBinaryMessageReceived(const QByteArray& message)
{
    onTextMessageReceived(QString(message));
}

void AisStreamClient::onSslErrors(const QList<QSslError>& errors)
{
    qWarning() << "SSL errors:" << errors;
    qApp->quit();
}

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    AisStreamClient client;
    Q_UNUSED(client);

    return a.exec();
}
