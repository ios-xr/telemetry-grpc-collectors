#include "IosxrTelemetryDecode.h" 

namespace iosxr {

namespace {
    SensorPaths sensorPaths = {
                                {
                                  "iosxr-ipv6-nd-address",
                                  "Cisco-IOS-XR-ipv6-nd-oper:ipv6-node-discovery/nodes/node/neighbor-interfaces/neighbor-interface/host-addresses/host-address"
                                }
                              };


    std::map<std::string, std::string>
    active_path_map = {
                        { "interface", "GigabitEthernet0/0/0/0" },
                        { "neighbor", "1010:1010::20" },
                        { "reachability_state", "reachable" }
                      };

    std::map<std::string, std::string>
    backup_path_map = { 
                         { "interface", "GigabitEthernet0/0/0/1" },
                         { "neighbor", "2020:2020::20" },
                         { "reachability_state", "reachable" }
                      };
}

template <typename Map>
bool map_compare (Map const &lhs, Map const &rhs) {
    // No predicate needed because there is operator== for pairs already.
    return lhs.size() == rhs.size()
        && std::equal(lhs.begin(), lhs.end(),
                      rhs.begin());
}




TelemetryDecode::TelemetryDecode()
{
   decodeSensorPathMapGPB.insert(
                 std::make_pair(
                      sensorPaths["iosxr-ipv6-nd-address"],
                      &TelemetryDecode::DecodeIPv6NeighborsGPB));

   decodeSensorPathMapGPBKV.insert(
                 std::make_pair(
                      sensorPaths["iosxr-ipv6-nd-address"],
                      &TelemetryDecode::DecodeIPv6NeighborsGPBKV));
}

TelemetryDecode::~TelemetryDecode() {};


std::string
gpbMsgToJson(const google::protobuf::Message& message)
{
    std::string json_string;

    google::protobuf::util::JsonPrintOptions options;

    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    
    auto status = google::protobuf::util::
               MessageToJsonString(message, 
                                   &json_string, 
                                   options);

    if (status.ok()) { 
        return json_string;
    } else {
        LOG(ERROR) << "Failed to convert protobuf message to json";
        LOG(ERROR) << "Error: " << status.error_message();
        LOG(ERROR) << "Error Code: " << status.error_code();
        return "";
    }
}


void
TelemetryDecode::
DecodeIPv6NeighborsGPB(const ::telemetry::TelemetryRowGPB& telemetry_gpb_row)
{

    using namespace cisco_ios_xr_ipv6_nd_oper::
                    ipv6_node_discovery::
                    nodes::node::neighbor_interfaces::
                    neighbor_interface::host_addresses::host_address;

    auto ipv6_nd_neigh_entry_keys = ipv6_nd_neighbor_entry_KEYS();
    if(ipv6_nd_neigh_entry_keys.ParseFromString(telemetry_gpb_row.keys()))
    {
        LOG(INFO) << "IPv6 ND entry keys \n"
                << gpbMsgToJson(ipv6_nd_neigh_entry_keys);

        auto interface = ipv6_nd_neigh_entry_keys.interface_name();
        LOG(INFO) << "Interface is\n"
                  << interface;

        auto neighbor = ipv6_nd_neigh_entry_keys.host_address();
        LOG(INFO) << "Host Address  is\n"
                  << neighbor;
 
    } else {
        throw IosxrTelemetryException(std::string(
                    "Failed to fetch IPv6 neighbor entry keys"));
    }

    auto ipv6_nd_neigh_entry = ipv6_nd_neighbor_entry();
    if(ipv6_nd_neigh_entry.ParseFromString(telemetry_gpb_row.content()))
    {
        LOG(INFO) << "IPv6 ND entry \n"
                << gpbMsgToJson(ipv6_nd_neigh_entry);
        auto lladdr = ipv6_nd_neigh_entry.link_layer_address();
        LOG(INFO) << "Link Layer Address is\n"
                  << lladdr; 

        auto reachability_state = ipv6_nd_neigh_entry.reachability_state();
        LOG(INFO) << "Reachability State is\n"
                  <<  reachability_state;

    } else {
        throw IosxrTelemetryException(std::string(
                    "Failed to fetch IPv6 neighbor entry"));
    }

    std::map<std::string, std::string>
    path_map = {
                 { "interface", ipv6_nd_neigh_entry_keys.interface_name() },
                 { "neighbor", ipv6_nd_neigh_entry_keys.host_address() },
                 { "reachability_state", ipv6_nd_neigh_entry.reachability_state() }
               };


    path_map_vector.push_back(path_map);
 
}

                    
void
TelemetryDecode::
DecodeIPv6NeighborsGPBKV(const ::telemetry::TelemetryField& telemetry_gpbkv_field)
{
  //TODO for the reader
}

void
TelemetryDecode::DecodeTelemetryDataGPB(const telemetry::Telemetry& telemetry_data)
{

    auto& pathmap = decodeSensorPathMapGPB;
    if (pathmap.find(telemetry_data.encoding_path()) != pathmap.end()) {
        auto telemetry_gpb_table = telemetry_data.data_gpb();   
        for (auto row_index=0;
             row_index < telemetry_gpb_table.row_size();)
        {
           auto telemetry_gpb_row = telemetry_gpb_table.row(row_index);
            VLOG(3) << "Telemetry GPB row \n"
                    << gpbMsgToJson(telemetry_gpb_row);
            (this->*pathmap[telemetry_data.encoding_path()])(telemetry_gpb_row);

            row_index++;
        }

        bool active_path_up = false;

        for (auto &path_map : path_map_vector) {
            active_path_up = map_compare(path_map, active_path_map);
            if (active_path_up) {break;}
        }
        if (active_path_up) {
            VLOG(1) << "Active path up"
                    << "Performing Active Path action";
        } else {
            VLOG(1) << "Active path not up"
                    << "Performing Backup Path action";
        }

        // Decision taken, now clear current vector
        path_map_vector.clear();

    } else {
        throw IosxrTelemetryException(std::string("Encoding Path")+telemetry_data.encoding_path()+std::string("not found in registered sensor paths"));
    }
}

void
TelemetryDecode::DecodeTelemetryDataGPBKV(const telemetry::Telemetry& telemetry_data)
{
   //TODO for the reader
    
}  


void
TelemetryDecode::DecodeTelemetryData(const telemetry::Telemetry& telemetry_data)
{
    VLOG(3) << "Telemetry Data: \n"
            << gpbMsgToJson(telemetry_data);

    VLOG(3) << "Encoding Path : \n"
            << telemetry_data.encoding_path();

    if (telemetry_data.has_data_gpb()) {
      try
      {
        DecodeTelemetryDataGPB(telemetry_data);
      } catch (IosxrTelemetryException const& ex) {
        LOG(ERROR) << "Failed to decode Telemetry data as GPB";
        LOG(ERROR) << ex.what();
      } catch (std::exception const& ex) {
        LOG(ERROR) << "Failed to decode Telemetry data as GPB";
        LOG(ERROR) << ex.what();
      }
    } else {
      try
      {
        DecodeTelemetryDataGPBKV(telemetry_data);
      } catch (IosxrTelemetryException const& ex) {
        LOG(ERROR) << "Failed to decode Telemetry data as GPBKV";
        LOG(ERROR) << ex.what();
      } catch (std::exception const& ex) {
        LOG(ERROR) << "Failed to decode Telemetry data as GPBKV";
        LOG(ERROR) << ex.what();
      }
    }

}

}
