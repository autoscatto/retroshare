#ifndef __ConnectFriendWizard__
#define __ConnectFriendWizard__

#include <map>
#include <QWizard>

//QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QTextEdit;
class QLineEdit;
class QRadioButton;
class QVBoxLayout;
class QHBoxLayout;
class QGroupBox;
class QGridLayout;
class QComboBox;
class QTableWidget;
//QT_END_NAMESPACE

const std::string LOCAL_IP = "---LOCAL---";
const std::string EXT_IP = "---EXT---";

//============================================================================
//! A wizard for adding friends. Based on standard QWizard component

//! The process of adding friends follows this scheme:
//!         /-> Use text certificates \       /-> errorpage(if  went wrong)
//! intro -|                           |-> ->
//!         \-> Use *.pqi files      /        \-> fill peer details
//!  
//! So, there are five possible pages in this wizard.

class ConnectFriendWizard : public QWizard
{// 
    Q_OBJECT

public:

    enum { Page_Intro, Page_Text, Page_Cert, Page_ErrorMessage, Page_Conclusion,Page_Foff };

    ConnectFriendWizard(QWidget *parent = 0);

    void accept();

private slots:
//    void showHelp(); // we'll have to implement it in future
};

//============================================================================
//!  Introduction page for "Add friend" wizard.
class IntroPage : public QWizardPage
{
    Q_OBJECT

public:
    IntroPage(QWidget *parent = 0);

    int nextId() const;

private:
    QLabel *topLabel;
    QRadioButton *textRadioButton;
    QRadioButton *certRadioButton;
    QRadioButton *foffRadioButton;
};

//============================================================================
//! Text page (for exchnging text certificates) for "Add friend" wizard.
class TextPage : public QWizardPage
{
    Q_OBJECT

public:
    TextPage(QWidget *parent = 0);

    int nextId() const;

private:
    QLabel*      userCertLabel;
    QTextEdit*   userCertEdit;
    QHBoxLayout* userCertLayout;
    QVBoxLayout* userCertButtonsLayout;
    QPushButton* userCertHelpButton;
    #if defined(Q_OS_WIN)
    QPushButton* userCertMailButton;//! on Windows, click on this button
                                   //! launches default email client
    #endif
    QLabel*      friendCertLabel;
    QTextEdit*   friendCertEdit;
    
    QVBoxLayout* textPageLayout;

private slots:
    void showHelpUserCert();
    
    #if defined(Q_OS_WIN)
    //! launches default email client (on windows)
    
    //! Tested on Vista, it work normally... But a bit slowly.
    void runEmailClient();
    #endif
};

//============================================================================
//! A page for exchanging *.pqi files, for "Add friend" wizard.
class CertificatePage : public QWizardPage
{
    Q_OBJECT

public:
    CertificatePage(QWidget *parent = 0);

    int nextId() const;
    bool isComplete() const ;

private:
    QGroupBox* userFileFrame;
    QLabel *userFileLabel;
    QPushButton* userFileCreateButton;
    QHBoxLayout* userFileLayout;
    
    QLabel* friendFileLabel;
    QLineEdit *friendFileNameEdit;
    QPushButton* friendFileNameOpenButton;
    QHBoxLayout* friendFileLayout;

    QVBoxLayout* certPageLayout;    

private slots:
    void generateCertificateCalled();
    void loadFriendCert();
};

//============================================================================
//! A page for signing certificates from some people on the network (e.g. friends 
// of friends, people trusting me...)
//
class FofPage : public QWizardPage
{
    Q_OBJECT

public:
    FofPage(QWidget *parent = 0);

    int nextId() const;
    bool isComplete() const ;

private:
    QLabel *userFileLabel;
    QVBoxLayout *userFileLayout;
    QComboBox *userSelectionCB;
    QPushButton* makeFriendButton;
    QTableWidget *selectedPeersTW;

    QVBoxLayout* certPageLayout;    

	 bool _friends_signed ;
	 std::map<QCheckBox*,std::string> _id_boxes ;

private slots:
	void signAllSelectedUsers() ;
	void updatePeersList(int) ;
};


//============================================================================
//! Page for displaying error messages (for "Add friend" wizard).
class ErrorMessagePage : public QWizardPage
{
    Q_OBJECT

public:
    ErrorMessagePage(QWidget *parent = 0);

    int nextId() const;

private:
    QLabel *messageLabel;
    QVBoxLayout* errMessLayout;
};

//============================================================================
//! Page for filling peer details in "Add friend" wizard.

//! Warning: This page duplicates functionality of the ConnectDialo class (and
//! also some pieces of code). TODO: remove this duplication
class ConclusionPage : public QWizardPage
{
    Q_OBJECT

public:
    ConclusionPage(QWidget *parent = 0);

    void initializePage();
    int nextId() const;

private slots:
//    void printButtonClicked();

private:
    QGroupBox* peerDetailsFrame;
    QLabel* trustLabel;
    QLineEdit* trustEdit;
    QLabel* nameLabel;
    QLineEdit* nameEdit;
    QLabel* orgLabel;
    QLineEdit* orgEdit;
    QLabel* locLabel;
    QLineEdit* locEdit;
    QLabel* countryLabel;
    QLineEdit* countryEdit;
    QLabel* signersLabel;
    QTextEdit* signersEdit;
    QGridLayout* peerDetailsLayout;
    
    QLabel* authCodeLabel;
    QLineEdit* authCodeEdit;
    QHBoxLayout* authCodeLayout;

    QVBoxLayout* conclusionPageLayout;

    //! peer id

    //! It's a hack; This widget is used only to register "id" field in the
    //! wizard. Really the widget isn't displayed.
    QLineEdit* peerIdEdit;
    QLineEdit* ext_friend_ip;
    QLineEdit* ext_friend_port;
    QLineEdit* local_friend_ip;
    QLineEdit* local_friend_port;
};

//============================================================================

#endif
