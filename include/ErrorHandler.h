#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include "ResponseHandler.h"
#include <QString>

class ErrorHandler : public ResponseHandler {
    Q_OBJECT
public:
    explicit ErrorHandler(const QString& errorMsg, QObject* parent = nullptr)
        : ResponseHandler(parent), m_errorMessage(errorMsg) {}

    void process(const QJsonObject&) override {
        emit error(m_errorMessage);
    }

private:
    QString m_errorMessage;
};

#endif // ERRORHANDLER_H
