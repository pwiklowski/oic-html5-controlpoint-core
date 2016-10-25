#include "WebSocketServer.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"

WebSocketServer::WebSocketServer(SmartHomeServer* server) : QObject(server)
{
    m_server = server;
    connect(m_server, SIGNAL(valueChanged(QString,QString)), this, SLOT(onValueChanged(QString, QString)));
    connect(m_server, SIGNAL(devicesChanged()), this, SLOT(onDeviceListUpdate()));

    m_webSocketServer = new QWebSocketServer("wiklosoft_iot", QWebSocketServer::NonSecureMode, this);
    if (m_webSocketServer->listen(QHostAddress::Any, 7102)) {
        connect(m_webSocketServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    }
}

void WebSocketServer::onNewConnection()
{
    QWebSocket *socket = m_webSocketServer->nextPendingConnection();

    qDebug() << "new connection" << socket->requestUrl();\

    m_socketList.append(socket);


    connect(socket, SIGNAL(textMessageReceived(QString)), this, SLOT(processTextMessage(QString)));
    connect(socket, SIGNAL(disconnected()), this,  SLOT(socketDisconnected()));

}


void WebSocketServer::processTextMessage(QString message)
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    QString url  = socket->requestUrl().path();

    qDebug() << "processTextMessage" << message;


    QRegExp deviceValue("/device/(.+)/value");


    QJsonObject msg =  QJsonDocument::fromJson(message.toLatin1()).object();

    QJsonObject payload = msg.value("payload").toObject();
    int mid = msg.value("mid").toInt(-1);

    QString request = payload.value("request").toString();
    if (request == "RequestGetDevices"){
        onDeviceListUpdate();
    }else if(request == "RequestSetValue"){
        QString id = payload.value("di").toString();
        QString resource = payload.value("resource").toString();
        QVariantMap value = payload.value("value").toObject().toVariantMap();
        qDebug() << value;

        IotDevice* device = m_server->getDeviceById(id);
        if (device){
            IotDeviceVariable* variable = device->getVariable(resource);
            variable->set(value);
        }

    }else if(request == "RequestRunScript"){
        QScriptEngine* engine = m_server->getEngine();
        QScriptValue e = engine->newObject();

        QString id = payload.value("uuid").toString();
        QJsonObject obj = payload.value("uuid").toObject();

        foreach(QString key, obj.keys()){
            e.setProperty(key, engine->newVariant(obj.value(key).toVariant()));
        }

        m_server->runScript(id, e);
        qDebug() << "Run script" << id;
    }else if(request == "RequestGetDevice"){
        QString uuid = payload.value("uuid").toString();
        onGetDevice(uuid, mid, socket);
    }else if(request == "RequestSearchDevices"){
        emit m_server->searchDevices();
    }


}
void WebSocketServer::socketDisconnected()
{
    qDebug() << "socketDisconnected";
    QWebSocket* client = qobject_cast<QWebSocket *>(sender());
    m_socketList.removeAll(client);
    if (client) {
        client->deleteLater();
    }
}

void WebSocketServer::onDeviceListUpdate(){
    QJsonObject event;
    event.insert("event", "EventDeviceListUpdate");

    QList<IotDevice*> devices = m_server->getClientList();
    QJsonArray devs;

    for(int i=0; i<devices.length();i++)
    {
        IotDevice* device = devices.at(i);
        QJsonObject dev;
        dev["name"] = device->getName();
        dev["id"] = device->getID().remove("device:");
        devs.append(dev);
    }
    event.insert("payload", devs);

    foreach (QWebSocket* s, m_socketList) {
        s->sendTextMessage(QJsonDocument(event).toJson());
    }
}

void WebSocketServer::onGetDevice(QString id, quint32 mid, QWebSocket* s){
    qDebug() << "WebSocket onGetDevice" << id;

    IotDevice* dev = m_server->getDeviceById(id);

    QJsonObject obj;
    obj.insert("event", "ResponseGetDevice");

    QVariantMap* storedVariables = m_server->getVariablesStorage(id);

    QJsonArray vars;
    if (dev)
    {
        for(int i=0; i<dev->getVariables()->size(); i++)
        {
            IotDeviceVariable* var = dev->getVariables()->at(i);
            QVariantMap res = storedVariables->value(var->getResource()).toMap();

            QJsonObject v;
            v["name"] = var->getResource();
            v["rt"] = "oic.r.light.dimming";
            v["values"]= QJsonObject::fromVariantMap(res);
            vars.append(v);
        }
        QJsonObject d;

        d.insert("name", dev->getName());
        d.insert("resources", vars);
        d.insert("uuid", id);

        obj.insert("payload",d);

        obj.insert("mid", QJsonValue::fromVariant(mid));

        s->sendTextMessage(QJsonDocument(obj).toJson());
    }
}

void WebSocketServer::onValueChanged(QString id, QString resource){
    qDebug() << "WebSocket onValueChanged" << id << resource;

    IotDevice* dev = m_server->getDeviceById(id);

    QJsonObject obj;
    obj.insert("event", "EventValueUpdate");

    QVariantMap* storedVariables = m_server->getVariablesStorage(id);

    if (dev)
    {
        QVariantMap res = storedVariables->value(resource).toMap();

        QJsonObject d;

        d.insert("name", dev->getName());
        d.insert("resource",resource);
        d.insert("di", id);
        d.insert("value", QJsonObject::fromVariantMap(res));

        obj.insert("payload",d);

        foreach (QWebSocket* s, m_socketList) {
            s->sendTextMessage(QJsonDocument(obj).toJson());
        }
    }


}

