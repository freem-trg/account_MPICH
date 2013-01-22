#include<stdio.h>
#include<mpi.h>
#include<iostream>
#include <stdlib.h>
#include <cstdlib>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[])
{	
	//For MPI
    MPI_Status status;
    int pid;	
    int TAG = 0;
    int threads = 0;

    // for account:
    int balance = 0; //curent balance
    int debt = 0; // current debt

    //for users:
    const int MAX_SLEEP = 5;
    const int MAX_MONEY = 50;

    //Enter the MPI:
    MPI_Init( &argc, &argv );
    MPI_Comm_size(MPI_COMM_WORLD, &threads);
	MPI_Comm_rank( MPI_COMM_WORLD, &pid);
	if ( pid != 0){
        //Users code here:
        while (true){
            //For rand:
            srand( time(NULL) + pid );
            sleep(rand() % MAX_SLEEP + 1); // wait for [1..5] sec and make request to account
            MPI_Request request;
            MPI_Status st;
            int money = rand() % ( MAX_MONEY * 2 ) - MAX_MONEY; // random [-49..50 ]
            if ( money > 0 ) {
                cout << pid << " want to put " << money << " to balance" << endl;
            } else {
                cout << pid << " want to take " << money*(-1) << " from balance" << endl;
            }
            MPI_Isend( &money, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &request);
            MPI_Wait( &request, &st );
            sleep(rand() % MAX_SLEEP + 1); // wait for [1..5] sec
        }
	} else {
        //Account code here:
        while (true){ //wath every 1 sec if there new messages:
            for (int i = 1; i < threads; i++){
                int buf = 0;
                int flag;
                MPI_Request req;
                MPI_Status st;
                MPI_Irecv( &buf, 1, MPI_INT, i, TAG, MPI_COMM_WORLD, &req );
                MPI_Test( &req, &flag, &st  );
                if ( buf != 0 ){ //message came
                    if ( buf > 0 ){
                        balance += buf;
                        cout << buf << "$ successfully enrolled. Now : " << balance << " $ " << endl;
                        if( debt > 0 && balance > debt){
                            debt = 0;
                            balance -= debt;
                            cout << debt << "$ have been removed for debt. Now : " << balance << " $ " << endl;
                        }
                    } else {
                        if ( balance > buf*(-1)){ // we have more than need.
                            balance -= buf*(-1);
                            cout << buf*(-1) << "$ successfully removed. Now :" << balance << " $ " << endl;
                        } else { // we'll have debt
                            debt += buf*(-1);
                            cout << "There is not enough money. Debt now : " << debt << " $ " << endl;
                        }
                    }
                    buf = 0;
                }
            }
            sleep(1);
        }
    }
    MPI_Finalize();
}

