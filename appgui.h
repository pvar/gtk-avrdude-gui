#ifndef PROGGUI_H
#define PROGGUI_H

#include <sigc++/sigc++.h>
#include <gtkmm.h>
#include <iostream>
#include <iomanip>
#include "devdesc.h"
#include "micro.h"
#include "dude.h"


enum file_op { open_f, save_f, open_e, save_e };
enum op_code { sig_check, dev_erase, eeprom_w, eeprom_r, eeprom_v, flash_w, flash_r, flash_v, fuse_w, fuse_r };

class CBRecordModel : public Gtk::TreeModel::ColumnRecord
{
        public:
                CBRecordModel();
                Gtk::TreeModelColumn<std::string> col_name;
                Gtk::TreeModelColumn<std::string> col_data;
};

struct FuseWidget {
        FuseWidget();
        Gtk::CheckButton *check;
        Gtk::ComboBox *combo;
        Gtk::Label *reg_label;
        Gtk::Label *combo_label;
        Glib::RefPtr<Gtk::ListStore> model;
        sigc::connection *callback;
        guint max_value;
        guint bitmask;
        guint bytenum;
};

class gtkGUI
{
        public:
                gtkGUI();
                virtual ~gtkGUI();

                Gtk::Window *main_window = nullptr;

                void data_prep (void);

        protected:
                // path to executable
                Glib::ustring exec_path;
                // list of widgets used for fuse-settings
                std::list<FuseWidget> *fuse_tab_widgets = nullptr;
                // object for retrieving microcontroller data
                Micro *microcontroller;
                // object for interfacing with avrdude
                Dude *avrdude;
                // record-model for the tree-models used by combo boxes
                CBRecordModel cbm_generic;
                // wether warnigns should be displayed or not
                gboolean display_warnings;

                // signal connection handlers
                sigc::connection dev_combo_signal;
                sigc::connection dev_combo_hwprog;
                sigc::connection check_button_erase;
                sigc::connection check_button_check;
                sigc::connection check_button_verify;

                // tree-models for combo boxes
                Glib::RefPtr<Gtk::ListStore> tm_family;
                Glib::RefPtr<Gtk::ListStore> tm_device;
                Glib::RefPtr<Gtk::ListStore> tm_port;
                Glib::RefPtr<Gtk::ListStore> tm_protocol;

                // text buffer for diaplsying avrdude output
                Glib::RefPtr<Gtk::TextBuffer> dude_output_buffer;

                // widgets
                Gtk::CheckButton *auto_erase   = nullptr;
                Gtk::CheckButton *auto_verify  = nullptr;
                Gtk::CheckButton *auto_check   = nullptr;
                Gtk::ComboBox *cb_family       = nullptr;
                Gtk::ComboBox *cb_device       = nullptr;
                Gtk::ComboBox *cb_protocol     = nullptr;
                Gtk::Grid *fuse_grid           = nullptr;
                Gtk::Box *box_flash_ops        = nullptr;
                Gtk::Box *box_eeprom_ops       = nullptr;
                Gtk::Button *btn_fuse_read     = nullptr;
                Gtk::Button *btn_fuse_write    = nullptr;
                Gtk::Button *btn_fuse_def      = nullptr;
                Gtk::Button *btn_check_sig     = nullptr;
                Gtk::Button *btn_erase_dev     = nullptr;
                Gtk::Label *lbl_spec_flash     = nullptr;
                Gtk::Label *lbl_spec_sram      = nullptr;
                Gtk::Label *lbl_spec_eeprom    = nullptr;
                Gtk::Label *lbl_spec_speed     = nullptr;
                Gtk::Label *lbl_spec_xml       = nullptr;
                Gtk::Label *lbl_signature      = nullptr;
                Gtk::Label *lbl_fusebytes      = nullptr;
                Gtk::Label *lbl_dude_command   = nullptr;
                Gtk::TextView *tv_dude_output  = nullptr;
                Gtk::Entry *ent_flash_file     = nullptr;
                Gtk::Entry *ent_eeprom_file    = nullptr;
                Gtk::FileChooserDialog *browser= nullptr;

                // signal handlers
                void cb_new_device (void);
                void cb_new_family (void);
                void cb_dude_settings (void);
                void cb_check_signature (void);
                void cb_erase_devive (void);
                void cb_eeprom_read (void);
                void cb_eeprom_write (void);
                void cb_eeprom_verify (void);
                void cb_flash_read (void);
                void cb_flash_write (void);
                void cb_flash_verify (void);
                void cb_fuse_write (void);
                void cb_fuse_read (void);
                void cb_fuse_default (void);
                void select_file (file_op action);
                void calculate_fuse_values (void);
                bool exit_application(GdkEventAny* any_event);

                // utilities
                bool data_prep_start (void);
                void populate_static_treemodels (void);
                void get_executable_path (void);
                void prep_fuse_meta (void);

                void controls_lock (void);
                void controls_unlock (void);

                void device_data_clear (void);
                void device_data_show (void);

                void update_console_view (void);
                void message_popup (Glib::ustring title, Glib::ustring message);

                void display_specifiactions (gboolean have_specs);
                void display_fuse_settings (gboolean have_fuses);
                void display_fuse_warnings (void);
                void display_fuse_values (void);
                void clear_fuse_widget (FuseWidget* settings_widget);
                void process_fuse_values (void);

                void execution_done (void);
                void execution_started (void);
                void execution_outcome (gboolean show_success_message);
};

#endif
