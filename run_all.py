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

        score = None
        for line in out.splitlines():
            m = re.match(r'Score = (\d+)$', line)
            if m is not None:
                score = int(m.group(1))

        assert score is not None, '\n' + out

        return dict(
            score=score,
            seed=str(seed),
            time=default_timer() - start)

    except Exception as e:
        raise Exception('seed={}, out={}, err={}'.format(seed, out, err)) from e


def worker(task):
    return run_solution(*task)


def main():
    subprocess.check_call(
        'g++ --std=c++11 main.cc -Wall -Wno-sign-compare -o main',
        shell=True)
    command = './main'

    tasks = [(command, seed) for seed in range(3, 13)]

    map = multiprocessing.Pool(5).imap

    with run_db.RunRecorder() as run:
        for result in map(worker, tasks):
            print(result['seed'], result['score'])
            run.add_result(result)
            run.save()


if __name__ == '__main__':
    main()
