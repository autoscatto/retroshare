/***************************************************************************
 *   Copyright (C) 2008 by normal   *
 *   normal@Desktop2   *
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

#include "form_settingsgui.h"

form_settingsgui::form_settingsgui(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
    	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	settings= new QSettings(QApplication::applicationDirPath()+"/application.ini",QSettings::IniFormat);

    	loadqss();
	styleCombo->addItems(QStyleFactory::keys());
	loadSettings();

	connect(ok_Button,     SIGNAL(clicked(bool) ),this,SLOT(saveSettings() ) );
	connect(cancel_Button, SIGNAL(clicked(bool) ),this,SLOT(close() ) );
	connect(cmd_openFile,  SIGNAL(clicked(bool) ),this,SLOT(on_cmd_openFile()));
	connect(cmd_openFile_2,SIGNAL(clicked(bool) ),this,SLOT(on_cmd_openFile2()));
	connect(cmd_openFile_3,SIGNAL(clicked(bool) ),this,SLOT(on_cmd_openFile3()));
	connect(cmd_openFile_4,SIGNAL(clicked(bool) ),this,SLOT(on_cmd_openFile4()));
	connect(cmd_openFile_5,SIGNAL(clicked(bool) ),this,SLOT(on_cmd_openFile5()));
	connect(cmd_openFile_6,SIGNAL(clicked(bool) ),this,SLOT(on_cmd_openFile6()));

}

form_settingsgui::~form_settingsgui()
{
	delete (settings);
}

void form_settingsgui::loadSettings()
{
	settings->beginGroup("General");
		spinBox->setValue(settings->value("Debug_Max_Message_count","20").toInt());
		spinBox_3->setValue(settings->value("Waittime_between_rechecking_offline_users","30000").toInt()/1000);
		

		if(settings->value("current_Style","").toString().isEmpty()==false)
			styleCombo->setCurrentIndex(styleCombo->findText(settings->value("current_Style","").toString()));
		else
		{
			//find default Style for this System
			QRegExp regExp("Q(.*)Style");
			QString defaultStyle = QApplication::style()->metaObject()->className();
			
			if (defaultStyle == QLatin1String("QMacStyle"))
				defaultStyle = QLatin1String("Macintosh (Aqua)");
			else if (regExp.exactMatch(defaultStyle))
				defaultStyle = regExp.cap(1);
			
			//styleCombo->addItems(QStyleFactory::keys());
			styleCombo->setCurrentIndex(styleCombo->findText(defaultStyle));

		}

		styleSheetCombo->setCurrentIndex(styleSheetCombo->findText(settings->value("current_Style_sheet","Default").toString()));
	settings->endGroup();

	settings->beginGroup("Network");
		lineEdit_3->setText(settings->value("SamHost","127.0.0.1").toString());
		lineEdit_2->setText(settings->value("Destination","test").toString());
		lineEdit->setText(settings->value("TunnelName","I2PChat").toString());
		spinBox_10->setValue(settings->value("SamPort","7656").toInt());
		
		spinBox_4->setMinimum(1);
		spinBox_4->setValue(settings->value("inbound.length","1").toInt());
		spinBox_4->setMaximum(3);
	
		spinBox_5->setMinimum(0);
		spinBox_5->setValue(settings->value("inbound.quantity","1").toInt());
		spinBox_5->setMaximum(3);
		
		spinBox_6->setMinimum(0);
		spinBox_6->setValue(settings->value("inbound.backupQuantity","1").toInt());
		spinBox_6->setMaximum(3);
	
		spinBox_7->setMinimum(0);
		spinBox_7->setValue(settings->value("outbound.backupQuantity","1").toInt());
		spinBox_7->setMaximum(3);
	
		spinBox_8->setMinimum(1);
		spinBox_8->setValue(settings->value("outbound.length","1").toInt());
		spinBox_8->setMaximum(3);
		
		spinBox_9->setMinimum(0);
		spinBox_9->setValue(settings->value("outbound.quantity","1").toInt());
		spinBox_9->setMaximum(3);
	settings->endGroup();


	settings->beginGroup("Sound");
		settings->beginGroup("SoundFilePath");
			txt_SoundFile->setText(settings->value("User_go_Online","").toString());
			txt_SoundFile2->setText(settings->value("User_go_Offline","").toString());
			txt_SoundFile3->setText(settings->value("FileSend_Finished","").toString());
			txt_SoundFile4->setText(settings->value("FileRecive_Incoming","").toString());
			txt_SoundFile5->setText(settings->value("FileRecive_Finished","").toString());
			txt_SoundFile6->setText(settings->value("NewChatMessage","").toString());

			if(!txt_SoundFile->text().isEmpty())checkBoxSound->setEnabled(true);
			if(!txt_SoundFile2->text().isEmpty())checkBoxSound_2->setEnabled(true);
			if(!txt_SoundFile3->text().isEmpty())checkBoxSound_3->setEnabled(true);
			if(!txt_SoundFile4->text().isEmpty())checkBoxSound_4->setEnabled(true);
			if(!txt_SoundFile5->text().isEmpty())checkBoxSound_5->setEnabled(true);
			if(!txt_SoundFile6->text().isEmpty())checkBoxSound_6->setEnabled(true);

		settings->endGroup();

		settings->beginGroup("Enable");
			checkBoxSound->setChecked(settings->value("User_go_Online",false).toBool());
			checkBoxSound_2->setChecked(settings->value("User_go_Offline",false).toBool());
			checkBoxSound_3->setChecked(settings->value("FileSend_Finished",false).toBool());
			checkBoxSound_4->setChecked(settings->value("FileRecive_Incoming",false).toBool());
			checkBoxSound_5->setChecked(settings->value("FileRecive_Finished",false).toBool());
			checkBoxSound_6->setChecked(settings->value("NewChatMessage",false).toBool());
		settings->endGroup();	
		
	settings->endGroup();

}
void form_settingsgui::saveSettings()
{	
	QString SessionOptionString;
	QString temp;

	SessionOptionString="inbound.nickname="+lineEdit->text();
	
	SessionOptionString+=" inbound.quantity=";
	temp.setNum(spinBox_5->value());
	SessionOptionString+=temp;
	
	SessionOptionString+=" inbound.backupQuantity=";
	temp.setNum(spinBox_6->value());
	SessionOptionString+=temp;
	
	SessionOptionString+=" inbound.length=";
	temp.setNum(spinBox_4->value());
	SessionOptionString+=temp;
	
	SessionOptionString+=" outbound.quantity=";
	temp.setNum(spinBox_9->value());
	SessionOptionString+=temp;

	SessionOptionString+=" outbound.backupQuantity=";
	temp.setNum(spinBox_7->value());
	SessionOptionString+=temp;
	
	SessionOptionString+=" outbound.length=";
	temp.setNum(spinBox_8->value());
	SessionOptionString+=temp;

	settings->beginGroup("General");
		settings->setValue("Debug_Max_Message_count",spinBox->value());
		settings->setValue("Waittime_between_rechecking_offline_users",spinBox_3->value()*1000);
		settings->setValue("current_Style",styleCombo->currentText());
		settings->setValue("current_Style_sheet",styleSheetCombo->currentText());
	settings->endGroup();

	settings->beginGroup("Network");
		settings->setValue("SamHost",lineEdit_3->text());
		settings->setValue("Destination",lineEdit_2->text());
		settings->setValue("TunnelName",lineEdit->text());
		settings->setValue("SamPort",spinBox_10->value());
		//Inbound options
		settings->setValue("inbound.quantity",spinBox_5->value());
		settings->setValue("inbound.backupQuantity",spinBox_6->value());
		settings->setValue("inbound.length",spinBox_4->value());
		//Outpound options
		settings->setValue("outbound.quantity",spinBox_9->value());
		settings->setValue("outbound.backupQuantity",spinBox_7->value());
		settings->setValue("outbound.length",spinBox_8->value());
		
		settings->setValue("SessionOptionString",SessionOptionString);
	settings->endGroup();
	settings->beginGroup("Sound");
		settings->beginGroup("Enable");
			settings->setValue("User_go_Online",checkBoxSound->isChecked());
			settings->setValue("User_go_Offline",checkBoxSound_2->isChecked());
			settings->setValue("FileSend_Finished",checkBoxSound_3->isChecked());
			settings->setValue("FileRecive_Incoming",checkBoxSound_4->isChecked());
			settings->setValue("FileRecive_Finished",checkBoxSound_5->isChecked());
			settings->setValue("NewChatMessage",checkBoxSound_6->isChecked());
			settings->endGroup();
		settings->beginGroup("SoundFilePath");
			settings->setValue("User_go_Online",txt_SoundFile->text());
			settings->setValue("User_go_Offline",txt_SoundFile2->text());
			settings->setValue("FileSend_Finished",txt_SoundFile3->text());
			settings->setValue("FileRecive_Incoming",txt_SoundFile4->text());
			settings->setValue("FileRecive_Finished",txt_SoundFile5->text());
			settings->setValue("NewChatMessage",txt_SoundFile6->text());
		settings->endGroup();
	settings->endGroup();
	this->close();
}


void form_settingsgui::on_styleCombo_activated(const QString &styleName)
{
	qApp->setStyle(styleName);

}

void form_settingsgui::on_styleSheetCombo_activated(const QString &sheetName)
{
	loadStyleSheet(sheetName);
}

void form_settingsgui::loadStyleSheet(const QString &sheetName)
{
	// external Stylesheets
	QFile file(QApplication::applicationDirPath() + "/qss/" + sheetName.toLower() + ".qss");
	
	file.open(QFile::ReadOnly);
	QString styleSheet = QLatin1String(file.readAll());
	
	
	qApp->setStyleSheet(styleSheet); 
}

void form_settingsgui::loadqss()
{

	QFileInfoList slist = QDir(QApplication::applicationDirPath() + "/qss/").entryInfoList();
	foreach(QFileInfo st, slist)
	{
	if(st.fileName() != "." && st.fileName() != ".." && st.isFile())
	styleSheetCombo->addItem(st.fileName().remove(".qss"));
	}
}
////FilePath=QFileDialog::getOpenFileName(this,"Open File", ".", "all Files (*.*)");
void form_settingsgui::on_cmd_openFile()
{

	txt_SoundFile->setText(QFileDialog::getOpenFileName(this,"Open File", ".", "wav (*.wav)"));
	if(txt_SoundFile->text().isEmpty()){
		checkBoxSound->setChecked(false);
		checkBoxSound->setEnabled(false);
	}
	else
		checkBoxSound->setEnabled(true);
}

void form_settingsgui::on_cmd_openFile2()
{
	txt_SoundFile2->setText(QFileDialog::getOpenFileName(this,"Open File", ".", "wav (*.wav)"));
	if(txt_SoundFile2->text().isEmpty()){	
		checkBoxSound_2->setChecked(false);
		checkBoxSound_2->setEnabled(false);
	}	
	else
		checkBoxSound_2->setEnabled(true);

}
void form_settingsgui::on_cmd_openFile3()
{
	txt_SoundFile3->setText(QFileDialog::getOpenFileName(this,"Open File", ".", "wav (*.wav)"));
	if(txt_SoundFile3->text().isEmpty()){
		checkBoxSound_3->setChecked(false);
		checkBoxSound_3->setEnabled(false);
	}
	else
		checkBoxSound_3->setEnabled(true);
}
void form_settingsgui::on_cmd_openFile4()
{
	txt_SoundFile4->setText(QFileDialog::getOpenFileName(this,"Open File", ".", "wav (*.wav)"));
	if(txt_SoundFile4->text().isEmpty()){
		checkBoxSound_4->setChecked(false);
		checkBoxSound_4->setEnabled(false);
	}
	else
		checkBoxSound_4->setEnabled(true);
}
void form_settingsgui::on_cmd_openFile5()
{
	txt_SoundFile5->setText(QFileDialog::getOpenFileName(this,"Open File", ".", "wav (*.wav)"));
	if(txt_SoundFile5->text().isEmpty()){
		checkBoxSound_5->setChecked(false);
		checkBoxSound_5->setEnabled(false);
	}
	else
		checkBoxSound_5->setEnabled(true);
}
void form_settingsgui::on_cmd_openFile6()
{
	txt_SoundFile6->setText(QFileDialog::getOpenFileName(this,"Open File", ".", "wav (*.wav)"));
	if(txt_SoundFile6->text().isEmpty()){
		checkBoxSound_6->setChecked(false);
		checkBoxSound_6->setEnabled(false);
	}
	else
		checkBoxSound_6->setEnabled(true);
	
}