/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006 - 2009 RetroShare Team
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#include <QMessageBox>
#include <QClipboard>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>

#include "CryptoPage.h"
#include "util/misc.h"

#include <retroshare/rspeers.h> //for rsPeers variable

/** Constructor */
CryptoPage::CryptoPage(QWidget * parent, Qt::WFlags flags)
    : ConfigPage(parent, flags)
{
  /* Invoke the Qt Designer generated object setup routine */
  ui.setupUi(this);

  connect(ui.copykeyButton, SIGNAL(clicked()), this, SLOT(copyPublicKey()));
  connect(ui.saveButton, SIGNAL(clicked()), this, SLOT(fileSaveAs()));


  loadPublicKey();


  /* Hide platform specific features */
#ifdef Q_WS_WIN

#endif
}

CryptoPage::~CryptoPage()
{
}

void
CryptoPage::closeEvent (QCloseEvent * event)
{

    QWidget::closeEvent(event);
}

/** Saves the changes on this page */
bool
CryptoPage::save(QString &errmsg)
{
 	return true;
}

/** Loads the settings for this page */
void
CryptoPage::load()
{

}

/** Loads ouer default Puplickey  */
void
CryptoPage::loadPublicKey()
{
    QFont font("Courier New",9,50,false) ;
    ui.certtextEdit->setFont(font) ;

    ui.certtextEdit->setPlainText(QString::fromStdString(rsPeers->GetRetroshareInvite()));
    ui.certtextEdit->setReadOnly(true);
    ui.certtextEdit->setMinimumHeight(200);
}

void
CryptoPage::copyPublicKey()
{
    QMessageBox::information(this,
                             tr("RetroShare"),
                             tr("Your Public Key is copied to Clipboard, paste and send it to your"
                                " friend via email or some other way"));
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui.certtextEdit->toPlainText());

}

bool CryptoPage::fileSave()
{
    if (fileName.isEmpty())
        return fileSaveAs();

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly))
        return false;
    QTextStream ts(&file);
    ts.setCodec(QTextCodec::codecForName("UTF-8"));
    ts << ui.certtextEdit->document()->toPlainText();
    ui.certtextEdit->document()->setModified(false);
    return true;
}

bool CryptoPage::fileSaveAs()
{
    QString fn;
    if (misc::getSaveFileName(this, RshareSettings::LASTDIR_CERT, tr("Save as..."), tr("RetroShare Certificate (*.rsc );;All Files (*)"), fn)) {
        setCurrentFileName(fn);
        return fileSave();
    }
    return false;
}

void CryptoPage::setCurrentFileName(const QString &fileName)
{
    this->fileName = fileName;
    ui.certtextEdit->document()->setModified(false);

    setWindowModified(false);
}
