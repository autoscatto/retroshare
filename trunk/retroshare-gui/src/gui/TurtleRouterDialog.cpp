#include <iostream>
#include <QTimer>
#include <QMenu>
#include <QMouseEvent>
#include <rsiface/rsturtle.h>
#include "TurtleRouterDialog.h"

TurtleRouterDialog *TurtleRouterDialog::_instance = NULL ;

TurtleRouterDialog::TurtleRouterDialog(QWidget *parent)
	: RsAutoUpdatePage(2000,parent)
{
	setupUi(this) ;
	connect( _hashes_TW, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showCtxMenu(const QPoint&)));

	_hashes_TW->setToolTip(tr("As a helper, you can manually remove hashes\nfrom there. Warning: doing so on currently\nactive transfers will block the transfer process.")) ;
}

void TurtleRouterDialog::showCtxMenu(const QPoint& point)
{
	if(_hashes_TW->currentItem() == NULL)
		return ;

	std::cerr << "execing context menu" << std::endl ;
	// create context menus.
	QMenu contextMnu( this );
	QAction *removeHashAction = new QAction(QIcon(":/images/delete.png"), tr("Stop handling this hash"), this);
	connect(removeHashAction , SIGNAL(triggered()), this, SLOT(removeFileHash()));
	contextMnu.addAction(removeHashAction);

	QMouseEvent *mevent = new QMouseEvent(QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier ) ;
	contextMnu.exec( mevent->globalPos() );
}

void TurtleRouterDialog::showUp()
{
	if(_instance == NULL)
		_instance = new TurtleRouterDialog ;

	_instance->show() ;
	_instance->update() ;
}

void TurtleRouterDialog::removeFileHash()
{
	QTableWidgetItem *item = _hashes_TW->currentItem();

	std::cout << "in remove hash " << std::endl ;
	if(item == NULL)
		return ;

	std::cout << "item->row = " << item->row() << std::endl ;
	std::string hash = _hashes_TW->item(item->row(),0)->text().toStdString() ;

	std::cout << "remove ing hash: " << hash << std::endl ;

	rsTurtle->stopMonitoringFileTunnels(hash) ;
}

void TurtleRouterDialog::updateDisplay()
{
	std::cout << "updatign turtle router console."<< std::endl ;

	std::vector<std::vector<std::string> > hashes_info ;
	std::vector<std::vector<std::string> > tunnels_info ;
	std::vector<std::vector<std::string> > search_reqs_info ;
	std::vector<std::vector<std::string> > tunnel_reqs_info ;

	rsTurtle->getInfo(hashes_info,tunnels_info,search_reqs_info,tunnel_reqs_info) ;

	// now display this in the QTableWidgets

	fillTable( _hashes_TW, hashes_info) ;
	fillTable( _tunnels_TW, tunnels_info) ;

	std::vector<std::vector<std::string> >& reqs(search_reqs_info) ;

	for(uint i=0;i<search_reqs_info.size();++i) search_reqs_info[i].push_back("Search request") ;
	for(uint i=0;i<tunnel_reqs_info.size();++i) 
	{
		tunnel_reqs_info[i].push_back("Tunnel request") ;
		reqs.push_back(tunnel_reqs_info[i]) ;
	}

	fillTable( _reqs_TW, reqs) ;
}

void TurtleRouterDialog::fillTable(QTableWidget *table,const std::vector<std::vector<std::string> >& data)
{
	table->clearContents() ;

	for(uint i=0;i<data.size();++i)
	{
		if(table->rowCount() <= i)
			table->insertRow(i) ;

		for(uint j=0;j<data[i].size();++j)
			table->setItem(i,j,new QTableWidgetItem(QString::fromStdString(data[i][j].empty()?"Unknown":data[i][j]))) ;
	}

	for(uint i=data.size();i<table->rowCount();)
		table->removeRow(i) ;
}

