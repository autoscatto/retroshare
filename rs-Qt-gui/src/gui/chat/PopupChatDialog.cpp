/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006,  crypton
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

#include <QtGui>

#include "rshare.h"
#include "PopupChatDialog.h"
#include "config/gconfig.h"

#include <QTextCodec>
#include <QTextEdit>
#include <QToolBar>
#include <QTextCursor>
#include <QTextList>


/* Define the format used for displaying the date and time */
#define DATETIME_FMT  "MMM dd hh:mm:ss"



/** Default constructor */
PopupChatDialog::PopupChatDialog(QWidget *parent, Qt::WFlags flags)
  : QMainWindow(parent, flags)
{
  /* Invoke Qt Designer generated QObject setup routine */
  ui.setupUi(this);

  GConfig config;
  config.loadWidgetInformation(this);

  connect(ui.colorButton, SIGNAL(clicked()), this, SLOT(setColor()));
  
  connect(ui.textboldButton, SIGNAL(clicked()), this, SLOT(textBold()));
  
  connect(ui.textunderlineButton, SIGNAL(clicked()), this, SLOT(textUnderline()));
  
  connect(ui.textitalicButton, SIGNAL(clicked()), this, SLOT(textItalic()));

  // Create the status bar
  statusBar()->showMessage("");
 
}



/** 
 Overloads the default show() slot so we can set opacity*/

void
PopupChatDialog::show()
{
  //loadSettings();
  if(!this->isVisible()) {
    QMainWindow::show();

  }
}

void PopupChatDialog::closeEvent (QCloseEvent * event)
{
 GConfig config;
 config.saveWidgetInformation(this);

 QWidget::closeEvent(event);
}

void PopupChatDialog::setColor()
{
    QColor col = QColorDialog::getColor(Qt::green, this);
    if (col.isValid()) {

        ui.colorButton->setPalette(QPalette(col));
        QTextCharFormat fmt;
        fmt.setForeground(col);
        mergeFormatOnWordOrSelection(fmt);
        colorChanged(col);
    }
}

void PopupChatDialog::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(ui.textboldButton->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void PopupChatDialog::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(ui.textunderlineButton->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void PopupChatDialog::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(ui.textitalicButton->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void PopupChatDialog::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
    colorChanged(format.foreground().color());
}
   
void PopupChatDialog::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = ui.textBrowser->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    ui.textBrowser->mergeCurrentCharFormat(format);
}

void PopupChatDialog::fontChanged(const QFont &f)
{
    //comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    //comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    ui.textboldButton->setChecked(f.bold());
    ui.textunderlineButton->setChecked(f.italic());
    ui.textitalicButton->setChecked(f.underline());
}



void PopupChatDialog::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    ui.colorButton->setIcon(pix);
}


void PopupChatDialog::updateChat()
{
	/* get chat off queue */

	/* write it out */

}


void PopupChatDialog::sendChat()
{
	/* get the current txt */

	/* send to server */


}


