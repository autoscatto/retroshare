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

#include <QWidget>

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
    Expression* getRsExpression();

signals:
    /** associates an expression object with the delete event */
    void signalDelete(ExpressionWidget*);

private slots:
    /** emits the signalDelete signal with a pointer to this object
        for use by listeners */
    void deleteExpression();

private:
    QList <GuiExprElement *> * elements;
};

#endif // _ExpressionWidget_h_
