#ifndef DUDE_H
#define DUDE_H

#include <sigc++/sigc++.h>
#include <glibmm.h>
#include <iostream>

using namespace std;

//enum memory_type { flash, eeprom, fuse };
enum error_codes {
                        no_error,
                        invalid_signature,
                        unknown_device,
                        cannot_read_signature,
                        command_not_found,
                        insufficient_permissions,
                        programmer_not_found
                 };

/* an array af strings that indicate an error has occured */
const vector<Glib::ustring> er_strings = {
                                                "error reading signature data",
                                                "Double check chip, or use -F to override this check.",
                                                "Valid parts are:",
                                                "command not found",
                                                "can't open device",
                                                "could not find",
                                                "did not find any USB device"
                                         };
/*
   an array af the correspsonding error-codes
   these two arrays should always have the same length
*/
const vector<error_codes> er_codes = {
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

                /* fuse-byte values as read from the device */
                guint dev_fusebytes[3] = {255, 255, 255};
                /* console output from last command execution */
                Glib::ustring raw_exec_output;
                /* the most interesting part of outpout -- according to the nature of executed command and the outcome */
                Glib::ustring processed_output;
                /* the code of the error that occured during last execution */
                error_codes exec_error;

                /* public function members */
                void setup (gboolean auto_erase,
                            gboolean auto_verify,
                            gboolean auto_check,
                            Glib::ustring programmer,
                            Glib::ustring microcontroller);
                void device_erase (void);
                void get_signature (void);
                void eeprom_write (Glib::ustring file);
                void eeprom_read (Glib::ustring file);
                void eeprom_verify (Glib::ustring file);
                void flash_write (Glib::ustring file);
                void flash_read (Glib::ustring file);
                void flash_verify (Glib::ustring file);
                void fuse_write (Glib::ustring data);
                void fuse_read (void);

                typedef sigc::signal<void> type_sig_exec_done;
                type_sig_exec_done signal_exec_done();

        protected:
                // execution has been completed!
                // this signal is meant to be connected to a function of another object
                // (the connection will be initialized by that other object)
                type_sig_exec_done sig_exec_done;

                // execution has been completed!
                // this is meant to be used internally -- to notify a function of this object
                // (dispatcher is like sigc<void>, but is suitable for inter-thread communication)
                Glib::Dispatcher local_sig_exec_done;

                Glib::ustring oneliner;
                Glib::ustring protocol;
                Glib::ustring options;
                Glib::ustring device;

                void execute (Glib::ustring command);
                void post_execution (void);
                void check_for_errors (void);
};

#endif
