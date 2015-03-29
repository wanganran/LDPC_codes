#include <iostream>
#include <functional>
#include "LDPC_decoder.h"
#include "LDPC_encoder.h"
#include "LDPC_generator.h"
using namespace std;


struct symbol_s{
	bool* symbol;
	int len;
	symbol_s() :symbol(NULL), len(0){}
	symbol_s(int _len, const bool* _content):len(_len), symbol(new bool[_len]){
		if(_content)memcpy_s(symbol, len, _content, len);
	}
	~symbol_s(){ if(symbol)delete[] symbol; }
	symbol_s& operator ^=(const symbol_s& s){
		if (!symbol){
			symbol = new bool[s.len];
			memset(symbol, 0, sizeof(bool)*s.len);
			len = s.len;
		}
		assert(len == s.len);
		for (int i = 0; i < len; i++)
			symbol[i] ^= s.symbol[i];
		return *this;
	}
	symbol_s& operator = (const symbol_s& s){
		if (symbol)delete[] symbol;
		len = s.len;
		if (!len)symbol = NULL;
		else{
			symbol = new bool[s.len];
			memcpy_s(symbol, len, s.symbol, len);
		}
		return *this;
	}
	void reset(int len, const bool* content){
		this->len = len;
		if (this->symbol)delete[] this->symbol;
		this->symbol = new bool[len];
		if (content)memcpy_s(this->symbol, len, content, len);
	}
};

int main(int argc, char** args){
	if (argc <= 1)return -1;
	if (strcmp(args[1], "prepare")==0){
		int code_len = atoi(args[2]);
		int msg_len = atoi(args[3]);
		char* plist_out = args[4];
		char* parity_out = args[5];
		LDPC_generator* gen;
		if (strcmp(args[6], "quasicyclic") == 0){
			int Wr = atoi(args[7]);
			int Wc = atoi(args[8]);
			int seed = atoi(args[9]);
			gen = new LDPC_QuasiCyclic_generator(Wr, Wc, seed);
		}
		else if (strcmp(args[6], "arrayregular") == 0){
			int Wr = atoi(args[7]);
			int Wc = atoi(args[8]);
			gen = new LDPC_array_generator(Wr, Wc, false);
		}
		else if (strcmp(args[6], "arrayirregular") == 0){
			int Wr = atoi(args[7]);
			int Wc = atoi(args[8]);
			gen = new LDPC_array_generator(Wr, Wc, true);
		}
		else if (strcmp(args[6], "gallager") == 0){
			int Wr = atoi(args[7]);
			int Wc = atoi(args[8]);
			int seed = atoi(args[9]);
			gen = new LDPC_Gallager_generator(Wr, Wc, seed, true);
		}
		else if (strcmp(args[6], "plist") == 0){
			gen = new LDPC_plist_generator(args[7]);
		}
		else return -2;
		LDPC_encoder* enc = new LDPC_encoder(code_len, msg_len);
		int act;
		if (!enc->init(gen, act))return -3;
		enc->save_parities_to(parity_out);
		gen->save_to_plist(plist_out);
		return 0;
	}
	else if (strcmp(args[1], "encodearbitary")==0){
		int code_len = atoi(args[2]);
		int msg_len = atoi(args[3]);
		char* parity_in = args[4];
		int len = atoi(args[5]);
		LDPC_encoder* enc = new LDPC_encoder(code_len, msg_len);
		if (!enc->init(parity_in))return -3;
		symbol_s* in = new symbol_s[msg_len];
		symbol_s* out = new symbol_s[code_len];
		while (true){
			for (int i = 0; i < msg_len; i++){
				in[i].reset(len, NULL);
				for (int j = 0; j < len; j++){
					int c = getc(stdin);
					if (c == EOF)goto RET1;
					in[i].symbol[j] = c == '1';
				}
			}
			enc->encode<symbol_s>(in, msg_len, out);
			for (int i = 0; i < code_len; i++)
				for (int j = 0; j < len; j++)
					putc(out[i].symbol[j] ? '1' : '0', stdout);
		}
		RET1:
		return 0;
	}
	else if (strcmp(args[1], "encodebin") == 0){
		int code_len = atoi(args[2]);
		int msg_len = atoi(args[3]);
		char* parity_in = args[4]; 
		LDPC_encoder* enc = new LDPC_encoder(code_len, msg_len);
		if (!enc->init(parity_in))return -3;
		bit_array_t in(msg_len);
		bit_array_t out(code_len);
		while (true){
			for (int i = 0; i < msg_len; i++){
				int c = getc(stdin);
				if (c == EOF)goto RET2;
				in.set(i, c == '1');
			}
			enc->encode(in, out);
			for (int j = 0; j < code_len; j++){
				putc(out[j] ? '1' : '0', stdout);
			}
		}
		RET2:
		return 0;
	}
	else if (strcmp(args[1], "decodebsc") == 0){
		int code_len = atoi(args[2]);
		int msg_len = atoi(args[3]);
		char* plist_in = args[4];
		double p = atof(args[5]);
		LDPC_bp_decoder* dec = new LDPC_bp_decoder(code_len, msg_len);
		if (!dec->init(plist_in))return -3;
		bit_array_t in(code_len);
		while (true){
			for (int i = 0; i < code_len; i++){
				int c = getc(stdin);
				if (c == EOF)goto RET3;
				in.set(i, c == '1');
			}
			if (!dec->decode_BSC(in, p, 30))putc('0', stdout);
			else{
				putc('1', stdout);
				for (int i = 0; i < code_len; i++){
					putc(in[i] ? '1' : '0', stdout);
				}
			}
		}
		RET3:
		return 0;
	}
	else if (strcmp(args[1], "decodebec") == 0){
		int code_len = atoi(args[2]);
		int msg_len = atoi(args[3]);
		char* plist_in = args[4];
		int len = atoi(args[5]);
		LDPC_bp_decoder* dec = new LDPC_bp_decoder(code_len, msg_len);
		if (!dec->init(plist_in))return -3;
		bit_array_t mask(code_len);
		symbol_s* in = new symbol_s[code_len];
		while (true){
			//first:mask
			for (int i = 0; i < code_len; i++){
				int c = getc(stdin);
				if (c == EOF)goto RET4;
				mask.set(i, c == '1');
			}
			for (int i = 0; i < code_len; i++){
				in[i].reset(len, NULL);
				for (int j = 0; j < len; j++){
					int c = getc(stdin);
					if (c == EOF)goto RET4;
					in[i].symbol[j] = (c == '1');
				}
			}
			if (!dec->decode_BEC<symbol_s>(in, mask))putc('0', stdout);
			else{
				putc('1', stdout);
				for (int i = 0; i < code_len; i++){
					for (int j = 0; j < len; j++){
						putc(in[i].symbol[j] ? '1' : '0', stdout);
					}
				}
			}
		}
		RET4:
		return 0;
	}
	else return -2;
}