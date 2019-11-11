import socket

class SSDP_Protocol():
    """
    This class is used to send sddp_request's,
    and receive ssdp_responses.
    """
    
    SSDP_MULTICAST_IP = '239.255.255.250'
    SSDP_PORT = '1900'
    SSDP_MSG_MAX_SIZE = 1024

    search_message = \
        '{0} * HTTP/1.1\r\n' \
        'HOST:' + SSDP_MULTICAST_IP + ':' + SSDP_PORT + '\r\n' \
        'ST: {1}\r\n' \
        'MX: 1\r\n' \
        'MAN:"ssdp:discover"\r\n' \
        '\r\n'

    def __init__(self, method, search_target):
        self.method = method
        self.search_target = search_target
        self.protocol_socket = socket.socket(socket.AF_INET,
            socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.protocol_socket.bind(('', int(SSDP_Protocol.SSDP_PORT)))
        self.request_message = SSDP_Protocol.search_message.format(
            self.method,
            self.search_target
        )

    def send_request(self):
        self.protocol_socket.sendto(self.request_message.encode(),
            (SSDP_Protocol.SSDP_MULTICAST_IP.encode(), int(SSDP_Protocol.SSDP_PORT)))

    def fetch_response(self, timeout_seconds):
        """
        Return's response message as dictionary of header rows, name:value.
        """
        self.protocol_socket.settimeout(timeout_seconds)
        data_dict = {}
        try:
            data, addr = self.protocol_socket.recvfrom(
                SSDP_Protocol.SSDP_MSG_MAX_SIZE)
            print(addr)
            header = data.splitlines()
            if header[0] == 'HTTP/1.1 200 OK':
                for line in header[1:]:
                    param_name = line.split(':', maxsplit=1)[0]
                    param_value = line.split(':', maxsplit=1)[1]
                    data_dict[param_name] = param_value 

        except socket.timeout:
            return None
        except IndexError:
            return None
        finally:
            return data_dict

        
