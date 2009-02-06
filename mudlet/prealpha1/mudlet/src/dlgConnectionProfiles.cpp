/***************************************************************************
 *   Copyright (C) 2008 by Heiko Koehn                                     *
 *   KoehnHeiko@googlemail.com                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QInputDialog>
#include <QMessageBox>
#include "dlgConnectionProfiles.h"
#include "Host.h"
#include "HostManager.h"
#include "mudlet.h"

#define _DEBUG_

dlgConnectionProfiles::dlgConnectionProfiles(QWidget * parent) : QDialog(parent)
{
    setupUi( this );
    
    // selection mode is important. if this is not set the selection behaviour is
    // undefined. this is an undocumented qt bug, as it only shows on certain OS
    // and certain architectures.
    
    profiles_tree_widget->setSelectionMode( QAbstractItemView::SingleSelection );
    
    QAbstractButton * abort = dialog_buttonbox->button( QDialogButtonBox::Cancel );
    abort->setIcon(QIcon(":/icons/dialog-close.png"));
    QPushButton *connect_button = dialog_buttonbox->addButton(tr("Connect"), QDialogButtonBox::AcceptRole);
    connect_button->setIcon(QIcon(":/icons/dialog-ok-apply.png"));
    
    QStringList headerList;
    headerList << "MUD name" << "Language";
    mud_list_treewidget->setHeaderLabels( headerList );
    
    connect( connect_button, SIGNAL(clicked()), this, SLOT(slot_connectToServer()));
    connect( abort, SIGNAL(clicked()), this, SLOT(slot_cancel()));
    connect( new_profile_button, SIGNAL( clicked() ), this, SLOT( slot_addProfile() ) );
    connect( copy_profile_button, SIGNAL( clicked() ), this, SLOT( slot_copy_profile() ) );
    connect( remove_profile_button, SIGNAL( clicked() ), this, SLOT( slot_deleteProfile() ) );
    connect( profile_name_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_name(const QString)));
    connect( host_name_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_url(const QString)));
    connect( port_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_port(const QString)));   
    connect( autologin_checkBox, SIGNAL(stateChanged( int )), this, SLOT(slot_update_autologin(int)));    
    connect( login_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_login(const QString)));
    connect( character_password_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_pass(const QString)));
    connect( mud_description_textedit, SIGNAL(textChanged()), this, SLOT(slot_update_description()));
    connect( website_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_website(const QString)));
    connect( this, SIGNAL( update() ), this, SLOT( slot_update() ) );
    //connect( profiles_tree_widget, SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ), SLOT( slot_item_clicked(QTreeWidgetItem *) ) );
    connect( profiles_tree_widget, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ), this, SLOT( slot_item_clicked( QTreeWidgetItem * )));
    connect( profiles_tree_widget, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ), this, SLOT ( slot_connectToServer() ) );
    //connect( mud_list_treewidget, SIGNAL( itemClicked(QTreeWidgetItem *, int) ), SLOT( slot_item_clicked(QTreeWidgetItem *) ) );
    //connect( mud_list_treewidget, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ), this, SLOT ( slot_connection_dlg_finnished() ) );
    
    connect( this, SIGNAL (accept()), this, SLOT (slot_connection_dlg_finnished()));
    connect( this, SIGNAL (finished(int)), this, SLOT (slot_finished(int)));
    
    notificationArea->hide();
    notificationAreaIconLabelWarning->hide();
    notificationAreaIconLabelError->hide();
    notificationAreaIconLabelInformation->hide();
    notificationAreaMessageBox->hide();
    
    mRegularPalette.setColor(QPalette::Text,QColor(0,0,192));
    mRegularPalette.setColor(QPalette::Highlight,QColor(0,0,192));
    mRegularPalette.setColor(QPalette::HighlightedText, QColor(255,255,255));
    mRegularPalette.setColor(QPalette::Base,QColor(255,255,255));
    
    mReadOnlyPalette.setColor(QPalette::Base,QColor(212,212,212));
    mReadOnlyPalette.setColor(QPalette::Text,QColor(0,0,192));
    mReadOnlyPalette.setColor(QPalette::Highlight,QColor(0,0,192));
    mReadOnlyPalette.setColor(QPalette::HighlightedText, QColor(255,255,255));
    
    mOKPalette.setColor(QPalette::Text,QColor(0,0,192));
    mOKPalette.setColor(QPalette::Highlight,QColor(0,0,192));
    mOKPalette.setColor(QPalette::HighlightedText, QColor(255,255,255));
    mOKPalette.setColor(QPalette::Base,QColor(235,255,235));    
    
    mErrorPalette.setColor(QPalette::Text,QColor(0,0,192));
    mErrorPalette.setColor(QPalette::Highlight,QColor(0,0,192));
    mErrorPalette.setColor(QPalette::HighlightedText, QColor(255,255,255));
    mErrorPalette.setColor(QPalette::Base,QColor(255,235,235));    
    
    resize( 698, 419 );
    
    
}

void dlgConnectionProfiles::slot_update_description()
{
    QTreeWidgetItem * pItem = profiles_tree_widget->currentItem();
    
    if( pItem )
    {
        QString profile = pItem->text(0);
        QString desc = mud_description_textedit->toPlainText();
        writeProfileData( profile, "description", desc );
    }
}

void dlgConnectionProfiles::slot_update_website( const QString url )
{
    QTreeWidgetItem * pItem = profiles_tree_widget->currentItem();
    if( pItem )
    {
        QString profile = pItem->text(0);
        writeProfileData( profile, "website", url );
    }
}

void dlgConnectionProfiles::slot_update_pass( const QString pass )
{
    QTreeWidgetItem * pItem = profiles_tree_widget->currentItem();
    if( pItem )
    {
        QString profile = pItem->text(0);
        writeProfileData( profile, "password", pass );
    }
}

void dlgConnectionProfiles::slot_update_login( const QString login )
{
    QTreeWidgetItem * pItem = profiles_tree_widget->currentItem();
    if( pItem )
    {
        QString profile = pItem->text(0);
        writeProfileData( profile, "login", login );
    }
}

void dlgConnectionProfiles::slot_update_url( const QString url )
{
    QTreeWidgetItem * pItem = profiles_tree_widget->currentItem();
    
    if( pItem )
    {
        QString profile = pItem->text(0);
        QUrl check;
        check.setHost( url );         
        if( check.isValid() )
        {
            host_name_entry->setPalette( mOKPalette );
            notificationArea->hide();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->hide();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->hide();
            writeProfileData( profile, "url", url );
        }
        else
        {
            host_name_entry->setPalette( mErrorPalette );
            notificationArea->show();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->show();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->show();
            notificationAreaMessageBox->setText( QString("Please enter the URL or IP address of the MUD server.")+check.errorString() );
        }
    }
}

void dlgConnectionProfiles::slot_update_autologin( int state )
{
    QTreeWidgetItem * pItem = profiles_tree_widget->currentItem();
    if( ! pItem ) 
        return;
    QString profile = pItem->text( 0 );
    writeProfileData( profile, "autologin", QString::number( state ) );    
}

void dlgConnectionProfiles::slot_update_port( const QString port )
{
    const QString zahlen = "012345789";
    if( ! zahlen.contains( port.right( 1 ) ) )
    {
        QString val = port;
        val.chop( 1 );
        port_entry->setText( val );
        notificationArea->show();
        notificationAreaIconLabelWarning->show();
        notificationAreaIconLabelError->hide();
        notificationAreaIconLabelInformation->hide();
        notificationAreaMessageBox->show();
        notificationAreaMessageBox->setText( tr("You have to enter a number. Other characters are not permitted.") );
        return; 
    }
    QTreeWidgetItem * pItem = profiles_tree_widget->currentItem();
    
    if( pItem )
    {
        QString profile = pItem->text(0);
        int num = port.trimmed().toInt();
        if( num < 65536 ) 
        {
            port_entry->setPalette( mOKPalette );
            notificationArea->hide();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->hide();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->hide();
            writeProfileData( profile, "port", port );
        }
        else
        {
            notificationArea->show();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->show();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->show();
            port_entry->setPalette( mErrorPalette );
        }
    }
}

void dlgConnectionProfiles::slot_update_name( const QString name )             
{
    QTreeWidgetItem * pItem = profiles_tree_widget->currentItem();
    const QString allowedChars = "012345789 _-#aAbBcCdDeEfFg GhHiIjJkKl LmMnNoOpPqQ rRsStTuUvV wWxXyYzZ";
    if( ! allowedChars.contains( name.right( 1 ) ) )
    {
        QString val = name;
        val.chop( 1 );
        profile_name_entry->setText( val );
        notificationArea->show();
        notificationAreaIconLabelWarning->show();
        notificationAreaIconLabelError->hide();
        notificationAreaIconLabelInformation->hide();
        notificationAreaMessageBox->show();
        notificationAreaMessageBox->setText( tr("This character is not permitted. Use one of the following: %1\n").arg(allowedChars) );
        return; 
    }
    if( pItem )
    {
        if( ! mProfileList.contains( name ) ) 
            mEditOK = true;
        else
            mEditOK = false;
        
        
        if( ! mSavedNewName )
        {
            // keep track of the new profile name that isnt yet valid
            // and thus hasnt been written to disc yet
            mUnsavedProfileName = name;
            pItem->setText( 0, name );
        }
        else
        {
            mCurrentProfileEditName = pItem->text( 0 );
            int row = mProfileList.indexOf( mCurrentProfileEditName );
            if( ( row >= 0 ) && ( row < mProfileList.size() ) )
            {
                mProfileList[row] = name;
                pItem->setText( 0, name );
            }
        }
        
        if( mEditOK )
        {
            QDir dir(QDir::homePath()+"/.config/mudlet/profiles");
            if( ! mSavedNewName )
            {
                dir.mkpath( QDir::homePath()+"/.config/mudlet/profiles/"+mUnsavedProfileName );
                mProfileList << name;
                pItem->setText( 0, name );
                
                if( ( ! mOrigin.isEmpty() ) && ( ! mSavedNewName ) )
                {
                    // special case of a new profile that has to be cloned from
                    // an existing profile
                    QString what = QDir::homePath()+"/.config/mudlet/profiles/"+mOrigin;
                    QString where = QDir::homePath()+"/.config/mudlet/profiles/"+mUnsavedProfileName;
                    QDir dirFrom = QDir( what );
                    QStringList entries = dirFrom.entryList( QDir::Files );
                    
                    for( int i=0; i<entries.size(); i++ )
                    {
                        QFile file( where + mOrigin+"/"+entries[i] );    
                        file.copy( where );
                    }
                    mOrigin.clear();
                    mSavedNewName = true;
                    mUnsavedProfileName = "";
                }
            }
            else     
            {
                dir.rename( mCurrentProfileEditName, name );
            }
            
            profile_name_entry->setPalette( mOKPalette );
            notificationArea->hide();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->hide();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->hide();
        }
        else
        {
            profile_name_entry->setPalette( mErrorPalette );
            notificationArea->show();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->show();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->show();
            notificationAreaMessageBox->setText(tr("A profile with the current name already exists. Please use a longer name or a different name."));
        }
    }
}

void dlgConnectionProfiles::slot_showmudlist_clicked ( bool checked )
{
}

void dlgConnectionProfiles::slot_addProfile()
{
    fillout_form();
    welcome_message->hide();
    
    requiredArea->show();
    informationalArea->show();
    optionalArea->show();
    
    QStringList newname;
    mUnsavedProfileName = tr("new profile name");
    newname << mUnsavedProfileName;
    
    QTreeWidgetItem * pItem = new QTreeWidgetItem( (QTreeWidgetItem *)0, newname);
    if( ! pItem )
    {
        return;
    }
    mSavedNewName = false;
    
    profiles_tree_widget->setSelectionMode( QAbstractItemView::SingleSelection );
    profiles_tree_widget->insertTopLevelItem( 0, pItem );    
    
    // insert newest entry on top of the list as the general sorting 
    // is always newest item first -> fillout->form() filters
    // this is more practical for the user as they use the same profile most of the time
    
    profiles_tree_widget->setItemSelected(profiles_tree_widget->currentItem(), false); // Unselect previous item
    profiles_tree_widget->setCurrentItem( pItem );
    profiles_tree_widget->setItemSelected( pItem, true );
    
    profile_name_entry->setText( mUnsavedProfileName );
    profile_name_entry->setFocus();
    profile_name_entry->selectAll();
    
}

// if the user has chosen to connect to an already loaded profile
// the old profile will be copied and he has to give a new name for it
void dlgConnectionProfiles::copy_profile( QString oldProfile )
{
    
    if( ! mSavedNewName )
    {
        mOrigin = oldProfile; // remember the original profile
    }
    
    profile_name_entry->setPalette( mErrorPalette );
    notificationArea->show();
    notificationAreaIconLabelWarning->show();
    notificationAreaIconLabelError->hide();
    notificationAreaIconLabelInformation->hide();
    notificationAreaMessageBox->show();
    notificationAreaMessageBox->setText(tr("A profile with the current name has already been loaded. The profile you have chosen will be copied, but you have to find a new name for it. Then press on connect again."));
    
    QStringList newname;
    mUnsavedProfileName = oldProfile;
    
    QTreeWidgetItem * pItem_old = profiles_tree_widget->currentItem();
    
    if( ! pItem_old ) return;

#ifdef _DEBUG_ 
    QString oldName = pItem_old->text( 0 );
    Q_ASSERT( oldProfile == oldName ); 
#endif
    
    slot_item_clicked( pItem_old ); //fillout the form with the data of the original profile
    
    QString oldUrl = host_name_entry->text();
    QString oldPort = port_entry->text();
    QString oldPassword = character_password_entry->text();
    QString oldLogin = login_entry->text();
    QString oldWebsite = website_entry->text();
    QString oldDescription = mud_description_textedit->toPlainText();
    
    if (mUnsavedProfileName[mUnsavedProfileName.size()-1].isDigit())
    {
        int i=1;
        do {
            mUnsavedProfileName = mUnsavedProfileName.left(mUnsavedProfileName.size()-1) + QString::number(mUnsavedProfileName[mUnsavedProfileName.size()-1].digitValue() + i++);
        } while (mProfileList.contains(mUnsavedProfileName));
    } else {
        int i=1;
        QString mUnsavedProfileName2;
        do {
            mUnsavedProfileName2 = mUnsavedProfileName + '_' + QString::number(i++);
        } while (mProfileList.contains(mUnsavedProfileName2));
        mUnsavedProfileName = mUnsavedProfileName2;
    }
    
    newname << mUnsavedProfileName;
    
    QTreeWidgetItem * pItem = new QTreeWidgetItem( (QTreeWidgetItem *)0, newname);
    if( ! pItem )
    {
        return;
    }
    mSavedNewName = false;
    
    profiles_tree_widget->setSelectionMode( QAbstractItemView::SingleSelection );
    profiles_tree_widget->insertTopLevelItem( 0, pItem );    
    
    // fill out the form entries with the values of the original profile
    profiles_tree_widget->setCurrentItem( pItem );
    slot_update_name( mUnsavedProfileName );
    //profile_name_entry->setText( mUnsavedProfileName );
    
    host_name_entry->setText( oldUrl );
    slot_update_url( oldUrl );
    
    port_entry->setText( oldPort );
    slot_update_port( oldPort );
    
    character_password_entry->setText( oldPassword );
    slot_update_pass( oldPassword );
    
    login_entry->setText( oldLogin );
    slot_update_login( oldLogin );
    
    website_entry->setText( oldWebsite );
    slot_update_website( oldWebsite );
    
    mud_description_textedit->clear();
    mud_description_textedit->insertPlainText( oldDescription );
    slot_update_description();
    
    
    // insert newest entry on top of the list as the general sorting 
    // is always newest item first -> fillout->form() filters
    // this is more practical for the user as they use the same profile most of the time
    
    //profiles_tree_widget->setItemSelected(profiles_tree_widget->currentItem(), false); // Unselect previous item
    
    profiles_tree_widget->setItemSelected( pItem, true );
    
    profile_name_entry->setReadOnly( false );   
    host_name_entry->setReadOnly( false );
    port_entry->setReadOnly( false );  
    
    profile_name_entry->setFocusPolicy( Qt::StrongFocus );
    host_name_entry->setFocusPolicy( Qt::StrongFocus );
    port_entry->setFocusPolicy( Qt::StrongFocus );
    
    profile_name_entry->setPalette( mErrorPalette );
    host_name_entry->setPalette( mRegularPalette );
    port_entry->setPalette( mRegularPalette );
    profile_name_entry->setFocus();
    profile_name_entry->selectAll();
    
}



void dlgConnectionProfiles::slot_deleteProfile()
{
    if( ! profiles_tree_widget->currentItem() ) 
        return;
    
    QString profile = profiles_tree_widget->currentItem()->text( 0 );
    if ( QMessageBox::question(this, tr("Confirmation"), tr("Are you sure you want to delete %1 ?").arg( profile ), QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No )
        return;
    
    profiles_tree_widget->takeTopLevelItem( profiles_tree_widget->currentIndex().row() );
    QDir dir( QDir::homePath()+"/.config/mudlet/profiles/"+profile );
    QStringList deleteList = dir.entryList();
    for( int i=0; i<deleteList.size(); i++ )
    {
        dir.remove( deleteList[i] );
    }
    dir.rmpath( dir.path());
    
    if( ! mProfileList.size() )
    {
        welcome_message->show();
    }
    fillout_form();
    profiles_tree_widget->setFocus();
}

QString dlgConnectionProfiles::readProfileData( QString profile, QString item )
{
    QFile file( QDir::homePath()+"/.config/mudlet/profiles/"+profile+"/"+item );
    file.open( QIODevice::ReadOnly );
    QDataStream ifs( & file ); 
    QString ret;
    ifs >> ret;
    file.close();
    return ret;
}

QStringList dlgConnectionProfiles::readProfileHistory( QString profile, QString item )
{
    QFile file( QDir::homePath()+"/.config/mudlet/profiles/"+profile+"/"+item );
    file.open( QIODevice::ReadOnly );
    QDataStream ifs( & file ); 
    QString ret;
    QStringList historyList;
    while( ifs.status() == QDataStream::Ok )
    {
        ifs >> ret;
        historyList << ret;
    }
    file.close();
    return historyList;
}

void dlgConnectionProfiles::writeProfileData( QString profile, QString item, QString what )
{
    QFile file( QDir::homePath()+"/.config/mudlet/profiles/"+profile+"/"+item );
    file.open( QIODevice::WriteOnly | QIODevice::Unbuffered );
    QDataStream ofs( & file ); 
    ofs << what;
    file.close();
}

void dlgConnectionProfiles::slot_item_clicked(QTreeWidgetItem *pItem) 
{
    qDebug()<<"slot_item_clicked("<<pItem->text(0)<<")";
    if( pItem )
    {
        QString profile_name = pItem->text( 0 );
        QStringList loadedProfiles = HostManager::self()->getHostList();
        if( loadedProfiles.contains( profile_name ) )
        {
            profile_name_entry->setReadOnly( true );   
            host_name_entry->setReadOnly( true );  
            port_entry->setReadOnly( true );  
            
            profile_name_entry->setFocusPolicy( Qt::NoFocus );
            host_name_entry->setFocusPolicy( Qt::NoFocus );
            port_entry->setFocusPolicy( Qt::NoFocus );
            
            profile_name_entry->setPalette( mReadOnlyPalette );
            host_name_entry->setPalette( mReadOnlyPalette );
            port_entry->setPalette( mReadOnlyPalette );
            
            notificationArea->show();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->hide();
            notificationAreaIconLabelInformation->show();
            notificationAreaMessageBox->show();
            notificationAreaMessageBox->setText(tr("This profile is currently loaded. You cant change all parameters on loaded profiles. Disconnect the profile and then do the changes."));
            
        }
        else
        {
            profile_name_entry->setReadOnly( false );   
            host_name_entry->setReadOnly( false );
            port_entry->setReadOnly( false );  
            
            profile_name_entry->setFocusPolicy( Qt::StrongFocus );
            host_name_entry->setFocusPolicy( Qt::StrongFocus );
            port_entry->setFocusPolicy( Qt::StrongFocus );
            
            profile_name_entry->setPalette( mRegularPalette );
            host_name_entry->setPalette( mRegularPalette );
            port_entry->setPalette( mRegularPalette );
            
            notificationArea->hide();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->hide();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->hide();
            notificationAreaMessageBox->setText(tr(""));
            
        }
       
        profile_name_entry->setText( profile_name ); 
        
        QString profile = profile_name;
        
        QString item = "url";
        QString val = readProfileData( profile, item );
        host_name_entry->setText( val );
        item = "port";
        val = readProfileData( profile, item );
        port_entry->setText( val );
        item = "password";
        val = readProfileData( profile, item );
        character_password_entry->setText( val );
        item = "login";
        val = readProfileData( profile, item );
        login_entry->setText( val );
        item = "autologin";
        val = readProfileData( profile, item );
        if( val.toInt() == Qt::Checked )
        {
            autologin_checkBox->setChecked( true );    
        }
        else
        {
            autologin_checkBox->setChecked( false );
        }
        item = "description";
        val = readProfileData( profile, item );
        mud_description_textedit->clear();
        mud_description_textedit->insertPlainText( val );
        item = "website";
        val = readProfileData( profile, item );
        website_entry->setText( val );
        
        profile_history->clear();
        item = "history_version";
        QStringList historyList;
        historyList = readProfileHistory( profile, item );
        QStringList versionList;
        versionList << "Newest Profile";
        for( int i=historyList.size()-1; i>-1; i-- )
        {
            versionList << historyList[i];
        }
        versionList << "Oldest Profile";
        profile_history->insertItems( 1, versionList );
        
    }
}

void dlgConnectionProfiles::fillout_form()
{
    profiles_tree_widget->clear();
    profile_name_entry->clear();
    host_name_entry->clear();
    port_entry->clear();
    
    mProfileList = QDir(QDir::homePath()+"/.config/mudlet/profiles").entryList(QDir::Dirs, QDir::Name);
    
    if( mProfileList.size() < 3 ) 
    {
        welcome_message->show();
        requiredArea->hide();
        informationalArea->hide();
        optionalArea->hide();
        resize( 698, 419 );
    }
    else
    {
        welcome_message->hide();
        
        requiredArea->show();
        informationalArea->show();
        optionalArea->show();
    }
    
    QDateTime test_date;
    QTreeWidgetItem * toselect = 0;
    for( int i=0; i<mProfileList.size(); i++ )
    {
        QString s = mProfileList[i];
        if( s.size() < 1 ) 
            continue;
        if( (mProfileList[i] == ".") || (mProfileList[i] == ".." ) )
            continue;
        
        QStringList sList;
        sList << mProfileList[i];
        QTreeWidgetItem * pItem = new QTreeWidgetItem( (QTreeWidgetItem *)0, sList);
        profiles_tree_widget->insertTopLevelItem( profiles_tree_widget->topLevelItemCount(), pItem );    
        
        QDateTime profile_lastRead = QFileInfo(QDir::homePath()+"/.config/mudlet/profiles/"+mProfileList[i]+"/Host.dat").lastRead();
        if (profile_lastRead > test_date)
        {
            test_date = profile_lastRead;
            toselect = pItem;
        }
    }
    if( toselect )
        profiles_tree_widget->setCurrentItem( toselect );
}

void dlgConnectionProfiles::slot_connection_dlg_finnished()
{
}


void dlgConnectionProfiles::slot_finished(int f)
{
}

void dlgConnectionProfiles::slot_cancel()
{
    QDialog::done( 0 );    
}

void dlgConnectionProfiles::slot_copy_profile()
{
    QString profile_name = profile_name_entry->text().trimmed();
    
    if( profile_name.size() < 1 ) 
        return;
    
    // copy profile as the same profile is already loaded
    // show information that he has to select a new name for this copy
    copy_profile( profile_name );
    return;
}

void dlgConnectionProfiles::slot_connectToServer()
{
    QString profile_name = profile_name_entry->text().trimmed();
    
    if( profile_name.size() < 1 ) 
        return;
    
    QStringList loadedProfiles = HostManager::self()->getHostList();
    if( loadedProfiles.contains( profile_name ) )
    {
        // copy profile as the same profile is already loaded
        // show information that he has to select a new name for this copy
        copy_profile( profile_name );
        qDebug()<<"slot_connectToServer() mOrigin="<<profile_name;
        mOrigin = profile_name;
        return;
    }
    
    // load an old profile if there is any
    int historyVersion = profile_history->currentIndex();
    if( historyVersion == -1 )
        historyVersion = 0;
    QString hostPath = QDir::homePath()+"/.config/mudlet/profiles/"+profile_name;
    Host * pHost = HostManager::self()->loadHostProfile( hostPath, historyVersion );
    
    // the user *might* have changed some data
    
    if( ! pHost )
    {
        bool ok = HostManager::self()->addHost( profile_name, port_entry->text().trimmed(), "", "" );
        pHost = HostManager::self()->getHost( profile_name );
    }
    
    if( pHost )
    {
        pHost->setName( profile_name );
        
        if( host_name_entry->text().trimmed().size() > 0 )
            pHost->setUrl( host_name_entry->text().trimmed() );
        else
            slot_update_url( pHost->getUrl() );
        
        if( port_entry->text().trimmed().size() > 0 )
            pHost->setPort( port_entry->text().trimmed().toInt() );
        else
            slot_update_port( QString::number( pHost->getPort() ) );
            
        if( character_password_entry->text().trimmed().size() > 0 )
            pHost->setPass( character_password_entry->text().trimmed() );
        else
            slot_update_pass( pHost->getPass() );
        
        if( login_entry->text().trimmed().size() > 0 )
            pHost->setLogin( login_entry->text().trimmed() );
        else
            slot_update_login( pHost->getLogin() );
    }
    emit signal_establish_connection( profile_name, historyVersion );
    QDialog::accept();
}


void dlgConnectionProfiles::slot_update()
{
    update();      
}
