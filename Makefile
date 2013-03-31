secvential:
	g++ -Wall -lm -o secvential secvential/main.cpp
optims:
	g++ -Wall -lm -o optims optim_secvential/main.cpp
paralel:
	g++ -Wall -lm -fopenmp -o paralel simple_paralel/main.cpp
optimp:
	g++ -Wall -lm -fopenmp -o optimp optim_paralel/main.cpp
