#include "RegistrationHandler.h"

void RegistrationHandler::process(const QJsonObject& response) {
    if (response.contains("id") && response.contains("email") && response.contains("role")) {
        qInfo() << "Registration successful! User ID:" << response["id"].toInt();
        emit registrationSuccess();
    } else {
        emit error("Invalid registration response");
    }
}
