#ifndef DLLISTDELEGATE_H
#define DLLISTDELEGATE_H

#include <QAbstractItemDelegate>

// Defines for download list list columns
#define NAME 0
#define SIZE 1
#define PROGRESS 2
#define DLSPEED 3
#define SOURCES 4
#define STATUS 5
#define COMPLETED 6
#define REMAINING 7


#define MAX_CHAR_TMP 128

class QModelIndex;
class QPainter;
class QStyleOptionProgressBarV2;
class QProgressBar;
class QApplication;


class DLListDelegate: public QAbstractItemDelegate {

	Q_OBJECT

	public:
		DLListDelegate(QObject *parent=0);
		~DLListDelegate();
		void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
		QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

	private:

	public slots:

	signals:
};
#endif

