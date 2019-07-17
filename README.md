# telemetry-grpc-collectors
gRPC Collectors for IOS-XR Streaming Telemetry in different languages


# Running the python collector

The Telemetry client is written based on the same concept described in the following learning lab: https://learninglabs.cisco.com/tracks/iosxr-programmability/iosxr-streaming-telemetry/03-iosxr-02-telemetry-python/step/1

We make it slightly simpler and instead of requesting telemetry data in a GPB format, we simply request json and dump it.

Before we start, install the dependencies required to connect to the router over gRPC to receive the data.

## Install dependencies for the gRPC telemetry client . 

```
admin@devbox:~$ sudo pip2 install grpcio-tools==1.7.0 googleapis-common-protos
The directory '/home/admin/.cache/pip/http' or its parent directory is not owned by the current user and the cache has been disabled. Please check the permissions and owner of that directory. If executing pip with sudo, you may want sudo's -H flag.
The directory '/home/admin/.cache/pip' or its parent directory is not owned by the current user and caching wheels has been disabled. check the permissions and owner of that directory. If executing pip with sudo, you may want sudo's -H flag.
Collecting grpcio-tools==1.7.0
  Downloading https://files.pythonhosted.org/packages/0e/c3/d9a9960f12e0bab789da875b1c9a3eb348b51fa3af9544c1edd1f7ef6000/grpcio_tools-1.7.0-cp27-cp27mu-manylinux1_x86_64.whl (21.3MB)
    100% |████████████████████████████████| 21.3MB 50kB/s 
Collecting googleapis-common-protos
  Downloading https://files.pythonhosted.org/packages/61/29/1549f61917eadd11650e42b78b4afcfe9cb467157af4510ab8cb59535f14/googleapis-common-protos-1.5.6.tar.gz
Requirement already satisfied (use --upgrade to upgrade): protobuf>=3.3.0 in /usr/local/lib/python2.7/dist-packages (from grpcio-tools==1.7.0)
Requirement already satisfied (use --upgrade to upgrade): grpcio>=1.7.0 in /usr/local/lib/python2.7/dist-packages (from grpcio-tools==1.7.0)
Requirement already satisfied (use --upgrade to upgrade): setuptools in /usr/lib/python2.7/dist-packages (from protobuf>=3.3.0->grpcio-tools==1.7.0)
Requirement already satisfied (use --upgrade to upgrade): six>=1.9 in /usr/lib/python2.7/dist-packages (from protobuf>=3.3.0->grpcio-tools==1.7.0)
Requirement already satisfied (use --upgrade to upgrade): enum34>=1.0.4 in /usr/lib/python2.7/dist-packages (from grpcio>=1.7.0->grpcio-tools==1.7.0)
Requirement already satisfied (use --upgrade to upgrade): futures>=2.2.0 in /usr/local/lib/python2.7/dist-packages (from grpcio>=1.7.0->grpcio-tools==1.7.0)
Installing collected packages: grpcio-tools, googleapis-common-protos
  Running setup.py install for googleapis-common-protos ... done
Successfully installed googleapis-common-protos-1.5.6 grpcio-tools-1.7.0
You are using pip version 8.1.1, however version 19.0.1 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
admin@devbox:~$ 
```

## Clone the Telemetry gRPC collectors git repo
We have published a few samples for c++ and python to help developers write their own gRPC based collectors to receive telemetry data from IOS-XR.

The proto files that we use to create bindings and then write our own clients are published here: https://github.com/cisco/bigmuddy-network-telemetry-proto/

To start, first clone the telemetry-grpc-collectors repo into the devbox home directory:

```
admin@devbox:~$ 
admin@devbox:~$ git clone --recursive https://github.com/ios-xr/telemetry-grpc-collectors ~/telemetry-grpc-collectors
Cloning into 'telemetry-grpc-collectors'...
remote: Enumerating objects: 16, done.
remote: Counting objects: 100% (16/16), done.
remote: Compressing objects: 100% (11/11), done.
remote: Total 137 (delta 4), reused 15 (delta 4), pack-reused 121
Receiving objects: 100% (137/137), 3.67 MiB | 2.79 MiB/s, done.
Resolving deltas: 100% (62/62), done.
Checking connectivity... done.
Submodule 'bigmuddy-network-telemetry-proto' (https://github.com/cisco/bigmuddy-network-telemetry-proto) registered for path 'bigmuddy-network-telemetry-proto'
Cloning into 'bigmuddy-network-telemetry-proto'...
remote: Enumerating objects: 24542, done.
remote: Total 24542 (delta 0), reused 0 (delta 0), pack-reused 24542
Receiving objects: 100% (24542/24542), 6.06 MiB | 3.14 MiB/s, done.
Resolving deltas: 100% (8337/8337), done.
Checking connectivity... done.
Submodule path 'bigmuddy-network-telemetry-proto': checked out '4419cd20fb73f05d059a37fa3e41fe55f02a528f'
admin@devbox:~$ 
```

## Build the Bindings for Model Driven Telemetry gRPC clients
Hop into the build/python/ directory and generate the required bindings from the proto files:


```
admin@devbox:~$ 
admin@devbox:~$ cd ~/telemetry-grpc-collectors/build/python/
admin@devbox:python$ 
admin@devbox:python$ 
admin@devbox:python$ ./gen-mdt-dialin-bindings.sh 
Generating Python bindings...Done
admin@devbox:python$ 
admin@devbox:python$ tree src/genpy/
src/genpy/
├── __init__.py
├── mdt_grpc_dialin
│   ├── __init__.py
│   ├── mdt_grpc_dialin_pb2_grpc.py
│   └── mdt_grpc_dialin_pb2.py
├── mdt_grpc_dialout
│   ├── __init__.py
│   ├── mdt_grpc_dialout_pb2_grpc.py
│   └── mdt_grpc_dialout_pb2.py
├── telemetry_pb2_grpc.py
└── telemetry_pb2.py

2 directories, 9 files
admin@devbox:python$ 
```


## Running a simple python Telemetry client

Hop into the clients/python/ directory and dump the telemetry_client_json.py script which is written to connect to an IOS-XR router over gRPC, request a json data-stream and dump it to the screen.

 
Open up a new shell and run this script to start receiving data from IOS-XR. Make sure you export the connection details for the gRPC server running on router r1 before running the script.

```
export SERVER_IP=10.10.20.170
export SERVER_PORT=57021


```

```
admin@devbox:python$ 
admin@devbox:python$ export SERVER_IP=10.10.20.170
admin@devbox:python$ export SERVER_PORT=57021
admin@devbox:python$ 
admin@devbox:python$ ./telemetry_client_json.py 
Using GRPC Server IP(10.30.110.215) Port(57778)
{
   "encoding_path": "openconfig-network-instance:network-instances/network-instance/afts/mpls", 
   "subscription_id_str": "3", 
   "collection_start_time": "1563335924509", 
   "msg_timestamp": "1563335924515", 
   "collection_end_time": "1563335924520", 
   "node_id_str": "rtr1", 
   "data_json": [
      {
         "keys": [
            {
               "name": "default"
            }
         ], 
         "timestamp": "1563335924514", 
         "content": {
            "label-entry": {
               "next-hops": {
                  "next-hop": {
                     "index": 0, 
                     "state": {
                        "popped-mpls-label-stack": [
                           "IPV4_EXPLICIT_NULL"
                        ], 
                        "index": "0", 
                        "ip-address": "0.0.0.0", 
                        "weight": 0, 
                        "pushed-mpls-label-stack": [
                           "NO_LABEL"
                        ]
                     }
                  }
               }, 
               "label": "IPV4_EXPLICIT_NULL"
            }
         }
      }, 
      {
         "keys": [
            {
               "name": "default"
            }
         ], 
         "timestamp": "1563335924516", 
         "content": {
            "label-entry": {
               "next-hops": {
                  "next-hop": {
                     "index": 0, 
                     "state": {
                        "popped-mpls-label-stack": [
                           "ROUTER_ALERT"
                        ], 
                        "index": "0", 
                        "ip-address": "0.0.0.0", 
                        "weight": 0, 
                        "pushed-mpls-label-stack": [
                           "NO_LABEL"
                        ]
                     }
                  }
               }, 
               "label": "ROUTER_ALERT"
            }
         }
      }, 
      {
         "keys": [



```
