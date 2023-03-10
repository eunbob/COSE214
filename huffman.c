#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "node.h"
#include "heap.h"
#define BUF_SIZE 128

// 허프만 트리를 순회하며 허프만 코드를 생성하여 codes에 저장
// leaf 노드에서만 코드를 생성
// strdup 함수 사용
// make_huffman_code 함수에서 호출
static void traverse_tree( tNode *root, char *code, int depth, char *codes[])
{
	if(root->right || root->left){
		if(root->left){
			code[depth] = '0';
			traverse_tree(root->left, code, depth + 1, codes);
		}
		
		if(root->right){
			code[depth] = '1';
			traverse_tree(root->right, code, depth + 1, codes);
		}
	}
	
	else{
		code[depth] = '\0';
		codes[root->data] = strdup(code);
	}
	
}

// 새로운 노드를 생성
// 좌/우 subtree가 NULL(0)이고 문자(data)와 빈도값(freq)이 저장됨
// make_huffman_tree 함수에서 호출
// return value : 노드의 포인터
static tNode *newNode(unsigned char data, int freq)
{
	tNode* newNode = (tNode*)malloc(sizeof(tNode));
	
	newNode->data = data;
	newNode->freq = freq;
	newNode->right = NULL;
	newNode->left = NULL;
	
	return newNode;
}

////////////////////////////////////////////////////////////////////////////////
// 허프만 코드를 화면에 출력
void print_huffman_code( char *codes[])
{
	int i;
	
	for (i = 0; i < 256; i++)
	{
		printf( "%d\t%s\n", i, codes[i]);
	}
}

////////////////////////////////////////////////////////////////////////////////
// 허프만 트리로부터 허프만 코드를 생성
// traverse_tree 함수 호출
void make_huffman_code( tNode *root, char *codes[])
{
	char code[256];
	
	traverse_tree( root, code, 0, codes);
}















// 파일에 속한 각 문자(바이트)의 빈도 저장
// return value : 파일에서 읽은 바이트 수
int read_chars( FILE *fp, int ch_freq[])
{
	int ch, byte = 0;
	
	do{
		ch = fgetc(fp);
		ch_freq[ch]++;
		byte++;
	}while(ch != EOF);

	return byte - 1;
}

// 파일로부터 문자별 빈도(256개)를 읽어서 ch_freq에 저장
void get_char_freq( FILE *fp, int ch_freq[])
{	
	for(int i = 0; i < 256; i++)
		fread(&ch_freq[i], sizeof(int), 1, fp);
}


// 허프만 코드에 대한 메모리 해제
void free_huffman_code( char *codes[]){
	for(int i = 0 ; i < 256; i++){
		free(codes[i]);
	}
}

// 허프만 트리를 생성
// 1. capacity 256 짜리 빈(empty) 힙 생성 // HEAP *heap = heapCreate( 256);
// 2. 개별 알파벳에 대한 노드 생성
// 3. 힙에 삽입 (minheap 구성) // heapInsert( heap, node);
// 4. 2개의 최소값을 갖는 트리 추출 // tNode *left = heapDelete( heap); tNode *right = heapDelete( heap);
// 5. 두 트리를 결합 후 새 노드에 추가
// 6. 새 트리를 힙에 삽입
// 7. 힙에 한개의 노드가 남을 때까지 반복
// return value: 트리의 root 노드의 포인터
tNode *make_huffman_tree( int ch_freq[])
{
	tNode* left;
	tNode* right;
	HEAP *heap = heapCreate(256); 
	tNode* aNode;
	tNode* tree;
	int a;
	
	for(int i = 0; i < 256; i++){
		
		aNode = newNode(i, ch_freq[i]);
		a = heapInsert(heap, aNode);
	}
	
	while(heap -> last > 0){
		left = heapDelete(heap);
		right = heapDelete(heap);
	
		tree = newNode(-1, left->freq + right->freq);
		tree->right = right;
		tree->left = left;
	
		a = heapInsert(heap, tree);
	}

	heapDestroy(heap);
	return tree;
}

// 허프만 트리 메모리 해제
void destroyTree( tNode *root)
{
	if(root->right || root->left){
		if(root->right) destroyTree(root->right);
		if(root->left) destroyTree(root->left);
		free(root);
	}
	
	else{
		free(root);
	}
	/*
	if(root->right == NULL || root->left == NULL) {
		
		if(root->right){
			destroyTree(root->right);
			free(root);
		}
		if(root->left){
			destroyTree(root->left);
			free(root);
		}
	}
	*/
}

// 입력 텍스트 파일(infp)을 허프만 코드를 이용하여 출력 파일(outfp)로 인코딩
// return value : 인코딩된 텍스트의 바이트 수 (파일 크기와는 다름)
int encoding( char *codes[], int ch_freq[], FILE *infp, FILE *outfp)
{	
	for(int i = 0; i < 256; i++){
		fwrite(ch_freq + i, sizeof(int), 1, outfp);
	}
	
	int ch;
	char *s;
	char current_byte = 0;
	int nbits = 0;
	int nbytes = 0;
	int result = 0;
	
	while(1){
		ch = fgetc(infp);
		if(ch == EOF) break;
		
		for(s = codes[ch]; *s; s++){
			current_byte <<= 1;
			
			if(*s == '1') current_byte |= 1;
			
			nbits++;
			
			if(nbits == 8){
				fwrite(&current_byte, sizeof(char), 1, outfp);
				nbytes++;
				nbits = 0;
				current_byte = 0;
			}
		}
	}
	
	while (nbits){
		current_byte <<= 1;
			
		nbits++;
			
		if(nbits == 8){
			fwrite(&current_byte, sizeof(char), 1, outfp);
			nbytes++;
			nbits = 0;
			current_byte = 0;
		}
	} 
	
	for(int i = 0; i < 256; i++){
		result += strlen(codes[i]) * ch_freq[i];
	}
	fwrite(&result, sizeof(int), 1, outfp);
	return nbytes;
}

// 입력 파일(infp)을 허프만 트리를 이용하여 텍스트 파일(outfp)로 디코딩
void decoding( tNode *root, FILE *infp, FILE *outfp){

	//파일 포인터 위치가 제대로 시작하는지 모르겟음
	int nbit = 0;
	// 마지막 으로 포인터 이동
	fseek(infp, -sizeof(int), SEEK_END);
	
	fread(&nbit, sizeof(int), 1, infp);
	fseek(infp, 256 * sizeof(int), SEEK_SET); 

	tNode *p = root;
	
	char buffer[BUF_SIZE];
	
	while(1){
		int fend = fread(buffer, 1, BUF_SIZE, infp);
		
		if(fend == 0) break;
		
		else {
			for(int i = 0; i < fend; i++){
				for(int j = 0; j < 8; j++){				
					if((char)(buffer[i] & 0x80) == 0) p = p ->left;
					else p = p->right;
				
					buffer[i] <<= 1;
					nbit--;

					if((p->right == NULL) && (p->left == NULL)){
						fprintf(outfp, "%c", p->data);
						fflush(outfp);
						p = root;
					}
					if(nbit == 0) return;
				}
			}
		}
	}
}

	






