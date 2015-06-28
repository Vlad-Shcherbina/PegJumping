ulimit -v 1000000
tee in.txt | ./a.out | tee log.txt
cat log.txt 1>&2
