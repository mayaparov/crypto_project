// TopicHandler.h
#ifndef TOPICHANDLER_H
#define TOPICHANDLER_H

#include "ResponseHandler.h"
#include <QJsonObject>
#include <QJsonArray>    // Добавьте эту строку
#include <QJsonValue>

class TopicHandler : public ResponseHandler {
    Q_OBJECT
public:
    explicit TopicHandler(QObject* parent = nullptr);
    void process(const QJsonObject& response) override;

signals:
    void topicDataReceived(const QJsonObject& topic);
    void subtopicsReceived(int parentId, const QJsonArray& subtopics);
    void materialsReceived(int topicId, const QJsonArray& materials);
};

#endif // TOPICHANDLER_H
