import sys
import subprocess
import socket
import ipaddress

with open(sys.argv[-2]) as f:
    p = subprocess.Popen(sys.argv[-1], stdin=f, stdout=subprocess.PIPE)
    out, _ = p.communicate()

for line in out.decode().strip().split('\n'):
    host, port = input().split(maxsplit=1)
    try:
        addrs = socket.getaddrinfo(host, port, family=socket.AF_INET, type=socket.SOCK_STREAM)
        addr = min(ipaddress.IPv4Address(addr[-1][0]) for addr in addrs)
        res = f'{addr}:{addrs[0][-1][1]}'
    except socket.gaierror as e:
        res = e.strerror
    if (line.strip() != res
            and res != 'No address associated with hostname'
            and line.strip() != 'Name or service not known'):
        print(f"Answer for {host} {port} is wrong. Actual:", line.strip())
        print('Expected:', res)
