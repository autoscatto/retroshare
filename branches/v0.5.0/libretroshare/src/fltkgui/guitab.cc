// generated by Fast Light User Interface Designer (fluid) version 1.0107

#include "guitab.h"

Fl_Double_Window* UserInterface::make_windows() {
  Fl_Double_Window* w;
  { Fl_Double_Window* o = main_win = new Fl_Double_Window(710, 535, "RetroShare");
    w = o;
    o->box(FL_DOWN_BOX);
    o->user_data((void*)(this));
    { Fl_Tabs* o = gui_tabs = new Fl_Tabs(10, 10, 690, 470);
      o->box(FL_UP_BOX);
      { Fl_Group* o = neighbours_tab = new Fl_Group(15, 40, 680, 435, "Connect");
        o->labelfont(1);
        { Fl_Button* o = new Fl_Button(50, 439, 280, 31, "Load Certificate from File");
          o->callback((Fl_Callback*)file_import);
        }
        { Fl_Funky_Browser* o = cert_neighbour_list = new Fl_Funky_Browser(25, 70, 350, 355, "", 4);
          o->box(FL_NO_BOX);
          o->color(FL_BACKGROUND2_COLOR);
          o->selection_color(FL_SELECTION_COLOR);
          o->labeltype(FL_NORMAL_LABEL);
          o->labelfont(0);
          o->labelsize(14);
          o->labelcolor(FL_FOREGROUND_COLOR);
          o->textfont(4);
          o->callback((Fl_Callback*)cert_neighbour_list_select);
          o->align(FL_ALIGN_BOTTOM);
          o->when(FL_WHEN_RELEASE_ALWAYS);
        }
        { Fl_Button* o = neigh_add_button = new Fl_Button(420, 440, 240, 30, "Sign + Add  to Friends  >>>");
          o->callback((Fl_Callback*)cert_move_to_friends);
        }
        { Fl_Box* o = new Fl_Box(35, 49, 290, 20, "Your Neighbour List");
          o->box(FL_FLAT_BOX);
          o->labelfont(1);
        }
        { Fl_Input* o = neigh_authcode = new Fl_Input(540, 397, 70, 30, "AUTH CODE:");
          o->labelfont(1);
          o->callback((Fl_Callback*)neigh_auth_callback);
          o->when(FL_WHEN_CHANGED);
        }
        { Fl_Group* o = new Fl_Group(390, 54, 295, 305);
          o->box(FL_DOWN_BOX);
          { Fl_Box* o = new Fl_Box(395, 65, 125, 20, "Peer Details");
            o->box(FL_FLAT_BOX);
            o->labelfont(1);
            o->labelsize(16);
          }
          { Fl_Browser* o = neigh_signers = new Fl_Browser(440, 195, 225, 140, "Certificate signers");
            o->type(2);
            o->textfont(4);
            o->callback((Fl_Callback*)cert_neigh_signers_select);
            Fl_Group::current()->resizable(o);
          }
          neigh_name = new Fl_Output(450, 90, 226, 20, "Name:");
          neigh_org = new Fl_Output(450, 115, 226, 20, "Org:");
          neigh_loc = new Fl_Output(450, 140, 225, 20, "Loc:");
          neigh_country = new Fl_Output(495, 165, 168, 20, "Country:");
          neigh_trust = new Fl_Output(565, 65, 110, 20, "Trust:");
          o->end();
        }
        { Fl_Box* o = neigh_auth_notice = new Fl_Box(405, 369, 270, 26, "AUTH CODE REQUIRED");
          o->box(FL_FLAT_BOX);
          o->labelfont(1);
          o->labelsize(18);
          o->labelcolor(FL_YELLOW);
        }
        o->end();
      }
      { Fl_Group* o = connect_tab = new Fl_Group(15, 40, 680, 435, "Friends");
        o->labelfont(1);
        o->hide();
        { Fl_Button* o = new Fl_Button(62, 440, 185, 25, "<<< Remove Friend");
          o->callback((Fl_Callback*)cert_remove_cert);
        }
        { Fl_Button* o = new Fl_Button(264, 440, 185, 25, "Configure Friend");
          o->callback((Fl_Callback*)cert_show_config);
        }
        { Fl_Funky_Browser* o = cert_list = new Fl_Funky_Browser(25, 70, 660, 360, "", 5);
          o->type(2);
          o->box(FL_NO_BOX);
          o->color(FL_BACKGROUND2_COLOR);
          o->selection_color(FL_SELECTION_COLOR);
          o->labeltype(FL_NORMAL_LABEL);
          o->labelfont(0);
          o->labelsize(14);
          o->labelcolor(FL_FOREGROUND_COLOR);
          o->textfont(4);
          o->callback((Fl_Callback*)cert_list_select);
          o->align(FL_ALIGN_BOTTOM);
          o->when(FL_WHEN_RELEASE_ALWAYS);
        }
        { Fl_Button* o = new Fl_Button(466, 440, 185, 25, "Export Friend");
          o->callback((Fl_Callback*)file_export);
        }
        { Fl_Box* o = new Fl_Box(190, 50, 340, 20, "Your Friends ");
          o->box(FL_FLAT_BOX);
          o->labelfont(1);
        }
        o->end();
      }
      { Fl_Group* o = new Fl_Group(15, 40, 680, 435, "File Listing");
        o->labelfont(1);
        o->hide();
        { Fl_Widget *o = file_results = make_tree_browser(25, 50, 660, 380);
	}
//        { Fl_File_Browser* o = file_results = new Fl_File_Browser(45, 50, 625, 380);
//          o->callback((Fl_Callback*)file_result_select);
//        }
        { Fl_Button* o = file_download_button = new Fl_Button(65, 440, 185, 25, "Download");
          o->callback((Fl_Callback*)file_download);
        }
        { Fl_Button* o = file_recommend_button = new Fl_Button(465, 440, 185, 25, "Recommend to Friends");
          o->callback((Fl_Callback*)file_recommend);
          o->deactivate();
        }
        { Fl_Button* o = file_channel_button = new Fl_Button(265, 440, 185, 25, "Broadcast on Channel");
          o->callback((Fl_Callback*)file_channel_broadcast);
          o->deactivate();
        }
        o->end();
      }
      { Fl_Group* o = new Fl_Group(15, 40, 680, 435, "Search ");
        o->labelfont(1);
        o->hide();
        { Fl_Input* o = new_search = new Fl_Input(105, 50, 225, 25, "Terms:");
          o->labelfont(1);
          o->callback((Fl_Callback*)do_new_search);
        }
        { Fl_Button* o = new Fl_Button(535, 50, 125, 25, "Remove Search");
          o->callback((Fl_Callback*)search_remove);
        }
        { Fl_Button* o = recommend_button = new Fl_Button(460, 440, 185, 25, "Recommend to Friends");
          o->callback((Fl_Callback*)search_recommend);
          o->deactivate();
        }
        { Fl_Funky_Browser* o = srch_results = new Fl_Funky_Browser(25, 80, 660, 350, "", 5);
          o->type(2);
          o->box(FL_NO_BOX);
          o->color(FL_BACKGROUND2_COLOR);
          o->selection_color(FL_SELECTION_COLOR);
          o->labeltype(FL_NORMAL_LABEL);
          o->labelfont(0);
          o->labelsize(14);
          o->labelcolor(FL_FOREGROUND_COLOR);
          o->callback((Fl_Callback*)search_result_select);
          o->align(FL_ALIGN_BOTTOM);
          o->when(FL_WHEN_RELEASE_ALWAYS);
        }
        { Fl_Button* o = search_button = new Fl_Button(340, 50, 79, 25, "Search");
          o->labelfont(1);
          o->callback((Fl_Callback*)do_search_button);
        }
        { Fl_Button* o = download_button = new Fl_Button(60, 440, 185, 25, "Download");
          o->callback((Fl_Callback*)search_download);
        }
        { Fl_Button* o = search_channel_button = new Fl_Button(260, 440, 185, 25, "Broadcast on Channel");
          o->callback((Fl_Callback*)search_channel_broadcast);
          o->deactivate();
        }
        o->end();
      }
      { Fl_Group* o = new Fl_Group(15, 40, 680, 435, "File Transfer");
        o->hide();
        { Fl_Funky_Browser* o = transfer_downloads = new Fl_Funky_Browser(25, 50, 660, 250, "", 5);
          o->type(2);
          o->box(FL_NO_BOX);
          o->color(FL_BACKGROUND2_COLOR);
          o->selection_color(FL_SELECTION_COLOR);
          o->labeltype(FL_NORMAL_LABEL);
          o->labelfont(0);
          o->labelsize(14);
          o->labelcolor(FL_FOREGROUND_COLOR);
          o->callback((Fl_Callback*)transfer_select);
          o->align(FL_ALIGN_BOTTOM);
          o->when(FL_WHEN_RELEASE_ALWAYS);
        }
        { Fl_Button* o = transfer_cancel = new Fl_Button(70, 310, 165, 25, "Cancel");
          o->callback((Fl_Callback*)file_transfer_cancel);
        }
        { Fl_Button* o = transfer_clear = new Fl_Button(275, 310, 155, 25, "Clear Finished");
          o->callback((Fl_Callback*)file_transfer_clear);
        }
        { Fl_Text_Display* o = transfer_overview = new Fl_Text_Display(25, 345, 450, 120);
          o->textfont(5);
        }
        { Fl_Counter* o = rate_total = new Fl_Counter(510, 380, 135, 25, "Max Total Data Rate (kB/s)");
          o->step(0.1);
          o->callback((Fl_Callback*)file_transfer_total_rate);
        }
        { Fl_Counter* o = rate_indiv = new Fl_Counter(510, 425, 135, 25, "Rate Per Person (kB/s)");
          o->step(0.1);
          o->callback((Fl_Callback*)file_transfer_indiv_rate);
        }
        o->end();
      }
      { Fl_Group* o = new Fl_Group(15, 40, 680, 435, "Messages");
        o->labelfont(1);
        o->hide();
        o->deactivate();
        { Fl_Funky_Browser* o = msg_list = new Fl_Funky_Browser(25, 65, 660, 270, "", 5);
          o->type(2);
          o->box(FL_NO_BOX);
          o->color(FL_BACKGROUND2_COLOR);
          o->selection_color(FL_SELECTION_COLOR);
          o->labeltype(FL_NORMAL_LABEL);
          o->labelfont(0);
          o->labelsize(14);
          o->labelcolor(FL_FOREGROUND_COLOR);
          o->callback((Fl_Callback*)msg_select);
          o->align(FL_ALIGN_BOTTOM);
          o->when(FL_WHEN_RELEASE_ALWAYS);
        }
        { Fl_Button* o = new Fl_Button(45, 350, 125, 25, "New Msg");
          o->callback((Fl_Callback*)msg_dialog_show);
        }
        { Fl_Button* o = new Fl_Button(45, 380, 125, 25, "Reply to Msg");
          o->callback((Fl_Callback*)msg_dialog_reply);
        }
        { Fl_Button* o = new Fl_Button(245, 450, 380, 25, "(2) -- Get The Recommended File");
          o->labelfont(1);
          o->callback((Fl_Callback*)msg_get_recommendation);
        }
        { Fl_Button* o = new Fl_Button(45, 410, 125, 25, "Remove Msg");
          o->callback((Fl_Callback*)msg_remove);
        }
        { Fl_Text_Display* o = msg_details = new Fl_Text_Display(180, 340, 485, 105);
          o->textfont(5);
        }
        { Fl_Box* o = new Fl_Box(55, 45, 305, 20, "(1) -- Select Message");
          o->box(FL_FLAT_BOX);
          o->labelfont(1);
        }
        o->end();
      }
      { Fl_Group* o = channels = new Fl_Group(15, 40, 680, 435, "Channels");
        o->hide();
        o->deactivate();
        { Fl_Funky_Browser* o = channel_list = new Fl_Funky_Browser(25, 80, 660, 225, "", 5);
          o->type(2);
          o->box(FL_NO_BOX);
          o->color(FL_BACKGROUND2_COLOR);
          o->selection_color(FL_SELECTION_COLOR);
          o->labeltype(FL_NORMAL_LABEL);
          o->labelfont(0);
          o->labelsize(14);
          o->labelcolor(FL_FOREGROUND_COLOR);
          o->callback((Fl_Callback*)channel_list_select);
          o->align(FL_ALIGN_BOTTOM);
          o->when(FL_WHEN_RELEASE_ALWAYS);
        }
        { Fl_Box* o = new Fl_Box(40, 61, 170, 19, "Available Channels");
          o->labelfont(1);
        }
        { Fl_Group* o = new Fl_Group(25, 310, 655, 160);
          o->box(FL_DOWN_BOX);
          { Fl_Funky_Browser* o = channel_file_list = new Fl_Funky_Browser(35, 345, 635, 120, "", 5);
            o->box(FL_NO_BOX);
            o->color(FL_BACKGROUND2_COLOR);
            o->selection_color(FL_SELECTION_COLOR);
            o->labeltype(FL_NORMAL_LABEL);
            o->labelfont(0);
            o->labelsize(14);
            o->labelcolor(FL_FOREGROUND_COLOR);
            o->callback((Fl_Callback*)channel_file_list_select);
            o->align(FL_ALIGN_BOTTOM);
            o->when(FL_WHEN_RELEASE_ALWAYS);
          }
          { Fl_Output* o = channel_selected_name = new Fl_Output(185, 315, 270, 25, "Msgs on Channel");
            o->labelfont(1);
          }
          { Fl_Button* o = channel_show_button = new Fl_Button(485, 315, 160, 25, "Show Msg Details");
            o->callback((Fl_Callback*)channel_show_callback);
          }
          o->end();
        }
        { Fl_Button* o = channel_delete_button = new Fl_Button(290, 50, 160, 25, "Delete Channel");
          o->callback((Fl_Callback*)channel_delete_callback);
        }
        { Fl_Button* o = channel_create_button = new Fl_Button(475, 50, 160, 25, "Create Channel Msg");
          o->callback((Fl_Callback*)channel_create);
        }
        o->end();
      }
      { Fl_Group* o = new Fl_Group(15, 40, 680, 435, "Config");
        o->hide();
        { Fl_Group* o = new Fl_Group(460, 55, 210, 80);
          o->box(FL_DOWN_BOX);
          o->hide();
          { Fl_Check_Button* o = config_local_disc = new Fl_Check_Button(490, 80, 145, 25, "local discovery");
            o->down_box(FL_DOWN_BOX);
          }
          { Fl_Check_Button* o = config_remote_disc = new Fl_Check_Button(490, 100, 145, 25, "remote discovery");
            o->down_box(FL_DOWN_BOX);
          }
          { Fl_Box* o = new Fl_Box(470, 60, 165, 20, "Discovery Options:");
            o->box(FL_FLAT_BOX);
            o->labelfont(1);
          }
          o->end();
        }
        { Fl_Group* o = new Fl_Group(25, 225, 405, 240);
          o->box(FL_DOWN_BOX);
          { Fl_Browser* o = config_search_dir = new Fl_Browser(40, 245, 355, 105);
            o->type(2);
          }
          { Fl_Button* o = new Fl_Button(40, 370, 135, 25, "Add Directory");
            o->callback((Fl_Callback*)config_add_dir);
          }
          { Fl_Button* o = new Fl_Button(40, 400, 135, 25, "Remove Directory");
            o->callback((Fl_Callback*)config_remove_dir);
          }
          config_save_dir = new Fl_Output(145, 435, 250, 20, "Save Directory");
          { Fl_Button* o = new Fl_Button(230, 405, 165, 25, "Select save directory");
            o->callback((Fl_Callback*)config_save_dir_change);
          }
          { Fl_Box* o = new Fl_Box(41, 230, 174, 15, "Share Directories:");
            o->box(FL_FLAT_BOX);
            o->labelfont(1);
          }
          o->end();
        }
        { Fl_Group* o = new Fl_Group(25, 55, 405, 160);
          o->box(FL_DOWN_BOX);
          config_local_addr = new Fl_Input(165, 80, 130, 20, "Local Address:");
          config_local_port = new Fl_Value_Input(335, 80, 60, 20, "Port:");
          config_server_addr = new Fl_Input(165, 145, 130, 25, "External  Address:");
          config_server_port = new Fl_Value_Input(335, 145, 60, 25, "Port:");
          { Fl_Check_Button* o = config_firewall = new Fl_Check_Button(195, 100, 190, 25, "behind firewall");
            o->down_box(FL_DOWN_BOX);
            o->callback((Fl_Callback*)config_server_update);
          }
          { Fl_Check_Button* o = config_forward = new Fl_Check_Button(195, 120, 190, 25, "forwarded external port");
            o->down_box(FL_DOWN_BOX);
            o->callback((Fl_Callback*)config_server_update);
          }
          { Fl_Box* o = new Fl_Box(45, 60, 150, 20, "Server Settings:");
            o->box(FL_FLAT_BOX);
            o->labelfont(1);
          }
          { Fl_Button* o = config_server_button = new Fl_Button(185, 180, 190, 25, "Change + Restart Server");
            o->callback((Fl_Callback*)config_server_change);
          }
          o->end();
        }
        { Fl_Button* o = new Fl_Button(485, 434, 170, 25, "Save Configuration");
          o->callback((Fl_Callback*)cert_save_config);
        }
        o->end();
      }
      { Fl_Group* o = about_help_tab = new Fl_Group(15, 40, 680, 435, "About");
        o->hide();
        help_view = new Fl_Help_View(25, 50, 660, 415);
        o->end();
      }
      o->end();
    }
    onlinecounter = new Fl_Value_Output(100, 485, 35, 20, "#Online:");
    new Fl_Value_Output(100, 505, 35, 20, "New Msgs:");
    { Fl_Button* o = new Fl_Button(605, 490, 85, 30, "Hide");
      o->callback((Fl_Callback*)gui_quit);
    }
    new Fl_Text_Display(140, 485, 360, 40);
    { Fl_Button* o = chat_button = new Fl_Button(510, 490, 85, 30, "Chat");
      o->callback((Fl_Callback*)chat_open_callback);
    }
    o->end();
  }
  { Fl_Double_Window* o = msg_dialog = new Fl_Double_Window(580, 355, "Msg Dialog");
    w = o;
    o->user_data((void*)(this));
    msg_online = new Fl_Check_Browser(20, 25, 140, 215);
    { Fl_Button* o = msg_button_select = new Fl_Button(20, 240, 140, 25, "Select All/None");
      o->callback((Fl_Callback*)msg_toggle_select);
    }
    { Fl_Text_Editor* o = msg_text = new Fl_Text_Editor(175, 25, 385, 240, "(1) -- Enter Message Text:");
      o->labelfont(1);
    }
    msg_recommend = new Fl_Output(175, 275, 385, 30, "Recommendation:");
    { Fl_Button* o = new Fl_Button(245, 315, 160, 30, "(2) -- Send Msg");
      o->labelfont(1);
      o->callback((Fl_Callback*)msg_send);
    }
    { Fl_Button* o = new Fl_Button(415, 315, 115, 30, "Cancel");
      o->callback((Fl_Callback*)msg_dialog_hide);
    }
    o->end();
  }
  { Fl_Double_Window* o = cert_config = new Fl_Double_Window(430, 535, "Certificate Configuration");
    w = o;
    o->user_data((void*)(this));
    { Fl_Group* o = new Fl_Group(10, 10, 410, 490);
      o->box(FL_UP_BOX);
      { Fl_Group* o = new Fl_Group(25, 355, 380, 135);
        o->box(FL_DOWN_BOX);
        cert_server = new Fl_Input(155, 430, 130, 25, "Server Address:");
        { Fl_Button* o = new Fl_Button(165, 460, 220, 25, "Save and attempt to Connect");
          o->callback((Fl_Callback*)cert_save_n_connect);
        }
        { Fl_Button* o = new Fl_Button(35, 460, 115, 25, "Save Address");
          o->callback((Fl_Callback*)cert_save_servaddr);
        }
        cert_port = new Fl_Value_Input(325, 430, 60, 25, "Port:");
        { Fl_Check_Button* o = cert_connect = new Fl_Check_Button(30, 410, 365, 20, "Outgoing Connections (Server Address Required)");
          o->down_box(FL_DOWN_BOX);
          o->callback((Fl_Callback*)cert_connect_change);
        }
        { Fl_Check_Button* o = cert_allow = new Fl_Check_Button(30, 380, 115, 20, "Allow Access");
          o->down_box(FL_DOWN_BOX);
          o->callback((Fl_Callback*)cert_allow_change);
        }
        { Fl_Check_Button* o = cert_listen = new Fl_Check_Button(30, 395, 170, 20, "Listen for connection");
          o->down_box(FL_DOWN_BOX);
          o->callback((Fl_Callback*)cert_listen_change);
        }
        { Fl_Check_Button* o = cert_local = new Fl_Check_Button(220, 385, 130, 20, "Local Network");
          o->down_box(FL_DOWN_BOX);
          o->callback((Fl_Callback*)cert_local_change);
        }
        { Fl_Check_Button* o = cert_auto = new Fl_Check_Button(220, 362, 140, 20, "Auto Connect");
          o->down_box(FL_DOWN_BOX);
          o->labelfont(1);
          o->callback((Fl_Callback*)cert_auto_change);
        }
        { Fl_Box* o = new Fl_Box(32, 363, 190, 20, "Connectivity Options:");
          o->box(FL_FLAT_BOX);
          o->labelfont(1);
        }
        o->end();
      }
      cert_status = new Fl_Output(80, 20, 130, 25, "Status:");
      cert_details = new Fl_Text_Display(25, 50, 380, 210);
      { Fl_Group* o = new Fl_Group(25, 267, 380, 82);
        o->box(FL_DOWN_BOX);
        { Fl_Button* o = cert_sign_button = new Fl_Button(239, 295, 145, 29, "Sign Certificate");
          o->callback((Fl_Callback*)cert_sign);
        }
        { Fl_Check_Button* o = cert_trust_person = new Fl_Check_Button(95, 323, 230, 25, "Trust This Person\'s Signature");
          o->down_box(FL_DOWN_BOX);
          o->callback((Fl_Callback*)cert_trust_person_change);
        }
        { Fl_Input* o = cert_authcode = new Fl_Input(145, 295, 80, 28, "AUTH CODE:");
          o->labelfont(1);
          o->callback((Fl_Callback*)cert_auth_callback);
          o->when(FL_WHEN_CHANGED);
        }
        { Fl_Box* o = new Fl_Box(35, 272, 360, 20, "Authenticate Friend By Entering Their Code");
          o->box(FL_FLAT_BOX);
          o->labelfont(1);
        }
        o->end();
      }
      cert_trust = new Fl_Output(260, 20, 140, 25, "Trust:");
      o->end();
    }
    { Fl_Button* o = new Fl_Button(305, 505, 100, 25, "Done");
      o->callback((Fl_Callback*)cert_hide_config);
    }
    o->end();
  }
  { Fl_Double_Window* o = new Fl_Double_Window(360, 290);
    w = o;
    o->user_data((void*)(this));
    { //Fl_File_Chooser* o = file_chooser = new Fl_File_Chooser(5, 5, 350, 280);
	Fl_File_Chooser* o = file_chooser = new Fl_File_Chooser("/",
		"Certificate Files (*.{pem,pqi})", 0, "Select File/Dir");

//      o->box(FL_NO_BOX);
//      o->color(FL_BACKGROUND_COLOR);
//      o->selection_color(FL_BACKGROUND_COLOR);
//      o->labeltype(FL_NORMAL_LABEL);
//      o->labelfont(0);
//      o->labelsize(14);
//      o->labelcolor(FL_FOREGROUND_COLOR);
//      o->callback((Fl_Callback*)file_chooser_select);
      o->callback(file_chooser_select);
//      o->align(FL_ALIGN_TOP);
//      o->when(FL_WHEN_RELEASE);
//      o->end();
    }
    o->end();
  }
  { Fl_Double_Window* o = welcome_window = new Fl_Double_Window(405, 580, "RetroShare Setup");
    w = o;
    o->user_data((void*)(this));
    { Fl_Group* o = new Fl_Group(10, 10, 385, 35);
      o->box(FL_DOWN_BOX);
      { Fl_Box* o = new Fl_Box(85, 15, 230, 20, "Welcome to RetroShare");
        o->box(FL_FLAT_BOX);
        o->labelfont(1);
        o->labelsize(16);
      }
      o->end();
    }
    { Fl_Group* o = new Fl_Group(10, 55, 385, 140);
      o->box(FL_DOWN_BOX);
      { Fl_Box* o = new Fl_Box(75, 65, 285, 20, "Please login ....");
        o->box(FL_FLAT_BOX);
        o->labelfont(1);
      }
      load_name = new Fl_Output(90, 90, 250, 25, "Name:");
      { Fl_Input* o = load_passwd = new Fl_Input(90, 125, 250, 25, "Password");
        o->type(5);
        o->callback((Fl_Callback*)load_passwd_callback);
        o->when(FL_WHEN_ENTER_KEY);
      }
      { Fl_Button* o = load_button = new Fl_Button(110, 160, 210, 30, "Load Existing User");
        o->callback((Fl_Callback*)load_existing);
      }
      o->end();
    }
    { Fl_Group* o = new Fl_Group(10, 205, 385, 365);
      o->box(FL_DOWN_BOX);
      { Fl_Box* o = new Fl_Box(55, 210, 300, 20, "Or create a New User...");
        o->box(FL_FLAT_BOX);
        o->labelfont(1);
      }
      { Fl_Group* o = new Fl_Group(50, 360, 300, 5);
        o->box(FL_UP_BOX);
        o->end();
      }
      { Fl_Group* o = new Fl_Group(50, 440, 300, 5);
        o->box(FL_UP_BOX);
        o->end();
      }
      { Fl_Group* o = new Fl_Group(50, 511, 300, 5);
        o->box(FL_UP_BOX);
        o->end();
      }
      gen_name = new Fl_Input(120, 235, 250, 25, "Name:");
      gen_org = new Fl_Input(120, 265, 250, 25, "Organisation:");
      gen_loc = new Fl_Input(120, 295, 250, 25, "Location:");
      gen_country = new Fl_Input(120, 325, 250, 25, "Country:");
      { Fl_Input* o = gen_passwd = new Fl_Input(170, 375, 130, 25, "New Password");
        o->type(5);
      }
      { Fl_Input* o = gen_passwd2 = new Fl_Input(170, 405, 130, 25, "Password (Again)");
        o->type(5);
      }
      { Fl_Check_Button* o = gen_trusted_tick_box = new Fl_Check_Button(50, 450, 305, 20, "Load Trusted Certificate (Optional)");
        o->down_box(FL_DOWN_BOX);
        o->labelfont(1);
        o->callback((Fl_Callback*)gen_trusted_tick_callback);
      }
      gen_trusted_peer = new Fl_Output(105, 474, 135, 25, "Friend:");
      { Fl_Button* o = gen_trusted_select_button = new Fl_Button(252, 470, 93, 30, "Select File");
        o->callback((Fl_Callback*)gen_load_trusted);
      }
      { Fl_Button* o = gen_button = new Fl_Button(100, 528, 210, 30, "Generate New Certificate");
        o->callback((Fl_Callback*)generate_certificate);
      }
      o->end();
    }
    o->end();
  }
  { Fl_Double_Window* o = chatter_window = new Fl_Double_Window(455, 435, "ChatterBox");
    w = o;
    o->user_data((void*)(this));
    chatter_box = new Fl_Text_Display(5, 10, 445, 370);
    { Fl_Input* o = chatter_input = new Fl_Input(5, 390, 445, 40);
      o->callback((Fl_Callback*)chatterbox_message);
      o->when(FL_WHEN_ENTER_KEY);
    }
    o->end();
  }
  { Fl_Double_Window* o = alert_window = new Fl_Double_Window(540, 200, "Alerts");
    w = o;
    o->user_data((void*)(this));
    { Fl_Return_Button* o = alert_okay = new Fl_Return_Button(75, 175, 180, 20, "OK");
      o->callback((Fl_Callback*)alert_okay_msg);
    }
    alert_box = new Fl_Text_Display(15, 5, 515, 165);
    { Fl_Button* o = alert_cancel = new Fl_Button(270, 175, 180, 20, "Cancel");
      o->labelfont(3);
      o->labelsize(16);
      o->labelcolor((Fl_Color)80);
      o->callback((Fl_Callback*)alert_cancel_msg);
    }
    o->end();
  }
  { Fl_Double_Window* o = channel_create_window = new Fl_Double_Window(460, 535, "Create New Channel Msg");
    w = o;
    o->user_data((void*)(this));
    { Fl_Funky_Browser* o = chan_createmsg_filelist = new Fl_Funky_Browser(10, 195, 440, 265, "", 2);
      o->type(2);
      o->box(FL_NO_BOX);
      o->color(FL_BACKGROUND2_COLOR);
      o->selection_color(FL_SELECTION_COLOR);
      o->labeltype(FL_NORMAL_LABEL);
      o->labelfont(0);
      o->labelsize(14);
      o->labelcolor(FL_FOREGROUND_COLOR);
      o->callback((Fl_Callback*)chan_createmsg_list_select);
      o->align(FL_ALIGN_BOTTOM);
      o->when(FL_WHEN_RELEASE_ALWAYS);
    }
    { Fl_Button* o = chan_createmsg_sendmsg_button = new Fl_Button(25, 500, 125, 25, "Send Msg");
      o->callback((Fl_Callback*)chan_createmsg_sendmsg_callback);
    }
    { Fl_Button* o = chan_createmsg_postpone_button = new Fl_Button(165, 500, 125, 25, "Postpone Msg");
      o->callback((Fl_Callback*)chan_createmsg_postpone_callback);
    }
    { Fl_Button* o = chan_createmsg_cancel_button = new Fl_Button(305, 500, 125, 25, "Cancel Msg");
      o->callback((Fl_Callback*)chan_createmsg_cancel_callback);
    }
    chan_createmsg_msg = new Fl_Text_Editor(10, 110, 440, 75);
    { Fl_Box* o = new Fl_Box(10, 91, 135, 19, "Message Text:");
      o->labelfont(1);
    }
    { Fl_Button* o = chan_createmsg_remove_button = new Fl_Button(305, 470, 125, 25, "Remove File");
      o->callback((Fl_Callback*)chan_createmsg_remove_callback);
    }
    { Fl_Group* o = new Fl_Group(10, 5, 440, 80);
      o->box(FL_DOWN_BOX);
      { Fl_Input* o = chan_createmsg_newname = new Fl_Input(155, 50, 280, 25);
        o->callback((Fl_Callback*)chan_createmsg_newname_callback);
      }
      chan_createmsg_title = new Fl_Output(155, 15, 280, 25);
      { Fl_Round_Button* o = chan_createmsg_newname_button = new Fl_Round_Button(20, 55, 135, 20, "Create Channel");
        o->type(102);
        o->down_box(FL_ROUND_DOWN_BOX);
        o->callback((Fl_Callback*)chan_createmsg_newname_button_callback);
      }
      { Fl_Round_Button* o = chan_createmsg_title_button = new Fl_Round_Button(20, 15, 135, 20, "Active Channel");
        o->type(102);
        o->down_box(FL_ROUND_DOWN_BOX);
        o->callback((Fl_Callback*)chan_createmsg_title_button_callback);
      }
      { Fl_Box* o = new Fl_Box(45, 33, 70, 22, "OR");
        o->labelfont(3);
      }
      o->end();
    }
    o->end();
  }
  { Fl_Double_Window* o = channel_details_window = new Fl_Double_Window(460, 445, "Channel Msg Details");
    w = o;
    o->user_data((void*)(this));
    { Fl_Funky_Browser* o = chan_msgdetails_filelist = new Fl_Funky_Browser(10, 146, 440, 255, "", 2);
      o->type(2);
      o->box(FL_NO_BOX);
      o->color(FL_BACKGROUND2_COLOR);
      o->selection_color(FL_SELECTION_COLOR);
      o->labeltype(FL_NORMAL_LABEL);
      o->labelfont(0);
      o->labelsize(14);
      o->labelcolor(FL_FOREGROUND_COLOR);
      o->callback((Fl_Callback*)chan_msgdetails_list_select);
      o->align(FL_ALIGN_BOTTOM);
      o->when(FL_WHEN_RELEASE_ALWAYS);
    }
    { Fl_Button* o = chan_msgdetails_download_button = new Fl_Button(165, 410, 125, 25, "Download Files");
      o->callback((Fl_Callback*)chan_msgdetails_download_callback);
    }
    { Fl_Box* o = new Fl_Box(10, 42, 135, 19, "Message Text:");
      o->labelfont(1);
    }
    { Fl_Button* o = chan_msgdetails_subscribe_button = new Fl_Button(25, 410, 125, 25, "Subscribe ");
      o->callback((Fl_Callback*)chan_msgdetails_subscribe_callback);
    }
    { Fl_Output* o = chan_msgdetails_title = new Fl_Output(75, 10, 210, 25, "Channel");
      o->labelfont(1);
    }
    chan_msgdetails_msg = new Fl_Text_Display(10, 61, 440, 75);
    { Fl_Output* o = chan_msgdetails_date = new Fl_Output(330, 10, 120, 25, "Date");
      o->labelfont(1);
    }
    { Fl_Button* o = chan_msgdetails_close_button = new Fl_Button(305, 410, 125, 25, "Close WIndow");
      o->callback((Fl_Callback*)chan_msgdetails_close_callback);
    }
    o->end();
  }
  return w;
}
