
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

#include "moviethumbs.h"
#include "fileparser.h"

#include <QtCore/QEventLoop>
#include <QtCore/QString>
#include <QtGui/QImage>
#include <QtNetwork/QNetworkAccessManager>

#include <KDebug>
#include <solid/networking.h>

extern "C"
{
    KDE_EXPORT ThumbCreator *new_creator()
    {
        return new MovieThumbs;
    }
}

MovieThumbs::MovieThumbs()
{
    m_networkManager = new QNetworkAccessManager(this);
    m_movie = new MovieService(m_networkManager);
    m_series = new TvService(m_networkManager);
}

MovieThumbs::~MovieThumbs()
{
    delete m_networkManager;
    delete m_movie;
    delete m_series;
}

bool MovieThumbs::create(const QString &path, int /*w*/, int /*h*/, QImage &img)
{
    if(Solid::Networking::status() == Solid::Networking::Unconnected)
    {
        kDebug() << "No network connection available";
        return false;
    }

    QString baseName = FileParser::baseName(path);
    QString year = FileParser::year(baseName);
    QString name = FileParser::cleanName(baseName);

    QString filteredName;

    if(FileParser::isSeries(baseName)){
        //Is the poster already in cache?
        if(m_series->duplicate(name, year)) {
            img = m_series->Poster();
            return true;
        }
        //Retry cache with cleaner filename
        if(filteredName.isEmpty()){
            filteredName = FileParser::filterBlacklist(name);
        }
        if(m_series->duplicate(filteredName, year)) {
            img = m_series->Poster();
            return true;
        }

        if(seriesDownload(name, year)) {
            img = m_series ->Poster();
        } else if(seriesDownload(filteredName, year)) {
            img = m_series->Poster();
        }

        if(!img.isNull()) {
            return true;
        }
    }

    if(movieDownload(name, year)) {
        img = m_movie->Poster();
    } else {
        //Retry search with cleaner filename
        if(filteredName.isEmpty()){
            filteredName = FileParser::filterBlacklist(name);
        }
        if(movieDownload(filteredName, year)) {
            img = m_movie->Poster();
        }
    }
    return !img.isNull();
}

bool MovieThumbs::seriesDownload(const QString &seriesName, const QString &year)
{
    QEventLoop loop;
    connect(m_series, SIGNAL(posterFound()), &loop, SLOT(quit()));
    connect(m_series, SIGNAL(downloadError()), &loop, SLOT(quit()));
    connect(m_series, SIGNAL(posterDownloaded()), &loop, SLOT(quit()));

    m_series->startSearch(seriesName, year);
    loop.exec();

    if(m_series->hasPoster) {
        m_series->startDownload();
        loop.exec();
        return true;
    }
    return false;
}

bool MovieThumbs::movieDownload(const QString &movieName, const QString &movieYear)
{
    QEventLoop loop;
    connect(m_movie, SIGNAL(posterDownloaded()), &loop, SLOT(quit()));
    connect(m_movie, SIGNAL(posterFound()), &loop, SLOT(quit()));
    connect(m_movie, SIGNAL(downloadError()), &loop, SLOT(quit()));

    m_movie->startSearch(movieName, movieYear);
    loop.exec();

    if(m_movie->hasPoster) {
        m_movie->startDownload();
        loop.exec();
        return true;
    }
    return false;
}

ThumbCreator::Flags MovieThumbs::flags() const
{
    return (Flags)(None);
}

#include "moviethumbs.moc"
