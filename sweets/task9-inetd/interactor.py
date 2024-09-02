#!/usr/bin/python3

import traceback
import sys
import signal
import asyncio
import os
import subprocess
from time import sleep


def parse_ti(info_path):
    result = {}
    with open(info_path, 'rt') as inf:
        for line in inf:
            line = line.strip()
            if not line:
                continue
            key, _, val = line.partition('=')
            result[key.strip()] = val.strip()
    return result


class PresentationError(Exception):
    pass


class Latch:
    def __init__(self, expected, id_=None):
        self.id = id_
        self.remaining = expected
        self.done = asyncio.Event()

    async def arrive_and_wait(self):
        if self.remaining == 0:
            raise RuntimeError('Unexpected arrive_and_wait() call')
        self.remaining -= 1
        if self.remaining == 0:
            self.done.set()
        else:
            await self.done.wait()

    async def wait(self):
        await self.done.wait()

    def __str__(self):
        return f'Latch {self.id}' if self.id is not None else 'Latch'


class Connection:
    def __init__(self, name, prog_pid, port, protocol):
        self.queue = asyncio.Queue()
        self.name = name
        self.prog_pid = prog_pid  # not really related to client/connection, but we need synchronization, and Connections are already synchronized
        self.port = port
        self.protocol = protocol
        self.reader = None
        self.writer = None
        self.writer_closed = False
        self.task = asyncio.create_task(self.run_connection())

    def log(self, message):
        print(f'{self.name}: {message}', file=self.protocol)

    def check_pids(self):
        proc = subprocess.run(['pgrep', '-c', '--parent', str(self.prog_pid)], capture_output=True, text=True)
        if proc.returncode != 0:
            if proc.returncode == 1 and proc.stdout == '0\n' and not proc.stderr:
                return  # no children -> status = 1, empty output
            raise RuntimeError(f'Cannot get children. stdout: {proc.stdout!r}, stderr: {proc.stderr!r}, status: {proc.returncode}')
        num_children = int(proc.stdout)
        if num_children > 0:
            raise PresentationError(f'Found {num_children} children, expected 0')

    async def init_connection(self):
        if self.reader is None:
            self.log('starting connection')
            self.reader, self.writer = await asyncio.open_connection('::1', self.port)
            self.log('started connection')

    async def send(self, data):
        if self.writer_closed:
            self.log('Cannot send(), writer is closed')
            raise RuntimeError()
        await self.init_connection()
        self.writer.write(data)
        await self.writer.drain()

    async def recv(self, expected):
        await self.init_connection()
        try:
            received = await self.reader.readexactly(len(expected))
        except asyncio.exceptions.IncompleteReadError as e:
            self.log(f'received: {e.partial}')
            assert False, (f'{self.name}: '
                           f'Unexpected EOF (expected {e.expected} bytes, got {len(e.partial)})')
        self.log(f'received: {received}')
        assert received == expected, (f'{self.name}: '
                                      f'Incorrect data (expected {expected}, received {received})')

    async def recv_any_of(self, expected):
        variants = sorted((elem + b'\n' for elem in expected.split()), key=len)
        await self.init_connection()
        received = b''
        while True:
            if received in variants:
                return
            if len(received) == len(variants[-1]):
                assert False, f'{self.name}: Incorrect data (expected {variants}, received {received})'
            to_recv = next(len(elem) - len(received) for elem in variants if len(elem) > len(received))
            print('!!!', to_recv, file=sys.stderr)
            try:
                received += await self.reader.readexactly(to_recv)
            except asyncio.exceptions.IncompleteReadError as e:
                self.log(f'received: {e.partial}')
                assert False, (f'{self.name}: '
                               f'Unexpected EOF (expected {e.expected} bytes, got {len(e.partial)})')
            self.log(f'received: {received}')

    def close_writer(self):
        if not self.writer_closed:
            self.writer.write_eof()
            self.writer_closed = True

    async def run_connection(self):
        try:
            while True:
                method, arg = await self.queue.get()
                self.log(f'method: {method}, arg: {arg}')
                if method == 'check_pids':
                    self.check_pids()
                elif method == 'sleep':
                    await asyncio.sleep(arg)
                elif method == 'wait':
                    await arg.wait()
                elif method == 'arrive':
                    await arg.arrive_and_wait()
                elif method == 'send':
                    await self.send(arg)
                elif method == 'recv':
                    await self.recv(arg)
                elif method == 'recv_any_of':
                    await self.recv_any_of(arg)
                elif method == 'stop':
                    self.close_writer()
                elif method == 'done':
                    self.close_writer()
                    tail = await self.reader.read(10)
                    self.log(f'tail: {tail}')
                    assert tail == b'', f'{self.name}: Garbage data ({tail}) where EOF was expected'
                    break
                else:
                    self.log(f'unexpected method {method}')
                    raise RuntimeError()
        except (ConnectionError, OSError) as e:
            raise PresentationError(f'{self.name}: {e}')


async def process_input(dat_path, prog_pid, port, protocol):
    connections = {}
    last_latch = None
    with open(dat_path, 'rt') as dat:
        for line in dat:
            # comments
            line = line.split('#', 1)[0].strip()
            if not line:
                continue
            # barriers / latches
            if line[0] == '=' and set(line) == {'='}:
                last_id = last_latch.id if last_latch else -1
                last_latch = Latch(len(connections), last_id + 1)
                for conn in connections.values():
                    await conn.queue.put(('arrive', last_latch))
                continue
            line += '\n'  # All data chunks end with newlines (from 2021-22 tests)
            # Regular commands
            conn_idx, method, arg = line.split(' ', 2)
            if method == 'sleep':
                arg = float(arg)
            else:
                arg = arg.encode('ascii')
            try:
                conn = connections[conn_idx]
            except KeyError:
                conn = Connection(conn_idx, prog_pid, port, protocol)
                connections[conn_idx] = conn
                if last_latch:
                    await conn.queue.put(('wait', last_latch))
            await conn.queue.put((method, arg))
    for conn in connections.values():
        await conn.queue.put(('done', None))
    await asyncio.gather(*[conn.task for conn in connections.values()])


RUN_OK = 0
RUN_WA = 1
RUN_PE = 2
RUN_CF = 6


def main():
    _, dat_path, out_path, corr_path, prog_pid, info_path = sys.argv
    info = parse_ti(info_path)
    signal.signal(signal.SIGPIPE, signal.SIG_IGN)
    protocol = open(out_path, 'wt')

    cmd_argv = info.get('params', '').split()
    port = cmd_argv[0]
    status = RUN_OK
    sleep(0.1)
    try:
        asyncio.run(process_input(dat_path, prog_pid, port, protocol), debug=True)
    except AssertionError as e:
        print(str(e) or 'something is wrong (WA)', file=sys.stderr)
        status = RUN_WA
    except PresentationError as e:
        print(str(e) or 'something is wrong (PE)', file=sys.stderr)
        status = RUN_PE
    except Exception:
        print(traceback.format_exc(), file=sys.stderr)
        status = RUN_CF

    if prog_pid:
        os.kill(int(prog_pid), signal.SIGTERM)
    return status


if __name__ == '__main__':
    sys.exit(main())
