#include "parser_v1.h"

using namespace std;

bool Parser_v1::is_description (string filename, std::string &device_name)
{
        bool desc_file = false;

        xmlpp::DomParser parser;
        if (Parser::status::success == get_content (filename, parser))
                desc_file = true;
        else
                desc_file = false;

        if (desc_file) {
                xmlpp::Node *xml_node = parser.get_document()->get_root_node();
                xml_node = xml_node->get_first_child("ADMIN");
                xml_node = xml_node->get_first_child("PART_NAME");
                device_name = this->get_txt_value(xml_node);
        } else
                device_name = "NONE";

        return desc_file;
}


bool Parser_v1::is_valid (xmlpp::Node *root_node)
{
        // check if a valid description file
        const string node_name = root_node->get_name();
        if (node_name.compare("AVRPART") != 0) {
                return false;
        } else
                return true;
}

Parser::status Parser_v1::get_settings (xmlpp::Node *root_node, DeviceDescription &description)
{
        //cout << "PARSING FUSES..." << endl;

        xmlpp::Node* xml_node = root_node;
        xmlpp::Node* tmp_node = nullptr;

        // go to V2 node
        xml_node = xml_node->get_first_child("V2");
        // if no such node, it must be a device without fuses...
        if (!xml_node)
                return Parser::status::no_content;
        // go to templates node
        xml_node = xml_node->get_first_child("templates");
        // if no such node, it must be a device without fuses...
        if (!xml_node)
                return Parser::status::no_content;
        // go to node with attribute: class = "FUSE"
        tmp_node = get_child_with_attr(xml_node,  "class", "FUSE");

        // if no such node, check if it's a device with NVM...
        if (!tmp_node) {
                // go to node with attribute: class = "NVM"
                tmp_node = get_child_with_attr(xml_node,  "class", "NVM");
                // if no such node, it must be a device without fuses...
                if (!tmp_node)
                        return Parser::status::no_content;
                // go to node with attribute: name = "NVM_FUSES"
                tmp_node = get_child_with_attr(tmp_node,  "name", "NVM_FUSES");
                // if no such node, there must be a problem...
                if (!tmp_node)
                        return Parser::status::doc_error;
                // use default node pointer
                xml_node = tmp_node;
        } else {
                // go to registers node
                tmp_node = tmp_node->get_first_child("registers");
                // use default node pointer
                xml_node = tmp_node;
        }

        int fuse_byte_counter = 0;
        // loop through children (reg nodes)
        xmlpp::Node::NodeList children = xml_node->get_children();
        xmlpp::Node::NodeList::iterator node_iterator;
        for (node_iterator = children.begin(); node_iterator != children.end(); ++node_iterator) {
                // get offset value for this reg node
                string regoffset = get_att_value((*node_iterator), "offset");
                // if no offset attribute was found, proceed to next iteration
                if (regoffset.size() < 1)
                        continue;
                // increase fuse-byte counter
                fuse_byte_counter++;
                uint offset = ::atoi((regoffset.substr(2, 2)).c_str());
                // get name for this reg node
                string regname = get_att_value((*node_iterator), "name");
                // prepare pseudo-setting with fuse-byte name
                DeviceDescription::FuseSetting *entry = new DeviceDescription::FuseSetting;
                entry->single_option = true;
                entry->fdesc = "FUSE REGISTER:  " + regname;
                entry->offset = 512; // special value that denotes a pseudo-entry
                // prepare list of settings from this node
                list<DeviceDescription::FuseSetting> *fuse_listing = new list<DeviceDescription::FuseSetting>;
                fuse_listing = get_fuse_list ((*node_iterator), offset);
                // add pseudo-setting in this list of settings
                fuse_listing->push_front(*entry);
                // add this settings list to the rest
                list<DeviceDescription::FuseSetting>::iterator pos = (description.fuse_settings)->begin();
                (description.fuse_settings)->splice(pos, *fuse_listing);
                delete entry;
        }
        //cout << "FUSE BYTES: " << fuse_byte_counter << endl;
        description.fusebytes_count = fuse_byte_counter;

        // debug print-out...
        //print_settings(description);

        // go up to parent node
        xml_node = xml_node->get_parent();
        // loop through children (registers and enumerator nodes)
        children = xml_node->get_children();
        for (node_iterator = children.begin(); node_iterator != children.end(); ++node_iterator) {
                // skip irrelevant nodes (namely registers)
                if (((*node_iterator)->get_name()).compare("enumerator") != 0)
                        continue;
                // prepare list with this enumerator's entries
                list<DeviceDescription::OptionEntry> *enum_listing = new list<DeviceDescription::OptionEntry>;
                enum_listing = get_enum_list ((*node_iterator));
                // get enumerator name
                string enum_name = get_att_value ((*node_iterator), "name");
                // add new entry in map
                (*description.option_lists)[enum_name] = enum_listing;
        }

        // debug print-out...
        //print_options(description);

        return Parser::status::success;
}

// ******************************************************************************
// Parse fuse warnings
// ******************************************************************************

Parser::status Parser_v1::get_warnings (xmlpp::Node *root_node, DeviceDescription &description)
{
        //cout << "PARSING WARNINGS..." << endl;

        // Fuse Warnings in XML file...
        // <FuseWarning>[fuse-byte-no],[AND-mask],[result],[warning text]</FuseWarning>

        string raw_warning;
        DeviceDescription::FuseWarning *warning_entry;

        xmlpp::Node* xml_node = root_node;

        // go to PROGRAMMING node
        xml_node = xml_node->get_first_child("PROGRAMMING");
        // if no such node, it must be a device without fuses...
        if (!xml_node)
                return Parser::status::no_content;

        // go to ISPInterface node
        xml_node = xml_node->get_first_child("ISPInterface");
        // if no such node, it must be a device without fuses...
        if (!xml_node)
                return Parser::status::no_content;

        // loop through children and get warnings
        xmlpp::Node::NodeList::iterator node_iterator;
        xmlpp::Node::NodeList children = xml_node->get_children();
        for (node_iterator = children.begin(); node_iterator != children.end(); ++node_iterator) {
                // get name of current sibling...
                const string node_name = (*node_iterator)->get_name();
                // if not a FuseWarning, proceed to next sibling...
                if (node_name.compare("FuseWarning") != 0)
                        continue;
                // get string (raw) representation of warning...
                raw_warning = get_txt_value ((*node_iterator));
                // assemble fuse warning structure
                warning_entry = new DeviceDescription::FuseWarning;
                stringstream(raw_warning.substr(0,1)) >> hex >> warning_entry->fbyte;
                stringstream(raw_warning.substr(2,4)) >> hex >> warning_entry->fmask;
                stringstream(raw_warning.substr(7,4)) >> hex >> warning_entry->fresult;
                warning_entry->warning = raw_warning.substr(21,1000);
                // inset warning in list
                description.warnings->push_back(*warning_entry);
                // delete temporary wanring entry
                delete warning_entry;
        }

        // debug print-out...
        //print_warnings(description);

        return Parser::status::success;
}

// ******************************************************************************
// Parse default fuse settings
// ******************************************************************************

Parser::status Parser_v1::get_default (xmlpp::Node *root_node, DeviceDescription &description)
{
        xmlpp::Node* xml_node = root_node;
        xmlpp::Node* tmp_node = root_node;

        // go to FUSE node
        xml_node = xml_node->get_first_child("FUSE");
        // if no such node, it must be a device without fuses...
        if (!xml_node)
                return Parser::status::no_content;

        // go to LIST node
        xml_node = xml_node->get_first_child("LIST");
        // if no such node, it must be a device without fuses...
        if (!xml_node)
                return Parser::status::no_content;

        // get list of fuse-byte names
        string name_list = get_txt_value (xml_node);
        name_list = name_list.substr(1, name_list.length() - 2);

        // extract individual byte names
        int byte_count = 0;
        string byte_names[3];
        while (true) {
                // locate separator string
                size_t name_end = name_list.find(":");
                // extract name
                byte_names[byte_count] = name_list.substr(0, name_end);
                // check if it was the last name in the list
                if (name_end == string::npos)
                        break;
                // delete name and separator from list
                name_list.erase(0, name_end + 1);

                byte_count++;
        }

        xmlpp::Node::NodeList children;
        xmlpp::Node::NodeList::iterator node_iterator;
        string node_name, bit_val;
        int bit_order;

        // get value for each fuse-byte
        for (int i = 0; i < (byte_count + 1); i++) {
                // go up to FUSE node
                // (fuse-byte description nodes are children of FUSE node)
                xml_node = xml_node->get_parent();
                // go to <FUSE-BYTE-NAME> node
                tmp_node = xml_node->get_first_child(byte_names[i]);
                // if no such node, proceed to next fuse-byte...
                if (!tmp_node)
                        continue;
                else
                        xml_node = tmp_node;
                // loop through all children with name "FUSEx", where x in [0..7]
                children = xml_node->get_children();
                for (node_iterator = children.begin(); node_iterator != children.end(); ++node_iterator) {
                        // get name of current sibling...
                        node_name = (*node_iterator)->get_name();
                        // if does not contain "FUSE", proceed to next sibling...
                        if (node_name.find("FUSE") == string::npos)
                                continue;
                        // if more than 5 letters in name, proceed to next sibling...
                        if (node_name.length() > 5)
                                continue;
                        // erase constant part (FUSE)
                        node_name.erase(0,4);
                        // transform to integer
                        bit_order = atoi(node_name.c_str());
                        // get DEFAULT node
                        tmp_node = (*node_iterator)->get_first_child("DEFAULT");
                        // if not found, no default values mentioned in XML!
                        if (tmp_node == nullptr)
                                return Parser::status::no_content;
                        bit_val = get_txt_value (tmp_node);
                        // apply bit value on fuse-byte
                        if (bit_val.find("0") == string::npos)
                                description.fusebytes_default[i] |= 1 << bit_order;
                        else
                                description.fusebytes_default[i] &= ~(1 << bit_order);
                }
        }

        //print_defaults(description);

        return Parser::status::success;
}

// ******************************************************************************
// Parse specifications
// ******************************************************************************

Parser::status Parser_v1::get_specs (xmlpp::Node *root_node, DeviceDescription &description)
{
        //cout << "PARSING SPECIFICATIONS..." << endl;

        xmlpp::Node* xml_node = root_node;
        xmlpp::Node* tmp_node = nullptr;
        string txtvalue;
        gfloat numvalue;

        // go to ADMIN node
        xml_node = xml_node->get_first_child("ADMIN");
        // go to SPEED node
        xml_node = xml_node->get_first_child("SPEED");
        // get node content
        txtvalue = get_txt_value(xml_node);
        txtvalue.resize(txtvalue.size() - 3);
        description.max_speed = txtvalue + " MHz";
        //cout << "speed: " << description.max_speed << endl;
        // go up to ADMIN node
        xml_node = xml_node->get_parent();
        // go to BUILD node
        xml_node = xml_node->get_first_child("BUILD");
        // get node content
        txtvalue = get_txt_value(xml_node);
        description.xml_version = txtvalue;
        //cout << "build: " << description.xml_version << endl;
        // go up to ADMIN node
        xml_node = xml_node->get_parent();
        // go to SIGNATURE node
        xml_node = xml_node->get_first_child("SIGNATURE");
        // get node content
        description.signature = get_signature_bytes(xml_node);
        //cout << "signature: \"" << description.signature << "\"" << endl;
        // go up to AVRPART node
        xml_node = xml_node->get_parent();
        xml_node = xml_node->get_parent();
        // go to MEMORY node
        xml_node = xml_node->get_first_child("MEMORY");
        // go to PROG_FLASH node
        xml_node = xml_node->get_first_child("PROG_FLASH");
        // get node content
        txtvalue = get_txt_value(xml_node);
        numvalue = ::atof(txtvalue.c_str()) / 1024;
        txtvalue = float_to_string(numvalue);
        description.flash_size = txtvalue + " KB";
        //cout << "flash: " << description.flash_size << endl;
        // go up to MEMORY node
        xml_node = xml_node->get_parent();
        // go to EEPROM node
        tmp_node = xml_node->get_first_child("EEPROM");
        // if no such node, it must be a device without EEPROM
        if (!tmp_node) {
                description.eeprom_exists = false;
                txtvalue = "0 Bytes";
                description.sram_size = txtvalue;
        } else {
                description.eeprom_exists = true;
                xml_node = tmp_node;
                // get node content
                txtvalue = get_txt_value(xml_node);
                numvalue = ::atof(txtvalue.c_str()) / 1024;
                // check for zero eeprom size
                if (numvalue == 0)
                        description.eeprom_exists = false;
                // express size in KBytes
                if (numvalue > 1)
                        txtvalue = float_to_string(numvalue) + " KB";
                else
                        txtvalue += " Bytes";
                description.eeprom_size = txtvalue;
                //cout << "eeprom: " << description.eeprom_size << endl;
                // go up to MEMORY node
                xml_node = xml_node->get_parent();
        }
        // go to INT_SRAM node
        tmp_node = xml_node->get_first_child("INT_SRAM");
        // if no such node, it must be a device without SRAM!
        if (!tmp_node) {
                txtvalue = "0 Bytes";
                description.sram_size = txtvalue;
        } else {
                xml_node = tmp_node;
                // go to SIZE node
                xml_node = xml_node->get_first_child("SIZE");
                // get node content
                txtvalue = get_txt_value(xml_node);
                numvalue = ::atof(txtvalue.c_str()) / 1024;
                if (numvalue > 1)
                        txtvalue = float_to_string(numvalue) + " KB";
                else
                        txtvalue += " Bytes";
                description.sram_size = txtvalue;
                //cout << "sram_size: " << description.eeprom_size << endl;
        }

        return Parser::status::success;
}

// ******************************************************************************
// Go to the specified node (by attribute)
// ******************************************************************************

xmlpp::Node* Parser_v1::get_child_with_attr (xmlpp::Node* starting_node, string att_name, string att_value)
{
        xmlpp::Node *target = nullptr;
        xmlpp::Node::NodeList::iterator node_iterator;
        xmlpp::Element::AttributeList::const_iterator att_iterator;
        xmlpp::Node::NodeList children = starting_node->get_children();

        // loop through child-nodes
        for (node_iterator = children.begin(); node_iterator != children.end(); ++node_iterator) {
                if (const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(*node_iterator)) {
                        // loop through attribute-nodes
                        const xmlpp::Element::AttributeList& attributes = nodeElement->get_attributes();
                        for(att_iterator = attributes.begin(); att_iterator != attributes.end(); ++att_iterator) {
                                // break inner loop if target has been pinpointed
                                if ((*att_iterator)->get_name().compare(att_name) == 0)
                                        if ((*att_iterator)->get_value().compare(att_value) == 0) {
                                                //cout << (*att_iterator)->get_name() << " = " << (*att_iterator)->get_value() << endl;
                                                target = *node_iterator;
                                                break;
                                        }
                        }
                        // break outer loop if target has been defined
                        if (target != nullptr)
                                break;
                }
        }
        return target;
}

// ******************************************************************************
// Get text value in specified node
// ******************************************************************************

string Parser_v1::get_txt_value (xmlpp::Node* starting_node)
{
        const xmlpp::TextNode *text = NULL;
        xmlpp::Node::NodeList::iterator iter;
        xmlpp::Node::NodeList children = starting_node->get_children();

        // loop through child-nodes
        for (iter = children.begin(); iter != children.end(); ++iter) {
                text = dynamic_cast<const xmlpp::TextNode*>(*iter);
                if (text)
                        break;
                else
                        continue;
        }
        return text->get_content();
}

// ******************************************************************************
// Get the value of specified attribute
// ******************************************************************************

string Parser_v1::get_att_value (xmlpp::Node* xml_node, string att_name)
{
        xmlpp::Node::NodeList::iterator node_iterator;
        xmlpp::Element::AttributeList::const_iterator att_iterator;
        string data_string;

        if (const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(xml_node)) {
                // loop through attribute-nodes
                const xmlpp::Element::AttributeList& attributes = nodeElement->get_attributes();
                for(att_iterator = attributes.begin(); att_iterator != attributes.end(); ++att_iterator) {
                        // break inner loop if target has been pinpointed
                        if ((*att_iterator)->get_name().compare(att_name) == 0) {
                                //cout << (*att_iterator)->get_name() << " = " << (*att_iterator)->get_value() << endl;
                                data_string = (*att_iterator)->get_value();
                                break;
                        }
                }
        }
        return data_string;
}

// ******************************************************************************
// Get list of enumerator members
// ******************************************************************************

list<DeviceDescription::OptionEntry>* Parser_v1::get_enum_list (xmlpp::Node* xml_node)
{
        list<DeviceDescription::OptionEntry> *enum_listing = new list<DeviceDescription::OptionEntry>;

        xmlpp::Node::NodeList::iterator node_iterator;
        xmlpp::Element::AttributeList::const_iterator att_iterator;
        xmlpp::Node::NodeList children = xml_node->get_children();
        string tmp_string;
        uint max_value = 0;

        // loop through child-nodes (enum nodes)
        for (node_iterator = children.begin(); node_iterator != children.end(); ++node_iterator) {
                if (const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(*node_iterator)) {
                        // loop through attribute-nodes
                        DeviceDescription::OptionEntry *entry = new DeviceDescription::OptionEntry;
                        const xmlpp::Element::AttributeList& attributes = nodeElement->get_attributes();
                        for(att_iterator = attributes.begin(); att_iterator != attributes.end(); ++att_iterator) {
                                if ((*att_iterator)->get_name().compare("text") == 0) {
                                        entry->ename = (*att_iterator)->get_value();
                                        //cout << entry->ename << endl;
                                } else if ((*att_iterator)->get_name().compare("val") == 0) {
                                        string strvalue = ((*att_iterator)->get_value()).substr(2, 2);
                                        entry->value = stol (strvalue, nullptr, 16);
                                        if (max_value < entry->value)
                                                max_value = entry->value;
                                        //cout << entry->value << endl;
                                }
                        }
                        enum_listing->push_back(*entry);
                        delete entry;
                }
        }
        // insert pseudo enumerator-entry with maximum value
        DeviceDescription::OptionEntry *entry = new DeviceDescription::OptionEntry;
        entry->ename = "PSEUDO_ENTRY";
        entry->value = max_value;
        enum_listing->push_front(*entry);
        delete entry;
        return enum_listing;
}

// ******************************************************************************
// Get list of settings
// ******************************************************************************

list<DeviceDescription::FuseSetting>* Parser_v1::get_fuse_list (xmlpp::Node* xml_node, uint offset)
{
        list<DeviceDescription::FuseSetting> *fuse_listing = new list<DeviceDescription::FuseSetting>;

        xmlpp::Node::NodeList::iterator node_iterator;
        xmlpp::Element::AttributeList::const_iterator att_iterator;

        xmlpp::Node::NodeList children = xml_node->get_children();
        string tmp_string;

        // loop through child-nodes (bitfield nodes)
        for (node_iterator = children.begin(); node_iterator != children.end(); ++node_iterator) {
                if (const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(*node_iterator)) {
                        // loop through attribute-nodes
                        DeviceDescription::FuseSetting *entry = new DeviceDescription::FuseSetting;
                        entry->fenum = ""; // must initialize to empty
                        entry->offset = offset;
                        const xmlpp::Element::AttributeList& attributes = nodeElement->get_attributes();
                        for(att_iterator = attributes.begin(); att_iterator != attributes.end(); ++att_iterator) {
                                if ((*att_iterator)->get_name().compare("name") == 0) {
                                        entry->fname = (*att_iterator)->get_value();
                                        //cout << entry->fname << endl;
                                } else if ((*att_iterator)->get_name().compare("text") == 0) {
                                        entry->fdesc = (*att_iterator)->get_value();
                                        //cout << entry->fdesc << endl;
                                } else if ((*att_iterator)->get_name().compare("enum") == 0) {
                                        entry->fenum = (*att_iterator)->get_value();
                                        //cout << entry->fenum << endl;
                                } else if ((*att_iterator)->get_name().compare("mask") == 0) {
                                        string strvalue = ((*att_iterator)->get_value()).substr(2, 2);
                                        entry->fmask = stol (strvalue, nullptr, 16);
                                        //cout << entry->fmask << endl;
                                }
                        }
                        if ((entry->fenum).size() == 0)
                                entry->single_option = true;
                        else
                                entry->single_option = false;
                        fuse_listing->push_back(*entry);
                        delete entry;
                }
        }
        return fuse_listing;
}

// ******************************************************************************
// Get the device signature
// ******************************************************************************

string Parser_v1::get_signature_bytes (xmlpp::Node* signature_node)
{
        const xmlpp::TextNode *text = NULL;
        xmlpp::Node::NodeList::iterator iter;
        xmlpp::Node::NodeList children = signature_node->get_children();
        string signatrure;

        signatrure = "0x";
        for (iter = children.begin(); iter != children.end(); ++iter) {
                text = dynamic_cast<const xmlpp::TextNode*>(*iter);
                if (!text)
                        signatrure += (get_txt_value((*iter))).substr(1, 2);
        }
        return signatrure;
}

// ******************************************************************************
// Create the string representation of a float
// ******************************************************************************

string Parser_v1::float_to_string (float number)
{
        string value(16, '\0');
        int chars_written = snprintf(&value[0], value.size(), "%.1f", number);
        value.resize(chars_written);

        return (string)value;
}