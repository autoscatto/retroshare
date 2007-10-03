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

#include "guiexprelement.h"

QString GuiExprElement::AND     = QObject::tr("And");
QString GuiExprElement::XOR     = QObject::tr("Or");
QString GuiExprElement::OR      = QObject::tr("And / or");

QString GuiExprElement::NAME    = QObject::tr("Name");
QString GuiExprElement::PATH    = QObject::tr("Path");
QString GuiExprElement::EXT     = QObject::tr("Extension");
QString GuiExprElement::KEYWORDS= QObject::tr("Keywords");
QString GuiExprElement::COMMENTS= QObject::tr("Comments");
QString GuiExprElement::META    = QObject::tr("Meta");
QString GuiExprElement::DATE    = QObject::tr("Date");
QString GuiExprElement::SIZE    = QObject::tr("Size");
QString GuiExprElement::POP     = QObject::tr("Popularity");

QString GuiExprElement::CONTAINS= QObject::tr("contains");
QString GuiExprElement::CONTALL = QObject::tr("contains all");
QString GuiExprElement::IS      = QObject::tr("is");
QString GuiExprElement::LT      = QObject::tr("less than");
QString GuiExprElement::LTE     = QObject::tr("less than or equal");
QString GuiExprElement::EQUAL   = QObject::tr("equal");
QString GuiExprElement::GTE     = QObject::tr("greater than or equal");
QString GuiExprElement::GT      = QObject::tr("greater than");
QString GuiExprElement::RANGE   = QObject::tr("is in range");

QStringList * GuiExprElement::exprOpsList = new QStringList();
QStringList * GuiExprElement::searchTermsOptionsList = new QStringList();
QStringList * GuiExprElement::stringOptionsList = new QStringList();
QStringList * GuiExprElement::relOptionsList = new QStringList();

bool GuiExprElement::initialised = false;


GuiExprElement::GuiExprElement(QWidget * parent)
                : QWidget(parent)
{
    if (!GuiExprElement::initialised) 
    {
        initialiseOptionsLists();
    }
}


void GuiExprElement::initialiseOptionsLists()
{
    exprOpsList->append(AND);
    exprOpsList->append(XOR);
    exprOpsList->append(OR);

    GuiExprElement::searchTermsOptionsList->append(NAME); 
    GuiExprElement::searchTermsOptionsList->append(PATH); 
    GuiExprElement::searchTermsOptionsList->append(EXT); 
    GuiExprElement::searchTermsOptionsList->append(KEYWORDS); 
    GuiExprElement::searchTermsOptionsList->append(COMMENTS); 
    GuiExprElement::searchTermsOptionsList->append(META); 
    GuiExprElement::searchTermsOptionsList->append(DATE); 
    GuiExprElement::searchTermsOptionsList->append(SIZE); 
    GuiExprElement::searchTermsOptionsList->append(POP);
    
    GuiExprElement::stringOptionsList->append(CONTAINS);
    GuiExprElement::stringOptionsList->append(CONTALL);
    GuiExprElement::stringOptionsList->append(IS);
    
    GuiExprElement::relOptionsList->append(LT);
    GuiExprElement::relOptionsList->append(LTE);
    GuiExprElement::relOptionsList->append(EQUAL);
    GuiExprElement::relOptionsList->append(GTE);
    GuiExprElement::relOptionsList->append(GT);
    GuiExprElement::relOptionsList->append(RANGE);
    GuiExprElement::initialised = true;
}


ExprOpElement::ExprOpElement(QWidget * parent)
                : GuiExprElement(parent)
{
    cb = new QComboBox(parent);
    cb->addItems(*(GuiExprElement::exprOpsList));
    cb->show();
}

ExprTermsElement::ExprTermsElement(QWidget * parent)
                : GuiExprElement(parent)
{
    cb = new QComboBox(parent);
    cb->addItems(*(GuiExprElement::searchTermsOptionsList));
    cb->show();
}

ExprConditionElement::ExprConditionElement(QWidget * parent)
                : GuiExprElement(parent)
{
    cb = new QComboBox(parent);
    cb->addItems(*(GuiExprElement::stringOptionsList));
    cb->show();
}

ExprParamElement::ExprParamElement(QWidget * parent)
                : GuiExprElement(parent)
{
    // set up for default of a simple input field
    QLineEdit * elem = new QLineEdit(parent);
    elem->show();
}


