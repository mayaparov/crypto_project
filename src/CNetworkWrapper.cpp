#include "CNetworkWrapper.h"
#include "HandlerFactory.h"
#include "AuthHandler.h"
#include "CoursesHandler.h"
#include "RegistrationHandler.h"
#include "ErrorHandler.h"

CNetworkWrapper::CNetworkWrapper(QObject *parent)
    : QObject(parent),
      manager(new QNetworkAccessManager(this)),
      tokenRefreshTimer(this)
{
    // Настройка SSL
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    QSslConfiguration::setDefaultConfiguration(sslConfig);

    connect(manager, &QNetworkAccessManager::finished, this, [this](QNetworkReply* reply) {
        QByteArray data = reply->readAll(); // Читаем данные здесь
        handleNetworkReply(reply, data);    // Передаем reply и данные в слот
        reply->deleteLater();               // Удаляем reply после обработки
    });
    
    initRefreshTimer();
    loadTokens();
    
    // Отложенная проверка сессии после инициализации
        QTimer::singleShot(0, this, [this]() {
            if (hasActiveSession()) {
                qDebug() << "[CNetworkWrapper] Active session found. Refreshing tokens...";
                refreshAuthToken();
            } else {
                qDebug() << "[CNetworkWrapper] Active session not found. Auth required...";
                emit reauthenticationRequired(); // Сигнал будет обработан
            }
        });
}

void CNetworkWrapper::initRefreshTimer() {
    connect(&tokenRefreshTimer, &QTimer::timeout, this, [this]() {
        if (!refreshToken.isEmpty()) {
            refreshAuthToken();
        }
    });
    tokenRefreshTimer.setInterval(1740000); // 29 минут (токен живет 30)
}

void CNetworkWrapper::restoreSession() {
    loadTokens();
    if (!refreshToken.isEmpty()) {
        refreshAuthToken();
    }
}

void CNetworkWrapper::clearSession() {
    accessToken.clear();
    refreshToken.clear();
    QSettings().remove("auth");
    tokenRefreshTimer.stop();
}

void CNetworkWrapper::authenticate(const QString& email, const QString& password) {
    QJsonObject data{
        {"email", email},
        {"password", password}
    };
    sendPostRequest("/api/auth/login/", data);
}

void CNetworkWrapper::registerUser(const QString& email, const QString& password) {
    QJsonObject data{
        {"email", email},
        {"password", password}
    };
    sendPostRequest("/api/auth/register/", data);
}

void CNetworkWrapper::fetchCourses() {
    if (accessToken.isEmpty()) {
        emit errorOccurred("Not authenticated");
        return;
    }

    QNetworkRequest request(QUrl(baseUrl + "/api/courses/"));
    request.setRawHeader("Authorization", "Bearer " + accessToken.toUtf8());
    
    // Отладочный вывод заголовка
    qDebug() << "[fetchCourses] Authorization header:" << request.rawHeader("Authorization");
    
    manager->get(request);
}

void CNetworkWrapper::refreshAuthToken() {
    if (refreshToken.isEmpty()) {
        emit reauthenticationRequired();
        return;
    }

    QJsonObject data{{"refresh", refreshToken}};
    sendPostRequest("/api/auth/refresh/", data);
}

void CNetworkWrapper::saveTokens() {
    QSettings settings;
    settings.setValue("auth/accessToken", accessToken);
    settings.setValue("auth/refreshToken", refreshToken);
}

void CNetworkWrapper::loadTokens() {
    QSettings settings;
    accessToken = settings.value("auth/accessToken").toString();
    refreshToken = settings.value("auth/refreshToken").toString();
    
    if (!accessToken.isEmpty() && !refreshToken.isEmpty()) {
        tokenRefreshTimer.start();
    }
}

void CNetworkWrapper::sendPostRequest(const QString& endpoint, const QJsonObject& data) {
    // 1. Формируем полный URL
    QUrl fullUrl(baseUrl + endpoint);
    
    qDebug() << "Request URL:" << fullUrl.toString();
    QNetworkRequest request(fullUrl);

       // 2. Устанавливаем заголовки
       request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("User-Agent", "YourApp/1.0");

    qDebug() << "Request headers:" << request.rawHeaderList();

       // 3. Сериализуем JSON
       QByteArray jsonData = QJsonDocument(data).toJson(QJsonDocument::Compact).trimmed();
    qDebug() << "Sending RAW JSON:" << jsonData.constData();

       // 4. Отправляем запрос и получаем ответ
       QNetworkReply* reply = manager->post(request, jsonData);

    /*/ 5. Чтение данных по мере поступления
        connect(reply, &QNetworkReply::readyRead, this, [reply]() {
            
            
            qDebug() << "Response data chunk:" << QString::fromUtf8(reply->readAll());
        });

    */// 5. Обработка завершения
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QByteArray response = reply->readAll();
      //  qDebug() << "Full response:" << response;

        if (reply->error() != QNetworkReply::NoError) {
            //qDebug() << "Error:" << reply->errorString();
            handleNetworkError(reply, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), response);
        } else {
            handleNetworkReply(reply, response);
        }

        reply->deleteLater();
    });
}
   

void CNetworkWrapper::handleNetworkReply(QNetworkReply* reply, const QByteArray& data) {
    const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    // Выводим сырые байты для отладки
        qDebug() << "Raw response bytes:" << data.toHex();
        qDebug() << "Response as string:" << QString::fromUtf8(data);

    if (data.isEmpty()) {
            emit errorOccurred("Empty response from server");
            reply->deleteLater();
            return;
        }
    
    if (reply->error() != QNetworkReply::NoError) {
        handleNetworkError(reply, status, data);
        reply->deleteLater();
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred("Invalid JSON response");
        reply->deleteLater();
        return;
    }

    // Создаем обработчик через фабрику
    ResponseHandler* handler = HandlerFactory::createHandler(doc.object());
    if (!handler) {
        emit errorOccurred("Unknown response type");
        reply->deleteLater();
        return;
    }

    // Подключаем обработчики сигналов
    connect(handler, &ResponseHandler::error,
            this, &CNetworkWrapper::errorOccurred);

    if (auto authHandler = qobject_cast<AuthHandler*>(handler)) {
        connect(authHandler, &AuthHandler::authSuccess,
                this, [this](const QString& access, const QString& refresh, const QString& role) {
                    accessToken = access;
                    refreshToken = refresh;
                    userRole = role; // Используем роль из ответа
                    saveTokens();
                    tokenRefreshTimer.start();
                    emit authSuccess(access, refresh, role); // Обновляем сигнал
                });
    }
    else if (auto coursesHandler = qobject_cast<CoursesHandler*>(handler)) {
        connect(coursesHandler, &CoursesHandler::coursesDataReceived,
                this, &CNetworkWrapper::coursesReceived);
    }
    else if (auto regHandler = qobject_cast<RegistrationHandler*>(handler)) {
        connect(regHandler, &RegistrationHandler::registrationSuccess,
                this, [this]() {
                    // После регистрации автоматически аутентифицируемся
                    authenticate("user@example.com", "securePassword123");
                });
        connect(regHandler, &RegistrationHandler::error,
                this, &CNetworkWrapper::errorOccurred);
    }

    else if (auto errorHandler = qobject_cast<ErrorHandler*>(handler)) {
        connect(errorHandler, &ErrorHandler::error,
                this, &CNetworkWrapper::errorOccurred);
    }

    // Запускаем обработку
    handler->process(doc.object());
    handler->deleteLater();
    reply->deleteLater();
}

bool CNetworkWrapper::hasActiveSession() const {
    return !accessToken.isEmpty() && !refreshToken.isEmpty();
}

QString CNetworkWrapper::getUserRole() const {
    return userRole;
}


void CNetworkWrapper::handleNetworkError(QNetworkReply* reply, int status, const QByteArray &responseData) {
    const auto error = reply->errorString();

    // Парсинг JSON из responseData
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred("Invalid JSON response");
        return;
    }
    QJsonObject response = doc.object();
    
    // Обработка ошибки аутентификации (400 + non_field_errors)
    if (status == 400 && response.contains("non_field_errors")) {
                QString errorMsg = response["non_field_errors"].toArray().first().toString();
                emit invalidCredentials(errorMsg); // Новый сигнал
                return;
    }

    // Обработка 401 (Unauthorized)
    if (status == 401) {
        if (!refreshToken.isEmpty()) {
            // Проверка на "Token is blacklisted"
            if (response.contains("detail") && response["detail"].toString().contains("blacklisted")) {
                qDebug() << "[CNetworkWrapper] Refresh token is blacklisted. Session cleared.";
                clearSession();
                emit reauthenticationRequired();
                return;
            }
            qDebug() << "[CNetworkWrapper] Token expired. Refreshing...";
            refreshAuthToken();
        } else {
            qDebug() << "[CNetworkWrapper] Session invalid. Reauth required.";
            emit reauthenticationRequired();
        }
        return;
    }

    // Обработка 403 (Forbidden)
    if (status == 403) {
        qDebug() << "[CNetworkWrapper] Access forbidden. Clearing session.";
        clearSession();
        emit forbidden_signal(); // Новый сигнал
        return;
    }

    // Общая ошибка
    emit errorOccurred(QString("Network error (%1): %2").arg(status).arg(error));
}
