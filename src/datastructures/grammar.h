#ifndef GRAMMAR_H
#define GRAMMAR_H
#include "../common.h"

class Grammar {
	private:
		char rawLabel[256][GRAMMAR_STR_LEN];    // rawLabel[0]~rawLabel[255] -> (char)-128 ~ (char)127
		int numRawLabels;
		bool erules[256];          				// e-rule
		char rules[65536];		   				// s-rule && d-rule
												// rules[0]~rules[65535] -> (short)-32768 ~ (short)32767

	public:
		Grammar();
		bool loadGrammar(char *filename);
		int addRawLabel(char *label);
		char getLabelValue(char *str);

		int getNumErules();
		void myTrim(char *src);
		inline short changeShort(char a,char b);
		void test();
};

#endif
