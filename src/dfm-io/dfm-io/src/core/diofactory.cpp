/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "core/diofactory.h"
#include "error.h"
#include "core/diofactory_p.h"

USING_IO_NAMESPACE

DIOFactoryPrivate::DIOFactoryPrivate(DIOFactory *q)
    : q(q)
{

}

DIOFactoryPrivate::~DIOFactoryPrivate()
{

}

DIOFactory::DIOFactory(const QUrl &uri)
    : d(new DIOFactoryPrivate(this))
{
    d->uri = uri;
}

DIOFactory::~DIOFactory()
{

}

void DIOFactory::setUri(const QUrl &uri)
{
    if (!d)
        return;

    d->uri = uri;
}

QUrl DIOFactory::uri() const
{
    if (!d)
        return QUrl();

    return d->uri;
}

DFM_VIRTUAL QSharedPointer<DFileInfo> DIOFactory::createFileInfo() const
{
    if (!d->createFileInfoFunc)
        return nullptr;

    return d->createFileInfoFunc();
}

DFM_VIRTUAL QSharedPointer<DFile> DIOFactory::createFile() const
{
    if (!d->createFileFunc)
        return nullptr;

    return d->createFileFunc();
}

DFM_VIRTUAL QSharedPointer<DEnumerator> DIOFactory::createEnumerator() const
{
    if (!d->createEnumeratorFunc)
        return nullptr;

    return d->createEnumeratorFunc();
}

DFM_VIRTUAL QSharedPointer<DWatcher> DIOFactory::createWatcher() const
{
    if (!d->createWatcherFunc)
        return nullptr;

    return d->createWatcherFunc();
}

DFM_VIRTUAL QSharedPointer<DOperator> DIOFactory::createOperator() const
{
    if (!d->createOperatorFunc)
        return nullptr;

    return d->createOperatorFunc();
}

void DIOFactory::registerCreateFileInfo(const DIOFactory::CreateFileInfoFunc &func)
{
    d->createFileInfoFunc = func;
}

void DIOFactory::registerCreateFile(const DIOFactory::CreateFileFunc &func)
{
    d->createFileFunc = func;
}

void DIOFactory::registerCreateEnumerator(const DIOFactory::CreateEnumeratorFunc &func)
{
    d->createEnumeratorFunc = func;
}

void DIOFactory::registerCreateWatcher(const DIOFactory::CreateWatcherFunc &func)
{
    d->createWatcherFunc = func;
}

void DIOFactory::registerCreateOperator(const DIOFactory::CreateOperatorFunc &func)
{
    d->createOperatorFunc = func;
}

DFMIOError DIOFactory::lastError() const
{
    if (!d)
        return DFMIOError();

    return d->error;
}