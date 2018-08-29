all: bigresult bigoutput bigcompare
a: aa
	meld test/IncreasingStuffResult.txt test/IncreasingStuffOut.txt
aa: compile
	cat test/IncreasingStuff.txt | ./p > test/IncreasingStuffResult.txt
b: bb
	meld test/FancyLoopsResult.txt test/FancyLoopsOut.txt
bb: compile
	cat test/FancyLoops.txt | ./p > test/FancyLoopsResult.txt
c: cc
	meld test/MindYourLeftResult.txt test/MindYourLeftOut.txt
cc: compile
	cat test/MindYourLeft.txt | ./p > test/MindYourLeftResult.txt
d: dd
	meld test/UnionStuckResult.txt test/UnionStuckOut.txt
dd: compile
	cat test/UnionStuck.txt | ./p > test/UnionStuckResult.txt
e: ee
	meld test/DontGetLostResult.txt test/output_public.txt
ee: compile
	cat test/input_public | ./p > test/DontGetLostResult.txt
f: ff
	meld test/tocornottocResult.txt test/tocornottocOut.txt
ff: compile
	cat test/tocornottoc.txt | ./p > test/tocornottocResult.txt
val: compileval
	cat test/IncreasingStuff.txt | valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./p
valb: compileval
	cat test/FancyLoops.txt | valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./p
valc: compileval
	cat test/MindYourLeft.txt | valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./p
vald: compileval
	cat test/UnionStuck.txt | valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./p
vale: compileval
	cat test/input_public | valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./p
valf: compileval
	cat test/tocornottoc.txt | valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./p
compile:
	gcc -DEVAL -std=c11 -O2 -pipe -static -s -o p main.c -lm
compileval:
	gcc -g -o p main.c
compileval1:
	gcc -g -DEVAL -std=c11 -O2 -pipe -s -o p main.c -lm
bigresult: aa bb cc dd ee ff
	cat test/IncreasingStuffResult.txt test/FancyLoopsResult.txt test/MindYourLeftResult.txt test/UnionStuckResult.txt test/DontGetLostResult.txt test/tocornottocResult.txt > test/bigresult.txt
bigoutput:
	cat test/IncreasingStuffOut.txt test/FancyLoopsOut.txt test/MindYourLeftOut.txt test/UnionStuckOut.txt test/output_public.txt test/tocornottocOut.txt >test/bigoutput.txt
bigcompare:
	meld test/bigresult.txt test/bigoutput.txt
	
valreport: compileval
	cat test/IncreasingStuff.txt | valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all     --log-file="valreport/1_1" ./p
	cat test/FancyLoops.txt | valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all 			--log-file="valreport/2_1" ./p
	cat test/MindYourLeft.txt | valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all 		--log-file="valreport/3_1" ./p
	cat test/UnionStuck.txt | valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all 			--log-file="valreport/4_1" ./p
	cat test/input_public | valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all 			--log-file="valreport/5_1" ./p
	cat test/tocornottoc.txt | valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all 		--log-file="valreport/6_1" ./p
performances:
	meld valreport/1_0 valreport/1_1
	meld valreport/2_0 valreport/2_1
	meld valreport/3_0 valreport/3_1
	meld valreport/4_0 valreport/4_1
	meld valreport/5_0 valreport/5_1
	meld valreport/6_0 valreport/6_1
prova:compileval
	cat test/tocornottoc.txt | valgrind --tool=massif --stacks=yes ./p


