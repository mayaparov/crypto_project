#ifndef REGISTRATIONHANDLER_H
#define REGISTRATIONHANDLER_H

#include "ResponseHandler.h"
#include <QJsonObject>
#include <QDebug>

class RegistrationHandler : public ResponseHandler {
    Q_OBJECT
public:
    explicit RegistrationHandler(QObject* parent = nullptr) : ResponseHandler(parent) {}

    void process(const QJsonObject& response) override;
signals:
    void registrationSuccess();
};

#endif // REGISTRATIONHANDLER_H
