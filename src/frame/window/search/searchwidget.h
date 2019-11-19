/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     wubw <wubowen_cm@deepin.com>
 *
 * Maintainer: wubw <wubowen_cm@deepin.com>
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
#pragma once

#include "window/namespace.h"

#include "dsearchedit.h"

#include <QWidget>
#include <QList>
#include <QLocale>
#include <QListView>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QXmlStreamReader;
QT_END_NAMESPACE

#define DEBUG_XML_SWITCH 0

const QString RES_TS_PATH = ":/translations/";
const QString XML_Source = "source";
const QString XML_Title = "translation";
const QString XML_Numerusform = "numerusform";
const QString XML_Explain_Path = "extra-contents_path";
const QString XML_Child_Path = "extra-child_page";

namespace DCC_NAMESPACE {
namespace search {

class SearchWidget : public DTK_WIDGET_NAMESPACE::DSearchEdit
{
    Q_OBJECT
public:
    struct SearchBoxStruct {
        QString translateContent;
        QString actualModuleName;
        QString childPageName;
        QString fullPagePath;
    };

    struct SearchDataStruct {
        QString chiese;
        QString pinyin;
    };

    struct UnexsitStruct {
        QString module;
        QString datail;
    };

public:
    SearchWidget(QWidget *parent = nullptr);
    ~SearchWidget() override;

    bool jumpContentPathWidget(QString path);
    void setLanguage(QString type);
    void addModulesName(QString moduleName, QString searchName);
    void addUnExsitData(QString module = "", QString datail = "");
    void removeUnExsitData(QString module = "", QString datail = "");
    void setRemoveableDeviceStatus(QString name, bool isExist);

Q_SIGNALS:
    void notifyModuleSearch(QString, QString);

private:
    void loadxml();
    SearchBoxStruct getModuleBtnString(QString value);
    QString getXmlFilePath();
    QString getModulesName(QString name, bool state = true);
    QString removeDigital(QString input);
    QString transPinyinToChinese(QString pinyin);
    QString containTxtData(QString txt);
    void appendChineseData(SearchBoxStruct data);
    void clearSearchData();

private:
    QStandardItemModel *m_model;
    QCompleter *m_completer;
    QList<SearchBoxStruct> m_EnterNewPagelist;
    SearchBoxStruct m_searchBoxStruct;
    QString m_xmlExplain;
    QString m_xmlFilePath;
    QList<QPair<QString, QString>> m_moduleNameList;//用于存储如 "update"和"Update"
    QList<SearchDataStruct> m_inputList;
    bool m_bIsChinese;
    QList<UnexsitStruct> m_unexsitList;
    QString m_searchValue;
    bool m_bIstextEdited;
    QStringList m_defaultRemoveableList;//存储已知全部模块是否存在
    QList<QPair<QString, QString>> m_removedefaultWidgetList;//用于存储可以出设备名称，和该名称对应的页面
    QList<QPair<QString, QString>> m_removeableActualExistList;//存储实际模块是否存在
    bool m_bIsServerType;
};

}// namespace search
}// namespace DCC_NAMESPACE
