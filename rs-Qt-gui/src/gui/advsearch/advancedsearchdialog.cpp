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

#include "advancedsearchdialog.h"

AdvancedSearchDialog::AdvancedSearchDialog(QWidget * parent) : QDialog (parent) 
{
    setupUi(this);
    
    // the list of expressions
    expressions = new QList<ExpressionWidget*>();

    // a scrollable area for holding the objects
    expressionsLayout = new QVBoxLayout();
    expressionsLayout->setSpacing(6);
    expressionsLayout->setMargin(9);
    expressionsLayout->setObjectName(QString::fromUtf8("expressionsLayout"));

    expressionsFrame->setLayout(expressionsLayout);
    
    // we now add the first expression widgets to the dialog via a vertical
    // layout 
    addNewExpression();

    connect (this->addExprButton, SIGNAL(clicked()),
             this, SLOT(addNewExpression()));
    connect (this->resetButton, SIGNAL(clicked()),
             this, SLOT(reset()));
    this->adjustSize();
}


void AdvancedSearchDialog::addNewExpression()
{
    ExpressionWidget *expr;
    if (expressions->size() == 0)
    {
        //create an initial expression
        expr = new ExpressionWidget(expressionsFrame, true);
    } else {
        expr = new ExpressionWidget(expressionsFrame);
    }
    
    expressions->append(expr);
    expressionsLayout->addWidget(expr, 1, Qt::AlignLeft);

    connect(expr, SIGNAL(signalDelete(ExpressionWidget*)),
            this, SLOT(deleteExpression(ExpressionWidget*)));
    expr->show();
}

void AdvancedSearchDialog::deleteExpression(ExpressionWidget* expr)
{
    expressions->removeAll(expr);
    expressionsLayout->removeWidget(expr);
    expressionsLayout->invalidate();
    this->adjustSize();
    delete expr;
}

void AdvancedSearchDialog::reset()
{
    ExpressionWidget *expr;
    while (!expressions->isEmpty())
    {
        expr = expressions->takeLast();
        expr->hide();
        expressionsLayout->removeWidget(expr);
        delete expr;
    }
    expressionsLayout->invalidate();
    this->adjustSize();

    // now add a new default expressions
    addNewExpression();
}

