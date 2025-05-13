// Файл: AuthHandler.h
#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include "ResponseHandler.h"
#include <QJsonObject>
#include <QDebug>


/**
* @class AuthHandler
* @brief Обработчик ответов аутентификации
*
* Извлекает access token, refresh token и роль пользователя из ответа сервера.
*/
class AuthHandler : public ResponseHandler {
    Q_OBJECT
public:
    explicit AuthHandler(QObject *parent = nullptr);

    void process(const QJsonObject& response) override;

signals:
    /**
     * @brief Сигнал успешной аутентификации
     * @param access Токен доступа
     * @param refresh Токен обновления
     * @param role Роль пользователя
     */
    void authSuccess(const QString& access, const QString& refresh, const QString& role);
};

#endif // AUTHHANDLER_H
