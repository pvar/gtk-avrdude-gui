#ifndef MICRO_H
#define MICRO_H

#include <libxml++/libxml++.h>
#include <dirent.h>
#include <gtkmm.h>
#include <iostream>
#include <fstream>

// description of a single fuse warning
struct FuseWarning {
        guint fbyte;
        guint fmask;
        guint fresult;
        std::string warning;
};

// description of a single fuse setting
struct FuseSetting {
        gboolean single_option; // wether it's a boolean or an enumerator
        Glib::ustring fname;    // setting name
        Glib::ustring fdesc;    // setting description
        Glib::ustring fenum;    // enumerator name or null
        guint fmask;            // bit-mask
        guint offset;           // fuse byte
};

// description of an one of many options in a single fuse setting
struct OptionEntry {
        Glib::ustring ename;    // entry name
        guint value;            // entry value
};

// preliminary specifications of a micro
struct DeviceSpecifications {
        Glib::ustring max_speed;
        Glib::ustring sram_size;
        Glib::ustring flash_size;
        Glib::ustring eeprom_size;
        Glib::ustring signature;
        Glib::ustring xml_filename;
        Glib::ustring xml_version;
        gboolean eeprom_exists;
};

// full description of all fuse settings of a micro
class DeviceFuseSettings {
        public:
                DeviceFuseSettings();
                virtual ~DeviceFuseSettings();

                std::list<FuseSetting> *fuse_settings = nullptr;
                std::map <Glib::ustring, std::list<OptionEntry>*> *option_lists = nullptr;
                gint fusebytes_count;
};



class Micro
{
        public:
                Micro(Glib::ustring path);
                virtual ~Micro();

                void parse_data(Glib::ustring xml_file);
                void get_device_list ();

                // user defined fuse settings
                guint usr_fusebytes[3] = {255, 255, 255};
                // default fuse settings
                guint def_fusebytes[3] = {255, 255, 255};
                // device specifications extracted from XML description
                DeviceSpecifications *specifications = nullptr;
                // description of all fuse settings, extracted from XML
                DeviceFuseSettings *settings = nullptr;
                // list of fuse warnings, extracted from XML
                std::list<FuseWarning> *warnings = nullptr;
                // device to XML-file mapping
                std::map <Glib::ustring, Glib::ustring> *device_map = nullptr;

        protected:
                // data
                Glib::ustring device_xml;
                Glib::ustring exec_path;

                // xml parsing
                gint parse_specifications (xmlpp::Node *root_node);
                gint parse_settings (xmlpp::Node *root_node);
                gint parse_warnings (xmlpp::Node *root_node);
                gint parse_default (xmlpp::Node *root_node);

                std::list<OptionEntry>* get_enum_list (xmlpp::Node* xml_node);
                std::list<FuseSetting>* get_fuse_list (xmlpp::Node* xml_node, guint offset);
                Glib::ustring get_signature_bytes (xmlpp::Node* signature_node);

                xmlpp::Node* get_child_with_attr (xmlpp::Node* starting_node, Glib::ustring att_name, Glib::ustring att_value);
                Glib::ustring get_txt_value (xmlpp::Node* starting_node);
                Glib::ustring get_att_value (xmlpp::Node* xml_node, Glib::ustring att_name);

                // utilities
                Glib::ustring float_to_string (gfloat number);
                void print_fuse_warnings ();
                void print_fuse_defaults ();
                void print_fuse_settings ();
                void print_option_lists ();
                void save_xml_map (void);
                gboolean load_xml_map (void);
};

#endif
