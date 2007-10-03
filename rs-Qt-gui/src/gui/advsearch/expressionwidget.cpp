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

#include "expressionwidget.h"

ExpressionWidget::ExpressionWidget(QWidget * parent, bool initial) : QWidget(parent)
{
    setupUi(this);
    
    elements = new QList<GuiExprElement*>();
    
    // now fill with sensible defaults
    GuiExprElement * tempElem;
    
    // the expression operator combobox
    tempElem = new ExprOpElement(exprOpFrame);
    elements->append(tempElem);
    
    // the terms combobox
    tempElem = new ExprTermsElement(exprTermFrame);
    elements->append(tempElem);
    
    // the condition combobox
    tempElem = new ExprConditionElement(exprConditionFrame);
    elements->append(tempElem);
    
    // the parameter expression: can be a date, 1-2 edit fields
    //    or a size with units etc
    tempElem = new ExprParamElement(exprParamFrame);
    elements->append(tempElem);
    
    deleteExprButton    ->setVisible(!initial);
    exprOpFrame         ->setVisible(!initial);
    exprTermFrame       ->show();
    exprConditionFrame  ->show();
    exprParamFrame      ->show();
    
    // connect the delete button signal        
    connect (deleteExprButton, SIGNAL (clicked()),
                this, SLOT(deleteExpression()));
    
    this->show();
}

Expression* getRsExpression()
{
    // iterate through the items in elements and
    // call getRSExpr on each
    return 0;
}

void ExpressionWidget::deleteExpression() 
{
   this->hide();
   emit signalDelete(this); 
}
