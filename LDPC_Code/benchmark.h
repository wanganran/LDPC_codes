#include "LDPC_decoder.h"
#include "LDPC_encoder.h"
#include <iostream>
#include <memory>

struct benchmark_s{
	float rate;
	float actual_rate;
	int encoding_prepare_time;
	int decoding_prepare_time;
	int encoding_time_per_100_codes;
	int decoding_time_per_100_codes;
	int case_num;
	float *BER_before;
	float *BER_after;
	int* failed_cases;
	float* BER_after_succeed;

	void output(){
		std::cout << "Rate:\t" << rate << std::endl <<
			"Actual rate:\t" << actual_rate << std::endl <<
			"Encoding prepare time:\t" << encoding_prepare_time << std::endl <<
			"Decoding prepare time:\t" << decoding_prepare_time << std::endl <<
			"Encoding time per 100 codes:\t" << encoding_time_per_100_codes << std::endl <<
			"Decoding time per 100 codes:\t" << decoding_time_per_100_codes << std::endl <<
			"BER change:" << std::endl;
		for (int i = 0; i < case_num; i++){
			std::cout << BER_before[i] << "\t-->\t" << BER_after[i] << "\t" << BER_after_succeed[i] << "\t" << failed_cases[i] << std::endl;
		}
	}
	benchmark_s() :BER_before(NULL), BER_after(NULL), failed_cases(NULL), BER_after_succeed(NULL){}
	~benchmark_s(){
		if (BER_before)delete[] BER_before;
		if (BER_after)delete[] BER_after;
		if (failed_cases)delete[] failed_cases;
		if (BER_after_succeed)delete[] BER_after_succeed;
	}
};
std::unique_ptr<benchmark_s> do_benchmark(int code_len, int msg_len, LDPC_generator* gen, float max_BER, float min_BER, int case_num, int iteration);