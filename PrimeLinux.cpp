#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

bool IsPrime(int n){
	if (n < 2) return false;
	for (int d = 2; d * d <= n; d++)
		if (n % d == 0) return false;
	return true;
}

int main(){
	const int NUM_PROCESSES = 20;
	const int RANGE_SIZE = 1000;
	const int MAX_NUMBER = 10000;

	cout << "Numere prime intre 1 si " << MAX_NUMBER << ":\n";

	for (int i = 0; i < NUM_PROCESSES; i++){
		int p2c[2];
		int c2p[2];
		
		pipe(p2c);
		pipe(c2p);
		
		int pid = fork();

		if (pid == 0){
			close(p2c[1]);
			close(c2p[0]);

			int start, end;
			read(p2c[0], &start, sizeof(int));
			read(p2c[0], &end, sizeof(int));
			close(p2c[0]);

			for (int n = start; n <= end; n++){
				if (IsPrime(n)){
					int prime = n;
					write(c2p[1], &prime, sizeof(int));
				}
			}
			
			int done = -1;
			write(c2p[1], &done, sizeof(int));
			close(c2p[1]);

			return 0;
		}else{
			close(p2c[0]);
			close(c2p[1]);

			int start = i * RANGE_SIZE + 1;
			int end = (i + 1) * RANGE_SIZE;
			if (end > MAX_NUMBER) end = MAX_NUMBER;

			write(p2c[1], &start, sizeof(int));
			write(p2c[1], &end, sizeof(int));
			close(p2c[1]);
	
			while (true){
				int value;
				int bytes = read(c2p[0], &value, sizeof(int));
				if (bytes <= 0) break;
				if (value == -1) break;
				cout << value << " ";
			}

			close(c2p[0]);
			wait(NULL);
		}
	}

	cout << "\n";
	return 0;
}
