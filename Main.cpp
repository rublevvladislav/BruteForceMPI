#include <stdio.h>
#include "BruteFunc.h"

int main(int *argc, char **argv)
{
	int numtasks, rank;

	MPI_Init(argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	
	std::vector<std::string> passwords;

	//master thread	
	if (rank == 0) {
		std::cout << "master: There are " << numtasks - 1 << " slave processes" << std::endl;

		passwords = get_passwords_from_file("C:/Users/vlade/source/repos/BruteForceMpi/BruteForceMpi/database.txt");
		
		std::cout << "master: passwords lenght = " << passwords.size() << std::endl;

		double start = MPI_Wtime();

		if (passwords.empty())
		{
			std::cout << "File was not opened, terminating program." << std::endl;
			MPI_Abort(MPI_COMM_WORLD, 0);
			exit(0);
		}
		else
		{
			std::string hash = md5("omnipote");
			//split passwords on parts
			int sep =  passwords.size() / (numtasks - 1);
			int remainder = passwords.size() % (numtasks - 1);

			for (int i = 1; i < numtasks; i++) {
				MPI_Send(hash.c_str(), 200, MPI_CHAR, i, 100, MPI_COMM_WORLD);	// send the hash string to every slave
				int start = (i - 1)*sep, end = i * sep;
				int count = sep;
				if (numtasks == i + 1)
				{
					count = sep + remainder;
					end = passwords.size();
				}
				MPI_Send(&count, 1, MPI_INT, i, 150, MPI_COMM_WORLD);
				for (int j = start; j < end; j++)
				{
					MPI_Send(passwords[j].c_str(), 30, MPI_CHAR, i, 101, MPI_COMM_WORLD);// send every slave their allocated strings
				}
			}
		}

		char password[10];
		MPI_Status status;
		MPI_Recv(password, 10, MPI_CHAR, MPI_ANY_SOURCE, 200, MPI_COMM_WORLD, &status);	// blocking recieve waiting for any process to report

		std::cout << "Process " << status.MPI_SOURCE << " has cracked the password: " << password << std::endl;
		std::cout << "Terminating all processes" << std::endl;
		double end = MPI_Wtime();
		std::cout << "The process took " << end - start << " seconds to run." << std::endl;

		MPI_Abort(MPI_COMM_WORLD, 0);
	}
	else {
		char hash[200], password[30];
		int count;
		MPI_Recv(hash, 200, MPI_CHAR, 0, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	// recieve hash (same for every slave)
		MPI_Recv(&count, 1, MPI_INT, 0, 150, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //receive count of passwords to receive
		for (int i = 0; i < count; i++)
		{
			MPI_Recv(password, 30, MPI_CHAR, 0, 101, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	// recieve passwords (different for every slave)
			passwords.push_back(password);
		}

		runDictBrute(passwords, hash);
	}

	MPI_Finalize();
}