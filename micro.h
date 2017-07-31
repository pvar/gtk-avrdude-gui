#ifndef MICRO_H
#define MICRO_H

#include <libxml++/libxml++.h>
#include <dirent.h>
#include <iostream>
#include <fstream>

class DeviceDescription {
        public:
                // description of a single fuse warning
                struct FuseWarning {
                        uint fbyte;
                        uint fmask;
                        uint fresult;
                        std::string warning;
                };

                // description of a single fuse setting
                struct FuseSetting {
                        gboolean single_option; // wether it's a boolean or an enumerator
                        std::string fname;      // setting name
                        std::string fdesc;      // setting description
                        std::string fenum;      // enumerator name or null
                        uint fmask;             // bit-mask
                        uint offset;            // fuse byte
                };

                // description of an one-of-many options in a single fuse setting
                struct OptionEntry {
                        std::string ename;      // entry name
                        uint value;             // entry value
                };


                DeviceDescription();
                virtual ~DeviceDescription();

                // fundamental MCU properties
                bool eeprom_exists;
                std::string max_speed;
                std::string sram_size;
                std::string flash_size;
                std::string eeprom_size;
                std::string signature;
                std::string xml_filename;
                std::string xml_version;
                // fuse settings
                std::list<FuseSetting> *fuse_settings;
                std::map <std::string, std::list<OptionEntry>*> *option_lists;
                int fusebytes_count;
                // default fuse values
                int fusebytes_default[3];
                // fuse warnings
                std::list<FuseWarning> *warnings;
};

class Micro
{
        public:
                Micro(std::string path);
                virtual ~Micro();

                void parse_data(std::string xml_file);
                void get_device_list ();

                // device description extracted from xml/atdf file
                DeviceDescription *description = nullptr;

                // user defined fuse settings
                int fusebytes_custom[3] = {255, 255, 255};

                // device to description-file mapping
                std::map <std::string, std::string> *device_map = nullptr;

        protected:
                // data
                std::string device_xml;
                std::string exec_path;

                // xml parsing
                int parse_specifications (xmlpp::Node *root_node);
                int parse_settings (xmlpp::Node *root_node);
                int parse_warnings (xmlpp::Node *root_node);
                int parse_default (xmlpp::Node *root_node);

                std::list<DeviceDescription::OptionEntry>* get_enum_list (xmlpp::Node* xml_node);
                std::list<DeviceDescription::FuseSetting>* get_fuse_list (xmlpp::Node* xml_node, uint offset);
                std::string get_signature_bytes (xmlpp::Node* signature_node);

                xmlpp::Node* get_child_with_attr (xmlpp::Node* starting_node, std::string att_name, std::string att_value);
                std::string get_txt_value (xmlpp::Node* starting_node);
                std::string get_att_value (xmlpp::Node* xml_node, std::string att_name);

                // utilities
                std::string float_to_string (float number);
                void print_fuse_warnings ();
                void print_fuse_defaults ();
                void print_fuse_settings ();
                void print_option_lists ();
                void save_xml_map (void);
                gboolean load_xml_map (void);
};

#endif
