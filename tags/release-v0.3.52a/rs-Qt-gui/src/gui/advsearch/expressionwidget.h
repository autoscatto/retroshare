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
#ifndef _ExpressionWidget_h_
#define _ExpressionWidget_h_
#include <iostream>
#include <QWidget>
#include <QLabel>

#include "rsiface/rsexpr.h"
#include "guiexprelement.h"
#include "ui_expressionwidget.h"


/**
    Represents an Advanced Search GUI Expression object which acts as a container
    for a series of GuiExprElement objects. The structure of the expression can
    change dynamically, dependant on user choices made while assembling an expression. 
    The default appearance is simply a search on name.
*/
class ExpressionWidget : public QWidget, public Ui::ExpressionWidget 
{
    Q_OBJECT

public:
    ExpressionWidget( QWidget * parent = 0, bool initial=false );
    
    /** delivers the expression represented by this widget
     the operator to join this expression with any previous 
     expressions is provided by the getOperator method */
    Expression* getRsExpression();

    /** supplies the operator to be used when joining this expression
        to the whole query */
    LogicalOperator getOperator();
    
    QString toString();

signals:
    /** associates an expression object with the delete event */
    void signalDelete(ExpressionWidget*);

private slots:
    /** emits the signalDelete signal with a pointer to this object
        for use by listeners */
    void deleteExpression();

    /** dynbamically changes the structure of the expression based on
        the terms combobox changes */
    void adjustExprForTermType(int);
    
    /** dynamically adjusts the expression dependant on the choices 
        made in the condition combobox e.g. inRange and equals
        have different parameter fields */
    void adjustExprForConditionType(int);


private:
    QLayout * createLayout(QWidget* parent = 0);
    
    bool isStringSearchExpression();
    
    QList <GuiExprElement *> * elements;
    QLayout * exprLayout;

    ExprOpElement *        exprOpElem;
    ExprTermsElement *     exprTermElem;
    ExprConditionElement * exprCondElem;
    ExprParamElement*      exprParamElem;
    
    bool inRangedConfig;
    bool isFirst;
    ExprSearchType searchType;
};

#endif // _ExpressionWidget_h_
