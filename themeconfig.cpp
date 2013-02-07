/*
    Copyright 2013 by Reza Fatahilah Shah <rshah0385@kireihana.com>
 
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.
   
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
   
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "themeconfig.h"
#include "ui_themeconfig.h"
#include "themesmodel.h"
#include "themesdelegate.h"

#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/QDeclarativeContext>
#include <KDebug>
#include <KMessageBox>
#include <KStandardDirs>

ThemeConfig::ThemeConfig(QWidget *parent) :
    QWidget(parent)
{
    mConfig = KSharedConfig::openConfig("/etc/sddm.conf", KConfig::SimpleConfig);
    
    configUi = new Ui::ThemeConfig();
    configUi->setupUi(this);
    
    ThemesModel *model = new ThemesModel(this);
    configUi->themesListView->setModel(model);
    
    ThemesDelegate *delegate = new ThemesDelegate(configUi->themesListView);
    delegate->setPreviewSize(QSize(128,128));
    configUi->themesListView->setItemDelegate(delegate);
    model->populate();
    
    connect(configUi->themesListView, SIGNAL(activated(QModelIndex)), SLOT(themeSelected(QModelIndex)));
    connect(configUi->themesListView, SIGNAL(clicked(QModelIndex)), SLOT(themeSelected(QModelIndex)));
   
    prepareInitialTheme();
    
    dump();
}

ThemeConfig::~ThemeConfig()
{
    delete configUi;
}

QVariantMap ThemeConfig::save()
{
    QModelIndex index = configUi->themesListView->currentIndex();
    if (!index.isValid()) {
        return QVariantMap();
    }
    
    QVariantMap args;
    
    args["sddm.conf/General/CurrentTheme"] = index.data(ThemesModel::IdRole);
    
    return args;
}

void ThemeConfig::prepareInitialTheme()
{
    const QString mainQmlPath = KStandardDirs::locate("data", "sddm-kcm/main.qml");
    //configUi->declarativeView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    //configUi->declarativeView->setResizeMode( QDeclarativeView::SizeRootObjectToView );
    configUi->declarativeView->setSource(mainQmlPath);
    
    QString initialTheme = mConfig->group("General").readEntry("CurrentTheme");
    
    QModelIndex index = findThemeIndex(initialTheme);
    if (!index.isValid()) {
        //KMessageBox::error(this, i18n("Could not find any themes. \nPlease install SDDM themes."), i18n("No SDDM themes"));
        return;
    }
    configUi->themesListView->setCurrentIndex(index);
    themeSelected(index);
}

QModelIndex ThemeConfig::findThemeIndex(const QString &id) const
{
    QAbstractItemModel* model = configUi->themesListView->model();
    
    for (int i=0; i < model->rowCount(); i++) {
        QModelIndex index = model->index(i, 0);
        if (index.data(ThemesModel::IdRole).toString() == id) {
            return index;
        }
    }
    
    return QModelIndex();
}

void ThemeConfig::themeSelected(const QModelIndex &index)
{
    QString previewFilename = index.model()->data(index, ThemesModel::PathRole).toString();
    previewFilename += index.model()->data(index, ThemesModel::PreviewRole).toString();
    
    
    configUi->declarativeView->rootContext()->setContextProperty("themeName", index.data().toString());
    configUi->declarativeView->rootContext()->setContextProperty("previewPath", previewFilename);
    configUi->declarativeView->rootContext()->setContextProperty("authorName", index.data(ThemesModel::AuthorRole).toString());
    configUi->declarativeView->rootContext()->setContextProperty("description", index.data(ThemesModel::DescriptionRole).toString());
    configUi->declarativeView->rootContext()->setContextProperty("license", index.data(ThemesModel::LicenseRole).toString());
    configUi->declarativeView->rootContext()->setContextProperty("email", index.data(ThemesModel::EmailRole).toString());
    configUi->declarativeView->rootContext()->setContextProperty("website", index.data(ThemesModel::WebsiteRole).toString());
    configUi->declarativeView->rootContext()->setContextProperty("copyright", index.data(ThemesModel::CopyrightRole).toString());
    configUi->declarativeView->rootContext()->setContextProperty("version", index.data(ThemesModel::VersionRole).toString());
    
    
    /*configUi->nameLabel->setText(index.data().toString());
    configUi->descriptionLabel->setText(index.data(ThemesModel::DescriptionRole).toString());
    configUi->authorLabel->setText(index.data(ThemesModel::AuthorRole).toString());

    
    QPixmap preview(previewFilename);
    if (! preview.isNull()) {
        configUi->preview->setPixmap(preview.scaled(QSize(350, 350), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        configUi->preview->setPixmap(QPixmap());
    }*/
    
    emit changed(true);
}

void ThemeConfig::dump()
{
    //dump sddm conf
     KConfigGroup config = mConfig->group("General");
     
     kDebug() << "Current theme:" << config.readEntry("CurrentTheme");
}