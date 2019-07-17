#!/usr/bin/env python

# Standard python libs
import os,sys
sys.path.append("../../build/python/src/genpy")

import ast, pprint 
import pdb
import yaml, json
import telemetry_pb2
from mdt_grpc_dialin import mdt_grpc_dialin_pb2
from mdt_grpc_dialin import mdt_grpc_dialin_pb2_grpc
import grpc
 
#
# Get the GRPC Server IP address and port number
#
def get_server_ip_port():
    # Get GRPC Server's IP from the environment
    if 'SERVER_IP' not in os.environ.keys():
        print("Need to set the SERVER_IP env variable e.g.")
        print("export SERVER_IP='10.30.110.214'")
        os._exit(0)
    
    # Get GRPC Server's Port from the environment
    if 'SERVER_PORT' not in os.environ.keys():
        print("Need to set the SERVER_PORT env variable e.g.")
        print("export SERVER_PORT='57777'")
        os._exit(0)
    
    return (os.environ['SERVER_IP'], int(os.environ['SERVER_PORT']))


#
# Setup the GRPC channel with the server, and issue RPCs
#
if __name__ == '__main__':
    server_ip, server_port = get_server_ip_port()

    print("Using GRPC Server IP(%s) Port(%s)" %(server_ip, server_port))

    # Create the channel for gRPC.
    channel = grpc.insecure_channel(str(server_ip)+":"+str(server_port))

    unmarshal = True

    # Ereate the gRPC stub.
    stub = mdt_grpc_dialin_pb2_grpc.gRPCConfigOperStub(channel)

    metadata = [('username', 'vagrant'), ('password', 'vagrant')]
    Timeout = 3600*24*365 # Seconds

    sub_args = mdt_grpc_dialin_pb2.CreateSubsArgs(ReqId=99, encode=4, subidstr='1')
    stream = stub.CreateSubs(sub_args, timeout=Timeout, metadata=metadata)
    for segment in stream:
        if not unmarshal:
            print(segment)
        else:
            # Go straight for telemetry data
            telemetry_pb = telemetry_pb2.Telemetry()

            encoding_path_1 = 'Cisco-IOS-XR-ipv4-bgp-oper:bgp/instances/'+\
                            'instance/instance-active/default-vrf/sessions/session'

            encoding_path_2 = 'openconfig-network-instance:network-instances/network-instance/afts/'
            
            try:
                # Return in JSON format instead of protobuf.
                if json.loads(segment.data)["encoding_path"] == encoding_path_1:
                    print(json.dumps(json.loads(segment.data), indent=3))
                elif encoding_path_2 in json.loads(segment.data)["encoding_path"]:
                    print(json.dumps(json.loads(segment.data), indent=3))
            except Exception as e: 
                print("Failed to receive data, error: " +str(e))
    os._exit(0)
