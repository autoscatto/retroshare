######################################################################
# Automatically generated by qmake (2.00a) Fr 18. Aug 22:48:56 2006
######################################################################
OBJECTS_DIR = temp/obj
RCC_DIR = temp/qrc
UI_DIR  = temp/ui
MOC_DIR = temp/moc
CONFIG += qt release
QT     += network xml
TEMPLATE = app
TARGET += 
DEPENDPATH += . \
            rsiface \
            config \
            control \
            gui \
            lang \
            util \
            gui\bwgraph \
            gui\chat \
            gui\connect \
            gui\filehash \
            gui\images \              
            gui\moreinfo \
            gui\Preferences \
            gui\common\
            gui\Settings \
            gui\toaster \
            gui\authdlg
            
INCLUDEPATH += .

# Input
HEADERS +=  rshare.h \
            rsiface/rsiface.h \
            rsiface/rstypes.h \
            rsiface/notifyqt.h \
            rsiface/RemoteDirModel.h \
            config/rshareSettings.h \
            control/bandwidthevent.h \
            control/eventtype.h \
            gui/DLListDelegate.h \
            gui/ULListDelegate.h \
            gui/StartDialog.h \
            gui/ChatDialog.h \
            gui/NetworkDialog.h \
            gui/GenCertDialog.h \
            gui/TransfersDialog.h \
            gui/graphframe.h \
            gui/linetypes.h \
            gui/mainpage.h \
            gui/mainpagestack.h \
            gui/MainWindow.h \
            gui/MessengerWindow.h \
            gui/PeersDialog.h \
            gui/SearchDialog.h \
            gui/SharedFilesDialog.h \
            gui/StatisticDialog.h \
            gui/ChannelsDialog.h \
            gui/HelpDialog.h \
            gui/LogoBar.h \
            lang/languagesupport.h \
            util/process.h \
            util/registry.h \
            util/string.h \
            util/win32.h \
            util/RetroStyleLabel.h \
            util/dllexport.h \
            util/NonCopyable.h \
            util/rsutildll.h \ 
            util/dllexport.h \
            util/global.h \
            util/rsqtutildll.h \
            util/Interface.h \
            util/PixmapMerging.h \
            util/MouseEventFilter.h \
            util/EventFilter.h \
            util/Widget.h \
            util/rsversion.h \
            gui/bwgraph/bwgraph.h \
            gui/chat/PopupChatDialog.h \
            gui/connect/ConnectDialog.h \
            gui/connect/ConfCertDialog.h \
            gui/connect/InviteDialog.h \
            gui/connect/AddFriendDialog.h \
            gui/connect/AddFriendWizard.h \
            gui/msgs/ChanMsgDialog.h \
            gui/msgs/ChanCreateDialog.h \
            gui/filehash/FileHashDialog.h \
            gui/images/retroshare_win.rc.h \
            gui/moreinfo/moreinfo.h \ 
            gui/Preferences/configpage.h \
            gui/Preferences/configpagestack.h \
            gui/Preferences/CryptographyDialog.h \
            gui/Preferences/DirectoriesDialog.h \
            gui/Preferences/LogDialog.h \
            gui/Preferences/PreferencesDialog.h \
            gui/Preferences/PreferencesWindow.h \
            gui/Preferences/ServerDialog.h \          
            gui/Preferences/ConfirmQuitDialog.h \
            gui/toaster/MessageToaster.h \
            gui/toaster/ChatToaster.h \
            gui/toaster/CallToaster.h \
            gui/toaster/QtToaster.h \
            gui/toaster/IQtToaster.h \
            gui/toaster/RetroStyleLabelProxy.h \
            gui/common/vmessagebox.h \
            gui/common/rwindow.h \
            gui/MessagesDialog.h \
            gui/Settings/gsettingswin.h \
            gui/Settings/GeneralPage.h \
            gui/Settings/DirectoriesPage.h \
            gui/Settings/ServerPage.h \
            gui/Settings/NetworkPage.h \
            gui/authdlg/AuthorizationDialog.h \
            gui/advsearch/advancedsearchdialog.h \
            gui/advsearch/expressionwidget.h \
            gui/advsearch/guiexprelement.h

FORMS +=    gui/ChatDialog.ui \
            gui/StartDialog.ui \
            gui/GenCertDialog.ui \
            gui/NetworkDialog.ui \
            gui/TransfersDialog.ui \
            gui/MainWindow.ui \
            gui/MessengerWindow.ui \
            gui/PeersDialog.ui \
            gui/SearchDialog.ui \
            gui/SharedFilesDialog.ui \
            gui/StatisticDialog.ui \
            gui/ChannelsDialog.ui \
            gui/MessagesDialog.ui \
            gui/HelpDialog.ui \
            gui/bwgraph/bwgraph.ui \
            gui/chat/PopupChatDialog.ui \
            gui/connect/ConnectDialog.ui \
            gui/connect/ConfCertDialog.ui \
            gui/connect/InviteDialog.ui \
            gui/connect/AddFriendDialog.ui \
            gui/connect/AddFriendWizard.ui \
            gui/msgs/ChanMsgDialog.ui \
            gui/msgs/ChanCreateDialog.ui \
            gui/filehash/FileHashDialog.ui \
            gui/moreinfo/moreinfo.ui \ 
            gui/Preferences/CryptographyDialog.ui \
            gui/Preferences/DirectoriesDialog.ui \
            gui/Preferences/LogDialog.ui \
            gui/Preferences/PreferencesDialog.ui \
            gui/Preferences/PreferencesWindow.ui \
            gui/Preferences/ServerDialog.ui \
            gui/Preferences/ConfirmQuitDialog.ui \
            gui/toaster/CallToaster.ui \
            gui/toaster/ChatToaster.ui \
            gui/toaster/MessageToaster.ui \
            gui/Settings/settings.ui \
            gui/Settings/GeneralPage.ui \
            gui/Settings/DirectoriesPage.ui \
            gui/Settings/ServerPage.ui \
            gui/Settings/NetworkPage.ui \
            gui/authdlg/AuthorizationDialog.ui \
            gui/advsearch/advancedsearchdialog.ui \
            gui/advsearch/expressionwidget.ui

SOURCES += main.cpp \
            rshare.cpp \
            rsiface/notifyqt.cpp \
            rsiface/RemoteDirModel.cpp \
            config/rshareSettings.cpp \
            gui/DLListDelegate.cpp \
            gui/ULListDelegate.cpp \
            gui/StartDialog.cpp \
            gui/GenCertDialog.cpp \
            gui/ChatDialog.cpp \
            gui/NetworkDialog.cpp \
            gui/TransfersDialog.cpp \
            gui/graphframe.cpp \
            gui/mainpagestack.cpp \
            gui/MainWindow.cpp \
            gui/MessengerWindow.cpp \
            gui/PeersDialog.cpp \
            gui/SearchDialog.cpp \
            gui/SharedFilesDialog.cpp \
            gui/StatisticDialog.cpp \
            gui/ChannelsDialog.cpp \
            gui/MessagesDialog.cpp \
            gui/HelpDialog.cpp \
            gui/LogoBar.cpp \
            lang/languagesupport.cpp \
            util/process.cpp \
            util/registry.cpp \
            util/string.cpp \
            util/win32.cpp \
            util/RetroStyleLabel.cpp \
            util/WidgetBackgroundImage.cpp \
            util/NonCopyable.cpp \
            util/PixmapMerging.cpp \
            util/MouseEventFilter.cpp \
            util/EventFilter.cpp \
            util/Widget.cpp \
            util/rsversion.cpp \
            gui/bwgraph/bwgraph.cpp \
            gui/chat/PopupChatDialog.cpp \
            gui/connect/ConnectDialog.cpp \
            gui/connect/ConfCertDialog.cpp \
            gui/connect/InviteDialog.cpp \
            gui/connect/AddFriendDialog.cpp \
            gui/connect/AddFriendWizard.cpp \
            gui/msgs/ChanMsgDialog.cpp \
            gui/msgs/ChanCreateDialog.cpp \
            gui/filehash/FileHashDialog.cpp \
            gui/moreinfo/moreinfo.cpp \ 
            gui/Preferences/configpagestack.cpp \
            gui/Preferences/CryptographyDialog.cpp \
            gui/Preferences/DirectoriesDialog.cpp \
            gui/Preferences/LogDialog.cpp \
            gui/Preferences/PreferencesDialog.cpp \
            gui/Preferences/PreferencesWindow.cpp \
            gui/Preferences/ServerDialog.cpp \
            gui/Preferences/ConfirmQuitDialog.cpp \
            gui/common/vmessagebox.cpp \
            gui/common/rwindow.cpp \         
            gui/Settings/gsettingswin.cpp \
            gui/Settings/GeneralPage.cpp \
            gui/Settings/DirectoriesPage.cpp \
            gui/Settings/ServerPage.cpp \
            gui/Settings/NetworkPage.cpp \    
            gui/toaster/ChatToaster.cpp \
            gui/toaster/MessageToaster.cpp \
            gui/toaster/CallToaster.cpp \
            gui/toaster/QtToaster.cpp \
            gui/authdlg/AuthorizationDialog.cpp \
            gui/advsearch/advancedsearchdialog.cpp \
            gui/advsearch/expressionwidget.cpp \
            gui/advsearch/guiexprelement.cpp
            
RESOURCES += gui/images.qrc lang/lang.qrc
TRANSLATIONS +=  \
            lang/retroshare_en.ts \
            lang/retroshare_de.ts \
            lang/retroshare_bg.ts \
            lang/retroshare_es.ts \
            lang/retroshare_fi.ts \
            lang/retroshare_fr.ts \
            lang/retroshare_af.ts  \
            lang/retroshare_cn_simp.ts  \
            lang/retroshare_cn_trad.ts  \
            lang/retroshare_gr.ts  \
            lang/retroshare_it.ts  \
            lang/retroshare_jp.ts  \
            lang/retroshare_kr.ts  \
            lang/retroshare_pl.ts  \
            lang/retroshare_pt.ts  \
            lang/retroshare_ru.ts  \
            lang/retroshare_tr.ts \
            lang/retroshare_dk.ts \
            lang/retroshare_sl.ts 

#LIBS += -L gui/lib/ -lskin
#win32:LIBS += -lgdi32

!macx {
    # On non-Mac, make the binary all lowercase
    TARGET = RetroShare
}

win32 {

    RC_FILE = gui/images/retroshare_win.rc

    "LIBS += -L"../../winlibs" -lretroshare -lssl -lcrypto -lpthreadGC2d -lKadC -lminiupnpc -lz -lws2_32 -luuid -lole32 -liphlpapi -lcrypt32-cygwin -lskin -lgdi32
    CONFIG += qt release"

}

!win32 {
    LIBS += -L../../../../../lib -lretroshare -lKadC -lminiupnpc -lssl -lcrypto 
}