from pathlib import Path
import argparse
import subprocess
import typing as tp
import os
import re
import shlex
from functools import cache
from typing import Optional, Tuple


def check_ban(regex_filter, text, name='common', reason=None) -> bool:
    found = regex_filter.search(text)
    if found:
        descr = f'\nReason: {reason}' if reason else f' ({name.lower()})'
        print(f"Found banned sequence '{found[0].strip()}' by {repr(regex_filter.pattern)}{descr}\n")
        return False
    return True


def check_req(regex_filter, text, name, reason=None) -> bool:
    found = regex_filter.search(text)
    if not found:
        pre_descr = f' {name.lower()}' if not reason else ''
        post_descr = f'\nReason: {reason}' if reason else ''
        print(f"Did not find required sequence{pre_descr} {repr(regex_filter.pattern)}{post_descr}\n")
        return False
    return True


def extract_solution_without_includes(file: str) -> str:
    lines = []
    with open(file, 'r') as f:
        for i, orig_line in enumerate(f):
            line = orig_line.strip()
            if line.startswith('#include'):
                continue
            elif line.startswith('#'):
                raise RuntimeError(f'Line markers are not allowed. Bad line {i}: {repr(line)}')
            else:
                lines.append(orig_line)
    return ''.join(lines)


def preprocess(file: str):
    compiler_cmd = 'g++' if file.endswith('.cpp') else 'gcc'  # Not sure if there's any difference
    proc = subprocess.run([compiler_cmd, '-E', '-'], input=extract_solution_without_includes(file).encode(), capture_output=True)
    if proc.returncode != 0:
        print('Preprocessor returned error:')
        try:
            print(proc.stderr.decode())
        except UnicodeError:
            print(proc.stderr)
        exit(1)

    try:
        preprocessed = proc.stdout.decode()
    except UnicodeError:
        print('Source is binary')
        exit(1)

    lines = []
    in_source = False
    source_found = False
    for i, line in enumerate(preprocessed.splitlines(keepends=True)):
        if m := re.match(r'# \d+ "(.+?)"', line):
            # There still may be multiple <stdin> line markers,
            # e.g. if there are some comment-only lines which are stripped by the preprocessor (but not always?)
            filename = m.group(1)
            if filename == '<stdin>':
                in_source = True
                source_found = True
            else:
                in_source = False
        elif in_source:
            lines.append(line)

    if not source_found:
        raise RuntimeError('<stdin> line markers were not found in preprocessed source:\n'
                           f'\n==========\n{preprocessed}\n==========')
    return ''.join(lines)


@cache
def get_source(source_file: str, use_preprocesor_: bool = True) -> str:
    if use_preprocesor_:
        return preprocess(source_file)
    else:
        with open(source_file, 'r') as f:
            return f.read()


def check(source_file, regex_str, check_func, **extra) -> bool:
    text = get_source(source_file)
    flags = 0
    flags |= re.IGNORECASE
    flags |= re.DOTALL
    regex_filter = re.compile(regex_str, flags)
    return check_func(regex_filter, text, **extra)


def split_reason(regex: str) -> Tuple[str, Optional[str]]:
    parts = regex.rsplit(';;', 1)
    if len(parts) == 1:
        return parts[0], None
    elif len(parts) != 2:
        raise RuntimeError("Incorrect val " + regex)
    return parts


def run_clang_format(source_file: str, format_file: str, fix):
    args = ['python3', 'run-clang-format.py', f'--style=file:{format_file}', source_file]
    if fix:
        args.append('-i')
    proc = subprocess.run(args, capture_output=True)
    if proc.returncode != 0:
        print('Clang-format returned error(s):')
        try:
            print(proc.stderr.decode())
        except UnicodeError:
            print(proc.stderr)
        try:
            print(proc.stdout.decode())
        except UnicodeError:
            print(proc.stdout)
        raise RuntimeError("Clang-format not passed")


def check_style(source_file: str, fix: bool):
    clang_format_file = '.clang-format'
    if os.path.isfile(clang_format_file):
        run_clang_format(source_file, clang_format_file, fix)

    regex_checks_passed = True
    regex_filter = os.environ.get('EJ_BAN_BY_REGEX', '')
    if regex_filter:
        regex_filter, reason = split_reason(regex_filter)
        regex_checks_passed &= check(source_file, regex_filter, check_ban, reason=reason)

    for key, value in os.environ.items():
        if key.startswith('EJ_BAN_BY_REGEX_REQ_'):
            value, reason = split_reason(value)
            value = value.replace(' ', r'\s+')
            regex_checks_passed &= check(source_file, value, check_req, name=key[len('EJ_BAN_BY_REGEX_REQ_'):], reason=reason)
        elif key.startswith('EJ_BAN_BY_REGEX_BAN_'):
            value, reason = split_reason(value)
            value = value.replace(' ', r'\s+')
            regex_checks_passed &= check(source_file, value, check_ban, name=key[len('EJ_BAN_BY_REGEX_BAN_'):], reason=reason)
    if not regex_checks_passed:
        raise RuntimeError("Regex check failed")


def run_solution(input_file: Path, correct_file: Path, inf_file: Path, cmd: str, params: str,
                 output_file: tp.Optional[str], env_add: tp.Optional[tp.Dict[str, str]],
                 interactor: tp.Optional[str]) -> bytes:
    params = params.replace('input.txt', str(input_file.absolute()))
    cmd = cmd.replace('input.txt', str(input_file.absolute()))
    if 'params' in cmd:
        cmd = f'{cmd.replace("params", str(params))}'.strip()
    else:
        cmd = f'{cmd} {params}'.strip()
    print(cmd)
    env = os.environ
    if env_add:
        env = env.copy()
        env.update(env_add)
    if interactor:
        p = subprocess.Popen(shlex.split(cmd), stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=False, env=env)
        int_cmd = [interactor, str(input_file.absolute()),
                   'output', str(correct_file.absolute()),
                   str(p.pid), str(inf_file.absolute()) if inf_file.is_file() else '']
        print(shlex.join(int_cmd))
        i = subprocess.Popen(int_cmd, stdin=p.stdout.fileno(), stdout=p.stdin.fileno(), shell=False, env=env)
        p.stdout.close()
        p.stdin.close()
        p.wait()
        i.wait()
        if i.returncode != 0:
            raise RuntimeError(f'Interactor failed with code {i.returncode} on test {input_file}')
        with open('output', 'rb') as f:
            res = f.read()
    else:
        with open(input_file) as fin:
            p = subprocess.Popen(shlex.split(cmd), stdin=fin, stdout=subprocess.PIPE, shell=False, env=env)
            res, _ = p.communicate()
    if p.returncode != 0:
        print(res)
        raise RuntimeError(f'Solution failed with code {p.returncode} on test {input_file}')
    if output_file:
        if res:
            raise RuntimeError(f'Unexpected output on test {input_file}')
        with open(output_file, 'rb') as f:
            res = f.read()
        os.remove(output_file)
    return res


def parse_inf_file(f):
    res = {}

    def parse_param(key, val):
        if key == 'params':
            if key in res:
                raise RuntimeError("Duplicated params")
            res[key] = val
        elif key == 'environ':
            if not key in res:
                res[key] = {}
            eq = val.find('=')
            if not eq:
                raise RuntimeError("Unsupported env " + str(val))
            if len(val) > 2 and val[0] == '"' == val[-1]:
                res[key][val[1: eq]] = val[eq + 1: -1]
            else:
                res[key][val[: eq]] = val[eq + 1:]
        elif key == 'comment':
            pass
        else:
            raise RuntimeError(f"Unknown inf param {key} = {val}")

    for line in f.readlines():
        if not line.strip():
            continue
        if ' = ' in line:
            key, val = line.split(' = ', maxsplit=1)
            val = val.strip()
            parse_param(key, val)
        else:
            raise RuntimeError("Unknown param " + line)
    return res


def res_checker(res, ans, checker):
    with open(ans, 'rb') as expected:
        to_cmp = expected.read()
    if checker == 'cmp':
        if to_cmp != res:
            raise RuntimeError(f"Output missmatched on test {test}. Check \"output\" file")
    elif checker == 'sorted_lines':
        if sorted(to_cmp.strip().split(b'\n')) != sorted(res.strip().split(b'\n')):
            raise RuntimeError(f"Output missmatched on test {test}. Check \"output\" file")
    else:
        raise RuntimeError("Unknown checker " + checker)



parser = argparse.ArgumentParser()
parser.add_argument('--fill', action='store_true')
parser.add_argument('--output-file', required=False)
parser.add_argument('--source-file', default='solution.c')
parser.add_argument('--run-cmd', default='./solution')
parser.add_argument('--fix-style', action='store_true')
parser.add_argument('--checker', default='cmp')
parser.add_argument('--interactor', required=False)
args = parser.parse_args()

check_style(args.source_file, args.fix_style)

for test in sorted(Path('tests').glob('*.dat')):
    inf = Path(str(test.absolute()).removesuffix('.dat') + '.inf')
    ans = Path(str(test.absolute()).removesuffix('.dat') + '.ans')
    meta = {}
    if inf.is_file():
        with open(inf) as f:
            meta = parse_inf_file(f)
    if not ans.is_file() and not args.fill:
        raise RuntimeError("No answer for test " + test.name)
    res = run_solution(test, ans, inf, args.run_cmd, meta.get('params', ''), args.output_file, meta.get('environ'),
                       args.interactor)
    if not args.fill:
        try:
            res_checker(res, ans, args.checker)
        except:
            with open('output', 'wb') as f:
                f.write(res)
            raise
    else:
        with open(ans, 'wb') as fout:
            fout.write(res)

print("All tests passed")
