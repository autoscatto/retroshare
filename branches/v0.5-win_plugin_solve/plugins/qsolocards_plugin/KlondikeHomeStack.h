/*
    QSoloCards is a collection of Solitaire card games written using Qt
    Copyright (C) 2009  Steve Moore

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KLONDIKEHOMESTACK_H
#define KLONDIKEHOMESTACK_H

#include "CardStack.h"

class KlondikeHomeStack : public CardStack
{
    Q_OBJECT
public:
    KlondikeHomeStack();
    ~KlondikeHomeStack();

    inline bool isStackComplete(){return getCardVector().size()==PlayingCard::MaxCardIndex;}

    inline int score() const {return getCardVector().size();}
    bool canAddCards(const PlayingCardVector &);
protected:
    bool canMoveCard(unsigned int index) const;

};

#endif // KLONDIKEHOMESTACK_H
