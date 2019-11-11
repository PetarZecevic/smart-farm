import socket
from ssdp_protocol import SSDP_Protocol

if __name__ == '__main__':
    print('IOT Smart Farm Device')

    discovery_service = SSDP_Protocol('M-SEARCH', 'IOTGATEWAY')
    discovery_service.send_request()
    response = discovery_service.fetch_response(10)
    print(response)
