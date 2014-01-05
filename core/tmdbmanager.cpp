
/***************************************************************************
 *   This file is part of MovieThumbs.                                     *
 *   Copyright (C) 2013 De Meyer Maarten <de.meyer.maarten@gmail.com>      *
 *                                                                         *
 *   MovieThumbs is free software; you can redistribute it and/or modify   *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   MovieThumbs is distributed in the hope that it will be useful, but    *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#include "tmdbmanager.h"

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

#include <QtCore/QVariant>
#include <QtCore/QVariantList>
#include <QtCore/QVariantMap>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QSslError>

#include <QDebug>

#include <qjson/parser.h>

const QString TmdbManager::KEY = "5c8533aacb1fa275a5113d0728268d5a";

void TmdbManager::startSearch(const QString &name, const QString &year)
{
    QUrl urlQuery("https://api.themoviedb.org/3/search/movie");
    urlQuery.addQueryItem("api_key", KEY);
    urlQuery.addQueryItem("query", name);
    urlQuery.addQueryItem("language", language());
    if(!year.isEmpty()) {
        urlQuery.addQueryItem("year", year);
    }

    QNetworkRequest request;
    request.setUrl(urlQuery);
    request.setRawHeader("Accept", "application/json");

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(searchFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onNetworkError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(slotSslErrors(QList<QSslError>)));
}

void TmdbManager::searchFinished()
{
    QNetworkReply *queryReply = qobject_cast<QNetworkReply *>(sender());
    QByteArray data = queryReply->readAll();
    queryReply->deleteLater();

    QJson::Parser parser;
    bool ok;

    QVariantMap result = parser.parse(data, &ok).toMap();
    if (!ok) {
        qDebug() <<"An error occurred during parsing";
        emit downloadError();
        return;
    }

    QVariantList movies = result["results"].toList();
    if(movies.isEmpty()) {
        //No results found.
        emit downloadError();
        return;
    }

    QStringList posterPath;
    foreach(const QVariant &variant, movies) {
        QVariantMap poster = variant.toMap();
        posterPath << (poster["poster_path"]).toString();
    }

    // http://docs.themoviedb.apiary.io/#configuration
    setUrl("https://image.tmdb.org/t/p/w92/" + posterPath.at(0));
    emit posterFound();
}

void TmdbManager::slotSslErrors(const QList<QSslError> &sslErrors)
{
    foreach(const QSslError & error, sslErrors) {
        qDebug() << "SSL error: " << qPrintable(error.errorString());
    }

    emit downloadError();
}

#include "tmdbmanager.moc"