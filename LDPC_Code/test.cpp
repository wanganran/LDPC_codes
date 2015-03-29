#include <iostream>
#include <functional>

#include "LDPC_decoder.h"
#include "LDPC_encoder.h"
#include "LDPC_generator.h"
#include "benchmark.h"
using namespace std;

bit_array_t generate_bitarr(int len, function<bool(int)> gen_func){
	bit_array_t res(len);
	res.clear();
	for (int i = 0; i < len; i++)
		res.set(i, gen_func(i));
	return res;
}

void print_bitarr(const bit_array_t& src){
	for (int i = 0; i < src.size(); i++)
		printf(src[i] ? "1" : "0");
	printf("\n");
}
void print_matrix(const binary_matrix& mat){
	for (int i = 0; i < mat.height(); i++){
		auto bitarr = mat[i];
		print_bitarr(bitarr);
	}
	printf("\n");
}


int main(){
	
	auto res = do_benchmark(996, 498, new LDPC_QuasiCyclic_generator(6,3,3), 0.1f, 0.0f, 10, 50);
	res->output();

 	return 0;


#if 0
	//first generate
	auto gen = new LDPC_array_generator(4, 2, false);
	LDPC_encoder* encoder = new LDPC_encoder(16, 8);
	bit_array_t src = generate_bitarr(8, [](int i){return i % 3 == 0; });
	bit_array_t dest(16);

	int actual_msg_len;

	gen->init(16, 8);
	print_matrix(*(gen->as_binary_matrix()));
	print_matrix(*to_binary_matrix(gen->as_Tanner_graph(),16,8));

	if (encoder->init(gen,actual_msg_len)){
		printf("%d\n", actual_msg_len);
		print_matrix(*(gen->as_binary_matrix()));
		printf("\n");

		encoder->encode(src, dest);
		print_bitarr(src);
		print_bitarr(dest);
	}

	LDPC_bp_decoder* decoder = new LDPC_bp_decoder(16, 8);
	
	if (decoder->init(gen)){
		printf("%d\n", decoder->check(dest));
	}

	//dest.xor(2, true);

	//dest.xor(4, true);
	//dest.xor(6, true);

	double prob[16];

	for (int i = 0; i < 16; i++){
		if (dest[i])prob[i] = 0.9;
		else prob[i] = 0.1;
	}
	prob[4] = 0.11;
	prob[6] = 0.11;


	if (decoder->decode_BSC(dest, prob, 50)){
		print_bitarr(dest);
	}
	printf("\n");
	bool mask[16];
	for (int i = 0; i < 16; i++)mask[i] = true;
	mask[3] = mask[7] = mask[10]=mask[11]=false;
	dest.xor(3, true);
	dest.xor(7, true);
	dest.xor(10, true);
	dest.xor(11, true);
	if (decoder->decode_BEC(dest, bit_array_t(mask,16))){
		print_bitarr(dest);
	}
#endif
}