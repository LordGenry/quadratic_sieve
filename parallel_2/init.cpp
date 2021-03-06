#include <mpi.h>
#include "parameters.hpp"
#include "init.hpp"
#include "mod_arithm.hpp"

namespace quadratic_sieve{
	int B;	
	int base_size=0;
	int *base;
	int u;
	
	
	boost::multiprecision::cpp_int n;
	boost::multiprecision::cpp_int sqrt_n;
	std::vector<cpp_int> factors;
	std::string str_n;

	int MAX_SIZE_N=1001; // maximal size for number n is 1000 decimal places

	/*** Input the number to be factored. The input is stored in 
	 * parameters.hpp variable n. ***/
	bool input_number(){
		//str_n="1522605027922533360535618378132637429718068114961380688657908494580122963258952897654000350692006139";
		str_n="72328663292947";
		//str_n="11869840501";
		//std::cout<< "Please enter the number you wish to factorize: " ;
		// std::cin >> str_n;
		n=cpp_int(str_n); // the number to be factorized 
		sqrt_n=sqrt(n);
		return true;
	}




	void broadcast_number(){
		int sz=str_n.size();
		MPI_Status status;
		std::cout.flush();
		MPI_Request req;
		MPI_Ibcast(&sz,1,MPI_INT,MY_ROOT,MPI_COMM_WORLD,&req);  // no need for waiting, non-blocking call
		char *n_c_str=new char[str_n.size()+1];
		strcpy(n_c_str,str_n.c_str());
		MPI_Wait(&req,&status);
		MPI_Bcast(n_c_str,str_n.size()+1,MPI_CHAR,MY_ROOT,MPI_COMM_WORLD);
	}


	void recieve_number(){
		int sz;
		MPI_Request req;
		MPI_Status status;
		MPI_Ibcast(&sz,1,MPI_INT,MY_ROOT,MPI_COMM_WORLD,&req);
		MPI_Wait(&req,&status);
		char* n_c_str=new char[sz+1];
		MPI_Bcast(n_c_str,sz+1,MPI_CHAR,MY_ROOT,MPI_COMM_WORLD);
		str_n=std::string(n_c_str);
		n=cpp_int(str_n);
		sqrt_n=sqrt(n);
		std::cout << n << std::endl;
		std::cout.flush();
	}


	/*** Set up parameters, like smoothness bound etc.
	 * The smoothness bound size will depend only on the size of n
	 * At the later stage we can also set up different polynomials for
	 * sieveing process(currently we are only using x^2-n.
	 ***/
	void setup_parameters(){  
		B=5700000;
		u=1550000;
	}

	/*** For generating all primes, we use Erathostenes sieve. At this stage we also use Tonelli-Shanks
	 * algorithm to check whether the n is quadratic residual modulo respective primes.
	 ***/
	void generate_factor_base(){
		bool *composite=(bool*)malloc(sizeof(bool)*(B+1));  // this can be optimized with bitmasks, but there is no need
																								// since this part is not a bottleneck of algorithm	
		for( int i=0;i<=B;i++)
			composite[i]=false;
		int sqrt_B=sqrt(B);
		for ( int i=2;i<sqrt_B;i++){
			if ( !composite[i] ) {
				int tmp_sum=i*2;  // change multiplication with addition
				while ( tmp_sum <= B){
					composite[tmp_sum]=true;
					tmp_sum+=i;
				}
			}	
		}	
		// prime numbers are generated
	

		// check if n is quadratic residual module candidates for prime numbers	
		// by calculating Legendre symbol
		//
		// We can parallelize this part
		
		bool *good_candidate=(bool*)malloc(sizeof(bool)*(B+1));  // this can be optimized with bitmasks, but there is no need
		for ( int i=0;i<=B;i++)
			good_candidate[i]=false;
		/*** check divisibility ***/
		for ( int i=2;i<=B;i++){
			if ( !composite[i] ){
				while ( n%i==0 ){
					n/=i;
					factors.push_back(i);
					printf("FACTOR %d\n",i);
				}
			}
		}
		std::cout<< n << std::endl;

		for ( int i=2;i<=B;i++){
			if ( !composite[i] && n_is_quadratic_residual(i) ){
				good_candidate[i]=true;	
				base_size++;
			}
		}
		base=(int*)malloc(base_size*sizeof(int));	
		int tmp=0;
		for ( int i=2;i<=B;i++){
			if( !composite[i] && good_candidate[i]) 
				base[tmp++]=i;	
			}
		free(composite);
		free(good_candidate);
	}
}
