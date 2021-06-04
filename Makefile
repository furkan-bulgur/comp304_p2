tv_debate:
	g++ -o tv_debate.o tv_debate.cpp -lpthread
run:
	g++ -o tv_debate.o tv_debate.cpp -lpthread && ./tv_debate.o -n 4 -p 0.75 -q 5 -t 3 -b 0.05
