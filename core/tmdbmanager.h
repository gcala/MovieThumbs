
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

#ifndef TMDBMANAGER_H
#define TMDBMANAGER_H

#include "downloadmanager.h"

class TmdbManager : public DownloadManager
{
    Q_OBJECT

public:
    TmdbManager() : DownloadManager() {}
    void findMovie(const QString& name, const QString& year);
    void findTv(const QString& name, const QString& year);
    void setSize(int thumbSize);

private:
    static const QString KEY;
    void startSearch(const QUrl& query);

private slots:
    void searchFinished();
};

#endif // TMDBMANAGER_H
