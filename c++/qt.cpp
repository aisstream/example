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

private Q_SLOTS:
    void onConnected();
    void onTextMessageReceived(QString message);
    void onBinaryMessageReceived(const QByteArray& message);
    void onSslErrors(const QList<QSslError>& errors);

private:
    QWebSocket m_webSocket;
};

#include "qt.moc"

AisStreamClient::AisStreamClient(QObject* parent)
    : QObject(parent)
{
    connect(&m_webSocket, &QWebSocket::connected, this, &AisStreamClient::onConnected);
    connect(&m_webSocket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors), this, &AisStreamClient::onSslErrors);
    QSslConfiguration sslConfiguration;
    m_webSocket.setSslConfiguration(sslConfiguration);
    m_webSocket.open(QUrl("wss://stream.aisstream.io/v0/stream"));
}

void AisStreamClient::onConnected()
{
    qDebug() << "WebSocket connected";
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &AisStreamClient::onTextMessageReceived);
    connect(&m_webSocket, &QWebSocket::binaryMessageReceived, this, &AisStreamClient::onBinaryMessageReceived);
    m_webSocket.sendTextMessage(QStringLiteral("{ \"APIKey\": \"<YOUR API KEY>\", \"BoundingBoxes\": [[[-11.0, 178.0], [30.0, 74.0]]]}"));
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
