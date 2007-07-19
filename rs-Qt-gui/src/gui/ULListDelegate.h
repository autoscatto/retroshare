#ifndef ULLISTDELEGATE_H
#define ULLISTDELEGATE_H

#include <QAbstractItemDelegate>

// Defines for upload list list columns
#define UNAME 0
#define USERNAME 1
#define USIZE 2
#define UPROGRESS 3
#define ULSPEED 4
#define USTATUS 5
#define UTRANSFERRED 6



#define MAX_CHAR_TMP 128

class QModelIndex;
class QPainter;
class QStyleOptionProgressBarV2;
class QProgressBar;
class QApplication;


class ULListDelegate: public QAbstractItemDelegate {

	Q_OBJECT

	public:
		ULListDelegate(QObject *parent=0);
		~ULListDelegate();
		void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
		QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

	private:

	public slots:

	signals:
};
#endif

