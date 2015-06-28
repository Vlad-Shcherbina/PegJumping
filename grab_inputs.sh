for i in `seq 1000`; do
  java -jar tester.jar -exec "./sol.sh" -novis -seed $i
  cp in.txt inputs/$i.txt
done
