#ifndef DUDE_H
#define DUDE_H

#include <sigc++/sigc++.h>
#include <iostream>
#include <thread>
#include <glibmm.h>

// enum memory_type { flash, eeprom, fuse };
enum exec_status {
        no_error,
        invalid_signature,
        unknown_device,
        cannot_read_signature,
        command_not_found,
        insufficient_permissions,
        programmer_not_found
};

// array af strings that indicate an error has occured
const std::vector<Glib::ustring>
error_strings = {
        "error reading signature data",
        "Double check chip, or use -F to override this check.",
        "Valid parts are:",
        "command not found",
        "can't open device",
        "could not find",
        "did not find any USB device"
};

// array af the correspsonding error-codes
// (these two arrays should always have the same length)
const std::vector<exec_status>
error_codes = {
        cannot_read_signature,
        invalid_signature,
        unknown_device,
        command_not_found,
        programmer_not_found,
        programmer_not_found,
        programmer_not_found
};



class Dude
{
        public:
                Dude();
                virtual ~Dude();

                // fuse-byte values as read from the device
                guint dev_fusebytes[3] = {255, 255, 255};
                // console output from command execution
                Glib::ustring raw_exec_output;
                // usefull part of output -- according to nature of executed command
                Glib::ustring processed_output;
                // error code that occured during last execution
                exec_status execution_status;

                void setup (gboolean auto_erase,
                            gboolean auto_verify,
                            gboolean auto_check,
                            Glib::ustring programmer,
                            Glib::ustring microcontroller);

                // basic avrdude operations
                void do_clear_device (void);
                void do_read_signature (void);
                void do_eeprom_write (Glib::ustring file);
                void do_eeprom_read (Glib::ustring file);
                void do_eeprom_verify (Glib::ustring file);
                void do_flash_write (Glib::ustring file);
                void do_flash_read (Glib::ustring file);
                void do_flash_verify (Glib::ustring file);
                void do_fuse_write (Glib::ustring data);
                void do_fuse_read (void);

                // signal for end-of-execution
                typedef sigc::signal<void> type_sig_exec_done;
                typedef sigc::signal<void> type_sig_exec_started;
                type_sig_exec_done signal_exec_done();
                type_sig_exec_started signal_exec_started();

        protected:
                std::thread* avrdude_thread;

                // execution start/stop
                // this signal is meant to be connected to a function of another object
                // (the connection will be initialized by that other object)
                type_sig_exec_done sig_exec_done;
                type_sig_exec_started sig_exec_started;

                // execution completion
                // this is meant to be used internally -- to notify a function of this object
                // (dispatcher is like sigc<void>, but is suitable for inter-thread communication)
                Glib::Dispatcher local_sig_exec_done;

                Glib::ustring oneliner;
                Glib::ustring protocol;
                Glib::ustring options;
                Glib::ustring device;
                Glib::ustring command;

                void execute (void);
                void execution_begin (void);
                void execution_end (void);
                void check_for_errors (void);
};

#endif
