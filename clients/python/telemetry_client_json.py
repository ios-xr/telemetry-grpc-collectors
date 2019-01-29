#!/usr/bin/env python

# Standard python libs
import os,sys
sys.path.append("./src/genpy")

import ast, pprint 
import pdb
import yaml, json
import telemetry_pb2
from mdt_grpc_dialin import mdt_grpc_dialin_pb2
from mdt_grpc_dialin import mdt_grpc_dialin_pb2_grpc
import json_format
import grpc
 
#
# Get the GRPC Server IP address and port number
#
def get_server_ip_port():
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
    
    return (os.environ['SERVER_IP'], int(os.environ['SERVER_PORT']))


#
# Setup the GRPC channel with the server, and issue RPCs
#
if __name__ == '__main__':
    server_ip, server_port = get_server_ip_port()

    print "Using GRPC Server IP(%s) Port(%s)" %(server_ip, server_port)

    # Create the channel for gRPC.
    channel = grpc.insecure_channel(str(server_ip)+":"+str(server_port))

    unmarshal = True

    # Ereate the gRPC stub.
    stub = mdt_grpc_dialin_pb2_grpc.gRPCConfigOperStub(channel)

    metadata = [('username', 'vagrant'), ('password', 'vagrant')]
    Timeout = 3600*24*365 # Seconds

    sub_args = mdt_grpc_dialin_pb2.CreateSubsArgs(ReqId=99, encode=3, subidstr='BGP-SESSION')
    stream = stub.CreateSubs(sub_args, timeout=Timeout, metadata=metadata)
    for segment in stream:
        if not unmarshal:
            print segment
        else:
            # Go straight for telemetry data
            telemetry_pb = telemetry_pb2.Telemetry()

            encoding_path = 'Cisco-IOS-XR-ipv4-bgp-oper:bgp/instances/'+\
                            'instance/instance-active/default-vrf/sessions/session'


            # Return in JSON format instead of protobuf.
            if json.loads(segment.data)["encoding_path"] == encoding_path: 
                print json.dumps(json.loads(segment.data), indent=3)

    os._exit(0)
