import ast
import sys
import multiprocessing
import os
import re
import subprocess
from timeit import default_timer
import multiprocessing
import pprint

import run_db


def run_solution(command, seed):
    try:
        start = default_timer()
        p = subprocess.Popen(
            'java -jar tester.jar -exec "{}" '
            '-novis -seed {}'.format(command, seed),
            shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        out, err = p.communicate()
        out = out.decode()
        err = err.decode()

        p.wait()
        assert p.returncode == 0

        result = dict(
            seed=str(seed),
            time=default_timer() - start)

        #print('out', out)
        #print('err', err)
        for line in out.splitlines() + err.splitlines():
            #print(line)
            m = re.match(r'Score = (\d+)$', line)
            if m is not None:
                result['score'] = int(m.group(1))

            m = re.match(r'# (\w+) = (.*)$', line)
            if m is not None:
                result[m.group(1)] = ast.literal_eval(m.group(2))

        assert 'score' in result
        return result

    except Exception as e:
        raise Exception('seed={}, out={}, err={}'.format(seed, out, err)) from e


def worker(task):
    return run_solution(*task)


def main():
    subprocess.check_call(
        'g++ --std=c++11 -Wall -Wno-sign-compare -O2 main.cc -o main',
        shell=True)
    command = './main'

    tasks = [(command, seed) for seed in range(1, 201)]

    map = multiprocessing.Pool(5).imap

    with run_db.RunRecorder() as run:
        for result in map(worker, tasks):
            print(result['seed'], result['score'])
            run.add_result(result)
            run.save()


if __name__ == '__main__':
    main()
