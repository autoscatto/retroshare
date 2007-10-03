/****************************************************************
*  RetroShare is distributed under the following license:
*
*  Copyright (C) 2006, 2007 The RetroShare Team
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
#ifndef _GuiExprElement_h_
#define _GuiExprElement_h_

#include <QWidget>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QComboBox>
#include <QVariant>
#include <QLineEdit>

class GuiExprElement: public QWidget
{
    Q_OBJECT

    public:
        GuiExprElement(QWidget * parent = 0);
        virtual QVariant * getRSExprValue(){return 0;}
        virtual ~GuiExprElement(){}

    protected:
    /** prepare the option lists for use */
    void initialiseOptionsLists();
    
    static bool initialised;
    static QStringList * exprOpsList;
    static QStringList * searchTermsOptionsList;
    static QStringList * stringOptionsList;
    static QStringList * relOptionsList;

    /** translatable option strings for the comboboxes */
    static QString AND    ;
    static QString XOR    ;
    static QString OR     ;
    static QString NAME   ;
    static QString PATH   ;
    static QString EXT    ;
    static QString KEYWORDS;
    static QString COMMENTS;
    static QString META   ;
    static QString DATE   ;
    static QString SIZE   ;
    static QString POP    ;
    static QString CONTAINS;
    static QString CONTALL ;
    static QString IS     ;
    static QString LT     ;
    static QString LTE    ;
    static QString EQUAL  ;
    static QString GTE    ;
    static QString GT     ;
    static QString RANGE  ;
};

/** the Expression operator combobox element */
class ExprOpElement : public GuiExprElement
{
    Q_OBJECT
    
    public:
        ExprOpElement(QWidget * parent = 0);
        QVariant* getRSExprValue(){return 0;}
    private:
        QComboBox * cb;
};

/** the Terms combobox element */
class ExprTermsElement : public GuiExprElement
{
    Q_OBJECT
    
    public:
        ExprTermsElement(QWidget * parent = 0);
        QVariant* getRSExprValue(){return 0;}
    private:
        QComboBox * cb;
};

/** the Conditions combobox element */
class ExprConditionElement : public GuiExprElement
{
    Q_OBJECT
    
    public:
        ExprConditionElement(QWidget * parent = 0);
        QVariant* getRSExprValue(){return 0;}
    private:
        QComboBox * cb;
};

/** the Parameter element */
class ExprParamElement : public GuiExprElement
{
    Q_OBJECT
    
    public:
        ExprParamElement(QWidget * parent = 0);
        QVariant* getRSExprValue(){return 0;}
    private:
        QLineEdit * lineEdit;
};





#endif
