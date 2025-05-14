#include "CNetworkWrapper.h"
#include "HandlerFactory.h"
#include "AuthHandler.h"
#include "CoursesHandler.h"
#include "RegistrationHandler.h"
#include "ErrorHandler.h"
#include "TopicHandler.h"

CNetworkWrapper::CNetworkWrapper(QObject *parent)
    : QObject(parent),
      manager(new QNetworkAccessManager(this)),
      tokenRefreshTimer(this)
{
    // Настройка SSL
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone); // Временно для тестов
    QSslConfiguration::setDefaultConfiguration(sslConfig);

    
   /*connect(manager, &QNetworkAccessManager::finished, this, [this](QNetworkReply* reply) {
        QByteArray data = reply->readAll(); // Читаем данные здесь
        handleNetworkReply(reply, data);    // Передаем reply и данные в слот
        reply->deleteLater();               // Удаляем reply после обработки
    });*/
    
    initRefreshTimer();
    loadTokens();
    manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    
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

    QUrl url(baseUrl + "/api/courses/courses");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + accessToken.toUtf8());
    
    qDebug() << "[fetchCourses] Request URL:" << url.toString();
    
    QNetworkReply* reply = manager->get(request); // Сохраняем reply

    // Подключаем обработчик завершения запроса
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QByteArray response = reply->readAll();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        
        qDebug() << "Courses response status:" << status;
        qDebug() << "Courses response data:" << response;

        if (reply->error() == QNetworkReply::NoError) {
            handleNetworkReply(reply, response);
        } else {
            handleNetworkError(reply, status, response);
        }
        reply->deleteLater();
    });
    
}

void CNetworkWrapper::fetchTopics(int courseId, int parentTopicId) {
    if (accessToken.isEmpty()) {
        emit errorOccurred("Not authenticated");
        return;
    }

    // Формируем URL по структуре из curl-примера
    QString endpoint = parentTopicId == -1
        ? QString("/api/courses/%1/themes/").arg(courseId)
        : QString("/api/courses/%1/themes/%2/").arg(courseId).arg(parentTopicId);

    QUrl url(baseUrl + endpoint);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + accessToken.toUtf8());
    
    // Сохраняем контекст (parentTopicId) в свойстве reply
    QNetworkReply* reply = manager->get(request);
    reply->setProperty("parentTopicId", parentTopicId);
    
    // Обработка через общий handleNetworkReply
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QByteArray data = reply->readAll();
        handleNetworkReply(reply, data);
        reply->deleteLater();
    });
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
    QByteArray cleanedData = data.trimmed().replace("\\n", "");
    
    // Выводим сырые байты для отладки
        qDebug() << "Raw response bytes:" << data.toHex();
        qDebug() << "Response as string:" << QString::fromUtf8(data);

    // Игнорировать пустые ответы с допустимыми статусами
        if (cleanedData.isEmpty()) {
            if (status == 204) { // 204 No Content - нормальное поведение
                qDebug() << "Empty response (HTTP 204) - ignoring";
                reply->deleteLater();
                return;
            }
            qDebug() << "Empty response with status" << status << "- ignoring";
            reply->deleteLater();
            return; // Просто выходим, не эмитируя ошибку
        }
    
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
    // Декодируем HTML-сущности
    //cleanedData = QTextCodec::codecForName("UTF-8")->toUnicode(cleanedData).toUtf8();
        
    // Проверяем валидность JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(cleanedData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred("Invalid JSON response");
        reply->deleteLater();
        return;
    }
    // Если данные начинаются с '[', парсим как массив
        if (cleanedData.startsWith('[')) {
            doc = QJsonDocument::fromJson(cleanedData, &parseError);
            QJsonObject wrapper;
            wrapper["courses"] = doc.array(); // Оборачиваем массив в объект
            doc = QJsonDocument(wrapper);
        } else {
            doc = QJsonDocument::fromJson(cleanedData, &parseError);
        }

    
    // Создаем обработчик через фабрику
    ResponseHandler* handler = HandlerFactory::createHandler(doc.object());
    if (!handler) {
        qDebug() << "Empty or unrecognized response - ignoring";
        reply->deleteLater();
        return; // Не эмитируем ошибку
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
    else if (auto topicHandler = qobject_cast<TopicHandler*>(handler)) {
        int parentTopicId = reply->property("parentTopicId").toInt();
        
        connect(topicHandler, &TopicHandler::subtopicsReceived,
                this, [this, parentTopicId](int topicId, const QJsonArray& subtopics) {
                    emit subtopicsFetched(parentTopicId, subtopics);
                });
        
        connect(topicHandler, &TopicHandler::materialsReceived,
                this, &CNetworkWrapper::materialsFetched);
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
