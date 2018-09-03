#!/usr/bin/env python

# Standard python libs
import os,sys
sys.path.append("../../build/python/src/genpy")
sys.path.append("../../build/python/src/genpy/src/genpy/cisco_ios_xr_ipv6_nd_oper/ipv6_node_discovery/nodes/node/neighbor_interfaces/neighbor_interface/host_addresses/host_address/")
import ast, pprint 
import pdb
import yaml, json
import telemetry_pb2
from mdt_grpc_dialin import mdt_grpc_dialin_pb2
import json_format
from grpc.beta import implementations
from ipv6_nd_neighbor_entry_pb2 import ipv6_nd_neighbor_entry_KEYS, ipv6_nd_neighbor_entry
 
#
# Get the GRPC Server IP address and port number
#
def get_environment_variables():
    # Get GRPC Server's IP from the environment
    if 'SERVER_IP' not in os.environ.keys():
        print "Need to set the SERVER_IP env variable e.g."
        print "export SERVER_IP='10.30.110.214'"
        os._exit(0)
    
    # Get GRPC Server's Port from the environment
    if 'SERVER_PORT' not in os.environ.keys():
        print "Need to set the SERVER_PORT env variable e.g."
        print "export SERVER_PORT='57777'"
        os._exit(0)


    # Get XR Username from the environment
    if 'XR_USER' not in os.environ.keys():
        print "Need to set the XR_USER env variable e.g."
        print "export XR_USER='admin'"
        os._exit(0)

    # Get XR Password from the environment
    if 'XR_PASSWORD' not in os.environ.keys():
        print "Need to set the XR_PASSWORD env variable e.g."
        print "export XR_PASSWORD='admin'"
        os._exit(0)

    
    return (os.environ['SERVER_IP'], int(os.environ['SERVER_PORT']),
            os.environ['XR_USER'], os.environ['XR_PASSWORD'])


#
# Setup the GRPC channel with the server, and issue RPCs
#
if __name__ == '__main__':
    server_ip, server_port, xr_user, xr_passwd = get_environment_variables()
    
    print "Using GRPC Server IP(%s) Port(%s)" %(server_ip, server_port)

    # Create the channel for gRPC.
    channel = implementations.insecure_channel(server_ip, server_port)


    # Ereate the gRPC stub.
    stub = mdt_grpc_dialin_pb2.beta_create_gRPCConfigOper_stub(channel)

    metadata = [('username', xr_user), ('password', xr_passwd)]
    Timeout = 3600*24*365 # Seconds

    sub_args = mdt_grpc_dialin_pb2.CreateSubsArgs(ReqId=99, encode=2, subidstr='IPV6')
    stream = stub.CreateSubs(sub_args, timeout=Timeout, metadata=metadata)
    for segment in stream:
        telemetry_pb = telemetry_pb2.Telemetry()
        telemetry_pb.ParseFromString(segment.data)
        # Return in JSON format instead of protobuf.

        telemetry_gpb_table = telemetry_pb2.TelemetryGPBTable()
        telemetry_gpb_table.CopyFrom(telemetry_pb.data_gpb)

        gpb_rows = []

        while(len(telemetry_gpb_table.row)):
            gpb_row_dict = {}
            gpb_row_dict["keys"] = {}
            gpb_row_dict["content"] = {}
 
            telemetry_gpb_row = telemetry_pb2.TelemetryRowGPB()
            telemetry_gpb_row.CopyFrom(telemetry_gpb_table.row.pop())
            gpb_row_dict["timestamp"] = telemetry_gpb_row.timestamp

            ipv6_nd_neighbor_entry_keys = ipv6_nd_neighbor_entry_KEYS()
            ipv6_nd_neighbor_entry_keys.ParseFromString(telemetry_gpb_row.keys)


            ipv6_nd_neighbor_entry_content = ipv6_nd_neighbor_entry()
            ipv6_nd_neighbor_entry_content.ParseFromString(telemetry_gpb_row.content)

            content_dump = json_format.MessageToJson(ipv6_nd_neighbor_entry_content)
            keys_dump = json_format.MessageToJson(ipv6_nd_neighbor_entry_keys)

            gpb_row_dict["content"].update(yaml.safe_load(content_dump))
            gpb_row_dict["keys"].update(yaml.safe_load(keys_dump))

            gpb_rows.append(gpb_row_dict)

            print pprint.pprint(gpb_rows) 
    os._exit(0)
