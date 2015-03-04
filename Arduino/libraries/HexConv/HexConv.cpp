#include "HexConv.h"

int hexToDec(String hexIn){
	int digit[2];
	String chars[2];
	
	chars[0]	= hexIn.substring(0, 1);
	chars[1] 	= hexIn.substring(1, 2);

	for(int i = 0; i < 2; i++){
		const char* tempChar;
		tempChar = chars[i].c_str();
		int tempInt = atoi(tempChar);
		
		if (tempInt >= 0 || tempInt <= 9){
			digit[i] = tempInt;
		}
		
		if (chars[i] == "A"){
			digit[i] = 10;
		} 
		else if (chars[i] == "B"){
			digit[i] = 11;
		} 
		else if (chars[i] == "C"){
			digit[i] = 12;
		} 
		else if (chars[i] == "D"){
			digit[i] = 13;
		} 
		else if (chars[i] == "E"){
			digit[i] = 14;
		} 
		else if (chars[i] == "F"){
			digit[i] = 15;
		}
	}
	
	int out = digit[0] * 16;
	out += digit[1];
	
	return out;
}