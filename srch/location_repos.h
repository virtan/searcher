#ifndef LOCATION_REPOS_H
#define LOCATION_REPOS_H

#include <iostream>
#include <netinet/in.h>
#include <fstream>
#include <string>
#include <map>
#include <srch/config_branch.h>
#include <srch/location.pb.h>
#include <srch/itim_base.h>

namespace itim {

struct location {
    string id;
    string full_name;
    string post_code;
    bool is_centrally_governed;
    uint32_t phone_length;
    bool is_empty_location;

    location(): is_empty_location(true) {}
    operator bool() const { return !is_empty_location; }

    static location from_message(const proto::Province& message) {
        location loc;
        loc.id = message.code();
        loc.full_name = message.fullname();
        loc.post_code = message.postcode();
        loc.is_centrally_governed = message.iscentrallygoverned();
        loc.phone_length = message.phonelength();
        loc.is_empty_location = false;
        return loc;
    }
};

typedef std::vector<location> locations;

class location_repository {
public:
    location_repository(const config_branch &province_config) {
        using namespace std;
        string path = province_config.get<string>("file");
        ifstream in(path.c_str(), ios::binary);

        if (!in.is_open()) {
            throw exception("Failed to open the locations file")
                << where(__PRETTY_FUNCTION__) << file(path) << errno_code(errno);
        }

        while (true) {
            uint32_t mess_length;
            in.read((char*)&mess_length, sizeof(mess_length));
            if (in.eof()) {
                break;
            }
            mess_length = ntohl(mess_length);

            proto::Province message;
            string mess_data;
            mess_data.resize(mess_length);
            in.read(&mess_data[0], mess_length);
            if (!message.ParseFromString(mess_data)) {
                throw exception("Failed to recognize the Province protobuf messsage")
                    << where(__PRETTY_FUNCTION__) << file(path) << errno_code(errno);
            };
            location loc = location::from_message(message);
            id2location.insert( pair<string, location>(message.code(), loc) );
            name2location.insert( pair<string, location>(message.fullname(), loc) );
        };
    }

    location get_by_id(const std::string& id) {
        if (id2location.count(id) == 1) {
            return id2location.find(id)->second;
        }
        return location();
    }

    location get(const std::string& name) {
        if (name2location.count(name) == 1) {
            return name2location.find(name)->second;
        }
        return location();
    }

private:
    std::map<std::string, location> id2location;
    std::map<std::string, location> name2location;
};

}

#endif
