/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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

#include "dfmio_global.h"
#include "local/dlocaloperator.h"
#include "local/dlocaloperator_p.h"
#include "local/dlocalhelper.h"

#include "core/doperator_p.h"

#include <gio/gio.h>

#include <QDebug>

USING_IO_NAMESPACE

DLocalOperatorPrivate::DLocalOperatorPrivate(DLocalOperator *q)
    : q(q)
{
}

DLocalOperatorPrivate::~DLocalOperatorPrivate()
{
}

bool DLocalOperatorPrivate::renameFile(const QString &new_name)
{
    const QUrl &url = q->uri();

    GError *gerror = nullptr;

    const QString &nameTemp = "sssssss";
    char const *name = nameTemp.toLocal8Bit().data();

    GFile *gfile = g_file_new_for_uri(url.toString().toLocal8Bit().data());

    GFile *gfile_ret = g_file_set_display_name(gfile, name, nullptr, &gerror);
    g_object_unref(gfile);
    //g_free(name);

    if (!gfile_ret) {
        setErrorInfo(gerror);
        g_error_free(gerror);
        return false;
    }

    if (gerror)
        g_error_free(gerror);
    g_object_unref(gfile_ret);
    return true;
}

bool DLocalOperatorPrivate::copyFile(const QUrl &urlTo, DOperator::CopyFlag flag)
{
    GError *gerror = nullptr;

    const QUrl &urlFrom = q->uri();

    GFile *gfile_from = makeGFile(urlFrom);
    GFile *gfile_to = makeGFile(urlTo);

    GFile *gfileTarget = nullptr;
    if (DLocalHelper::checkGFileType(gfile_to, G_FILE_TYPE_DIRECTORY)) {
        char *basename = g_file_get_basename (gfile_from);
        gfileTarget = g_file_get_child(gfile_to, basename);
        g_free(basename);
    } else {
        gfileTarget = makeGFile(urlTo);
    }

    bool ret = g_file_copy(gfile_from, gfileTarget, GFileCopyFlags(flag), nullptr, nullptr, nullptr, &gerror);
    if (!ret)
        setErrorInfo(gerror);

    if (gerror)
        g_error_free(gerror);
    g_object_unref(gfile_from);
    g_object_unref(gfile_to);
    g_object_unref(gfileTarget);
    return ret;
}

bool DLocalOperatorPrivate::moveFile(const QUrl &to, DOperator::CopyFlag flag)
{
    GError *gerror = nullptr;

    const QUrl &from = q->uri();
    GFile *gfile_from = makeGFile(from);

    GFile *gfile_to = makeGFile(to);

    bool ret = g_file_move(gfile_from, gfile_to, GFileCopyFlags(flag), nullptr, nullptr, nullptr, &gerror);
    if (!ret)
        setErrorInfo(gerror);
    if (gerror)
        g_error_free(gerror);
    g_object_unref(gfile_from);
    g_object_unref(gfile_to);
    return ret;
}

bool DLocalOperatorPrivate::trashFile()
{
    GError *gerror = nullptr;

    const QUrl &uri = q->uri();
    GFile *gfile = makeGFile(uri);

    bool ret = g_file_trash(gfile, nullptr, &gerror);
    if (!ret)
        setErrorInfo(gerror);
    if (gerror)
        g_error_free(gerror);
    g_object_unref(gfile);
    return ret;
}

bool DLocalOperatorPrivate::deleteFile()
{
    GError *gerror = nullptr;

    const QUrl &uri = q->uri();
    GFile *gfile = makeGFile(uri);

    bool ret = g_file_delete(gfile, nullptr, &gerror);
    if (!ret)
        setErrorInfo(gerror);

    if (gerror)
        g_error_free(gerror);
    g_object_unref(gfile);
    return ret;
}

bool DLocalOperatorPrivate::restoreFile()
{
    GError *gerror = nullptr;

    const QUrl &uri = q->uri();
    GFile *file = makeGFile(uri);

    GFileInfo *gfileinfo = g_file_query_info(file, G_FILE_ATTRIBUTE_TRASH_ORIG_PATH, G_FILE_QUERY_INFO_NONE, nullptr, &gerror);
    g_object_unref(file);

    if (!gfileinfo) {
        setErrorInfo(gerror);
        g_error_free(gerror);
        return false;
    }
    if (gerror)
        g_error_free(gerror);

    const std::string &src_path = g_file_info_get_attribute_byte_string(gfileinfo, G_FILE_ATTRIBUTE_TRASH_ORIG_PATH);
    if (src_path.empty()) {
        g_object_unref(gfileinfo);
        return false;
    }

    QUrl url_dest;
    url_dest.setPath(QString::fromStdString(src_path.c_str()));
    url_dest.setScheme(QString("file"));

    bool ret = moveFile(url_dest, DOperator::CopyFlag::None);

    g_object_unref(gfileinfo);
    return ret;
}

bool DLocalOperatorPrivate::touchFile()
{
    // GFileCreateFlags
    // value:
    // G_FILE_CREATE_NONE
    // G_FILE_CREATE_PRIVATE
    // G_FILE_CREATE_REPLACE_DESTINATION

    GError *gerror = nullptr;

    const QUrl &uri = q->uri();
    GFile *gfile = makeGFile(uri);

    // if file exist, return failed
    GFileOutputStream *stream = g_file_create(gfile, GFileCreateFlags::G_FILE_CREATE_REPLACE_DESTINATION, nullptr, &gerror);
    if (stream)
        g_object_unref(stream);
    else
        setErrorInfo(gerror);

    if (gerror)
        g_error_free(gerror);
    g_object_unref(gfile);
    return stream != nullptr;
}

bool DLocalOperatorPrivate::makeDirectory()
{
    // only create direct path
    GError *gerror = nullptr;

    const QUrl &uri = q->uri();
    GFile *gfile = makeGFile(uri);

    bool ret = g_file_make_directory(gfile, nullptr, &gerror);
    if (!ret)
        setErrorInfo(gerror);

    if (gerror)
        g_error_free(gerror);
    g_object_unref(gfile);
    return ret;
}

bool DLocalOperatorPrivate::createLink(const QUrl &link)
{
    GError *gerror = nullptr;

    const QUrl &uri = q->uri();
    GFile *gfile = makeGFile(uri);

    const QString &qstr = link.path();
    const char *symlink_value = qstr.toLocal8Bit().data();
    bool ret = g_file_make_symbolic_link(gfile, symlink_value, nullptr, &gerror);
    if (!ret) {
        setErrorInfo(gerror);
    }

    if (gerror)
        g_error_free(gerror);
    g_object_unref(gfile);

    return ret;
}

bool DLocalOperatorPrivate::setFileInfo(const DFileInfo &fileInfo)
{
    GError *gerror = nullptr;

    const QUrl &uri = q->uri();
    GFile *gfile = makeGFile(uri);

    GFileInfo *file_info = DLocalHelper::getFileInfoFromDFileInfo(fileInfo);
    bool ret = g_file_set_attributes_from_info(gfile, file_info, GFileQueryInfoFlags::G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, &gerror);
    if (!ret)
        setErrorInfo(gerror);

    if (gerror)
        g_error_free(gerror);
    g_object_unref(file_info);
    g_object_unref(gfile);
    return ret;
}

GFile *DLocalOperatorPrivate::makeGFile(const QUrl &url)
{
    return g_file_new_for_uri(url.toString().toLocal8Bit().data());
}

void DLocalOperatorPrivate::setErrorInfo(GError *gerror)
{
    //error.setCode(DFMIOErrorCode(gerror->code));

    qWarning() << QString::fromStdString(gerror->message);
}

DLocalOperator::DLocalOperator(const QUrl &uri) : DOperator(uri)
  , d(new DLocalOperatorPrivate(this))
{
    registerRenameFile(std::bind(&DLocalOperator::renameFile, this, std::placeholders::_1));
    registerCopyFile(std::bind(&DLocalOperator::copyFile, this, std::placeholders::_1, std::placeholders::_2));
    registerMoveFile(std::bind(&DLocalOperator::moveFile, this, std::placeholders::_1, std::placeholders::_2));

    registerTrashFile(std::bind(&DLocalOperator::trashFile, this));
    registerDeleteFile(std::bind(&DLocalOperator::deleteFile, this));
    registerRestoreFile(std::bind(&DLocalOperator::restoreFile, this));

    registerTouchFile(std::bind(&DLocalOperator::touchFile, this));
    registerMakeDirectory(std::bind(&DLocalOperator::makeDirectory, this));
    registerCreateLink(std::bind(&DLocalOperator::createLink, this, std::placeholders::_1));
    registerSetFileInfo(std::bind(&DLocalOperator::setFileInfo, this, std::placeholders::_1));
}

DLocalOperator::~DLocalOperator()
{
}

bool DLocalOperator::renameFile(const QString &newName)
{
    return d->renameFile(newName);
}

bool DLocalOperator::copyFile(const QUrl &destUri, DOperator::CopyFlag flag)
{
    return d->copyFile(destUri, flag);
}

bool DLocalOperator::moveFile(const QUrl &destUri, DOperator::CopyFlag flag)
{
    return d->moveFile(destUri, flag);
}

bool DLocalOperator::trashFile()
{
    return d->trashFile();
}

bool DLocalOperator::deleteFile()
{
    return d->deleteFile();
}

bool DLocalOperator::restoreFile()
{
    return d->restoreFile();
}

bool DLocalOperator::touchFile()
{
    return d->touchFile();
}

bool DLocalOperator::makeDirectory()
{
    return d->makeDirectory();
}

bool DLocalOperator::createLink(const QUrl &link)
{
    return d->createLink(link);
}

bool DLocalOperator::setFileInfo(const DFileInfo &fileInfo)
{
    return d->setFileInfo(fileInfo);
}