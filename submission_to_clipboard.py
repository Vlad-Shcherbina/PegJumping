import subprocess
import re

p = subprocess.Popen('xsel --clipboard --input', shell=True, stdin=subprocess.PIPE)

text = open('sol.cc').read()

def replacer(m):
  return open(m.group(1)).read()

for _ in range(3):
  text = re.sub(r'#include "(.+\.h)"', replacer, text)

text = '#define SUBMISSION\n\n' + text

p.communicate(text.encode())
ret = p.wait()
assert ret == 0
print('solution ({} bytes) copied to clipboard'.format(len(text)))
