/*
 * TestSobol.cxx
 *
 *  Created on: Jul 5, 2015
 *      Author: vgupta
 */

#define DIM_MAX 3
#include "../src/sobol.hpp"
#include "iostream"
int main()
{
	int dim_num=3;
	  int i;
	  int j;
	  double r[DIM_MAX];
	  long long int seed=0;
	  long long int seed_in;
	  long long int seed_out;

	  for ( i = 0; i <= 10; i++ )
	    {
//	      seed_in = seed;
	      i8_sobol ( dim_num, &seed, r );
//	      seed_out = seed;
	      std::cout << i << std::endl;
	      std::cout << r[0] << " " << r[1] << " " << r[2] << std::endl;
	    }


	return 0;
}
