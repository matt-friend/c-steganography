#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct bmpinfo{
	int height;
	int width;
	int bpp;
	int filesize;
	int offset;
	int sizefromheader;
};

typedef struct bmpinfo bmpinfo;

unsigned int encodeBit(unsigned int inbyte, unsigned int outbyte, int i){
	//get bit to be encoded
	unsigned int val = inbyte & (1 << (7-i)); 
	//bit to be encoded is one
	if (val != 0) outbyte = outbyte | 0x1;
	//bit to be encoded is 0
	else outbyte = outbyte & 0xFE;
	return outbyte;
}

int decodeBit(unsigned int inbyte){
	if ((inbyte & 0x01) == 0) return 0;
	else return 1;
}

void getHeaderInfo(FILE *image, bmpinfo *binf){
	fseek(image,0,SEEK_SET);
	unsigned int b = fgetc(image);
	unsigned int header[14];
	for(int i = 0; i < 14; i++){
		header[i] = b;
		b = fgetc(image);
	}
	int filesize = header[2]+header[3]*256+header[4]*65536;
	int offset =  header[10]+header[11]*256+header[12]*65536;
	binf -> filesize = filesize;	
	binf -> offset = offset;
}

void getBmpInfo(FILE *image, bmpinfo *binf){
	fseek(image,14,SEEK_SET);
	unsigned int b = fgetc(image);
	unsigned int imgInfoHdr[40];
	for(int i = 0; i < 40; i++){
		imgInfoHdr[i] = b;
		b = fgetc(image);
	}
	int width = imgInfoHdr[4]+imgInfoHdr[5]*256+imgInfoHdr[5]*65536;
	int height = imgInfoHdr[8]+imgInfoHdr[9]*256+imgInfoHdr[10]*65536;
	int bpp = imgInfoHdr[14]+imgInfoHdr[15]*256;
	int imgsize = width*height*bpp/8;
	binf -> height = height;
	binf -> width = width;
	binf -> bpp = bpp;
	binf -> sizefromheader = imgsize;
}

void encodeText(FILE *img, bmpinfo *imgInfo, FILE *txt, FILE *out){
	fseek(img,0,SEEK_SET);
	unsigned char b = fgetc(img);
	unsigned char c = fgetc(txt);
	for(int i = 0; i < imgInfo->offset; i++){
		fputc(b,out);
		b = fgetc(img);
	}
	while (!feof(txt)){
		for(int i = 0; i<8; i++){
			fputc(encodeBit(c,b,i),out);
			b = fgetc(img);
		}
		c = fgetc(txt);
	}
	while (!feof(img)){
		fputc(b,out);
		b = fgetc(img);
	}

}

void decodeBmp(FILE *img, bmpinfo *imgInfo, FILE *out){
	fseek(img, imgInfo -> offset, SEEK_SET);
//	bool endOfText = false;
	unsigned int b = fgetc(img);
	int k;
	for(int i = 0; i < 200; i++){
		unsigned int c = 0;
		for (int j = 0; j<8; j++){
			k = decodeBit(b);
			c = (c << 1) | k;
			b = fgetc(img);

		}
		fputc(c, out);
	}
}

void usage(){
	printf("Usage:\nIf encoding image use format * ./bmp \"e\" x y z * where x is the orginal image to be used, y is the plaintext to be encoded into the image and z is the file name for the image being output\nIf decoding an image use format * ./bmp \"d\" x y * where x is the encoded image and y is the file where the message is to be output\n");
}

int main (int n, char *args[n]) {
	bmpinfo *imgInfo = malloc(sizeof(bmpinfo));
	printf("%d\n",n);
	if((strcmp(args[1], "e"))==0){
		if (n==5){
			printf("%s, %s, %s, %s\n", args[1], args[2], args[3], args[4]);
			FILE *originalImage = fopen(args[2], "r");
			FILE *plainText = fopen(args[3], "r");
			FILE *encodedImage = fopen(args[4], "w");
			getHeaderInfo(originalImage, imgInfo);
			getBmpInfo(originalImage, imgInfo);
			encodeText(originalImage, imgInfo, plainText, encodedImage);
			fclose(originalImage);
			fclose(plainText);
			fclose(encodedImage);
		}
		else usage();
	}
	else if((strcmp(args[1], "d"))==0){
		if (n==4){
			FILE *encodedImage = fopen(args[2], "r");
			FILE *outputText = fopen(args[3], "w");
			getHeaderInfo(encodedImage, imgInfo);
			getBmpInfo(encodedImage, imgInfo);
			decodeBmp(encodedImage, imgInfo, outputText);
			fclose(encodedImage);
			fclose(outputText);
		}
		else usage();
	}
	else usage();
	free(imgInfo);
	return -1;
}
