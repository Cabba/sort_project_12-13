============
COMPILAZIONE
============

Compilazione dei modi di funzionamento:

make no-dmiss: 
	compila l'executive con una schedule dei task perodici senza deadline miss.
make no-dmiss-sporadic: 
	compila l'executive con una schedule dei task periodici più un task sporadico senza deadline miss
make dmiss-task: 
	compila l'executive con una schedule dei task periodici in cui può verificarsi una deadline miss.
make dmiss-sporadic: 
	compila l'executive con una schedule dei task periodici più un task sporadico con deadine miss.
