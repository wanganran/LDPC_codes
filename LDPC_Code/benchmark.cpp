#include "benchmark.h"
#include <functional>
#include <chrono>

using namespace std;

static chrono::high_resolution_clock::time_point get_tick(){
	return chrono::high_resolution_clock::now();
}
static double get_duration(chrono::high_resolution_clock::time_point& t1, chrono::high_resolution_clock::time_point& t2){
	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	return time_span.count();
}

static bit_array_t generate_bitarr(int len, function<bool(int)> gen_func){
	bit_array_t res(len);
	res.clear();
	for (int i = 0; i < len; i++)
		res.set(i, gen_func(i));
	return res;
}
static void __rand_n_k(int n, int k, int* dest){
	memset(dest, 0, sizeof(int)*k);

	if (n <= k*k)
	{
		//O(n) implementation
		int t = 0;
		for (int i = 0; i < n && t < k; i++){
			if (rand()*(n - i) < (k - t)*RAND_MAX)
				dest[t++] = i;
		}
	}
	else
	{
		//O(k^2) implementation
		for (int j = 0; j < k; j++)
		{
			int x = rand() % n;
			for (int t = 0; t < j; t++)
				if (dest[t] == x){
					x = rand() % n;
					t = -1;
				}
			dest[j] = x;
		}
	}
}

void rand_flip(bit_array_t& arr, int num){
	int* place = new int[num];
	__rand_n_k(arr.size(), num, place);
	for (int i = 0; i < num; i++)
		arr.xor(place[i], true);
}

float get_ber(bit_array_t& truth, bit_array_t& fact){
	int error = 0;
	for (int i = 0; i < truth.size(); i++)
		if (truth[i] != fact[i])error++;
	return error / (float)truth.size();
}

const int duplication = 3;

unique_ptr<benchmark_s> do_benchmark(int code_len, int msg_len, LDPC_generator* gen, float max_BER, float min_BER, int case_num, int iteration){
	bit_array_t src = generate_bitarr(msg_len, [](int i){return rand() % 2 == 0; });
	bit_array_t dest(code_len);

	int actual_msg_len=0;
	unique_ptr<benchmark_s> res(new benchmark_s());
	res->case_num = case_num;
	res->BER_after = new float[case_num];
	res->BER_before = new float[case_num];
	res->failed_cases = new int[case_num];
	res->BER_after_succeed = new float[case_num];

	srand((int)time(0));

	auto t0 = get_tick();
	LDPC_encoder* encoder = new LDPC_encoder(code_len, msg_len);
	if (
		encoder->init(gen, actual_msg_len)
		//encoder->init("parities.txt")
		){
		auto t1 = get_tick();
		double d1 = get_duration(t0, t1);
		res->encoding_prepare_time = (int)(d1 * 1000);
		res->rate = msg_len / (float)code_len;
		res->actual_rate = actual_msg_len / (float)code_len;
		auto t2 = get_tick();
		encoder->encode(src, dest);
		auto t3 = get_tick();
		res->encoding_time_per_100_codes = (int)(get_duration(t2, t3) * 100000);

	}
	else return res;

	LDPC_bp_decoder* decoder = new LDPC_bp_decoder(code_len, msg_len);

	auto t4 = get_tick();
	if (
		//decoder->init("20000.10000.3.631.alist")
		decoder->init(gen)
		){
		auto t5 = get_tick();
		res->decoding_prepare_time = (int)(get_duration(t4, t5) * 1000);
		double decoding_time = 0;
		if (decoder->check(dest)){
			for (int i = 0; i < case_num; i++){
				float ber = min_BER + (max_BER - min_BER) / (case_num - 1)*i;
				res->BER_before[i] = ber;
				res->BER_after[i] = 0;
				res->failed_cases[i] = 0;
				res->BER_after_succeed[i] = 0;
				for (int j = 0; j < duplication; j++){
					bit_array_t dest_copy = dest;
					rand_flip(dest_copy, (int)(dest_copy.size()*ber+0.5));
					auto t6 = get_tick();
					if (!decoder->decode_BSC(dest_copy, ber, iteration)){
						res->failed_cases[i]++;
					}
					else res->BER_after_succeed[i] += get_ber(dest, dest_copy);
					auto t7 = get_tick();
					decoding_time += get_duration(t6, t7);
					res->BER_after[i] += get_ber(dest, dest_copy);
				}
				res->BER_after[i] /= duplication;
				res->BER_after_succeed[i] /= duplication - res->failed_cases[i];
			}
			res->decoding_time_per_100_codes = (int)(decoding_time / (case_num *duplication / 100.0) * 1000);
		}
	}
	return res;
}