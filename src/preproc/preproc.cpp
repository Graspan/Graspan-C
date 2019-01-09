#include "preproc.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string.h>
#include <stdio.h>

using std::cout;
using std::endl;

Preproc::Preproc() {
	totalNumEdges = totalNumVertex = maxVid = 0;	
}

Preproc::~Preproc() {
	if(numEdges != NULL)
		delete []numEdges;	
}

void Preproc::loadGrammar(Context &c) {
	c.grammar.loadGrammar(c.getGrammarFile());
}

void Preproc::setNumEdges(Context &c) {
	char *fname = c.getGraphFile();
	std::ifstream fin;
	fin.open(fname);
	if(!fin) {	
		cout << "can't load graph file: " << fname << endl;
		exit(-1);
	}

	char str[512];
	maxVid = totalNumEdges = totalNumVertex = 0;
	while(fin.getline(str,sizeof(str))) {
		char *p = strtok(str,"\t");
		if(p) {
			int value = atoi(p);
			if(value > maxVid) 
				maxVid = value;
		}	
	} 
	fin.close();

	numEdges = new int[maxVid+1];
	for(int i = 0;i <= maxVid;++i)
			numEdges[i] = 0;
	FILE *fp = fopen(fname,"r");
	if(!fp) {
		cout << "can't load graph file: " << fname << endl;	
		exit(-1);
	}
	else {
		int src,dst; char rawLabel[GRAMMAR_STR_LEN];	
		while(fscanf(fp,"%d\t%d\t%s\n",&src,&dst,rawLabel) != EOF) {
			++numEdges[src];
			++totalNumEdges;
		}	
	}
	for(int i = 0;i <= maxVid;++i) {
		if(numEdges[i])
			++totalNumVertex;
	}
	fclose(fp);
	
	cout << "totalNumVertex: " << totalNumVertex << endl;
	cout << "totalNumEdges: " << totalNumEdges << endl;
	cout << "maxVertexId: " << maxVid << endl;
}

// TODO: calculate numPartitions based on memoryBudget
void Preproc::setVIT(Context &c) {
	setNumEdges(c);	
	int numErules = c.grammar.getNumErules();
	int numPartitions = c.getNumPartitions();
	int total_size = totalNumEdges + (totalNumVertex) * numErules;
	int partition_size = (total_size) / numPartitions;

	int part_id = 0; int cur_size = 0; int i = 0; int v_start = 0;
	for(;i <= maxVid;++i) {
		cur_size += (numErules + numEdges[i]);
		if(part_id == numPartitions - 1) {
			for(int j = v_start+1;j <= maxVid;++j)
				cur_size += (numErules + numEdges[j]);
			c.vit.add(v_start,maxVid,cur_size);
			++part_id;
			break;
		}
		else {
			if(cur_size >= partition_size) {
				c.vit.add(v_start,i,cur_size);
				v_start = i+1;
				cur_size = 0;
				++part_id;
			}
		}	
	}
	cout << "numErules:" << numErules << endl;
	cout << "old numPartitions: " << numPartitions << endl;
	cout << "current numPartitions: " << part_id << endl;
	cout << "total_size: " << total_size << endl;
	cout << "per_partition_size: " << partition_size << endl; 
	c.vit.print();
}

void Preproc::savePartitions(Context &c) {
	
	vertexid_t **vertices;
	char **labels;
	int numErules = c.grammar.getNumErules();

	vertices = (vertexid_t**)malloc(sizeof(vertexid_t*) * (maxVid+1));
	labels = (char**)malloc(sizeof(char*) * (maxVid+1));
	int *index = (int*)calloc((maxVid+1),sizeof(int));

	for(int i = 0;i <= maxVid;++i) {
		*(vertices+i) = (vertexid_t*)malloc(sizeof(vertexid_t) * (numEdges[i] + numErules));
		*(labels+i) = (char*)malloc(sizeof(char) * numEdges[i] * (numEdges[i] + numErules));
	}	

	FILE *fp = fopen(c.getGraphFile(),"r");
	if(!fp) {
		cout << "can't open graph_file: " << c.getGraphFile() << endl;
		exit(-1);
	}
	vertexid_t src,dst; char rawLabel[GRAMMAR_STR_LEN];
	while(fscanf(fp,"%d\t%d\t%s\n",&src,&dst,rawLabel) != EOF) {
		vertices[src][index[src]] = dst;
		labels[src][index[src]] = c.grammar.getLabelValue(rawLabel);
		++index[src];
	}
	for(int i = 0;i <= maxVid;++i) {
		for(int j = 0;j < numErules;++j) {
			vertices[i][index[i]] = i;
			labels[i][index[i]] = (char)(j-128);
			++index[i];
		}	
	}
	
	//mergeAndSort	
	for(int i = 0;i <= maxVid;++i)
		quickSort3Way(vertices[i],labels[i],0,index[i]-1);

	//savePartitionsToFile (txt format for test)
	for(int i = 0;i < c.vit.getSize();++i) {
		vertexid_t v_start,v_end;
		v_start = c.vit.getStart(i);
		v_end = c.vit.getEnd(i);
		
		char filename[256];
		sprintf(filename,"%d.part",i);

		FILE *f = fopen(filename,"w");
		for(vertexid_t j = v_start; j <= v_end;++j) {
			int edgeNum = numEdges[j] + numErules;
			if(!edgeNum)
				continue;
			fprintf(f,"%d,%d\n",j,edgeNum);
			for(int k = 0;k < edgeNum;++k) {
				fprintf(f,"(%d,%d)\n",vertices[j][k],labels[j][k]);	
			}
		}
		fclose(f);
	}

	for(int i = 0;i <= maxVid;++i) {
		free(*(vertices+i));
		free(*(labels+i));
	}
	free(index);
	free(vertices);
	free(labels);
}

void insertSort(vertexid_t *A,char *B,int l,int r) {
	for(int j = l+1;j <= r;++j) {
		vertexid_t key_v = A[j];
		char key_c = B[j];
		int i = j-1;
		while(i >= l && key_v < A[i]) {
			A[i+1] = A[i];
			B[i+1] = B[i];
			--i;
		}
		A[i+1] = key_v;
		B[i+1] = key_c;
	}
}

void quickSort3Way(vertexid_t *A,char* B,int l,int r) {
	if(l >= r)
		return;	
	if(r - l + 1 <= 10)	
		insertSort(A,B,l,r);
	else {
		//TODO:
	}
}

int getPivot(vertexid_t *A,char *B,int l,int r) {
	//TODO:
}
